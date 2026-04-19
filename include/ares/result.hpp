#pragma once

#include <cstdlib>
#include <variant>

namespace ares {

    /// @brief First validation failure from the parser or related helpers.
    enum class ParseError {
        BUFFER_TOO_SHORT,         ///< Total length below minimum for this parser.
        INVALID_VERSION,          ///< CCSDS version field not 1.
        INVALID_PACKET_TYPE,      ///< Packet type not telecommand.
        CRC_MISMATCH,             ///< Trailing CRC does not match computed value.
        SEQUENCE_REPLAY,          ///< Sequence count did not advance for this APID.
        INVALID_APID,             ///< APID is 0x7FF (reserved) or beyond parser APID table.
        MALFORMED_LENGTH,         ///< Primary data_length field too small for CRC + payload.
        SEQUENCE_NOT_SUPPORTED,   ///< Segmentation flags not UNSEGMENTED.
    };

    /// @brief Discriminated success (@c T) or failure (@ref ParseError); no exceptions.
    template <typename T>
    class Result {
        private:
            std::variant<T, ParseError> data;
            Result() = default;

        public:
            /// @brief Constructs a successful value.
            static Result<T> ok(T val) {
                Result r;
                r.data = std::move(val);
                return r;
            }

            /// @brief Constructs a failed result with @p e.
            static Result<T> err(ParseError e) {
                Result r;
                r.data = e;
                return r;
            }

            /// @return True if this holds a @c T value.
            bool is_ok() const { return std::holds_alternative<T>(data); }
            /// @return True if this holds a @ref ParseError.
            bool is_err() const { return std::holds_alternative<ParseError>(data); }

            /// @return Const reference to the success value.
            /// @note Calls @c std::abort() if @ref is_ok is false.
            const T& value() const {
                if (!is_ok()) std::abort();

                return std::get<T>(data);
            }

            /// @return The error discriminant.
            /// @note Calls @c std::abort() if @ref is_err is false.
            ParseError error() const {
                if (!is_err()) std::abort();

                return std::get<ParseError>(data);
            }
    };

    /// @brief Tag type marking success in Result<void> (no payload).
    struct OkVoid {};

    /// @brief @ref Result specialization when there is no success value.
    template <>
    class Result<void> {
       private:
            std::variant<OkVoid, ParseError> data;
            Result() = default;
       public:
            /// @brief Successful void result.
            static Result<void> ok() {
                Result r;
                r.data = OkVoid{};
                return r;
            }

            /// @brief Failed result with @p e.
            static Result<void> err(ParseError e) {
                Result r;
                r.data = e;
                return r;
            }

            /// @return True if success (no error stored).
            bool is_ok() const { return std::holds_alternative<OkVoid>(data); }
            /// @return True if a @ref ParseError is stored.
            bool is_err() const { return std::holds_alternative<ParseError>(data); }

            /// @return The error discriminant.
            /// @note Calls @c std::abort() if @ref is_err is false.
            ParseError error() const {
                if (!is_err()) std::abort();

                return std::get<ParseError>(data);
            }
    };
}
