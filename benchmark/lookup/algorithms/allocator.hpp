#pragma once

#include <cstddef>
#include <memory>

inline std::size_t allocated_size = 0;

template <typename T> class TrackingAllocator {
  public:
    using value_type = T;

    TrackingAllocator() noexcept {}

    template <typename U>
    TrackingAllocator(TrackingAllocator<U> const &) noexcept {}

    template <typename U> struct rebind {
        using other = TrackingAllocator<U>;
    };

    T *allocate(std::size_t n) {
        T *ptr = std::allocator<T>{}.allocate(n);
        allocated_size += (n * sizeof(T));
        return ptr;
    }

    void deallocate(T *p, std::size_t n) {
        std::allocator<T>{}.deallocate(p, n);
        allocated_size -= (n * sizeof(T));
    }
};
