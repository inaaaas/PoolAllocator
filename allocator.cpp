#include "allocator.hpp"

template <typename T>
PoolAllocator<T>::PoolAllocator(std::size_t capacity) 
        : m_size(capacity), freeSlots(nullptr), availableSlots(size) {
            if (m_size == 0) {
                throw std::runtime_error("size must be greater than zero");
            }

            
            std::size_t slot_size = sizeof(T) > sizeof(void*) ? sizeof(T) : sizeof(void*);
            std::size_t size = slot_size * capacity;

            memory = ::operator new(size, std::align_val_t(allignof(T)));
            //aranc construction miayn malloc e anum


            uint8_t* ptr = static_cast<uint8_t>(memory);
            for (std::size_t i = 0; i < capacity; ++i) {
                void* slot = ptr + i * slot_size;
                *reinterpret_cast<void**>(slot) = (i + 1 < capacity) ? (ptr + (i + 1) * slot_size) : nullptr;
            }
            freeSlots = memory;
        }

template <typename T>
PoolAllocator<T>::~PoolAllocator() {
    ::operator delete(memory, std::align_val_t(alignof(T)));
}

template <typename T>
T* PoolAllocator<T>::allocate(){
    std::lock_guard<std::mutex> lock(mtx);
    if (!freeSlots) return nullptr;

    void* slot = freeSlots;
    freeSlots = *reinterpret_cast<void**>(slot);
    --availableSlots;
    return static_cast<T*>(slot);
}

template <typename T>
template <typename... Args>
 T* PoolAllocator<T>::construct(Args&&... args){
    T* mem = allocate();
    if (!mem) {
        throw std::bad_alloc();
    }
    return new(mem) T(std::forward<Args>(args)...);
}
        
template <typename T>
void PoolAllocator<T>::destroy(T* ptr){
    ptr->~T();
    ptr = nullptr;
}

template <typename T>
void PoolAllocator<T>::deallocate(T* ptr){
    if (!ptr) return;
    destroy(ptr);

    std::lock_guard<std::mutex> lock(mtx);
    *reinterpret_cast<void**>(ptr) = freeSlots;
    freeSlots = ptr;
    ++availableSlots;
}



template <typename T>
std::size_t PoolAllocator<T>::capacity() const{
    std::lock_guard<std::mutex> lock(mtx);
    return capacity;
}
        
        
template <typename T>
std::size_t PoolAllocator<T>::available() const{
    std::lock_guard<std::mutex> lock(mtx);
    return availableSlots;    
}
        
        
template <typename T>
bool PoolAllocator<T>::is_full() const{
    std::lock_guard<std::mutex> lock(mtx);
    return availableSlots == 0;
}
       
       
template <typename T>
bool PoolAllocator<T>::is_empty() const{
    std::lock_guard<std::mutex> lock(mtx);
    return availableSlots == capacity;
}