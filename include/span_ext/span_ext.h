/*
 Comparison operators for std::span

 Copyright (c) 2020 - present, Barry Revzin

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 --- Optional exception to the license ---

 As an exception, if, as a result of your compiling your source code, portions
 of this Software are embedded into a machine-executable object form of such
 source code, you may redistribute such embedded portions in such object form
 without including the above copyright and permission notices.
 */


#ifndef SPAN_EXT_COMPARISONS_H
#define SPAN_EXT_COMPARISONS_H

#include <compare>
#include <concepts>
#include <span>

namespace span_ext_expos {
  inline constexpr auto synth_three_way =
    []<class T, class U>(const T& t, const U& u)
        requires requires {
            { t < u } -> std::convertible_to<bool>;
            { u < t } -> std::convertible_to<bool>;
        }
    {
        if constexpr (std::three_way_comparable_with<T, U>) {
            return t <=> u;
        } else {
            if (t < u) return std::weak_ordering::less;
            if (u < t) return std::weak_ordering::greater;
            return std::weak_ordering::equivalent;
        }
    };

    template <typename T>
    concept synth_comparable = std::invocable<decltype(synth_three_way), T, T>;

    template <typename T, typename U>
    concept sameish = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

    template <typename R, typename T>
    concept contiguous_range_of = std::ranges::contiguous_range<R const>
        && sameish<T, std::ranges::range_value_t<R const>>;
}

namespace std {
    template <equality_comparable T, size_t E,
              span_ext_expos::contiguous_range_of<T> R>
    constexpr bool operator==(span<T, E> lhs, R const& rhs)
    {
        return ::std::equal(
            lhs.begin(), lhs.end(),
            rhs.begin(), rhs.end());        
    }

    template <span_ext_expos::synth_comparable T, size_t E,
              span_ext_expos::contiguous_range_of<T> R>
    constexpr auto operator<=>(span<T, E> lhs, R const& rhs)
    {
        return ::std::lexicographical_compare_three_way(
            lhs.begin(), lhs.end(),
            rhs.begin(), rhs.end(),
            span_ext_expos::synth_three_way);
    }    
}


#endif
