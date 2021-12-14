// TODO(Brandon): Replace with custom allocator.
#pragma once
#include <assert.h>
#include <memory>
#include "types.h"
#include <core/memory/vultr_memory.h>

namespace vtl
{
    template <typename T, size_t reserved = 10, u32 growth_numerator = 3, u32 growth_denominator = 2, u32 decay_percent_threshold = 30>
    struct DynamicArray : Vultr::MemoryStruct
    {
        // Initialize an empty DynamicArray
        // Reserved specifies a _size of the DynamicArray that will be reserved initially
        // which will be empty
        DynamicArray(Vultr::Allocator *a) : Vultr::MemoryStruct(a)
        {
            _size = reserved;
            len   = 0;
            if (reserved > 0)
            {
                // _array = static_cast<T *>(malloc(_size * sizeof(T)));
            }
        }

        DynamicArray(Vultr::Allocator *a, T *array, size_t count) : Vultr::MemoryStruct(a)
        {
            assert(count != 0 && "Count must be greater than 0!");
            assert(array != nullptr && "Array must not be null!");

            len   = count;
            _size = len * growth_factor;
            if (_size < reserved)
            {
                _size = reserved;
            }

            // _array = static_cast<T *>(malloc(sizeof(T) * _size));

            for (int i = 0; i < count; i++)
            {
                _array[i] = array[i];
            }
        }

        // Delete copy methods because we don't want this to be done on accident, we want it to be very very explicit since we are essentially duplicating a buffer
        DynamicArray<T> &operator=(const DynamicArray<T> &other) = delete;
        DynamicArray(const DynamicArray<T> &other)               = delete;

        // Destructor for dynamic array
        ~DynamicArray()
        {
            // if (_array != nullptr)
            //     free(_array);
        }

        // Push element to back of dynamic_array
        // If the dynamic_array does not have enough space, then a reallocation will
        // occur
        //
        // Returns the element just inserted
        T *push_back(const T &element)
        {
            len++;
            reallocate();

            // Increase the len amount and assign the last element of array to the new
            // element
            _array[len - 1] = element;

            return &_array[len - 1];
        }

        // Inserts element at specific index in dynamic_array
        // Shifts all elements at and to the right of that index 1 to the right
        // If the dynamic_array does not have enough space, then a reallocation will
        // occur
        //
        // Throws index out of bounds error if index is greater than the
        // dynamic_array._size or if index is negative
        //
        // Returns the element just inserted
        T *insert(s32 index, T element)
        {
            // Fail if the index is greater than the dynamic_array._size or negative
            assert(index <= _size && index >= 0 && "Index out of bounds!");

            // Increase the len amount
            len++;

            reallocate();

            if (len > 1)
            {
                // Shift all elements right
                for (s32 i = len - 2; i >= index; i--)
                {
                    _array[i + 1] = _array[i];
                }
            }

            // Assign last element of array to new element
            _array[index] = element;

            return &_array[index];
        }

        // Delete element at index in dynamic_array
        // Shifts all elements to the right of that index 1 to the left
        // If the dynamic_array len equals dynamic_array _size, then it will assume
        // nothing is reserved and a reallocation will occur unless reallocate is
        // explicitly false
        void remove(size_t index)
        {
            // Fail if the index is greater than the dynamic_array._size or negative
            assert(index <= len && index >= 0 && "Index out of bounds!");

            // Shift elements to the left
            for (uint i = index + 1; i < _size; i++)
            {
                _array[i - 1] = _array[i];
            }

            // Decrease len even if no reallocation
            len--;

            reallocate();
        }

        // Shorthand of removing last element of the array
        void remove_last()
        {
            assert(len > 0 && "Array is empty!");
            remove(len - 1);
        }

        bool empty() const { return len == 0; }

        void clear()
        {
            len   = 0;
            _size = reserved;
            if (reserved > 0)
            {
                if (_array == nullptr)
                {
                    // _array = (T *)malloc(_size * sizeof(T));
                }
                else
                {
                    // _array = (T *)realloc(_array, _size * sizeof(T));
                }
            }
        }

        // static DynamicArray<T> copy()
        // {
        // }

        // Ability to index into the array and assign and retreive elements by
        // reference
        T &operator[](int index) const
        {
            assert(index < len && index >= 0 && "Index out of bounds");
            return _array[index];
        }

        // Spaces len (not bytes)
        size_t len = 0;

        // Space available (not bytes)
        size_t _size = reserved;

        // To make sure that the buffer expansion is geometric
        f64 growth_factor = (f64)growth_numerator / (f64)growth_denominator;

      private:
        void reallocate()
        {
            // Only reallocate and expand the array if we need to
            if (len > _size)
            {
                _size = len * growth_factor;
            }
            else if ((f64)len / (f64)_size < (f64)decay_percent_threshold / 100.0)
            {
                _size = len * growth_factor;
            }
            else
            {
                return;
            }

            if (_size < reserved)
            {
                _size = reserved;
            }

            if (_size == 0)
            {
                if (_array != nullptr)
                {
                    // free(_array);
                    _array = nullptr;
                }
            }
            else
            {
                if (_array == nullptr)
                {
                    _array = malloc(_size * sizeof(T));
                }
                else
                {
                    _array = realloc(_array, _size * sizeof(T));
                }
            }
        }

        // The internal array
        T *_array;

        // Dumbass C++ shit
      public:
        struct Iterator
        {
            typedef std::random_access_iterator_tag IteratorCategory;
            typedef T ValueType;
            typedef ValueType *Pointer;
            typedef ValueType &Reference;

            Iterator(Pointer p_ptr) : ptr(p_ptr){};

            Iterator &operator++()
            {
                ptr++;
                return *this;
            }

            Iterator operator++(int)
            {
                Iterator iterator = *this;
                ++(*this);
                return iterator;
            }

            Iterator &operator--()
            {
                ptr--;
                return *this;
            }

            Iterator operator--(int)
            {
                Iterator iterator = *this;
                --(*this);
                return iterator;
            }

            Reference operator[](int index) { return *(ptr + index); }

            Pointer operator->() { return ptr; }

            Reference operator*() { return *ptr; }

            bool operator==(const Iterator &other) const { return ptr == other.ptr; }

            bool operator!=(const Iterator &other) const { return !(*this == other); }

          private:
            Pointer ptr;
        };

        Iterator begin() const { return Iterator(_array); }

        Iterator end() const { return Iterator(_array + len); }
    };

} // namespace vtl
