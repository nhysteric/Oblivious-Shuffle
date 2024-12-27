#include "shuffle.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <coproto/Common/macoro.h>
#include <coproto/Common/span.h>
#include <coproto/Socket/AsioSocket.h>
#include <coproto/Socket/Socket.h>
#include <cryptoTools/Common/BitVector.h>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Common/Log.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Crypto/PRNG.h>
#include <cstdint>
#include <cstdio>
#include <libOTe/Base/BaseOT.h>
#include <libOTe/Tools/Coproto.h>
#include <libOTe/TwoChooseOne/Silent/SilentOtExtReceiver.h>
#include <libOTe/TwoChooseOne/Silent/SilentOtExtSender.h>
#include <numeric>
#include <random>
#include <unistd.h>
#include <utility>
#include <vector>
#include <volePSI/Defines.h>
#include "context.h"
#include "khprf.h"
#include "utils.h"

Matrix BENES_MATRIX_SENDER(const Matrix &inputs, Context &context)
{
    if (!context.setuped) {
        context.setup(false);
    }
    std::vector<std::vector<uint64_t>> share_pi(
        context.layers / 2, std::vector<uint64_t>(context.rows));
    for (auto &s : share_pi) {
        coproto::sync_wait(context.chl.recv(s));
    }
    oc::PRNG prng(block(0, 0));
    std::vector<block> mask(context.rows);
    prng.get(mask.data(), context.rows);
    auto expand = khprf(mask, context.J);
    context.timer->setTimePoint("data preparation");
    for (int i = 0; i < context.layers; i++) {
        if (i % 2 == 0) {
            permute_layer_sender(mask, context);
            // context.timer->setTimePoint("layer " + std::to_string(i) + ":permute_layer");
        } else {
            permuteVector(mask, share_pi[i / 2]);
            // context.timer->setTimePoint("layer " + std::to_string(i) + ":share_layer");
        }
    }
    context.timer->setTimePoint("permute");
    expand = inputs - expand;
    osuCrypto::cp::sync_wait(context.chl.send(expand));
    context.timer->setTimePoint("mask send");
    context.close();
    context.timer->setTimePoint("end");
    return khprf(mask, context.J);
}

std::pair<Matrix, std::vector<uint64_t>> BENES_MATRIX_RECEIVER(Context &context)
{
    if (!context.setuped) {
        context.setup(true);
    }
    std::vector<std::vector<uint64_t>> share_pi(
        context.layers / 2, std::vector<uint64_t>(context.rows));
    std::vector<osuCrypto::BitVector> permute_pi(context.layers - (context.layers / 2));
    std::mt19937 rng(110);
    osuCrypto::PRNG prng(block(0, 0));
    for (auto &s : share_pi) {
        std::iota(s.begin(), s.end(), 0);
        std::shuffle(s.begin(), s.end(), rng);
        coproto::sync_wait(context.chl.send(s));
    }

    for (auto &p : permute_pi) {
        for (int i = 0; i < context.rows / 2; i++) {
            if (prng.get<bool>()) {
                p.pushBack(1);
                p.pushBack(0);
            } else {
                p.pushBack(0);
                p.pushBack(1);
            }
        }
    }
    auto final_pi = getFinalPI(share_pi, permute_pi);
    std::vector<block> mask(context.rows);
    for (int i = 0; i < context.layers; i++) {
        if (i % 2 == 0) {
            auto temp = permute_layer_receiver(context, permute_pi[i / 2]);
            for (int j = 0; j < context.rows / 2; j++) {
                if (permute_pi[i / 2][2 * j] == 0) {
                    std::swap(mask[2 * j], mask[2 * j + 1]);
                }
                mask[2 * j] += temp[2 * j];
                mask[2 * j + 1] += temp[2 * j + 1];
            }
        } else {
            permuteVector(mask, share_pi[i / 2]);
        }
    }

    Matrix mask2(context.rows, context.cols);
    osuCrypto::cp::sync_wait(context.chl.recv(mask2));
    permuteMatrix(mask2, final_pi);
    context.close();
    return std::make_pair(mask2 + khprf(mask, context.J), std::move(final_pi));
}

std::vector<block> BENES_VECTOR_SENDER(const std::vector<block> &inputs, Context &context)
{
    if (!context.setuped) {
        context.setup(false);
    }
    std::vector<std::vector<uint64_t>> share_pi(
        context.layers / 2, std::vector<uint64_t>(context.rows));
    for (auto &s : share_pi) {
        coproto::sync_wait(context.chl.recv(s));
    }
    oc::PRNG prng(block(0, 0));
    std::vector<block> mask(context.rows);
    prng.get(mask.data(), context.rows);
    auto mask2 = mask;
    auto expand = khprf(mask, context.J);
    context.timer->setTimePoint("data preparation");
    for (int i = 0; i < context.layers; i++) {
        if (i % 2 == 0) {
            permute_layer_sender(mask, context);
            // context.timer->setTimePoint("layer " + std::to_string(i) + ":permute_layer");
        } else {
            permuteVector(mask, share_pi[i / 2]);
            // context.timer->setTimePoint("layer " + std::to_string(i) + ":share_layer");
        }
    }
    context.timer->setTimePoint("permute");
    mask2 = inputs - mask2;
    osuCrypto::cp::sync_wait(context.chl.send(mask2));
    context.timer->setTimePoint("mask send");
    context.close();
    context.timer->setTimePoint("end");
    return mask;
}

