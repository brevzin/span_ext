#include <compare>
#include <algorithm>
#include <span>

#if __has_include(<concepts>) && __has_include(<ranges>)
#include <concepts>
#include <ranges>

namespace span_ext {
    using std::same_as;
    using std::convertible_to;
    using std::invocable;
    using std::equality_comparable;
    using std::three_way_comparable;

    using std::ranges::begin;
    using std::ranges::end;
    using std::ranges::range_value_t;
    using std::contiguous_iterator;
    using std::ranges::contiguous_range;
}

#else
// rely on range-v3 to provide the implementations of everything
#include <concepts/concepts.hpp>
#include <range/v3/functional/concepts.hpp>
#include <range/v3/range/access.hpp>
#include <range/v3/range/concepts.hpp>
#include <range/v3/range/traits.hpp>

namespace span_ext {
    using concepts::same_as;
    using concepts::convertible_to;
    using ranges::invocable;
    using concepts::equality_comparable;

    template<class T, class Cat>
    concept compares_as = same_as<std::common_type_t<T, Cat>, Cat>;

    template<class T>
    concept three_way_comparable = equality_comparable<T> &&
        requires (std::remove_reference_t<T> const& t) {
            { t < t } -> convertible_to<bool>;
            { t > t } -> convertible_to<bool>;
            { t <= t } -> convertible_to<bool>;
            { t < t } -> convertible_to<bool>;
            { t <=> t } -> compares_as<std::partial_ordering>;
        };

    using ranges::begin;
    using ranges::end;
    using ranges::iter_value_t;
    using ranges::range_value_t;
    using ranges::contiguous_iterator;
    using ranges::contiguous_range;
}
#endif

namespace span_ext {
  inline constexpr auto synth_three_way =
    []<class T>(const T& t, const T& u)
        requires requires {
            { t < u } -> convertible_to<bool>;
            { u < t } -> convertible_to<bool>;
        }
    {
        if constexpr (three_way_comparable<T>) {
            return t <=> u;
        } else {
            if (t < u) return std::weak_ordering::less;
            if (u < t) return std::weak_ordering::greater;
            return std::weak_ordering::equivalent;
        }
    };

    template <typename T>
    concept synth_comparable = invocable<decltype(synth_three_way), T, T>;

    template <typename T, typename U>
    concept sameish = same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

    template <typename R, typename T>
    concept contiguous_range_of = contiguous_range<R const>
        && sameish<T, range_value_t<R const>>;

    template <typename T>
    concept friendly =
        (std::is_arithmetic_v<T> && !std::is_signed_v<T>)
        || std::is_same_v<T, std::byte>;

    template <typename InputIter1, typename InputIter2>
    auto lexicographical_compare_three_way(
                    InputIter1 first1, InputIter1 last1,
                    InputIter2 first2, InputIter2 last2)
        -> decltype(synth_three_way(*first1, *first2))
    {
#if __cpp_lib_three_way_comparison >= 201711L
        return std::lexicographical_compare_three_way(
            first1, last1, first2, last2, synth_three_way);
#else
        if (!std::is_constant_evaluated()) {
            // if both iterators are contiguous and refer to memcmp-friendly types
            if constexpr (contiguous_iterator<InputIter1> &&
                          contiguous_iterator<InputIter2> &&
                          same_as<iter_value_t<InputIter1>,
                                  iter_value_t<InputIter2>> &&
                          friendly<iter_value_t<InputIter1>>)
            {
                auto const len1 = last1 - first1;
                auto const len2 = last2 - first2;

                auto const lencmp = len1 <=> len2;
                auto const len = lencmp < 0 ? len1 : len2;

                if (len != 0) {
                    auto const c = __builtin_memcmp(&*first1, &*first2, len) <=> c;
                    if (c != 0) {
                        return c;
                    }
                }
                return lencmp;
            }
        }

        // just loop
        while (first1 != last1) {
            if (first2 == last2) {
                return std::strong_ordering::greater;
            }
            if (auto const c = synth_three_way(*first1, *first2); c != 0) {
                return c;
            }
            ++first1;
            ++first2;
        }
        return (first2 == last2) <=> true;
#endif
    }
}

namespace std {
    template <span_ext::equality_comparable T, size_t E,
              span_ext::contiguous_range_of<T> R>
    constexpr bool operator==(span<T, E> lhs, R const& rhs)
    {
        return ::std::equal(
            lhs.begin(), lhs.end(),
            span_ext::begin(rhs), span_ext::end(rhs));
    }

    template <span_ext::synth_comparable T, size_t E,
              span_ext::contiguous_range_of<T> R>
    constexpr auto operator<=>(span<T, E> lhs, R const& rhs)
    {
        return span_ext::lexicographical_compare_three_way(
            lhs.begin(), lhs.end(),
            span_ext::begin(rhs), span_ext::end(rhs));
    }
}
