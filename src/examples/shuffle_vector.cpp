#include <cassert>
#include <cryptoTools/Common/Timer.h>
#include <cstdint>
#include <future>
#include <string>
#include <sys/types.h>
#include <vector>
#include "context.h"
#include "cryptoTools/Common/CLP.h"
#include "shuffle.h"
#include "utils.h"

uint64_t check(const std::vector<block> &a, const std::vector<block> &b)
{
    assert(a.size() == b.size());
    uint64_t count = 0;
    for (size_t i = 0; i < a.size(); i++) {
        if (a[i] == b[i]) {
            count++;
        }
    }
    return count;
}

int main(int argc, char *argv[])
{
    osuCrypto::CLP cmd(argc, argv);
    uint64_t rows = cmd.getOr<uint64_t>("rows", 1 << 16);
    uint64_t cols = 1;
    assert(rows != 0);
    assert(rows % 2 == 0);
    Context context_server(rows, cols);
    Context context_client(context_server);
    osuCrypto::Timer timer;
    context_client.setTimer(timer);
    std::vector<block> inputs(rows);
    osuCrypto::PRNG prng(block(rand(), rand()));
    prng.get(inputs.data(), rows);

    timer.setTimePoint("start");
    auto p1 = std::async(
        std::launch::async, BENES_VECTOR_SENDER, std::ref(inputs), std::ref(context_client));
    auto p2 = std::async(std::launch::async, BENES_VECTOR_RECEIVER, std::ref(context_server));
    auto share_p1 = p1.get();
    auto [share_p2, p] = p2.get();
    context_client.print();
    share_p1 += share_p2;
    permuteVector(inputs, p);
    std::cout << "check: " << check(inputs, share_p1) << "/" << rows * cols << std::endl;
    return 0;
}