#include "utils.h"
#include <algorithm>
#include <cassert>
#include <coproto/Common/macoro.h>
#include <coproto/Socket/AsioSocket.h>
#include <cryptoTools/Common/BitVector.h>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Common/config.h>
#include <cryptoTools/Crypto/PRNG.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Network/Session.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <libOTe/Tools/Coproto.h>
#include <utility>
#include <vector>
#include "context.h"

std::vector<block> operator+(const std::vector<block> &a, const std::vector<block> &b)
{
    assert(a.size() == b.size());
    std::vector<block> result(a.size());
    for (size_t i = 0; i < a.size(); i++) {
        result[i] = a[i] + b[i];
    }
    return result;
}

void operator+=(std::vector<block> &a, const std::vector<block> &b)
{
    a = a + b;
}

std::vector<block> operator-(const std::vector<block> &a, const std::vector<block> &b)
{
    assert(a.size() == b.size());
    std::vector<block> result(a.size());
    for (size_t i = 0; i < a.size(); i++) {
        result[i] = a[i] - b[i];
    }
    return result;
}

void operator-=(std::vector<block> &a, const std::vector<block> &b)
{
    a = a - b;
}

Matrix operator+(const Matrix &a, const Matrix &b)
{
    assert(a.rows() == b.rows() && a.cols() == b.cols());
    Matrix result(a.rows(), a.cols());
    for (size_t i = 0; i < a.rows(); i++) {
        for (size_t j = 0; j < a.cols(); j++) {
            result(i, j) = a(i, j) + b(i, j);
        }
    }
    return result;
}

Matrix operator-(const Matrix &a, const Matrix &b)
{
    assert(a.rows() == b.rows() && a.cols() == b.cols());
    Matrix result(a.rows(), a.cols());
    for (size_t i = 0; i < a.rows(); i++) {
        for (size_t j = 0; j < a.cols(); j++) {
            result(i, j) = a(i, j) - b(i, j);
        }
    }
    return result;
}

void operator+=(Matrix &a, const Matrix &b)
{
    a = a + b;
}

void operator-=(Matrix &a, const Matrix &b)
{
    a = a - b;
}

std::vector<uint64_t> generateRandomPermutation(const uint64_t &n, const int &seed)
{
    std::mt19937 rng(seed);
    std::vector<uint64_t> permutation(n);
    std::iota(permutation.begin(), permutation.end(), 0);
    std::shuffle(permutation.begin(), permutation.end(), rng);
    return permutation;
}

void permuteMatrix(Matrix &a, const std::vector<size_t> &permute)
{
    assert(a.rows() == permute.size());
    Matrix temp(a.rows(), a.cols());
    for (size_t i = 0; i < a.rows(); i++) {
        for (size_t j = 0; j < a.cols(); j++) {
            temp(i, j) = a(permute[i], j);
        }
    }
    a = std::move(temp);
}

void MatrixRecv(Matrix &result, Context &context)
{
    coproto::sync_wait(context.chl.recv(result));
    coproto::sync_wait(context.chl.flush());
}

void MatrixSend(const Matrix &value, Context &context)
{
    coproto::sync_wait(context.chl.send(value));
    coproto::sync_wait(context.chl.flush());
}

std::vector<uint64_t> getFinalPI(
    std::vector<std::vector<uint64_t>> &share_pi,
    const std::vector<osuCrypto::BitVector> &permute_pi)
{
    std::vector<uint64_t> final_pi(share_pi[0].size());
    std::iota(final_pi.begin(), final_pi.end(), 0);
    for (int i = 0; i < share_pi.size(); i++) {
        for (int j = 0; j < permute_pi[i].size() / 2; j++) {
            if (permute_pi[i][j * 2] == 0) {
                std::swap(final_pi[j * 2], final_pi[j * 2 + 1]);
            }
        }
        permuteVector(final_pi, share_pi[i]);
    }
    for (int j = 0; j < permute_pi[0].size() / 2; j++) {
        if (permute_pi[permute_pi.size() - 1][j * 2] == 0) {
            std::swap(final_pi[j * 2], final_pi[j * 2 + 1]);
        }
    }
    return final_pi;
}