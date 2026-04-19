#pragma once
#include "packet.hpp"
#include "result.hpp"

namespace ares {

    /// @brief Validates raw CCSDS telecommand buffers and tracks per-APID sequence counts.
    ///
    /// On success, the returned TcPacket references the input buffer; keep that buffer
    /// valid and unchanged while using the packet. On failure, returns the first
    /// ParseError that failed. After a partial reset use reset_sequence(); after a
    /// full restart use reset_all().
    class TcParser {
        public:
        /// @brief Constructs a parser with all per-APID sequence state cleared.
        ///
        /// Equivalent to calling reset_all() before first use.
        explicit TcParser();
        
        /// @brief Parses a raw byte buffer into a validated CCSDS TC packet.
        ///
        /// Runs the full validation pipeline: length, version, packet type,
        /// sequence flags, CRC, and sequence integrity. Each step must pass
        /// before the next runs. Returns the first error encountered.
        ///
        /// @param buffer  Pointer to raw packet bytes. Must remain valid for
        ///                the lifetime of the returned TcPacket.
        /// @param length  Number of bytes in buffer. Must be >= 9.
        ///
        /// @return Ok(TcPacket) if all validations pass.
        ///         Err(ParseError) describing the first failed validation.
        ///
        /// @note No dynamic allocation occurs during parsing.
        Result<TcPacket> parse(const Byte* buffer, U16 length);

        /// @brief Clears stored sequence state for a single application process identifier.
        ///
        /// Use after a subsystem reset affecting only that APID so the next packet
        /// for that APID is accepted without SEQUENCE_REPLAY.
        ///
        /// @param apid  CCSDS APID in range [0, 2048). Behaviour is undefined if
        ///              apid is out of range.
        void reset_sequence(U16 apid);

        /// @brief Clears all per-APID sequence counters.
        ///
        /// Use after a full system restart so no APID carries stale sequence history.
        void reset_all();
        
        private:
            static constexpr U16 MAX_APIDS = 2048;
            static constexpr U16 NO_PACKET_SEEN = 0xFFFF;

            U16 sequence_counts[MAX_APIDS]; // With max sequence count per APID being 0x3FFF (14-Bit field)

            Result<void>          validate_length(const Byte* buffer, U16 length);
            Result<void>          validate_version(const Byte* buffer);
            Result<void>          validate_packet_type(const Byte* buffer);
            Result<void>          validate_sequence_flags(const Byte* buffer);
            Result<void>          validate_crc(const Byte* buffer, U16 length);
            Result<PrimaryHeader> extract_header(const Byte* buffer);
            Result<void>          validate_sequence(const PrimaryHeader& header);
            Result<TcPacket>      build_packet(const PrimaryHeader& header, const Byte* buffer);
            U16 extract_crc(const Byte* buffer, U16 length);
        
    };

}