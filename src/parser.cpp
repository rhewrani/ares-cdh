#include "../include/ares/parser.hpp"
#include "../include/ares/crc.hpp"
#include <algorithm>

namespace ares {

    TcParser::TcParser() {
        reset_all();
    }

    void TcParser::reset_all() {
        std::fill_n(sequence_counts, MAX_APIDS, NO_PACKET_SEEN);
    }

    void TcParser::reset_sequence(U16 apid) {
        if (apid >= MAX_APIDS) std::abort();
        sequence_counts[apid] = NO_PACKET_SEEN;
    }

    Result<void> TcParser::validate_length(const Byte* /* buffer */, U16 length) {
        if (length < 8) return Result<void>::err(ParseError::BUFFER_TOO_SHORT);
        return Result<void>::ok();
    }

    Result<void> TcParser::validate_version(const Byte* buffer) {
        const U8 version = (buffer[0] >> 5) & 0x07;
        if (version != 1) return Result<void>::err(ParseError::INVALID_VERSION);
        return Result<void>::ok();
    }

    Result<void> TcParser::validate_packet_type(const Byte* buffer) {
        const U8 packet_type = static_cast<U8>((buffer[0] >> 4) & 0x01);
        if (packet_type != static_cast<U8>(PacketType::TC)) return Result<void>::err(ParseError::INVALID_PACKET_TYPE);
        return Result<void>::ok();
    }

    U16 TcParser::extract_crc(const Byte* buffer, U16 length) {
        U16 crc = static_cast<U16>(buffer[length - 2] << 8 | buffer[length - 1]);
        return crc;
    }

    Result<void> TcParser::validate_crc(const Byte* buffer, U16 length) {
        U16 calculated_crc = compute_crc16(buffer, length - 2);
        U16 received_crc = extract_crc(buffer, length);
        if (calculated_crc != received_crc) return Result<void>::err(ParseError::CRC_MISMATCH);
        return Result<void>::ok();
    }

    Result<PrimaryHeader> TcParser::extract_header(const Byte* buffer) {
        PrimaryHeader header;
        header.version = 1; // we know it's always 1 (important: validate first)
        header.packet_type = PacketType::TC; // same here

        header.has_secondary = (buffer[0] >> 3) & 0x01;
        header.apid = static_cast<U16>((buffer[0] & 0x07) << 8 | buffer[1]);
        header.sequence_flags = static_cast<SequenceFlags>((buffer[2] >> 6) & 0x03);
        header.sequence_count = static_cast<U16>((buffer[2] & 0x3F) << 8 | buffer[3]);
        header.data_length = static_cast<U16>((buffer[4] << 8) | buffer[5]);

        return Result<PrimaryHeader>::ok(header);

    }
}