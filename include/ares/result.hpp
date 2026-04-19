#pragma once

#include "types.hpp"
#include <cstdlib>
#include <variant>

namespace ares {
    enum class ParseError {
        BUFFER_TOO_SHORT,
        INVALID_VERSION,
        INVALID_PACKET_TYPE,
        CRC_MISMATCH,
        SEQUENCE_REPLAY,
        INVALID_APID,
        MALFORMED_LENGTH,
        SEQUENCE_NOT_SUPPORTED,
    };

    template <typename T>
    class Result {
        private:
            std::variant<T, ParseError> data;
            Result() = default;

        public:
            static Result<T> ok(T val) {
                Result r;
                r.data = std::move(val);
                return r;
            }
            
            static Result<T> err(ParseError e) {
                Result r;
                r.data = e;
                return r;
            }

            bool is_ok() const { return std::holds_alternative<T>(data); }
            bool is_err() const { return std::holds_alternative<ParseError>(data); }

            const T& value() const {
                if (!is_ok()) std::abort(); 

                return std::get<T>(data);
            }

            ParseError error() const {
                if (!is_err()) std::abort();

                return std::get<ParseError>(data);
            }
    };

    struct OkVoid {};

    template <>
    class Result<void> {
       private:
            std::variant<OkVoid, ParseError> data;
            Result() = default;
       public:
            static Result<void> ok() {
                Result r;
                r.data = OkVoid{};
                return r;
            }

            static Result<void> err(ParseError e) {
                Result r;
                r.data = e;
                return r;
            }

            bool is_ok() const { return std::holds_alternative<OkVoid>(data); }
            bool is_err() const { return std::holds_alternative<ParseError>(data); }


            ParseError error() const {
                if (!is_err()) std::abort();

                return std::get<ParseError>(data);
            }
    };
}