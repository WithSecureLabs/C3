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

#ifndef CPPCODEC_HEX_UPPER
#define CPPCODEC_HEX_UPPER

#include "detail/codec.hpp"
#include "detail/hex.hpp"

namespace cppcodec {

namespace detail {

static constexpr const char hex_upper_alphabet[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F'
};
static_assert(sizeof(hex_upper_alphabet) == 16, "hex alphabet must have 16 values");

class hex_upper : public hex_base
{
public:
    template <typename Codec> using codec_impl = stream_codec<Codec, hex_upper>;

    static inline constexpr char symbol(uint8_t index)
    {
        return hex_upper_alphabet[index];
    }
};

} // namespace detail

using hex_upper = detail::codec<detail::hex<detail::hex_upper>>;

} // namespace cppcodec

#endif // CPPCODEC_HEX_UPPER
