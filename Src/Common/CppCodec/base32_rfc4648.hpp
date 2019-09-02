/**
 *  Copyright (C) 2015 Topology LP
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
 */

#ifndef CPPCODEC_BASE32_RFC4648
#define CPPCODEC_BASE32_RFC4648

#include "detail/codec.hpp"
#include "detail/base32.hpp"

namespace cppcodec {

namespace detail {

// RFC 4648 uses a simple alphabet: A-Z starting at index 0, then 2-7 starting at index 26.
static constexpr const char base32_rfc4648_alphabet[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', // at index 26
    '2', '3', '4', '5', '6', '7'
};
static_assert(sizeof(base32_rfc4648_alphabet) == 32, "base32 alphabet must have 32 values");

class base32_rfc4648
{
public:
    template <typename Codec> using codec_impl = stream_codec<Codec, base32_rfc4648>;

    static inline constexpr bool generates_padding() { return true; }
    static inline constexpr bool requires_padding() { return true; }
    static inline constexpr char padding_symbol() { return '='; }

    static inline constexpr char symbol(uint8_t index)
    {
        return base32_rfc4648_alphabet[index];
    }

    static inline constexpr uint8_t index_of(char c)
    {
        return (c >= 'A' && c <= 'Z') ? (c - 'A')
                : (c >= '2' && c <= '7') ? (c - '2' + 26)
                : (c == padding_symbol()) ? 254
                : (c == '\0') ? 255 // stop at end of string
                : (c >= 'a' && c <= 'z') ? (c - 'a') // lower-case: not expected, but accepted
                : throw symbol_error(c);
    }

    // RFC4648 does not specify any whitespace being allowed in base32 encodings.
    static inline constexpr bool should_ignore(uint8_t /*index*/) { return false; }
    static inline constexpr bool is_special_character(uint8_t index) { return index > 32; }
    static inline constexpr bool is_padding_symbol(uint8_t index) { return index == 254; }
    static inline constexpr bool is_eof(uint8_t index) { return index == 255; }
};

} // namespace detail

using base32_rfc4648 = detail::codec<detail::base32<detail::base32_rfc4648>>;

} // namespace cppcodec

#endif // CPPCODEC_BASE32_RFC4648
