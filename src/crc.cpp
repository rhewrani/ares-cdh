#include "../include/ares/crc.hpp"
#include <array>

namespace ares {
    constexpr std::array<U16, 256> generate_crc_table() {
        std::array<U16, 256> table{};
        for (U16 i = 0; i < 256; ++i) {
            U16 crc = static_cast<U16>(i << 8);
            for (U8 j = 0; j < 8; j++) {
                bool high_bit_set = (crc & 0x8000) != 0;
                crc <<= 1;
                if (high_bit_set) {
                    crc ^= 0x1021;
                }
            }
            
            table[i] = crc;
        }
        return table;
    }

    static constexpr std::array<U16, 256> crc_table = generate_crc_table();

    U16 compute_crc16(const Byte* data, U16 length) {
        U16 crc = 0xFFFF;
        for (U16 i = 0; i < length; i++) {
            U8 index = static_cast<U8>(crc >> 8) ^ data[i];
            crc = static_cast<U16>((crc << 8) ^ crc_table[index]);
        }
        return crc;
    }
}