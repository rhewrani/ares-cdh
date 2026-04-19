#pragma once
#include "types.hpp"

namespace ares {

    enum class PacketType {
        TM = 0,  // Telemetry
        TC = 1,  // Telecommand
    };

    enum class SequenceFlags {
        CONTINUATION = 0b00,
        FIRST_SEGMENT = 0b01,
        LAST_SEGMENT = 0b10,
        UNSEGMENTED = 0b11
    };

    // CCSDS telecommand primary header:
    // Byte 0: [version: 3][packet_type: 1][has_secondary: 1][apid high: 3]
    // Byte 1: [apid low: 8]
    // Byte 2: [seq_flags: 2][seq_count high: 6]
    // Byte 3: [seq_count low: 8]
    // Byte 4: [data_length high: 8]
    // Byte 5: [data_length low: 8]
    struct PrimaryHeader {
        U8 version;                   // 3 bits extracted - valid value is always 1
        PacketType packet_type;       // 1 bit
        bool has_secondary;           // 1 bit
        U16 apid;                     // 11 bits - range 0x000 to 0x7FE (0x7FF is reserved)
        SequenceFlags sequence_flags; // 2 bits
        U16 sequence_count;           // 14 bits - (0x0000 to 0x3FFF)
        // data_length holds the raw CCSDS field value
        // Actual remaining byte count = data_length + 1
        // The +1 is the parser's responsibility
        U16 data_length;
    };

    // The payload pointer is non-owning and points directly into
    // the source buffer passed to the parser. The caller must ensure
    // that source buffer remains alive and unmodified for as long as
    // this TcPacket is in use. Accessing payload after the source
    // buffer is freed or goes out of scope is undefined behaviour.
    struct TcPacket {
        PrimaryHeader primary_header;
        const Byte* payload;          //non-owning pointer to the source buffer
        U16 payload_length;           // data_length + 1 adjusted here
        bool crc_valid;               // is always true. If not true, the caller won't get the packet (explicit invariant)
    };
}
