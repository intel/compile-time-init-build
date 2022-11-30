#pragma once

extern void operator delete(void *ptr, std::size_t) noexcept;

struct testing_logger {
    testing_logger();
    ~testing_logger();
};
