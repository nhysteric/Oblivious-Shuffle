#pragma once

#include <arpa/inet.h>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Common/block.h>
#include <cstddef>
#include <cstdint>
#include <unistd.h>
#include <vector>
#include "context.h"

std::vector<block> operator+(const std::vector<block> &a, const std::vector<block> &b);
void operator+=(std::vector<block> &a, const std::vector<block> &b);
std::vector<block> operator-(const std::vector<block> &a, const std::vector<block> &b);
void operator-=(std::vector<block> &a, const std::vector<block> &b);

Matrix operator+(const Matrix &a, const Matrix &b);
Matrix operator-(const Matrix &a, const Matrix &b);
void operator+=(Matrix &a, const Matrix &b);
void operator-=(Matrix &a, const Matrix &b);
std::vector<uint64_t> generateRandomPermutation(const uint64_t &n, const int &seed);

template <typename T>
void permuteVector(std::vector<T> &permutation, const std::vector<uint64_t> &pi)
{
    assert(permutation.size() == pi.size());
    std::vector<T> resultVector(permutation.size());
    std::transform(pi.begin(), pi.end(), resultVector.begin(), [&](uint64_t index) {
        return permutation[index];
    });
    permutation = std::move(resultVector);
}

void permuteMatrix(Matrix &a, const std::vector<size_t> &permute);
void MatrixRecv(Matrix &result, Context &context);
void MatrixSend(const Matrix &value, Context &context);
std::vector<uint64_t> getFinalPI(
    std::vector<std::vector<uint64_t>> &share_pi,
    const std::vector<osuCrypto::BitVector> &permute_pi);
