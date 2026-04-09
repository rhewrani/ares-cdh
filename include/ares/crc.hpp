#pragma once
#include "types.hpp"

namespace ares {
    U16 compute_crc16(const Byte* data, U16 length);
}