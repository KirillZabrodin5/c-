#include <iostream>
#include <windows.h>
#include <map>
#include <vector>
#include <algorithm>

#define BUFFER_SIZE 1024
#define LARGE_BUFFER_SIZE 1024*1024

using namespace std;

class CStringComparator
{
public:   
    bool operator()(const char* A, const char* B) const
    {
        while (true)
        {
            if (A[0] == B[0])
            {
                if (!A[0])
                    return false;

                A++;
                B++;
            }
            else
            {
                return A[0] < B[0];
            }
        }
    }
};

#pragma pack(push, 1)

struct BlockHeader {
    size_t size;
    union { //union используется для обеспечения доступа к данным блока как к массиву символов (data) или к его полям (prev и next).
        struct {
            BlockHeader* prev, * next;
        };
        char data[1];
    };
};

#pragma pack(pop)


class LinkedListAllocator {
    char* buffer = nullptr;
    BlockHeader* root = nullptr;
public:
    LinkedListAllocator() {
        buffer = (char*)malloc(LARGE_BUFFER_SIZE);
        root = (BlockHeader*)buffer;
        root->size = LARGE_BUFFER_SIZE;
        root->prev = root->next = root;
    }

    void remove(BlockHeader* cur) { //remove используются для удаления блоков в связанном списке
        BlockHeader* prev = cur->prev;
        BlockHeader* next = cur->next;
        prev->next = next;
        next->prev = prev;
        if (next == prev) {
            root = nullptr;
        }
        else if (root == cur) {
            root = next;
        }
    }

    void insert(BlockHeader* newBlock) {//insert используются для вставки блоков в связанном списке
        if (root == nullptr) {
            root = newBlock;
            root->prev = root;
            root->next = root;
            return;
        }
        BlockHeader* prevRoot = root->prev;
        prevRoot->next = newBlock;
        newBlock->prev = prevRoot;
        root->prev = newBlock;
        newBlock->next = root;
    }

    char* allocate(size_t size) {//allocate ищет подходящий блок памяти в связанном списке. Если подходящий 
        //блок найден, он разделяется на два блока: один для запрошенного размера и один для оставшегося свободного места.
        if (root == nullptr) {
            throw bad_alloc();
        }
        BlockHeader* cur = root;
        BlockHeader* result = nullptr;
        size_t generalSize = size + sizeof(size_t);
        while (true) {
            if (cur->size >= generalSize) {
                result = cur;
                break;
            }
            cur = cur->next;
            if (cur == root) {
                break;
            }
        }
        if (result == nullptr) {
            throw bad_alloc();
        }
        remove(result);
        size_t freeSpace = result->size - generalSize;
        if (freeSpace <= sizeof(BlockHeader)) {
            return &result->data[0];
        }
        char* newBuffer = ((char*)result) + generalSize;
        BlockHeader* newBlock = (BlockHeader*)newBuffer;
        newBlock->size = freeSpace;
        insert(newBlock);
        result->size = generalSize;
        return &result->data[0];
    }

    void deallocate(void* temp) { //deallocate объединяет освобожденный блок с соседними свободными блоками, если такие есть.
        char* buffer = (char*)temp;
        BlockHeader* findBlock = (BlockHeader*)(buffer - sizeof(size_t));
        BlockHeader* cur = root;
        BlockHeader* left = nullptr;
        BlockHeader* right = nullptr;
        while (true) {
            if (((char*)cur + cur->size) == (char*)findBlock) {
                left = cur;
            }
            if (((char*)findBlock + findBlock->size) == (char*)cur) {
                right = cur;
            }
            cur = cur->next;
            if (cur == root) {
                break;
            }
        }
        if (right != nullptr) {
            remove(right);
            findBlock->size += right->size;
        }
        insert(findBlock);
        if (left != nullptr) {
            remove(findBlock);
            left->size += findBlock->size;
        }
    }
};



struct Buffer {
    Buffer* prev = nullptr;
    size_t current = sizeof(Buffer);
    size_t size = 0;
};

