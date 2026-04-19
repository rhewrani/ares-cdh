#pragma once
#include "types.hpp"

namespace ares {

    /// @brief CRC-16/CCITT-FALSE over @p length bytes (polynomial 0x1021, init 0xFFFF).
    /// @param data   First byte of the region to checksum (read-only).
    /// @param length Number of bytes to include.
    /// @return       16-bit CRC in wire order (high byte first when stored big-endian).
    U16 compute_crc16(const Byte* data, U16 length);
}

