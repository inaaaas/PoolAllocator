#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP
#include <iostream>
#include <vector>
#include <memory>
#include <string>

template <typename T>
class PoolAllocator {
        std::size_t m_size;
        std::size_t availableSlots;
        void* freeSlots;
        void* memory;
        mutable std::mutex mtx;

    public:
        explicit PoolAllocator(std::size_t capacity);
        ~PoolAllocator();

        T* allocate();
        void deallocate(T* ptr);

        template <typename... Args>
        T* construct(Args&&... args);
        
        void destroy(T* ptr);

        std::size_t capacity() const;
        std::size_t available() const;
        bool is_full() const;
        bool is_empty() const;
};



#endif