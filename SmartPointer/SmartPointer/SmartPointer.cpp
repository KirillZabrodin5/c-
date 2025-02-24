﻿#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <stdexcept>

using namespace std;

template<class T>
class SmartPointer {
    T* object;
    size_t* count;

public:
    //констуктор оборачивающий простую ссылку. пометка explicit предотварщает неявный вызов конструктора.
    explicit SmartPointer(T* object) { 
        if (object == nullptr) {
            throw exception("bad value");
        }
        this->object = object;
        this->count = new size_t(1);
        printf("Method: constructor, count: %d, ref: %d\n", (unsigned int)*(this->count), this);
    }

    SmartPointer(const SmartPointer& other) {
        copyObject(other);
    }

    ~SmartPointer() {
        deleteObject();
    }

    T* operator->() {
        return object;
    }

    T& operator*() {
        return *object;
    }

    SmartPointer& operator=(const SmartPointer& other) {
        deleteObject();
        copyObject(other);
        return *this;
    }

private:
    void copyObject(const SmartPointer& other) {
        this->object = other.object;
        this->count = other.count;
        (*(this->count))++;
        printf("Method: copyObject, count: %d, ref: %d\n", (unsigned int)*(this->count), this);
    }

    void deleteObject() {
        if (*(this->count) > 0) {
            (*(this->count))--;
            printf("Method: deleteObject, count: %d, ref: %d\n", (unsigned int)*(this->count), (unsigned int)this);
        }
        if (*(this->count) == 0) {
            delete this->object;
            delete this->count;
        }
    }
};

int main()
{
    // Tests for SmartPointer
    vector<SmartPointer<string>> list;
    list.push_back(SmartPointer<string>(new string("Hello, world 1")));
    list.push_back(SmartPointer<string>(new string("Hello, world 2")));
    list.push_back(SmartPointer<string>(new string("Hello, world 3")));

    for (int i = 0; i < list.size(); i++) {
        printf("list[%d]: %s\n", i, list[i]->c_str());
    }
}