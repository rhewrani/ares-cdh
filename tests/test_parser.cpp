#include "../include/ares/parser.hpp"
#include "../include/ares/crc.hpp"
#include "catch_amalgamated.hpp"
#include <array>

std::array<ares::Byte, 9> make_valid_packet(ares::U16 apid, ares::U16 sequence_count, ares::U8 payload_byte) {
    /*
    Byte 0-5:   primary header      (6 bytes)
    Byte 6:     payload             (1 byte)  
    Byte 7-8:   CRC (optional)      (2 bytes)
    total:      9 bytes
    */
    std::array<ares::Byte, 9> packet{};

    // Primary header
    packet[0] = static_cast<ares::Byte>(
        (0x01 << 5) |               // version = 1
        (0x01 << 4) |               // packet type = TC
        (0x00 << 3) |               // no secondary header
        ((apid >> 8) & 0x07));      // apid high 3 bits

    packet[1] = static_cast<ares::Byte>(apid & 0xFF);

    packet[2] = static_cast<ares::Byte>(
        (0x03 << 6) |               // sequence flags = UNSEGMENTED
        ((sequence_count >> 8) & 0x3F));

    packet[3] = static_cast<ares::Byte>(sequence_count & 0xFF);

    packet[4] = 0x00;               // data_length high byte
    packet[5] = 0x02;               // data_length low byte (3 remaining bytes - 1)

    // Payload
    packet[6] = payload_byte;

    // CRC
    ares::U16 crc = ares::compute_crc16(packet.data(), 7);
    packet[7] = (crc >> 8) & 0xFF;
    packet[8] = crc & 0xFF;

    return packet;
}

TEST_CASE("Buffer of 0 bytes is too short") {
    ares::TcParser parser;
    std::array<ares::Byte, 0> buffer{};
    auto result = parser.parse(buffer.data(), static_cast<ares::U16>(buffer.size()));
    REQUIRE(result.is_err());
    REQUIRE(result.error() == ares::ParseError::BUFFER_TOO_SHORT);
}

TEST_CASE("Buffer of 8 bytes is too short") {
    ares::TcParser parser;
    std::array<ares::Byte, 8> buffer{};
    auto result = parser.parse(buffer.data(), static_cast<ares::U16>(buffer.size()));
    REQUIRE(result.is_err());
    REQUIRE(result.error() == ares::ParseError::BUFFER_TOO_SHORT);
}

TEST_CASE("Buffer of 9 bytes with valid contents passes length check") {
    ares::TcParser parser;
    std::array<ares::Byte, 9> buffer = make_valid_packet(0x0001, 0x0000, 0x00);
    auto result = parser.parse(buffer.data(), static_cast<ares::U16>(buffer.size()));
    REQUIRE(result.is_ok());
}

TEST_CASE("Valid header with version bit set to 0 is invalid") {
    ares::TcParser parser;
    std::array<ares::Byte, 9> buffer = make_valid_packet(0x0001, 0x0000, 0x00);
    buffer[0] = buffer[0] & ~(0x07 << 5);
    auto result = parser.parse(buffer.data(), static_cast<ares::U16>(buffer.size()));
    REQUIRE(result.is_err());
    REQUIRE(result.error() == ares::ParseError::INVALID_VERSION);
}

TEST_CASE("Valid header with version bit set to 1 is valid") {
    ares::TcParser parser;
    std::array<ares::Byte, 9> buffer = make_valid_packet(0x0001, 0x0000, 0x00);
    auto result = parser.parse(buffer.data(), static_cast<ares::U16>(buffer.size()));
    REQUIRE(result.is_ok());
}

TEST_CASE("Valid header with packet type bit set to 0 is invalid") {
    ares::TcParser parser;
    std::array<ares::Byte, 9> buffer = make_valid_packet(0x0001, 0x0000, 0x00);
    buffer[0] = buffer[0] & ~(0x01 << 4);
    ares::U16 crc = ares::compute_crc16(buffer.data(), 7);
    buffer[7] = static_cast<ares::Byte>(crc >> 8);
    buffer[8] = static_cast<ares::Byte>(crc & 0xFF);
    auto result = parser.parse(buffer.data(), static_cast<ares::U16>(buffer.size()));
    REQUIRE(result.is_err());
    REQUIRE(result.error() == ares::ParseError::INVALID_PACKET_TYPE);
}

TEST_CASE("Valid packet with wrong CRC fails CRC check") {
    ares::TcParser parser;
    std::array<ares::Byte, 9> buffer = make_valid_packet(0x0001, 0x0000, 0x00);
    buffer[8] = buffer[8] ^ 0x01;
    auto result = parser.parse(buffer.data(), static_cast<ares::U16>(buffer.size()));
    REQUIRE(result.is_err());
    REQUIRE(result.error() == ares::ParseError::CRC_MISMATCH);
    
}

TEST_CASE("Valid packet with correct CRC passes CRC check") {
    ares::TcParser parser;
    std::array<ares::Byte, 9> buffer = make_valid_packet(0x0001, 0x0000, 0x00);
    auto result = parser.parse(buffer.data(), static_cast<ares::U16>(buffer.size()));
    REQUIRE(result.is_ok());
}


