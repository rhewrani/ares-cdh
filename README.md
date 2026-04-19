# ares_cdh - Command & Data Handling Library

A CCSDS Space Packet Protocol telecommand parser built to 
safety-critical design principles. Implements the full validation 
pipeline from raw bytes to structured data with no dynamic allocation, 
no exceptions and deterministic control flow.

CCSDS (Consultative Committee for Space Data Systems) is the packet 
format used on missions from Voyager to the James Webb Space Telescope. 
This implementation follows constraints found in JPL's coding standard 
and MISRA C++ - the same rules governing flight software on real 
spacecraft.

---

## Packet Structure

```
┌──────────────────────────────────────────────────────────┐
│                  PRIMARY HEADER (6 bytes)                │
│  ┌──────────────┬──────────────┬────────────────────┐    │
│  │ Packet ID    │  Seq Control │   Packet Length    │    │
│  │  (2 bytes)   │  (2 bytes)   │     (2 bytes)      │    │
│  └──────────────┴──────────────┴────────────────────┘    │
├──────────────────────────────────────────────────────────┤
│              SECONDARY HEADER (optional, variable)       │
│         typically: timestamp, subsystem routing          │
├──────────────────────────────────────────────────────────┤
│                 USER DATA / PAYLOAD                      │
│              (the actual command content)                │
├──────────────────────────────────────────────────────────┤
│              ERROR CONTROL (optional, 2 bytes)           │
│                      CRC-16/CCITT                        │
└──────────────────────────────────────────────────────────┘
```
---

## Validation Pipeline

Every incoming packet passes through six sequential checks. 
A failure at any step immediately returns a typed error - 
no subsequent steps run on invalid data.

1. **Length**         Minimum 9 bytes (6 header + 1 payload + 2 CRC)
2. **Version**        Must be CCSDS version 1 (0b001)
3. **Packet Type**    Must be Telecommand (TC), not Telemetry (TM)
4. **CRC**            CRC-16/CCITT-FALSE verified over header + payload
5. **Header Extract** Bit fields unpacked into typed PrimaryHeader struct
6. **Sequence**       Per-APID counter checked for replays and gaps

---

## Design Decisions

| Decision | Reason |
|---|---|
| No dynamic allocation | Bounded memory, no fragmentation, safe for embedded targets |
| Errors as values (`Result<T>`) | No exceptions - deterministic control flow, no hidden paths |
| `constexpr` CRC lookup table | Computed at compile time, lives in read-only flash memory |
| `static_cast` on all narrowing conversions | Explicit intent - no silent truncation from implicit promotion |
| `std::abort()` on precondition violations | Fail-fast - programming bugs surface during ground testing |
| Per-APID sequence tracking | Independent fault isolation per subsystem |
| Fixed-size types (`U8`, `U16`, `U32`) | Platform-independent - same behaviour on every target |

---

## Error Handling

This library does not use C++ exceptions. All fallible operations 
return `Result<T>` - either `Ok(value)` or `Err(ParseError)`.

```cpp
auto result = parser.parse(buffer, length);

if (result.is_ok()) {
    const TcPacket& pkt = result.value();
    // use pkt
} else {
    ParseError e = result.error();
    // log fault, continue
}
```

Calling `value()` on a failed result or `error()` on a success 
calls `std::abort()` - a deliberate fail-fast mechanism that 
surfaces programming errors during ground testing.

---

## CRC

The error control field uses CRC-16/CCITT-FALSE 
(polynomial `0x1021`, init `0xFFFF`). A 256-entry lookup 
table is computed at compile time via `constexpr` - 
each byte requires only one table lookup and one XOR 
at runtime, giving O(n) validation with minimal overhead.

The CRC covers the entire packet except the CRC field itself. 
A mismatch causes the packet to be silently dropped and a 
`ParseError::CRC_MISMATCH` returned - the command is never 
executed.

---

## Building

```bash
git clone https://github.com/you/ares_cdh
cd ares_cdh
mkdir build && cd build
cmake ..
cmake --build .
```

## Running Tests

```bash
./tests/test_crc
./tests/test_parser
```

---

## Integration

### Prerequisites
- CMake 3.16 or higher
- C++17 compiler (GCC 10+, Clang 11+, MSVC 2019+)

### Option A - CMake FetchContent (Recommended)

CMakeLists.txt:

```cmake
include(FetchContent)
FetchContent_Declare(
    ares_cdh
    GIT_REPOSITORY https://github.com/rhewrani/ares_cdh
    GIT_TAG        main
)
FetchContent_MakeAvailable(ares_cdh)

target_link_libraries(your_target PRIVATE ares_cdh)
```

### Option B - Add As A Subdirectory

If you've cloned the repository into your project:

```cmake
add_subdirectory(path/to/ares_cdh)
target_link_libraries(your_target PRIVATE ares_cdh)
```

### Usage

```cpp
#include "ares/parser.hpp"

ares::TcParser parser;
auto result = parser.parse(buffer, length);

if (result.is_ok()) {
    const ares::TcPacket& pkt = result.value();
    // use pkt.payload, pkt.payload_length, etc.
} else {
    ares::ParseError e = result.error();
    // handle error
}
```

---

## Known Limitations

- Secondary header parsing not implemented - format is 
  mission-specific
- Segmented packet reassembly not implemented
- No APID dispatch table
- CRC variant is CRC-16/CCITT-FALSE - verify against your 
  mission ICD before integrating

---

## Standards Reference

CCSDS 133.0-B-2 - Space Packet Protocol (June 2020)
https://ccsds.org/Pubs/133x0b2e2.pdf