#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <stdexcept>

using namespace std;

template<class T>
class SmartPointer {
    T* value;
    size_t* count;

public:
    template<class V>
    static SmartPointer<V> makePointer(V* value) {
        return SmartPointer<V>(value);
    }

    explicit SmartPointer(T* value) { 
        if (value == nullptr) {
            throw exception("bad value");
        }
        this->value = value;
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
        return value;
    }

    T& operator*() {
        return *value;
    }

    SmartPointer& operator=(const SmartPointer& other) {
        deleteObject();
        copyObject(other);
        return *this;
    }

private:
    void copyObject(const SmartPointer& other) {
        this->value = other.value;
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
            delete this->value;
        }
    }
};

template<class T>
class SynchroSmartPointer {
    struct SynchroObject {
        size_t count;
        mutex locker;
    };
    T* value;
    SynchroObject* synchroCount;

public:
    template<class V>
    static SynchroSmartPointer<V> makePointer(V* value) {
        return SynchroSmartPointer<V>(value);
    }

    explicit SynchroSmartPointer(T* value) {
        if (value == nullptr) {
            throw exception("bad value");
        }
        this->value = value;
        this->synchroCount = new SynchroObject();
        this->synchroCount->count = 1;
        printf("Method: constructor, count: %d, ref: %d\n", (unsigned int)this->synchroCount->count, this);
    }

    SynchroSmartPointer(const SynchroSmartPointer& other) {
        this->synchroCount = new SynchroObject();
        lock();
        copyObject(other);
        unlock();
    }

    ~SynchroSmartPointer() {
        lock();
        deleteObject();
        unlock();
    }

    T* operator->() {
        return value;
    }

    T& operator*() {
        return *value;
    }

    SynchroSmartPointer& operator=(const SynchroSmartPointer& other) {
        lock();
        deleteObject();
        copyObject(other);
        unlock();
        return *this;
    }

private:
    void lock() {
        this->synchroCount->locker.lock();
    }

    void unlock() {
        this->synchroCount->locker.unlock();
    }

    bool isLock() {
        return this->synchroCount->locker.try_lock();
    }

    void copyObject(const SynchroSmartPointer& other) {
        this->value = other.value;
        if (this->synchroCount == nullptr) {
            this->synchroCount = new SynchroObject();
        }
        this->synchroCount->count = other.synchroCount->count;
        this->synchroCount->count++;
        printf("Method: copyObject, count: %d, ref: %d\n", (unsigned int)this->synchroCount->count, this);
    }

    void deleteObject() {
        if (this->synchroCount->count > 0) {
            this->synchroCount->count--;
            printf("Method: deleteObject, count: %d, ref: %d\n", (unsigned int)this->synchroCount->count, (unsigned int)this);
        }
        if (this->synchroCount->count == 0) {
            delete this->value;
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

    // Tests for SynchroSmartPointer
    SynchroSmartPointer<string> pointer = SynchroSmartPointer<string>(new string("Hello, world 1"));
    SynchroSmartPointer<string> pointer2 = SynchroSmartPointer<string>(pointer);
    SynchroSmartPointer<string> pointer3 = SynchroSmartPointer<string>(pointer2);
    SynchroSmartPointer<string> pointer4 = SynchroSmartPointer<string>(pointer);

    printf("pointer 2: %s\n", pointer2->c_str());
    printf("pointer 3: %s\n", pointer3->c_str());
    printf("pointer 4: %s\n", pointer4->c_str());
}