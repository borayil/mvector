/*
 * File: mvector.cpp
 * Author: Bora Yilmaz
 * Date: 28 May 2026
 * Description: RAM locked and memory zeroed generic vector container implementation (attempt)
 * explicit_memset can be used with C++26 
 */

#ifndef MVECTOR_H  // Header Guard
#define MVECTOR_H

#include <iostream>
#include <sys/mman.h>

template <typename T>
class mvector {

    public:
        mvector() {
            _data = nullptr;
            _sz = 0;
            _cap = 0;
        };

        mvector(const size_t& size) {
            _data = new T[size]{};
            lock(_data, size);
            _sz = size;
            _cap = size;
        };

        mvector(const size_t& size, const T& value) {
            _data = new T[size];
            lock(_data, size);
            std::fill(_data, _data + size, value);
            _sz = size;
            _cap = size;
        };

        mvector(std::initializer_list<T> init) {
            const size_t sz = init.size();
            _data = new T[sz];
            lock(_data, sz);
            std::copy(init.begin(), init.end(), _data);
            _sz = sz;
            _cap = sz;

        };

        mvector(mvector&& other) noexcept {
            _data = other._data;
            _sz = other._sz;
            _cap = other._cap;
        }

        mvector(const mvector& other) {
            _data = new T[other._cap];
            lock(_data, other._cap);
            std::copy(other._data, other._data + other._sz, _data);
            _sz = other._sz;
            _cap = other._cap;
        };

        ~mvector() noexcept { 
            clear();
        };

        size_t size() const {
            return _sz;
        }

        size_t capacity() const {
            return _cap;
        }

        T* begin() const {
            return _data;
        }

        T* end() const {
            return _data + _sz;
        }

        mvector& operator=(mvector&& other) noexcept {
            if (this != &other) {
                clear();
                _data = other._data; // already mlocked if _cap > 0
                other._data = nullptr;
                _sz = other._sz;
                other._sz = 0;
                _cap = other._cap;
                other._cap = 0;
            }
            return *this;
        }

        mvector& operator=(const mvector& other) {
            if (this != &other) {
                clear();
                _data = new T[other._cap];
                lock(_data, other._cap);
                std::copy(other._data, other._data + other._sz, _data);
                _sz = other._sz;
                _cap = other._cap;
            }
            return *this;
        }

        T operator[](const size_t& i) {
            return _data[i];
        }

        void clear() {
            if (!_data || !_cap) return;
            std::fill_n(_data, _cap, T{});
            unlock(_data, _cap);
            delete[] _data;
        }

        void reserve(size_t size) {
            grow(size);
        }

        void push_back(const T& value) {
            if (_sz == _cap) grow();
            _data[_sz++] = value;
        };

        template<class... Args>
        void emplace_back(Args&&... args) {
            if (_sz == _cap) grow();
            _data[_sz++] = T(std::forward<Args>(args)...);
        };  

    private:
        T* _data;
        size_t _sz;
        size_t _cap;
        
        void grow() {
            size_t larger_capacity = (_cap == 0) ? 2 : _cap * 2;
            T* larger_data = new T[larger_capacity];
            lock(larger_data, larger_capacity);

            std::copy(_data, _data + _sz, larger_data);
            clear();

            _data = larger_data;
            _cap = larger_capacity;
        }

        void grow(const size_t& size) {
            if (_cap >= size) return;
            T* larger_data = new T[size];
            lock(larger_data, size);

            std::copy(_data, _data + _sz, larger_data);
            clear();

            _data = larger_data;
            _cap = size;
        }

        void lock(T* data, const size_t& size) {
            if (!data || !size) return;

            int r = mlock(data, size);
            if (r != 0) {
                clear();
                std::abort();
            }
        }
    
        void unlock(const T* data, const size_t& size) {
            if (!_data || !_cap) return;

            int r = munlock(data, size);
            if (r != 0) std::cerr << "munlock NOK" << std::endl;
        }
};

#endif