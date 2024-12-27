#pragma once
#include <coproto/Socket/AsioSocket.h>
#include <coproto/Socket/Socket.h>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Common/Matrix.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Common/block.h>
#include <cryptoTools/Crypto/PRNG.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using block = osuCrypto::block;
using Matrix = osuCrypto::Matrix<block>;
struct Context {
    std::string address = "localhost:8081";
    coproto::Socket chl;
    uint64_t rows;
    uint64_t cols;
    uint64_t layers;
    uint64_t seedJ;
    std::vector<block> J;
    bool setuped = false;
    osuCrypto::Timer *timer;
    unsigned long totalSend = 0;
    unsigned long totalReceive = 0;

    Context(uint64_t rows, uint64_t cols)
        : rows(rows), cols(cols), seedJ(1999526), layers(2 * osuCrypto::log2ceil(rows) - 1)
    {}

    Context(const Context &other) = default;

    void print()
    {
        std::ostream &out = std::cout;
        out << "\n";
        out << *timer;
        out << "Receive: " << totalReceive * 1.0 / 1048576 << "MB\n";
        out << "Send:    " << totalSend * 1.0 / 1048576 << "MB\n";
        out << "Total:   " << (totalReceive + totalSend) * 1.0 / 1048576 << "MB\n";
        out << "\n";
        return;
    }

    void setup(bool isServer)
    {
        osuCrypto::PRNG prng((block(seedJ)));
        J = std::vector<block>(cols);
        prng.get(J.data(), cols);
        if (isServer) {
            chl = coproto::asioConnect(address, true);
        } else {
            chl = coproto::asioConnect(address, false);
        }
        setuped = true;
    }

    void close()
    {
        totalReceive = chl.bytesReceived();
        totalSend = chl.bytesSent();
        coproto::sync_wait(chl.close());
    }

    void setTimer(osuCrypto::Timer &timer)
    {
        this->timer = &timer;
    }
};
