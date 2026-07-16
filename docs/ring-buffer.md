# Ring Buffer

_[Containers library by Ross Smith](index.html)_

```c++
#include "rs-containers/ring-buffer.hpp"
namespace RS;
```

## Contents

* TOC
{:toc}

## Supporting types

```c++
enum class RingPolicy: unsigned char {
    queue,
    stack,
    evict,
    reject,
};
```

Bitmask flags that control the behaviour of the ring buffer.

## RingBuffer class template

```c++
template <std::movable T, RingPolicy P, std::size_t N = 0>
    class RingBuffer;
```

This class implements a circular buffer with a fixed capacity.

The `T` parameter is the value type. THis must be movable but need not be
copyable or default constructible. Behaviour is undefined if `T`'s move
operations can throw an exception.

The `P` parameter determines the buffer's behaviour on insertion or
extraction. Exactly one of each pair must be supplied.

* `queue` -- Queue semantics (first in, first out)
* `stack` -- Stack semantics (last in, first out)
* `evict` -- Discard oldest item on overflow
* `reject` -- Discard new item on overflow

The `N` parameter determines the buffer's capacity. If `N=0,` the capacity is
set at runtime through an argument to the constructor. Otherwise, the
capacity is fixed at compile time.

If the capacity is static, the buffer never performs any dynamic allocation;
all of its memory is allocated on the stack. Otherwise, the necessary memory
is allocated once during construction, and no other allocations are made
later.

```c++
using RingBuffer::value_type = T;
```

Member types.

```c++
static constexpr RingPolicy RingBuffer::policy = P;
static constexpr std::size_t RingBuffer::static_capacity = N;
```

Constants reflecting the template parameters.

```c++
static constexpr bool RingBuffer::is_static;
```

True if the capacity is fixed at compile time.

```c++
static constexpr bool RingBuffer::is_queue;
static constexpr bool RingBuffer::is_stack;
static constexpr bool RingBuffer::is_evict;
static constexpr bool RingBuffer::is_reject;
```

Constants reflecting the policy flags.

```c++
RingBuffer::RingBuffer() noexcept; // static only
explicit RingBuffer::RingBuffer(std::size_t n); // non-static only
```

Constructors. A buffer with static capacity only has a default constructor;
otherwise, the capacity must be supplied to the constructor. Behaviour is
undefined if `n=0.`

```c++
~RingBuffer() noexcept;
```

Destructor.

```c++
RingBuffer(RingBuffer&& rb) noexcept; // non-static only
RingBuffer& operator=(RingBuffer&& rb) noexcept; // non-static only
```

Buffers with non-static capacity are movable, but static ones are not. No
instantiation is copyable.

```c++
bool RingBuffer::empty() const noexcept;
```

True if the buffer is empty.

```c++
bool RingBuffer::full() const noexcept;
```

True if the buffer has reached its capacity.

```c++
std::size_t RingBuffer::size() const noexcept;
```

Returns the number of items currently in the buffer.

```c++
std::size_t RingBuffer::capacity() const noexcept;
```

Returns the buffer's capacity. For static capacity buffers, the returns the
`N` parameter of the class template, and is equal to `static_capacity.`
Otherwise, this returns the value supplied to the constructor.

```c++
void RingBuffer::clear() noexcept;
```

Clears the buffer.

```c++
const T& RingBuffer::next() const noexcept
```

Returns a reference to the next item that will be popped from the buffer. This
will be the first entry if queue semantics are selected, or the last for
stack semantics. Behaviour is undefined if the buffer is empty.

```c++
void RingBuffer::pop() noexcept
```

Deletes the first entry (queue semantics) or the last (stack). Behaviour is
undefined if the buffer is empty.

```c++
T RingBuffer::pull() noexcept
```

Returns (by moving) the first entry (queue semantics) or the last (stack) and
deletes it from the buffer. Behaviour is undefined if the buffer is empty.

```c++
bool RingBuffer::push(const T& t);
bool RingBuffer::push(T&& t) noexcept;
```

Insert a new item into the buffer, by copy (if `T` is copyable) or move. The
`push()` function will not throw any exceptions of its own but will propagate
any exception thrown by copying `T.` If the buffer is already full, behaviour
is determined by whether the `evict` or `reject` option was selected. The
return value indicates whether or not the item was inserted; this is always
true in evict mode.
