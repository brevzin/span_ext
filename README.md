# span_ext

When `std::span` was added by [P0122](https://wg21.link/p0122), it had comparison
operators that performed a deep comparison. Those comparison operators were
removed as a result of [P1085](https://wg21.link/p1085).

I disagree with that removal - I do not consider it important that `span`
be Regular (and am not even sure that a deep-comparing fail fails to model
[`std::equality_comparable`](http://eel.is/c++draft/concept.equalitycomparable))
and I consider comparisons for `std::span` to be quite useful, and that as a
result we're missing functionality.

The goal of this repository is to provide that missing functionality, if you
choose to use it, and to gather feedback about the use of these operations.
If this repo proves broadly useful, then we should reconsider P1085. If this
repo reveals problems with `std::span` having deep comparison, then I will
reconsider my position.

Note that this repository does provide comparison operators in `namespace std`.
This is technically UB, as only the standard library is allowed to put things
in there (outside of specific exceptions, like providing specializations for
specific class templates in `std`), but these operators do not exist today so
there is nothing to conflict with.

# Design

Given the premise that `std::span` should have deep comparisons, the question
is what comparisons should be allowed? Should `std::span<int>` be comparable
with:

1. `std::span<int const>`?
2. `std::vector<int>`?
3. `std::list<int>`?
4. `std::vector<long>`?

My position is that `span`'s direct comparison operators should behave basically
like a `memcmp` - the easy-to-use syntax should only be there when you're
directly comparing blocks of memory. To that end, (1) and (2) should be allowed
(differing `const`ness is irrelevant and we don't care who owns the block of
memory) but comparing against (3) non-contiguous containers or (4) containers
of a different type are not supported. There is always `std::ranges::equal` for
that.

In other words, in this repo,
`span<T>` is comparable with any `contiguous_range` with
`value_type` `U` such that `remove_cvref_t<T>` and `remove_cvref_t<U>` are the
same type.

Note that while with this design, you can compare a `span<int>` to a `vector<int>`,
it will still be the case that `std::equality_comparable_with<span<int>, vector<int>>`
fails while `std::equality_comparable_with<span<int const>, vector<int>>` holds.
This is because `span<int>` and `vector<int>` do not share a common reference
(while `span<int const>` and `vector<int>` do) - because `span<int>` is not
constructible from a `vector<int>` rvalue.

# Usage

Just include `<span_ext/span_ext.h>` and it's as if `std::span` itself had
comparison operators already.

# Compiler requirements

These operators depend on the existence of `std::span`, the Concepts language
feature, some of the functionality introduced by Ranges, and `<=>`. Currently,
that's just gcc trunk.