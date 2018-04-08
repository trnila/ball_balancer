#pragma once
#include <cstdint>
#include <cstddef>

size_t stuff_data(const uint8_t *ptr, size_t length, uint8_t *dst);
size_t unstuff_data(const uint8_t *ptr, size_t length, uint8_t *dst);
