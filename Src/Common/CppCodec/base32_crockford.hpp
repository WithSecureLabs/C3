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

#ifndef CPPCODEC_BASE32_CROCKFORD
#define CPPCODEC_BASE32_CROCKFORD

#include "detail/codec.hpp"
#include "detail/base32.hpp"

namespace cppcodec {

namespace detail {

static constexpr const char base32_crockford_alphabet[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', // at index 10
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',           // 18 - no I
    'J', 'K',                                         // 20 - no L
    'M', 'N',                                         // 22 - no O
    'P', 'Q', 'R', 'S', 'T',                          // 27 - no U
    'V', 'W', 'X', 'Y', 'Z'                           // 32
};
static_assert(sizeof(base32_crockford_alphabet) == 32, "base32 alphabet must have 32 values");

class base32_crockford_base
{
public:
    static inline constexpr bool generates_padding() { return false; }
    static inline constexpr bool requires_padding() { return false; }
    static inline constexpr bool is_padding_symbol(char /*c*/) { return false; }

    static inline constexpr char symbol(uint8_t index)
    {
        return base32_crockford_alphabet[index];
    }

    static inline constexpr uint8_t index_of(char c)
    {
        return (c >= '0' && c <= '9') ? (c - '0')
                // upper-case letters
                : (c >= 'A' && c <= 'H') ? (c - 'A' + 10) // no I
                : (c >= 'J' && c <= 'K') ? (c - 'J' + 18) // no L
                : (c >= 'M' && c <= 'N') ? (c - 'M' + 20) // no O
                : (c >= 'P' && c <= 'T') ? (c - 'P' + 22) // no U
                : (c >= 'V' && c <= 'Z') ? (c - 'V' + 27)
                // lower-case letters
                : (c >= 'a' && c <= 'h') ? (c - 'a' + 10) // no I
                : (c >= 'j' && c <= 'k') ? (c - 'j' + 18) // no L
                : (c >= 'm' && c <= 'n') ? (c - 'm' + 20) // no O
                : (c >= 'p' && c <= 't') ? (c - 'p' + 22) // no U
                : (c >= 'v' && c <= 'z') ? (c - 'v' + 27)
                : (c == '-') ? 253 // "Hyphens (-) can be inserted into strings [for readability]."
                : (c == '\0') ? 255 // stop at end of string
                // special cases
                : (c == 'O' || c == 'o') ? 0
                : (c == 'I' || c == 'i' || c == 'L' || c == 'l') ? 1
                : throw symbol_error(c);
    }

    static inline constexpr bool should_ignore(uint8_t index) { return index == 253; }
    static inline constexpr bool is_special_character(uint8_t index) { return index > 32; }
    static inline constexpr bool is_eof(uint8_t index) { return index == 255; }
};

// base32_crockford is a concatenative iterative (i.e. streaming) interpretation of Crockford base32.
// It interprets the statement "zero-extend the number to make its bit-length a multiple of 5"
// to mean zero-extending it on the right.
// (The other possible interpretation is base32_crockford_num, a place-based single number encoding system.
// See http://merrigrove.blogspot.ca/2014/04/what-heck-is-base64-encoding-really.html for more info.)
class base32_crockford : public base32_crockford_base
{
public:
    template <typename Codec> using codec_impl = stream_codec<Codec, base32_crockford>;
};

} // namespace detail

using base32_crockford = detail::codec<detail::base32<detail::base32_crockford>>;

} // namespace cppcodec

#endif // CPPCODEC_BASE32_CROCKFORD
