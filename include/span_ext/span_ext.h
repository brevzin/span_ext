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
