#pragma once
#include "packet.hpp"
#include "result.hpp"

namespace ares {

    
    // TcParser validates raw byte buffers against the CCSDS telecommand packet specification.
    // packet specification. On success it returns a TcPacket whose payload
    // points directly into the caller-provided buffer - the caller must
    // keep that buffer alive and unmodified for as long as the TcPacket
    // is in use. On failure it returns a ParseError describing the first
    // validation that failed. The parser maintains per-APID sequence counts
    // across calls to detect replayed and dropped packets. Call reset_sequence()
    // after a subsystem reset, or reset_all() after a full system restart.
    class TcParser { 
        public:
        explicit TcParser();
        
        Result<TcPacket> parse(const Byte* buffer, U16 length);

        // Precondition: apid < MAX_APIDS
        void reset_sequence(U16 apid);
        void reset_all();
        
        private:
            static constexpr U16 MAX_APIDS = 2048;
            static constexpr U16 NO_PACKET_SEEN = 0xFFFF;

            U16 sequence_counts[MAX_APIDS]; // With max sequence count per APID being 0x3FFF (14-Bit field)

            Result<void>          validate_length(const Byte* buffer, U16 length);
            Result<void>          validate_version(const Byte* buffer);
            Result<void>          validate_packet_type(const Byte* buffer);
            Result<void>          validate_crc(const Byte* buffer, U16 length);
            Result<PrimaryHeader> extract_header(const Byte* buffer);
            Result<void>          validate_sequence(const PrimaryHeader& header);
            Result<TcPacket>      build_packet(const PrimaryHeader& header, const Byte* buffer);
            U16 extract_crc(const Byte* buffer, U16 length);
        
    };

}