#pragma once

#include <cryptoTools/Common/BitVector.h>
#include <cstdint>
#include <sys/types.h>
#include <vector>
#include "context.h"

Matrix BENES_MATRIX_SENDER(const Matrix &inputs, Context &context);
std::pair<Matrix, std::vector<uint64_t>> BENES_MATRIX_RECEIVER(Context &context);

std::vector<block> BENES_VECTOR_SENDER(const std::vector<block> &inputs, Context &context);
std::pair<std::vector<block>, std::vector<uint64_t>> BENES_VECTOR_RECEIVER(Context &context);

void permute_layer_sender(std::vector<block> &inputs, Context &context);
std::vector<block> permute_layer_receiver(Context &context, const osuCrypto::BitVector &choices);