﻿#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <map>
#include <vector>
#include <algorithm>
#define BUFFER_SIZE 1024
#define LARGE_BUFFER_SIZE 10*1024*1024
#define DEBUG true
using namespace std;


struct Buffer {
	Buffer* prev = nullptr;
	size_t current = sizeof(Buffer);
	size_t size = 0;
};

#pragma pack(push, 1)

struct Descriptor {
	bool free; //флаг, указывающий, свободен ли блок.
	size_t size;
};

struct BlockHeader {
	Descriptor descriptor;
	union {
		struct {
			BlockHeader* prev, * next;
		};
		char data[1];
	};

	Descriptor* getRightDescriptor() {
		char* rightDescriptor = ((char*)this) + descriptor.size - sizeof(Descriptor);
		return (Descriptor*)rightDescriptor; //указатель на дескр, находящийся в конце блока
	}
};

#pragma pack(pop)

class LinkedListAllocator {
	char* buffer = nullptr;
	BlockHeader* root = nullptr;
	char* endBuffer = nullptr;
public:
	LinkedListAllocator() {
		buffer = (char*)malloc(LARGE_BUFFER_SIZE);
		root = (BlockHeader*)buffer;
		endBuffer = buffer + LARGE_BUFFER_SIZE;
		root->descriptor.size = LARGE_BUFFER_SIZE;
		root->descriptor.free = 1;
		root->getRightDescriptor()->free = 1;
		root->getRightDescriptor()->size = LARGE_BUFFER_SIZE;
		root->prev = root->next = root;
	}

	void remove(BlockHeader* cur) {
		cur->descriptor.free = 0;
		cur->getRightDescriptor()->free = 0;
		if (cur == cur->next) {
			root = nullptr;
			return;
		}
		BlockHeader* prev = cur->prev;
		BlockHeader* next = cur->next;
		prev->next = next;
		next->prev = prev;
		if (root == cur) {
			root = next;
		}
	}

