#include "../include/ares/parser.hpp"
#include "../include/ares/crc.hpp"
#include <algorithm>
#include <iostream>

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
        // Minimum valid packet for this parser is 9 bytes:
        // 6 (primary header) + 1 (minimum payload) + 2 (mandatory CRC).
        // Note: CCSDS base standard minimum is 7 bytes, but this parser
        // requires the optional error control field for all telecommands.
        if (length < 9) return Result<void>::err(ParseError::BUFFER_TOO_SHORT);
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
        header.sequence_count = static_cast<U16>((buffer[2] & 0x3F) << 8 | buffer[3]); // 14-bit mask makes sequence overflow impossible, so no need to check for it
        header.data_length = static_cast<U16>((buffer[4] << 8) | buffer[5]);

        return Result<PrimaryHeader>::ok(header);

    }

    Result<void> TcParser::validate_sequence(const PrimaryHeader& header) {
        U16 stored = sequence_counts[header.apid];

        if (stored == NO_PACKET_SEEN) {
            sequence_counts[header.apid] = header.sequence_count;
            return Result<void>::ok();
        }

        if (header.sequence_count <= stored) 
            return Result<void>::err(ParseError::SEQUENCE_REPLAY);

        if (header.sequence_count > stored + 1)
            std::cerr << "Gap in sequence detected\n";

        sequence_counts[header.apid] = header.sequence_count;
        return Result<void>::ok();
    }

    Result<TcPacket> TcParser::build_packet(const PrimaryHeader& header, const Byte* buffer) {
        TcPacket packet;

        // TODO: Implement extract_secondary_header() for full secondary header parsing.
        // Currently assumes fixed 4-byte secondary header as placeholder.

        packet.primary_header = header;
        U8 add_secondary = static_cast<U8>(header.has_secondary ? 4 : 0);
        packet.payload = buffer + 6 + add_secondary; // + 6 because the first 6 bytes are the primary header
        packet.payload_length = static_cast<U16>(header.data_length + 1 - 2); // -2 to subtract the CRC bytes
        packet.crc_valid = true;

        return Result<TcPacket>::ok(packet);
    }

    Result<TcPacket> TcParser::parse(const Byte* buffer, U16 length) {
        if (auto r = validate_length(buffer, length); r.is_err())
            return Result<TcPacket>::err(r.error()); // Isn't this confusing? We're returnign a error but as an packet. Essentialy is_err() is false here even though it's an error

        if (auto r = validate_version(buffer); r.is_err())
            return Result<TcPacket>::err(r.error());

        if (auto r = validate_packet_type(buffer); r.is_err())
            return Result<TcPacket>::err(r.error());

        if (auto r = validate_crc(buffer, length); r.is_err())
            return Result<TcPacket>::err(r.error());

        auto header_res = extract_header(buffer);
        if (header_res.is_err()) 
            return Result<TcPacket>::err(header_res.error());

        const PrimaryHeader& header = header_res.value();

        if (auto r = validate_sequence(header); r.is_err())
            return Result<TcPacket>::err(r.error());

        return build_packet(header, buffer);
    }
}