TEST_CASE("Sequence validation: second packet for same APID is valid") {
    ares::TcParser parser;
    std::array<ares::Byte, 9> buffer1 = make_valid_packet(0x0001, 0x0000, 0x00);
    std::array<ares::Byte, 9> buffer2 = make_valid_packet(0x0001, 0x0001, 0x00);
    parser.parse(buffer1.data(), static_cast<ares::U16>(buffer1.size()));
    auto result = parser.parse(buffer2.data(), static_cast<ares::U16>(buffer2.size()));
    REQUIRE(result.is_ok());    
}

TEST_CASE("Sequence validation: second packet for same APID with same sequence count is invalid") {
    ares::TcParser parser;
    std::array<ares::Byte, 9> buffer1 = make_valid_packet(0x0001, 0x0000, 0x00);
    std::array<ares::Byte, 9> buffer2 = make_valid_packet(0x0001, 0x0000, 0x00);
    parser.parse(buffer1.data(), static_cast<ares::U16>(buffer1.size()));
    auto result = parser.parse(buffer2.data(), static_cast<ares::U16>(buffer2.size()));
    REQUIRE(result.is_err());
    REQUIRE(result.error() == ares::ParseError::SEQUENCE_REPLAY);
}

TEST_CASE("Sequence validation: second packet for same APID with smaller sequence count is invalid") {
    ares::TcParser parser;
    std::array<ares::Byte, 9> buffer1 = make_valid_packet(0x0001, 0x0001, 0x00);
    std::array<ares::Byte, 9> buffer2 = make_valid_packet(0x0001, 0x0000, 0x00);
    parser.parse(buffer1.data(), static_cast<ares::U16>(buffer1.size()));
    auto result = parser.parse(buffer2.data(), static_cast<ares::U16>(buffer2.size()));
    REQUIRE(result.is_err());
    REQUIRE(result.error() == ares::ParseError::SEQUENCE_REPLAY);
}

TEST_CASE("Sequence validation: second packet for same APID with sequence count gap is valid") {
    ares::TcParser parser;
    std::array<ares::Byte, 9> buffer1 = make_valid_packet(0x0001, 0x0000, 0x00);
    std::array<ares::Byte, 9> buffer2 = make_valid_packet(0x0001, 0x0002, 0x00);
    parser.parse(buffer1.data(), static_cast<ares::U16>(buffer1.size()));
    auto result = parser.parse(buffer2.data(), static_cast<ares::U16>(buffer2.size()));
    REQUIRE(result.is_ok());
}

TEST_CASE("Valid packet returns correct header fields and payload") {
    ares::TcParser parser;
    auto buffer = make_valid_packet(0x0042, 0x0007, 0xAB);
    auto result = parser.parse(buffer.data(), 
                               static_cast<ares::U16>(buffer.size()));

    REQUIRE(result.is_ok());

    const auto& pkt = result.value();
    REQUIRE(pkt.primary_header.version == 1);
    REQUIRE(pkt.primary_header.packet_type == ares::PacketType::TC);
    REQUIRE(pkt.primary_header.has_secondary == 0);
    REQUIRE(pkt.primary_header.apid == 0x0042);
    REQUIRE(pkt.primary_header.sequence_count == 0x0007);
    REQUIRE(pkt.payload_length == 1);
    REQUIRE(pkt.payload[0] == 0xAB);
    REQUIRE(pkt.crc_valid == true);
}

TEST_CASE("reset_sequence clears state for one APID") {
    ares::TcParser parser;
    std::array<ares::Byte, 9> buffer1 = make_valid_packet(0x0001, 0x0005, 0x00);
    parser.parse(buffer1.data(), static_cast<ares::U16>(buffer1.size()));
    parser.reset_sequence(0x0001);

    // after resetting, sequence count 0 should be accepted
    std::array<ares::Byte, 9> buffer2 = make_valid_packet(0x0001, 0x0000, 0x00);
    auto result = parser.parse(buffer2.data(), static_cast<ares::U16>(buffer2.size()));
    REQUIRE(result.is_ok());
}

TEST_CASE("reset_all clears state for all APIDs") {
    ares::TcParser parser;
    std::array<ares::Byte, 9> buffer1 = make_valid_packet(0x0001, 0x0005, 0x00);
    std::array<ares::Byte, 9> buffer2 = make_valid_packet(0x0002, 0x0003, 0x00);
    parser.parse(buffer1.data(), static_cast<ares::U16>(buffer1.size()));
    parser.parse(buffer2.data(), static_cast<ares::U16>(buffer2.size()));
    parser.reset_all();

    std::array<ares::Byte, 9> buffer3 = make_valid_packet(0x0001, 0x0000, 0x00);
    std::array<ares::Byte, 9> buffer4 = make_valid_packet(0x0002, 0x0000, 0x00);
    auto result3 = parser.parse(buffer3.data(), static_cast<ares::U16>(buffer3.size()));
    auto result4 = parser.parse(buffer4.data(), static_cast<ares::U16>(buffer4.size()));
    REQUIRE(result3.is_ok());
    REQUIRE(result4.is_ok());
}

/*
Byte 0:  [version: 3][packet_type: 1][has_secondary: 1][apid high: 3]
Byte 1:  [apid low: 8]
Byte 2:  [seq_flags: 2][seq_count high: 6]
Byte 3:  [seq_count low: 8]
Byte 4:  [data_length high: 8]
Byte 5:  [data_length low: 8]
*/