std::pair<std::vector<block>, std::vector<uint64_t>> BENES_VECTOR_RECEIVER(Context &context)
{
    if (!context.setuped) {
        context.setup(true);
    }
    std::vector<std::vector<uint64_t>> share_pi(
        context.layers / 2, std::vector<uint64_t>(context.rows));
    std::vector<osuCrypto::BitVector> permute_pi(context.layers - (context.layers / 2));
    std::mt19937 rng(110);
    osuCrypto::PRNG prng(block(0, 0));
    for (auto &s : share_pi) {
        std::iota(s.begin(), s.end(), 0);
        std::shuffle(s.begin(), s.end(), rng);
        coproto::sync_wait(context.chl.send(s));
    }

    for (auto &p : permute_pi) {
        for (int i = 0; i < context.rows / 2; i++) {
            if (prng.get<bool>()) {
                p.pushBack(1);
                p.pushBack(0);
            } else {
                p.pushBack(0);
                p.pushBack(1);
            }
        }
    }
    auto final_pi = getFinalPI(share_pi, permute_pi);
    std::vector<block> mask(context.rows);
    for (int i = 0; i < context.layers; i++) {
        if (i % 2 == 0) {
            auto temp = permute_layer_receiver(context, permute_pi[i / 2]);
            for (int j = 0; j < context.rows / 2; j++) {
                if (permute_pi[i / 2][2 * j] == 0) {
                    std::swap(mask[2 * j], mask[2 * j + 1]);
                }
                mask[2 * j] += temp[2 * j];
                mask[2 * j + 1] += temp[2 * j + 1];
            }
        } else {
            permuteVector(mask, share_pi[i / 2]);
        }
    }
    std::vector<block> mask2(context.rows);
    osuCrypto::cp::sync_wait(context.chl.recv(mask2));
    permuteVector(mask2, final_pi);
    context.close();
    return std::make_pair(mask2 + mask, std::move(final_pi));
}

std::vector<block> permute_layer_receiver(Context &context, const osuCrypto::BitVector &choices)
{
    osuCrypto::PRNG prng(block(0, 0));
    std::vector<block> ot_message(context.rows);
    osuCrypto::SilentOtExtReceiver receiver;
    receiver.configure(context.rows);
    coproto::sync_wait(receiver.genSilentBaseOts(prng, context.chl));
    coproto::sync_wait(receiver.receiveChosen(choices, ot_message, prng, context.chl));
    std::vector<block> mask(context.rows);
    coproto::sync_wait(context.chl.recv(mask));
    std::vector<block> outputs(context.rows);
    for (int i = 0; i < context.rows / 2; i++) {
        if (choices[i * 2] == 1) {
            outputs[i * 2] = mask[i * 2] + ot_message[i * 2 + 1] - ot_message[i * 2];
            outputs[i * 2 + 1] = mask[i * 2 + 1] + ot_message[i * 2] - ot_message[i * 2 + 1];
        } else {
            outputs[i * 2] = mask[i * 2 + 1] + ot_message[i * 2 + 1] - ot_message[i * 2];
            outputs[i * 2 + 1] = mask[i * 2] + ot_message[i * 2] - ot_message[i * 2 + 1];
        }
    }
    return outputs;
}

void permute_layer_sender(std::vector<block> &inputs, Context &context)
{
    osuCrypto::PRNG prng(block(0, 0));
    std::vector<std::array<block, 2>> ot_message(context.rows);
    std::vector<block> mask(context.rows);
    prng.get(ot_message.data(), context.rows);
    std::vector<block> b(context.rows);
    auto xorfunc = [&ot_message, &inputs](std::vector<block> &b, std::vector<block> &mask) {
        for (int i = 0; i < ot_message.size() / 2; ++i) {
            b[i * 2] = ot_message[i * 2][0] + ot_message[i * 2][1];
            b[i * 2 + 1] = ot_message[i * 2 + 1][0] + ot_message[i * 2 + 1][1];
            mask[i * 2] = inputs[i * 2] - ot_message[i * 2][0] - ot_message[i * 2 + 1][0];
            mask[i * 2 + 1] = inputs[i * 2 + 1] - ot_message[i * 2][1] - ot_message[i * 2 + 1][1];
        }
    };
    auto xor_feature = std::async(std::launch::async, xorfunc, std::ref(b), std::ref(mask));
    osuCrypto::SilentOtExtSender sender;
    sender.configure(context.rows);
    // sender.setTimer(*context.timer);
    coproto::sync_wait(sender.genSilentBaseOts(prng, context.chl));
    coproto::sync_wait(sender.sendChosen(ot_message, prng, context.chl));
    xor_feature.get();
    coproto::sync_wait(context.chl.send(mask));
    inputs = b;
}
