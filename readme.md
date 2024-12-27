# Oblivious-Shuffle

My implementation is based on [this paper](https://eprint.iacr.org/2019/1340), with the concept of using the `Almost Key-Homomorphic Pseudo-Random Function (KHPRF)` for mask expansion inspired by [this paper](https://ieeexplore.ieee.org/document/10159165).

## Dependencies

- [libOTe](https://github.com/osu-crypto/libOTe): Provides Oblivious Transfer (OT) functionality.

## Build

```shell
git clone git@github.com:nhysteric/Oblivious-Shuffle.git --recursive
cd Oblivious-Shuffle/extern/libOTe
python build.py --all --boost --sodium --relic
cd ../..
cmake --no-warn-unused-cli -B build -S .  -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_CXX_STANDARD=20  -DCMAKE_BUILD_TYPE:STRING=Release
cmake --build build -j
```

Ensure that your C++ compiler supports C++20.

## Run

```shell
# Matrix_Shuffle
build/shuffle_matrix -rows 65536 -cols 10

# Vector_Shuffle
build/shuffle_vector -rows 65536
```

> For the sake of computational simplicity, it is required that the `rows` must be even.

## Remind

In the Matrix Shuffle process, the `KHPRF` is used to expand row masks, which may occasionally introduce a one-bit error.

If you encounter the following error due to a large dataset:

```shell
build/shuffle_matrix -rows 1048576 -cols 1000

Assertion `data.size() < std::numeric_limits<u32>::max()` failed.
  src location: extern/libOTe/out/coproto/coproto/../coproto/Socket/SocketScheduler.h:1016
  function: macoro::task<void> coproto::internal::SockScheduler::makeSendTask(Sock *) [Sock = coproto::detail::AsioSocket<>::Sock]
terminate called without an active exception
[1]    228797 abort (core dumped)  build/shuffle_matrix -rows 1048576 -cols 1000
```

You can try modifying `u32` to `u64` in the following lines and then recompile `libOTe` and this project:

- `extern/libOTe/out/coproto/coproto/Socket/SocketScheduler.h:55`
- `extern/libOTe/out/coproto/coproto/Socket/SocketScheduler.h:1016`
- `extern/libOTe/out/coproto/coproto/Socket/SocketScheduler.h:1046`
