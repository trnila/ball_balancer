#pragma once
#include <cstdint>
#include <cstddef>

size_t stuffData(const uint8_t *ptr, size_t length, uint8_t *dst);
size_t unstuffData(const uint8_t *ptr, size_t length, uint8_t *dst);