	void insert(BlockHeader* newBlock) {
		newBlock->descriptor.free = 1;
		newBlock->getRightDescriptor()->free = 1;
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

	char* allocate(size_t size) {
		if (root == nullptr) {
			throw bad_alloc();
		}
		BlockHeader* cur = root;
		BlockHeader* result = nullptr;
		size_t generalSize = size + sizeof(Descriptor) * 2;
		while (true) { //ищем блок достаточного размера
			if (cur->descriptor.size >= generalSize) {
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
		remove(result); //удаляем из списка свободных блоков
		size_t freeSpace = result->descriptor.size - generalSize;
		if (freeSpace <= sizeof(BlockHeader)) {
			return &result->data[0];
		}
		char* newBuffer = ((char*)result) + generalSize;
		BlockHeader* newBlock = (BlockHeader*)newBuffer;
		newBlock->descriptor.size = freeSpace;
		newBlock->getRightDescriptor()->size = freeSpace;
		insert(newBlock);
		result->descriptor.size = generalSize;
		result->getRightDescriptor()->size = generalSize;
		result->descriptor.free = 0;
		result->getRightDescriptor()->free = 0;
		return &result->data[0];
	}

	void deallocate(void* temp) {
		char* buffer = (char*)temp;
		BlockHeader* findBlock = (BlockHeader*)(buffer - sizeof(Descriptor));
		BlockHeader* left = nullptr;
		BlockHeader* right = nullptr;
		Descriptor* leftDescriptor = (Descriptor*)((char*)findBlock - sizeof(Descriptor));
		Descriptor* rightDescriptor = (Descriptor*)((char*)findBlock + findBlock->descriptor.size);

		if ((char*)findBlock == buffer) {
			leftDescriptor = nullptr;
		}

		if ((char*)findBlock + findBlock->descriptor.size == endBuffer) {
			rightDescriptor = nullptr;
		}

		//Проверка соседних блоков:
		if (leftDescriptor) 
			if (leftDescriptor->free == 1) {
				left = (BlockHeader*)(((char*)leftDescriptor)
					- leftDescriptor->size + sizeof(Descriptor));
			}
		
		if (rightDescriptor)
			if (rightDescriptor->free == 1)
			{
				right = (BlockHeader*)((char*)rightDescriptor);
			}
		//Объединение с правым и левым блоком:
		if (right != nullptr) {
			remove(right);
			findBlock->descriptor.size += right->descriptor.size;
			findBlock->getRightDescriptor()->size = findBlock->
				descriptor.size;
		}
		insert(findBlock);
		if (left != nullptr) {
			remove(findBlock);
			left->descriptor.size += findBlock->descriptor.size;
			left->getRightDescriptor()->size = left->descriptor.size;
			left->descriptor.free = 1;
			left->getRightDescriptor()->free = 1;
		}
	}

	void check_memory() {
		BlockHeader* cur = root;
		int i = 0;
		if (root == nullptr)
			throw exception("Error: root was nullptr");
		do {
			printf("Buffer #%d, size %d\n", i++, (int)cur->descriptor.size);
			cur = cur->next;
		} while (cur != root);
	}

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

LinkedListAllocator allocator2024;

class CStringComparator
{
public:

	/*
		A < B --> true
	*/
	bool operator()(const char* A, const char* B) const
	{
		while (true)
		{
			if (A[0] == B[0])
			{
				//A = B
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

template <class T>
class CMyAllocator {
public:
	typedef typename T value_type;

	CMyAllocator()
	{

	}

	template <class U>
	CMyAllocator(const CMyAllocator<U>& V)
	{

	}

	T* allocate(size_t Count) {
		//printf("Allocate %d\n", (int)(Count * sizeof(T)));
		//return (T*)malloc(sizeof(T) * Count);
		return (T*)allocator2024.allocate(sizeof(T) * Count);
	}

	void deallocate(T* V, size_t Count)
	{
		//printf("Free %d\n", (int)(Count * sizeof(T)));
		allocator2024.deallocate(V);
		//free(V);
	}
};

bool cmp(pair<const char*, int> First, pair<const char*, int> Second) {
	return First.second > Second.second;
}

void TextMapTest()
{
	map<const char*, size_t, CStringComparator, CMyAllocator<char*>> Map;
	const wchar_t* file_name = L"C:/Users/Кирилл/Desktop/задачи по плюсам/Descriptors/test.txt";
	HANDLE hFile = CreateFile(file_name,               // file to open
		GENERIC_READ,          // open for reading
		FILE_SHARE_READ,       // share for reading
		NULL,                  // default security
		OPEN_EXISTING,         // existing file only
		FILE_ATTRIBUTE_NORMAL, // normal file
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile");
		_tprintf(TEXT("Terminal failure: unable to open file \"%s\" for read.\n"), file_name);
		return;
	}
	LARGE_INTEGER file_size;
	bool get_size = GetFileSizeEx(hFile, &file_size);
	if (!get_size)
	{
		printf("GetFileSizeEx");
		_tprintf(TEXT("Terminal failure: unable to get file size \"%s\" for read.\n"), file_name);
		return;
	}
	//BUFFERSIZE = file_size;
	DWORD  dwBytesRead = 0;
	char* ReadBuffer = (char*)malloc(file_size.QuadPart + 1);
	OVERLAPPED ol = { 0 };
	printf("file_size is: %d\n\r", (int)file_size.QuadPart);

	const char* Words[] = { "Who", "Are", "You", "Who" };
	const char* Word;
	DWORD bytes_read = 0;
	bool OK = ReadFile(hFile, ReadBuffer, file_size.QuadPart, &bytes_read, 0);
	if (!OK) {
		printf("Unable to read file\n");
		return;
	}
	if (bytes_read != file_size.QuadPart) {
		printf("Bytes read isn't equal to file size\n");
		return;
	}
	ReadBuffer[file_size.QuadPart] = '\0';
	printf("bytes_read: %d\n", bytes_read);
	int len_word = 0;
	for (int i = 0; i < bytes_read; i++) {
		if ((unsigned char)ReadBuffer[i] > 32) len_word++;
		else {
			if (len_word == 0)
				continue;
			ReadBuffer[i] = '\0';
			char* Word = &ReadBuffer[i - len_word];
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
		//printf("Word %s, count %I64d\n", Entry.first, (uint64_t)Entry.second);
		statistics.push_back(Entry);
	}
	sort(statistics.begin(), statistics.end(), cmp);

	for (int i = 0; i < min(10, statistics.size()); i++) {
		printf("Word %s, count %I64d\n", statistics[i].first, (uint64_t)statistics[i].second);
	}
	printf("\n");

	CloseHandle(hFile);

}

#include <windows.h>

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
	if (DEBUG)
		allocator2024.check_memory();


	ULONGLONG End = GetCurrentTimeMs();

	printf("Time (ms) %d\n", (int)(End - Start));

	getchar();
	return 0;
}
