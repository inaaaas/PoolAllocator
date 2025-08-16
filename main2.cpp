#include "allocator.hpp"
#include <thread>

class MyObject {
    int x, y;
    MyObject(int a, int b) : x(a), y(b) {
        std::cout << "Object construction" << std::endl;
    }
    ~MyObject() {
        std::cout << "Object destruction" << std::endl;
    }
};
void worker(PoolAllocator<MyObject>& pool, int id) {
    try {
        MyObject* obj = pool.construct(id, id * 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        pool.deallocate(obj);
    }
    catch(const std::bad_alloc&) {
        std::cout << "Thread " << id << ": Pool exhausted!\n";
    }
}

int main() {
    size_t pool_size = 5;
    PoolAllocator<MyObject> pool(pool_size);
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.push_back(std::thread(worker, std::ref(pool), i));
    }

    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Pool available slots after threads: " << pool.available() << "\n";
    return 0;
}