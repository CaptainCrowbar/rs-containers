# Topological Sorting

_[Core utility library by Ross Smith](index.html)_

```c++
#include "rs-core/topological.hpp"
namespace RS;
```

## Contents

* TOC
{:toc}

## Exceptions

```c++
class TopologicalCycle: public std::runtime_error;
```

Exceptions reporting errors in topological order operations.

## Topological order container

```c++
template <std::regular T, std::strict_weak_order<T, T> Cmp = std::less<T>>
    class TopologicalOrder;
```

A container that sorts its elements according to a caller-supplied topological
order, given a set of _(before,after)_ relations. The comparison object is
used only to prioritize elements within a subset; it does not participate in
the topological relation.

```c++
using value_type = T;
using compare_type = Cmp;
```

Member types.

```c++
TopologicalOrder::TopologicalOrder();
explicit TopologicalOrder::TopologicalOrder(Cmp c);
TopologicalOrder::TopologicalOrder(const TopologicalOrder& topo);
TopologicalOrder::TopologicalOrder(TopologicalOrder&& topo) noexcept;
TopologicalOrder::~TopologicalOrder() noexcept;
TopologicalOrder&
    TopologicalOrder::operator=(const TopologicalOrder& topo);
TopologicalOrder&
    TopologicalOrder::operator=(TopologicalOrder&& topo) noexcept;
```

Life cycle functions.

```c++
Cmp TopologicalOrder::comp() const;
```

Returns a copy of the comparison predicate.

```c++
bool TopologicalOrder::contains(const T& t) const;
```

True if the element is in the container.

```c++
bool TopologicalOrder::empty() const noexcept;
```

True if the container is empty.

```c++
std::size_t TopologicalOrder::size() const noexcept;
```

Returns the number of elements in the container.

```c++
template <std::convertible_to<T>... Args>
    void TopologicalOrder::insert(const Args&... args);
template <ReadableRange<T> Range>
    void TopologicalOrder::insert(const Range& r);
```

Insert one or more elements. If only one element is inserted, it has no
implied ordering relations with any other elements. If multiple elements are
listed, an ordering relation is implied between each pair of adjacent
elements in the list. In all insert functions, implied links between an
element and itself are ignored. Duplicate links are also ignored.

```c++
template <ReadableForwardRange<T> Range>
    void TopologicalOrder::insert(const T& t1, const Range& r2);
void TopologicalOrder::insert(const T& t1, std::initializer_list<T> r2);
template <ReadableForwardRange<T> Range>
    void TopologicalOrder::insert(const Range& r1, const T& t2);
void TopologicalOrder::insert(std::initializer_list<T> r1, const T& t2);
template <ReadableForwardRange<T> Range1, ReadableForwardRange<T> Range2>
    void TopologicalOrder::insert(const Range1& r1, const Range2& r2);
void TopologicalOrder::insert(std::initializer_list<T> r1,
    std::initializer_list<T> r2);
```

Insert one or more pairs of elements, with ordering relations between the left
and right arguments. An ordering relation is created between every pair in
the Cartesian product of the two ranges.

```c++
bool TopologicalOrder::erase(const T& t);
```

Erase a specific element from the container. This will do nothing if the
element was not present. The return value indicates whether an element was
found and removed.

```c++
bool TopologicalOrder::erase_edge(const T& t1, const T& t2);
```

Erase the link between two elements, if there is one (in either direction).
The return value indicates whether a link was found and removed.

```c++
std::size_t TopologicalOrder::erase_orphans();
```

Remove all elements that have no incoming or outgoing links. The return value
indicates how many elements were removed.

```c++
void TopologicalOrder::clear() noexcept;
```

Remove all elements from the container.

```c++
std::vector<T> TopologicalOrder::front_set() const;
std::vector<T> TopologicalOrder::back_set() const;
```

Query the current front set (elements that never appear on the right of an
ordering relation) or back set (elements that never appear on the left). The
elements in each returned list will be ordered according to the comparison
predicate. An element that has no ordering relations with any other element
will appear in both sets.

These will throw `TopologicalCycle` if a cycle is detected. Not all cycles
will be detected by these functions; a cycle that involves only interior
elements, while still leaving well defined front and back sets, will not be
detected here.

```c++
std::optional<T> TopologicalOrder::front() const;
std::optional<T> TopologicalOrder::back() const;
```

Query the first element (in the order implied by the comparison predicate) of
the front or back set. These will return a null optional if the graph is
empty. They will throw `TopologicalCycle` if a cycle is detected, with the
same caveats noted above.

```c++
std::vector<T> TopologicalOrder::pop_front_set();
std::vector<T> TopologicalOrder::pop_back_set();
std::optional<T> TopologicalOrder::pop_front();
std::optional<T> TopologicalOrder::pop_back();
```

Remove all or one of the current front or back set, returning the removed
elements. These will throw `TopologicalCycle` under the same circumstances as
the functions described above.

```c++
bool TopologicalOrder::is_front(const T& t) const;
bool TopologicalOrder::is_back(const T& t) const;
```

True if the given element is part of the current front or back set.
