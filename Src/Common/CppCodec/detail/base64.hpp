/**
 *  Copyright (C) 2015 Topology LP
 *  Copyright (C) 2013 Adam Rudd (bit calculations)
 *  All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 *
 *  Bit calculations adapted from https://github.com/adamvr/arduino-base64,
 *  commit 999595783185a0afcba156d7276dfeaa9cb5382f.
 */

#ifndef CPPCODEC_DETAIL_BASE64
#define CPPCODEC_DETAIL_BASE64

#include <stdexcept>
#include <stdint.h>

#include "../data/access.hpp"
#include "../parse_error.hpp"
#include "config.hpp"
#include "stream_codec.hpp"

namespace cppcodec {
namespace detail {

template <typename CodecVariant>
class base64 : public CodecVariant::template codec_impl<base64<CodecVariant>>
{
public:
    static inline constexpr uint8_t binary_block_size() { return 3; }
    static inline constexpr uint8_t encoded_block_size() { return 4; }

    static CPPCODEC_ALWAYS_INLINE constexpr uint8_t num_encoded_tail_symbols(uint8_t num_bytes)
    {
        return (num_bytes == 1) ? 2    // 2 symbols, 2 padding characters
                : (num_bytes == 2) ? 3 // 3 symbols, 1 padding character
                : throw std::domain_error("invalid number of bytes in a tail block");
    }

    template <uint8_t I> CPPCODEC_ALWAYS_INLINE static constexpr uint8_t index(
            const uint8_t* b /*binary block*/)
    {
        return (I == 0) ? (b[0] >> 2) // first 6 bits
                : (I == 1) ? (((b[0] & 0x3) << 4) | (b[1] >> 4))
                : (I == 2) ? (((b[1] & 0xF) << 2) | (b[2] >> 6))
                : (I == 3) ? (b[2] & 0x3F) // last 6 bits
                : throw std::domain_error("invalid encoding symbol index in a block");
    }

    template <uint8_t I> CPPCODEC_ALWAYS_INLINE static constexpr uint8_t index_last(
            const uint8_t* b /*binary block*/)
    {
        return (I == 1) ? ((b[0] & 0x3) << 4)    // abbreviated 2nd symbol
                : (I == 2) ? ((b[1] & 0xF) << 2) // abbreviated 3rd symbol
                : throw std::domain_error("invalid last encoding symbol index in a tail");
    }

    template <typename Result, typename ResultState>
    static void decode_block(Result& decoded, ResultState&, const uint8_t* idx);

    template <typename Result, typename ResultState>
    static void decode_tail(Result& decoded, ResultState&, const uint8_t* idx, size_t idx_len);
};


template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base64<CodecVariant>::decode_block(
        Result& decoded, ResultState& state, const uint8_t* idx)
{
    data::put(decoded, state, static_cast<uint8_t>((idx[0] << 2) + ((idx[1] & 0x30) >> 4)));
    data::put(decoded, state, static_cast<uint8_t>(((idx[1] & 0xF) << 4) + ((idx[2] & 0x3C) >> 2)));
    data::put(decoded, state, static_cast<uint8_t>(((idx[2] & 0x3) << 6) + idx[3]));
}

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base64<CodecVariant>::decode_tail(
        Result& decoded, ResultState& state, const uint8_t* idx, size_t idx_len)
{
    if (idx_len == 1) {
        throw invalid_input_length(
                "invalid number of symbols in last base64 block: found 1, expected 2 or 3");
    }

    // idx_len == 2: decoded size 1
    data::put(decoded, state, static_cast<uint8_t>((idx[0] << 2) + ((idx[1] & 0x30) >> 4)));
    if (idx_len == 2) {
        return;
    }

    // idx_len == 3: decoded size 2
    data::put(decoded, state, static_cast<uint8_t>(((idx[1] & 0xF) << 4) + ((idx[2] & 0x3C) >> 2)));
}

} // namespace detail
} // namespace cppcodec

#endif // CPPCODEC_DETAIL_BASE64
