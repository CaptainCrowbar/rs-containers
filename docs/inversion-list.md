# Inversion list based associative containers

_[Containers library by Ross Smith](index.html)_

```c++
#include "rs-containers/inversion-list.hpp"
namespace RS::Containers;
```

## Contents

* TOC
{:toc}

## Inversion list classes

```c++
template <std::integral Key> class InversionSet;
template <std::integral Key, std::semiregular Value> class InversionMap;
```

These are associative containers based on the inversion list data structure,
optimized for the case where the container consist mostly of large contiguous
blocks with identical values.

In the documentation, `InversionContainer` denotes a member common to both
container types (there is no actual type by that name).

```c++
using InversionContainer::key_type = Key;
using InversionMap::mapped_type = Value;
```

Member types.

```c++
struct InversionSet::block_type {
    Key first;
    Key last;
    block_type() noexcept;
    block_type(Key left, Key right) noexcept;
};

struct InversionMap::block_type {
    Key first;
    Key last;
    Value value;
    block_type() noexcept;
    block_type(Key left, Key right, const Value& val) noexcept;
};
```

The type used to represent a contiguous block of identical keys or key-value
pairs. The two keys are the first and last keys in the block (note that this
is a closed range, not a half-open one).

```c++
using InversionContainer::iterator = [see below];
using InversionContainer::const_iterator = [see below];
```

Iterators over the blocks in an inversion list container. The `iterator` and
`const_iterator` are the same type.

```c++
InversionContainer::InversionContainer();
```

Default constructor. Not useful except for constructing temporaries that will
later be reassigned.

```c++
InversionContainer::InversionContainer
    (std::initializer_list<block_type> list);
```

Constructor from a list of blocks. These will be sorted into key order; any
blocks where the first key is greater than the last will be discarded.

For `InversionSet,` overlapping blocks have no effect, except to slow down
searching very slightly on average. For `InversionMap,` if overlapping blocks
have different mapped values, no promises are made as to which value will be
returned when a key in the overlapping range is lookup up.

```c++
explicit InversionMap::InversionMap
    (std::initializer_list<block_type> list,
        const Value& default_value);
```

Constructor from a list of blocks and a default value. The notes on sorting
and overlapping blocks above also apply to this constructor.

```c++
InversionContainer::InversionContainer();
InversionContainer::InversionContainer(const InversionContainer& c);
InversionContainer::InversionContainer(InversionContainer&& c);
InversionContainer&
    InversionContainer::operator=(const InversionContainer& c);
InversionContainer&
    InversionContainer::operator=(InversionContainer&& c);
```

Other life cycle operations.

```c++
InversionContainer::iterator
    InversionContainer::begin() const noexcept;
InversionContainer::iterator
    InversionContainer::end() const noexcept;
```

Iterators over the sequence of contiguous blocks within the container. The
blocks are returned in key order, regardless of the order in which they were
passed to the constructor. For an `InversionMap,` order is unspecified for
blocks with identical keys but different values.

```c++
bool InversionContainer::empty() const noexcept;
```

True if there are no blocks in the container.

```c++
std::size_t InversionContainer::size() const noexcept;
```

Returns the number of blocks in the container (excluding any invalid blocks
passed to the constructor, but potentially including duplicates).

```c++
bool InversionContainer::contains(Key key) const noexcept;
bool InversionSet::operator()(Key key) const noexcept;
```

True if the key is within any of the container's blocks. For `InversionSet,`
the function call operator is a synonym for `contains().`

```c++
const Value& InversionMap::get(Key key) const noexcept;
const Value& InversionMap::operator[](Key key) const noexcept;
```

Return the value associated with the given key, or the default value if no
block contains the key. The result is unspecified if the key is within more
than one block with different mapped values.

```c++
const Value& InversionMap::default_value() const noexcept;
```

Returns the container's default value.
