// Thanks to
// https://gist.github.com/navanchauhan/0641c902e4a754cba6cc6b553616072f

#include <errno.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"

static inline unsigned int enter_critical_section(void) {
    unsigned int status;
    asm volatile("mrs %0, primask" : "=r"(status));
    asm volatile("cpsid i" ::: "memory");
    return status;
}

static inline void exit_critical_section(unsigned int status) {
    asm volatile("msr primask, %0" ::"r"(status) : "memory");
}

uint32_t __atomic_load_4(volatile uint32_t* ptr) {
    unsigned int status = enter_critical_section();
    uint32_t value = *ptr;
    exit_critical_section(status);
    return value;
}

void __atomic_store_4(volatile uint32_t* ptr, uint32_t value) {
    unsigned int status = enter_critical_section();
    *ptr = value;
    exit_critical_section(status);
}

uint32_t __atomic_fetch_add_4(volatile uint32_t* ptr, uint32_t value) {
    unsigned int status = enter_critical_section();
    uint32_t original = *ptr;
    *ptr += value;
    exit_critical_section(status);
    return original;
}

uint32_t __atomic_fetch_sub_4(volatile uint32_t* ptr, uint32_t value) {
    unsigned int status = enter_critical_section();
    uint32_t original = *ptr;
    *ptr -= value;
    exit_critical_section(status);
    return original;
}

bool __atomic_compare_exchange_4(volatile uint32_t* ptr, uint32_t* expected,
                                 uint32_t desired) {
    unsigned int status = enter_critical_section();
    if (*ptr == *expected) {
        *ptr = desired;
        exit_critical_section(status);
        return true;
    } else {
        *expected = *ptr;
        exit_critical_section(status);
        return false;
    }
}

int posix_memalign(void** memptr, size_t alignment, size_t size) {
    if (alignment % sizeof(void*) != 0 || (alignment & (alignment - 1)) != 0) {
        return EINVAL;
    }

    void* original = malloc(size + alignment - 1);
    if (original == NULL) return ENOMEM;

    uintptr_t offset = (uintptr_t)original % alignment;
    uintptr_t shift = (alignment - offset) % alignment;
    uintptr_t aligned_address = (uintptr_t)original + shift;

    *memptr = (void*)aligned_address;
    return 0;
}