class Allocator {
    Buffer* some_buffer = nullptr;
public:

    void createNewBuffer(size_t size) {
        Buffer* New = (Buffer*)malloc(size + sizeof(Buffer));
        new (New) Buffer();
        New->prev = some_buffer;
        New->size = size + sizeof(Buffer);
        some_buffer = New;
    }

    Allocator() {
        createNewBuffer(BUFFER_SIZE);
    }

    ~Allocator() {
        while (some_buffer != nullptr) {
            Buffer* prev = some_buffer->prev;
            free(some_buffer);
            some_buffer = prev;
        }
    }

    char* allocate(size_t size) {
        if (some_buffer->size - some_buffer->current < size) {
            createNewBuffer(max(BUFFER_SIZE, size));
        }

        char* ret = ((char*)some_buffer) + some_buffer->current; 
        some_buffer->current = some_buffer->current + size;
        return ret;
    }
    void deallocate(void*) {

    }
};

//Allocator allocator1;
LinkedListAllocator allocator1;

template <class T>
class CMyAllocator {
public:
    typedef typename T value_type;

    CMyAllocator() {}

    template <class U>
    CMyAllocator(const CMyAllocator<U>& V) {}

    T* allocate(size_t Count) {
        return (T*)allocator1.allocate(sizeof(T) * Count);
    }

    void deallocate(T* V, size_t Count)
    {
        allocator1.deallocate(V);
    }
};

bool cmp(pair<const char*, int> First, pair<const char*, int> Second) {
    return First.second > Second.second;
}

void TextMapTest()
{
    map<const char*, size_t, CStringComparator, CMyAllocator<char*>> Map;

    const char* file_name = "C:/Users/Кирилл/Desktop/задачи по плюсам/Task1/test.txt";
    HANDLE hFile = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Unable to open file\n");
        return;
    }

    LARGE_INTEGER file_size;
    GetFileSizeEx(hFile, &file_size);

    char* ReadBuffer = (char*)malloc(file_size.QuadPart + 1);
    DWORD bytes_read = 0;
    BOOL is_read_result = ReadFile(hFile, ReadBuffer, file_size.QuadPart, &bytes_read, NULL);

    if (!is_read_result || bytes_read != file_size.QuadPart)
    {
        printf("Failed to read file\n");
        CloseHandle(hFile);
        delete[] ReadBuffer;
        return;
    }

    ReadBuffer[file_size.QuadPart] = '\0';

    int len_word = 0;
    for (int i = 0; i < bytes_read; ++i) {
        if ((unsigned char)ReadBuffer[i] > 32) len_word++;
        else {
            if (len_word == 0)
                continue;
            ReadBuffer[i] = '\0';
            const char* Word = &ReadBuffer[i - len_word];
            auto It = Map.find(Word);
            if (It == Map.end())
                Map.insert(make_pair(Word, 1));
            else
                It->second++;
            len_word = 0;
        }
    }

    vector<pair<const char*, int>> statistics;
    for (auto Entry : Map) {
        statistics.push_back(Entry);
    }
    sort(statistics.begin(), statistics.end(), cmp);

    for (int i = 0; i < min(10, (int)statistics.size()); i++) {
        printf("Word %s, count %I64d\n", statistics[i].first, (uint64_t)statistics[i].second);
    }

    CloseHandle(hFile);
    delete[] ReadBuffer;
}

ULONGLONG GetCurrentTimeMs()
{
    SYSTEMTIME S = { 0 };
    FILETIME F = { 0 };
    GetSystemTime(&S);
    SystemTimeToFileTime(&S, &F);
    LARGE_INTEGER Int;
    Int.HighPart = F.dwHighDateTime;
    Int.LowPart = F.dwLowDateTime;
    return Int.QuadPart / 10000;
}


int main()
{
    ULONGLONG Start = GetCurrentTimeMs();

    TextMapTest();

    ULONGLONG End = GetCurrentTimeMs();

    printf("Time (ms): %llu\n", End - Start);

    getchar();

    return 0;
}
