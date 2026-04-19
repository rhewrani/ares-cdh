#pragma once
#include "types.hpp"

namespace ares {

    /// @brief Space Packet Protocol packet type discriminator (1 bit on wire).
    enum class PacketType {
        TM = 0,  ///< Telemetry
        TC = 1,  ///< Telecommand
    };

    /// @brief CCSDS sequence flags (2 bits); this parser accepts UNSEGMENTED only.
    enum class SequenceFlags {
        CONTINUATION    = 0b00,  ///< Continuation segment
        FIRST_SEGMENT   = 0b01,  ///< First segment
        LAST_SEGMENT    = 0b10,  ///< Last segment
        UNSEGMENTED     = 0b11,  ///< Single unsegmented packet
    };

    /// @brief Decoded CCSDS primary header (6 bytes on wire).
    ///
    /// Byte 0: [version:3][packet_type:1][has_secondary:1][apid high:3]  
    /// Byte 1: [apid low:8]  
    /// Byte 2: [seq_flags:2][seq_count high:6]  
    /// Byte 3: [seq_count low:8]  
    /// Byte 4: [data_length high:8]
    /// Byte 5: [data_length low:8]
    struct PrimaryHeader {
        U8 version;                    ///< CCSDS version (3 bits); expected 1.
        PacketType packet_type;        ///< Packet type (1 bit).
        bool has_secondary;            ///< Secondary header present (1 bit).
        U16 apid;                      ///< APID (11 bits); 0x7FF is reserved.
        SequenceFlags sequence_flags;  ///< Segmentation (2 bits).
        U16 sequence_count;            ///< Source sequence count (14 bits).
        U16 data_length;               ///< Raw data_length; octets after header = field + 1.
    };

    /// The payload pointer is non-owning and points directly into
    /// the source buffer passed to the parser. The caller must ensure
    /// that source buffer remains alive and unmodified for as long as
    /// this TcPacket is in use. Accessing payload after the source
    /// buffer is freed or goes out of scope is undefined behaviour.
    /// @brief Parsed telecommand view; @c payload aliases the buffer passed to TcParser::parse.
    struct TcPacket {
        PrimaryHeader primary_header;  ///< Decoded primary header.
        const Byte* payload;           ///< Non-owning pointer into caller buffer.
        U16 payload_length;            ///< User payload length in bytes.
        bool crc_valid;                ///< True when returned from parse() after CRC check.
    };
}
