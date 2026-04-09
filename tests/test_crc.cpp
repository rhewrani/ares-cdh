#include "../include/ares/crc.hpp"
#include "catch_amalgamated.hpp"

TEST_CASE("CRC of a single zero byte matches known value") {
    const ares::Byte data[] = {0x00};
    REQUIRE(ares::compute_crc16(data, 1) == 0xE1F0);
}

TEST_CASE("CRC of a single 0xFF byte matches known value") {
    const ares::Byte data[] = {0xFF};
    REQUIRE(ares::compute_crc16(data, 1) == 0xFF00);
}

TEST_CASE("CRC of a sequence of bytes matches known value") {
    const ares::Byte data[] = {0x41, 0x52, 0x45, 0x53};
    REQUIRE(ares::compute_crc16(data, 4) == 0x34E9);
}

TEST_CASE("Flipping bit changes CRC") {
    const ares::Byte data[] = {0x41, 0x52, 0x45, 0x53};
    const ares::Byte data_corrupted[] = {0x41, 0x52, 0x45, 0x54};
    REQUIRE(ares::compute_crc16(data, 4) != ares::compute_crc16(data_corrupted, 4));
}