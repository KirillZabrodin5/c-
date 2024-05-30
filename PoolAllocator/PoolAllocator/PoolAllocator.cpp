//тут код: https://github.com/SamuraJey/FILP-MO/blob/develop/Allocators/PoolAllocator.cc


#ifndef POOLALLOCATOR_H
#define POOLALLOCATOR_H
#include <stddef.h>

#include "resources/Allocator.hh"
class PoolAllocator : public Allocator {
public:
    PoolAllocator();
    ~PoolAllocator();
    char* allocate(size_t size) override;
    void deallocate(void*) override;

private:
    struct Buffer;
    Buffer* buffer_list_head;
    size_t space, top;
    void createNewBuffer(size_t size);
};
#endif  // POOLALLOCATOR_H


#include "PoolAllocator.hh"

#include <cstdlib>
#include <new>

#include "resources/constants.hh"

struct PoolAllocator::Buffer {
    Buffer* prev = nullptr;
};

PoolAllocator::PoolAllocator() {
    buffer_list_head = nullptr;
    space = top = 0;
}

PoolAllocator::~PoolAllocator() {
    while (buffer_list_head != nullptr) {
        Buffer* prev = buffer_list_head->prev;
        free(buffer_list_head);
        buffer_list_head = prev;
    }
}

void PoolAllocator::createNewBuffer(size_t size) {
    Buffer* new_buffer = static_cast<Buffer*>(malloc(size + sizeof(Buffer)));
    space = size + sizeof(Buffer);
    top = sizeof(Buffer);
    new_buffer->prev = buffer_list_head;
    buffer_list_head = new_buffer;
}

char* PoolAllocator::allocate(size_t size) {
    if (top + size > space) createNewBuffer((POOL_BUFFER_SIZE > size) ? (POOL_BUFFER_SIZE) : (size));
    char* ret = reinterpret_cast<char*>(buffer_list_head) + top;
    top += size;
    return ret;
}
void PoolAllocator::deallocate(void* ptr) {
    return;  // that is fast code
}

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <map>
#include <vector>

#include "BuddyAllocator.hh"
#include "DescriptorAllocator.hh"
#include "LinkedListAllocator.hh"
#include "PoolAllocator.hh"
#include "ReferenceAllocator.hh"
#include "resources/File_Reading.cc"
#include "resources/STLAdapter.tpp"
#include "resources/TextContainer.hh"
#include "resources/constants.hh"

bool cmp(std::pair<const char*, size_t> First, std::pair<const char*, size_t> Second) noexcept {
    return First.second > Second.second;
}

class CStringComparator {
public:
    // A < B => true
    bool operator()(const char* A, const char* B) const noexcept {
        // Выход на первой отличающейся букве
        // Или на коде терменирующего нуля a = b ='\0' -> ret false
        while (A[0] == B[0] && A[0]) {
            A++;
            B++;
        }
        return A[0] < B[0];
    }
};

void TextMapTest(Allocator* allocator, const char* allocator_name, TextContainer text) {
    printf("\n>>%s test results\n", allocator_name);

    std::chrono::_V2::system_clock::time_point time_mark;
    std::chrono::duration<double> alloc_time;
    std::chrono::duration<double> dealloc_time;
    {
        STLAdapter<char*> WrapperAllocator(allocator);
        std::map<const char*, size_t, CStringComparator, STLAdapter<std::pair<const char* const, size_t>>> Map(WrapperAllocator);

        time_mark = std::chrono::high_resolution_clock::now();
        const char* word;
        while (word = text.GetNextWord()) {  // не нулевая ссылка кастуется к true
            Map[word]++;
        }
        alloc_time = std::chrono::high_resolution_clock::now() - time_mark;

        std::vector<std::pair<const char*, int>> SortedWords(Map.begin(), Map.end());
        std::sort(SortedWords.begin(), SortedWords.end(), cmp);
        int i = 0;
        int num_of_word = 0;
        int num_unique_words = 0;
        for (auto Pair : SortedWords) {
            if (i++ < 10)
                printf("%s: %d\n", Pair.first, Pair.second);
            num_of_word += Pair.second;
            num_unique_words++;
        }
        printf("Different words number: %d\nTotal words number:     %d\n", num_unique_words, num_of_word);

        time_mark = std::chrono::high_resolution_clock::now();
        // именно здесь разрушается Map и освобождается занятая ей память.
    }
    dealloc_time = std::chrono::high_resolution_clock::now() - time_mark;

    printf("counting time:     %f sec\ndeallocation time: %f sec\n\n", alloc_time.count(), dealloc_time.count());
}

int main() {
    char* ReadBuffer = ReadFromFile("../Allocators/resources/war_en.txt");
    TextContainer text_container(ReadBuffer);

    /*
        ReferenceAllocator* referenceAllocator = new ReferenceAllocator();
        TextMapTest(referenceAllocator, "Reference Allocator", text_container);
        delete referenceAllocator;

        PoolAllocator* poolAllocator = new PoolAllocator();
        TextMapTest(poolAllocator, "Pool allocator", text_container);
        delete poolAllocator;

        LinkedListAllocator* linkedListAllocator = new LinkedListAllocator();
        TextMapTest(linkedListAllocator, "Linked list allocator", text_container);
        delete linkedListAllocator;
    */
    DescriptorAllocator* descriptorAllocator = new DescriptorAllocator();
    TextMapTest(descriptorAllocator, "Descriptor allocator", text_container);
    delete descriptorAllocator;

    BuddyAllocator* buddyAllocator = new BuddyAllocator();
    TextMapTest(buddyAllocator, "Buddy allocator", text_container);
    delete buddyAllocator;

    free(ReadBuffer);
    return 0;
}