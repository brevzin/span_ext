#include <span>
#include <span_ext/span_ext.h>
#include <compare>
#include <vector>
#include <string>
#include <string_view>
#include <list>

// simplified versions of concepts that don't include common_reference
template <typename T, typename U>
concept equality_comparable_with = requires (T const& t, U const& u) {
    { t == u } -> span_ext::same_as<bool>;
    { u == t } -> span_ext::same_as<bool>;
    { t != u } -> span_ext::same_as<bool>;
    { u != t } -> span_ext::same_as<bool>;
};

template <typename T, typename U>
concept totally_ordered_with =
    equality_comparable_with<T, U> &&
    requires (T const& t, U const& u) {
        { t <  u } -> span_ext::same_as<bool>;
        { t <= u } -> span_ext::same_as<bool>;
        { t >  u } -> span_ext::same_as<bool>;
        { t >= u } -> span_ext::same_as<bool>;
        { u <  t } -> span_ext::same_as<bool>;
        { u <= t } -> span_ext::same_as<bool>;
        { u >  t } -> span_ext::same_as<bool>;
        { u >= t } -> span_ext::same_as<bool>;
    };

struct B {
    bool operator==(B const&) const;
};
struct D: B { };

static_assert(equality_comparable_with<std::span<int>, std::vector<int>>);
static_assert(totally_ordered_with<std::span<int>, std::vector<int>>);

// mixed const-ness is fine
static_assert(totally_ordered_with<std::span<int>, std::vector<int> const>);
static_assert(totally_ordered_with<std::span<int const>, std::vector<int>>);
static_assert(totally_ordered_with<std::span<int const>, std::vector<int> const>);

static_assert(not equality_comparable_with<std::span<int>, std::list<int>>);
static_assert(not equality_comparable_with<std::span<int>, std::vector<long>>);

static_assert(equality_comparable_with<std::span<B>, std::span<B>>);
static_assert(not totally_ordered_with<std::span<B>, std::span<B>>);
static_assert(equality_comparable_with<std::span<D>, std::span<D>>);
static_assert(not totally_ordered_with<std::span<D>, std::span<D>>);
static_assert(not equality_comparable_with<std::span<B>, std::span<D>>);

template <typename T>
struct X {
    std::vector<T> v;
};
template <typename T>
T const* begin(X<T> const& x) { return x.v.data(); }
template <typename T>
T const* end(X<T> const& x)   { return x.v.data() + x.v.size(); }

static_assert(equality_comparable_with<std::span<int const>, X<int>>);
static_assert(totally_ordered_with<std::span<int const>, X<int>>);

struct NonComparable { };
static_assert(not equality_comparable_with<std::span<NonComparable>, std::span<NonComparable>>);
static_assert(not totally_ordered_with<std::span<NonComparable>, std::span<NonComparable>>);

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// bunch of integer types
TEMPLATE_TEST_CASE("compare_self", "",
    uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t)
{
    std::vector<TestType> x = {1, 2, 3};
    std::span<TestType> s = x;

    CHECK(s == x);
    CHECK_FALSE(s != x);
    CHECK_FALSE(s < x);
    CHECK(s <= x);
    CHECK_FALSE(s > x);
    CHECK(s >= x);

    CHECK(x == s);
    CHECK_FALSE(x != s);
    CHECK_FALSE(x < s);
    CHECK(x <= s);
    CHECK_FALSE(x > s);
    CHECK(x >= s);
}

TEMPLATE_TEST_CASE("compare_same_length_diff", "",
    uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t)
{
    TestType x[] = {1, 2, 3};
    std::vector<TestType> y = {1, 2, 4};

    CHECK_FALSE(std::span(x) == y);
    CHECK_FALSE(x == std::span(y));
    CHECK(std::span(x) != y);
    CHECK(x != std::span(y));
    CHECK(std::span(x) < y);
    CHECK(x < std::span(y));
    CHECK(std::span(x) <= y);
    CHECK(x <= std::span(y));
    CHECK_FALSE(std::span(x) > y);
    CHECK_FALSE(x > std::span(y));
    CHECK_FALSE(std::span(x) >= y);
    CHECK_FALSE(x >= std::span(y));
}

TEMPLATE_TEST_CASE("compare_prefix", "",
    uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t)
{
    TestType x[] = {1, 2, 3};
    std::vector<TestType> y = {1, 2, 3, 4};

    CHECK_FALSE(std::span(x) == y);
    CHECK_FALSE(x == std::span(y));
    CHECK(std::span(x) != y);
    CHECK(x != std::span(y));
    CHECK(std::span(x) < y);
    CHECK(x < std::span(y));
    CHECK(std::span(x) <= y);
    CHECK(x <= std::span(y));
    CHECK_FALSE(std::span(x) > y);
    CHECK_FALSE(x > std::span(y));
    CHECK_FALSE(std::span(x) >= y);
    CHECK_FALSE(x >= std::span(y));
}
