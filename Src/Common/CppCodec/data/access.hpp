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

#ifndef CPPCODEC_DETAIL_DATA_ACCESS
#define CPPCODEC_DETAIL_DATA_ACCESS

#include <stdint.h> // for size_t

namespace cppcodec {
namespace data {

// This file contains a number of templated data accessors that can be
// implemented in the cppcodec::data namespace for types that don't fulfill
// the default type requirements:
// For result types: init(Result&, ResultState&, size_t capacity),
//     put(Result&, ResultState&, char), finish(Result&, State&)
// For const (read-only) types: char_data(const T&)
// For both const and result types: size(const T&)

template <typename T> inline size_t size(const T& t) { return t.size(); }
template <typename T, size_t N> inline constexpr size_t size(const T (&t)[N]) noexcept {
    return N * sizeof(t[0]);
}

class general_t {};
class specific_t : public general_t {};

class empty_result_state {
    template <typename Result> inline void size(const Result& result) { return size(result); }
};

// SFINAE: Generic fallback in case no specific state function applies.
template <typename Result>
inline empty_result_state create_state(Result&, general_t) { return empty_result_state(); }

//
// Generic templates for containers: Use these init()/put()/finish()
// implementations if no specialization was found.
//

template <typename Result>
inline void init(Result& result, empty_result_state&, size_t capacity)
{
    result.resize(0);
    result.reserve(capacity);
}

template <typename Result>
inline void finish(Result&, empty_result_state&)
{
    // Default is to push_back(), which already increases the size.
}

// For the put() default implementation, we try calling push_back() with either uint8_t or char,
// whichever compiles. Scary-fancy template magic from http://stackoverflow.com/a/1386390.
namespace fallback {
    struct flag { char c[2]; }; // sizeof > 1
    flag put_uint8(...);

    int operator,(flag, flag);
    template <typename T> void operator,(flag, T&); // map everything else to void
    char operator,(int, flag); // sizeof 1
}

template <typename Result> inline void put_uint8(Result& result, uint8_t c) { result.push_back(c); }

template <bool> struct put_impl;
template <> struct put_impl<true> { // put_uint8() available
    template<typename Result> static void put(Result& result, uint8_t c) { put_uint8(result, c); }
};
template <> struct put_impl<false> { // put_uint8() not available
    template<typename Result> static void put(Result& result, uint8_t c) {
        result.push_back(static_cast<char>(c));
    }
};

template <typename Result> inline void put(Result& result, empty_result_state&, uint8_t c)
{
    using namespace fallback;
    put_impl<sizeof(fallback::flag(), put_uint8(result, c), fallback::flag()) != 1>::put(result, c);
}

//
// Specialization for container types with direct mutable data access.
// The expected way to specialize is to subclass empty_result_state and
// return an instance of it from a create_state() template specialization.
// You can then create overloads for init(), put() and finish()
// that are more specific than the empty_result_state ones above.
// See the example below for direct access to a mutable data() method.
//
// If desired, a non-templated overload for both specific types
// (result & state) can be added to tailor it to that particular result type.
//

template <typename Result> class direct_data_access_result_state : empty_result_state
{
public:
    using result_type = Result;

    inline void init(Result& result, size_t capacity)
    {
        // resize(0) is not called here since we don't rely on it
        result.reserve(capacity);
    }
    inline void put(Result& result, char c)
    {
        // This only compiles if decltype(data) == char*
        result.data()[m_offset++] = static_cast<char>(c);
    }
    inline void finish(Result& result)
    {
        result.resize(m_offset);
    }
    inline size_t size(const Result&)
    {
        return m_offset;
    }
private:
    size_t m_offset = 0;
};

// SFINAE: Select a specific state based on the result type and possible result state type.
// Implement this if direct data access (`result.data()[0] = 'x') isn't already possible
// and you want to specialize it for your own result type.
template <typename Result, typename ResultState =
        typename direct_data_access_result_state<Result>::result_type::value>
inline ResultState create_state(Result&, specific_t) { return ResultState(); }

template <typename Result>
inline void init(Result& result, direct_data_access_result_state<Result>& state, size_t capacity)
{
    state.init(result);
}

// Specialized put function for direct_data_access_result_state.
template <typename Result>
inline void put(Result& result, direct_data_access_result_state<Result>& state, char c)
{
    state.put(result, c);
}

// char_data() is only used to read, not for result buffers.
template <typename T> inline const char* char_data(const T& t)
{
    return reinterpret_cast<const char*>(t.data());
}
template <typename T, size_t N> inline const char* char_data(const T (&t)[N]) noexcept
{
    return reinterpret_cast<const char*>(&(t[0]));
}

template <typename T> inline const uint8_t* uchar_data(const T& t)
{
    return reinterpret_cast<const uint8_t*>(char_data(t));
}

} // namespace data
} // namespace cppcodec

#endif
