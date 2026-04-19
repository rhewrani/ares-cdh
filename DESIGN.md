# Architecture
## Overview
```
parser.hpp/cpp
    depends on
    ├── packet.hpp
    ├── result.hpp
    │     └── types.hpp
    └── crc.hpp/cpp
          └── types.hpp
```


## The parsing pipeline
```
            Raw bytes arrive
                    │
                    ▼
 ┌─────────────────────────────────────┐
 │ 1. Length check                     │ 
 │    EXPECTS 9 bytes                  │
 │    REJECT if too short              │
 └──────────────────┬──────────────────┘
                    │
                    ▼
 ┌─────────────────────────────────────┐
 │ 2. Version check                    │
 │    EXPECTS 0b001                    │
 │    REJECT if unknown version        │
 └──────────────────┬──────────────────┘
                    │
                    ▼
 ┌─────────────────────────────────────┐
 │ 3. Type check                       │
 │    EXPECTS TC (1)                   │
 │    REJECT if telemetry packet       │
 └──────────────────┬──────────────────┘
                    │
                    ▼
 ┌─────────────────────────────────────┐
 │ 4. Sequence flags check             │
 │    EXPECTS UNSEGMENTED (0b11)       │
 │    REJECT if other                  │
 └──────────────────┬──────────────────┘
                    │
                    ▼
 ┌─────────────────────────────────────┐
 │ 5. CRC verification                 │
 │    EXPECTS computed CRC to match    │
 │    REJECT + log fault if not        │
 └──────────────────┬──────────────────┘
                    │
                    ▼
 ┌─────────────────────────────────────┐
 │ 6. Sequence validation              │
 │    EXPECTS continous sequence       │
 │    WARN on gap, REJECT replay       │
 └──────────────────┬──────────────────┘
                    │
                    ▼
 ┌─────────────────────────────────────┐
 │ 7. Packet dispatch                  │
 │    Route payload to subsystem       │
 └─────────────────────────────────────┘
```

## Memory Model
This library performs no heap-allocation. All parser state lives in a fixed-size array allocated the construction time. The CRC lookup table is a pre-generated `constexpr` and resides in read-only flash memory on embedded targets. The `TcPacket` payload field is a non-owning pointer and the caller is responsible for keeping the buffer alive and unmodified for the lifetime of the packet.

## Error Handling
This library uses `Result<T>` instead of exceptions to prevent crashes which can be dangerous for flight systems. It calls `std::abort()` on violated preconditions, which indicate a programming error. The distinction here is between programming bugs which should be detected and adressed during ground testing and runtime conditions which should be handled gracefully by the software.

## Standards Compliance
Throughout the library, the CCSDS 133.0-B-2 standard is followed. Though for this library a CRC mandatory: while the standard does include the CRC optionally (mission-specific), to prevent corrupted data being executed, no telecommand is executed without the CRC validation.

## Known Limitations
### Secondary header parsing not implemented
Implementing secondary header parsing would introduce complexity, as this component is mission-specific and would need the library to account for a wide range of variations.

### No Segmented packet reassembly
The parser accepts only unsegmented packets (SequenceFlags::UNSEGMENTED), with segmented commands being rejected. A production implementation would require a reassembly buffer and timeout management.

### No APID dispatch table
An APID dispatch table maps APID values to the subsystem handler functions, routing validated packets automatically. No routing APID table is implemented since routing validated packets to subsystem handlers is the callers responsibility.

### CRC variant is CRC-16/CCITT-FALSE
Supporting multiple variants would require runtime configuration and introduce misconfiguration risk. Verify this matches your mission ICD before integrating.


## Future work
This Library is one pillar of a total of four pillars. 

### Pillar 1 - Flight Software Core (C++ standalone library) / Command & Data Handling (C&DH) engine

### Pillar 2 - Autonomous Systems (C++ core + Python harness) / Guidance, Navigation & Control (GNC) simulator

### Pillar 3 - Ground Segment (CLI tool with real data) / Mission Operations Console

### Pillar 4 - Low-Level Embedded (Embedded target) / A Fault-Tolerant Data Recorder