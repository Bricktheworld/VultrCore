#pragma once

namespace Vultr
{
}

// #pragma once
// #include <types/types.h>
// #include <equals.h>
// #include <stdio.h>
// #include <string.h>
// #include <assert.h>
// #include <iterator>
// #include <cstddef>

// namespace vtl
// {
// #define TABLE_SIZE 10

//     template <typename k, typename v>
//     struct HashNode
//     {
//         k key;
//         v value;
//     };

//     namespace internal
//     {

//         template <typename k, typename v>
//         HashNode<k, v> *init_node(const HashNode<k, v> &other)
//         {
//             auto *n = (HashNode<k, v> *)malloc(sizeof(HashNode<k, v>));
//             n->key = other.key;
//             n->value = other.value;
//             return n;
//         }
//     } // namespace internal

//     // For providing custom hash functions
//     typedef uint (*hash_function)(void *);

//     // Default hash functions with explicit instantiation
//     template <typename k>
//     uint hash(k key);

//     template <typename k, typename v>
//     struct HashTable
//     {
//         HashTable()
//         {
//             for (int i = 0; i < TABLE_SIZE; i++)
//             {
//                 internal_array[i] = nullptr;
//             }
//         }
//         ~HashTable()
//         {
//             assert(hashtable_exists(t, key) && "Cannot delete nonexistent key-value pair inhashtable!");
//             int index = hash(key);
//             auto *node = t.internal_array[index];
//             free(node);
//             t.internal_array[index] = nullptr;
//         }

//         v &operator[](k key)
//         {
//             int index = hash(key);
//             auto *node = internal_array[index];
//             if (node != nullptr)
//             {
//                 if (!generic_equals(node->key, key))
//                 {
//                     assert(true && "Collision!");
//                 }
//             }
//             else
//             {
//                 internal_array[index] = (HashNode<k, v> *)malloc(sizeof(HashNode<k, v>));
//                 internal_array[index]->key = key;
//             }
//             return internal_array[index]->value;
//         }

//         struct Iterator
//         {
//             typedef std::random_access_iterator_tag IteratorCategory;
//             typedef HashNode<k, v> ValueType;
//             typedef ValueType **Pointer;
//             typedef ValueType &Reference;
//             Iterator(Pointer p_ptr) : ptr(p_ptr)
//             {
//             }

//             Iterator &operator++()
//             {
//                 // OMG THE FIRST TIME I'VE LITERALLY EVER USED DO WHILE
//                 do
//                 {
//                     ptr++;
//                 } while (*ptr == nullptr);
//                 return *this;
//             }

//             Iterator operator++(int)
//             {
//                 Iterator iterator = *this;
//                 ++(*this);
//                 return iterator;
//             }

//             Iterator &operator--()
//             {
//                 do
//                 {
//                     ptr--;
//                 } while (*ptr == nullptr);
//                 return *this;
//             }

//             Iterator operator--(int)
//             {
//                 Iterator iterator = *this;
//                 --(*this);
//                 return iterator;
//             }

//             Pointer *operator->()
//             {
//                 while (*ptr == nullptr)
//                 {
//                     (*this)++;
//                 }
//                 return *ptr;
//             }

//             Reference operator*()
//             {
//                 while (*ptr == nullptr)
//                 {
//                     (*this)++;
//                 }
//                 return **ptr;
//             }

//             bool operator==(const Iterator &other) const
//             {
//                 return ptr == other.ptr;
//             }

//             bool operator!=(const Iterator &other) const
//             {
//                 return !(*this == other);
//             }

//           private:
//             Pointer ptr;
//         };

//         Iterator begin()
//         {
//             HashNode<k, v> **ptr = internal_array;
//             while (ptr < internal_array + TABLE_SIZE && *ptr == nullptr)
//             {
//                 ptr++;
//             }
//             return Iterator(ptr);
//         }
//         Iterator end()
//         {
//             return Iterator(internal_array + TABLE_SIZE);
//         }

//       private:
//         HashNode<k, v> *internal_array[TABLE_SIZE];
//     };

//     template <typename k, typename v>
//     HashTable<k, v> new_hashtable()
//     {
//         HashTable<k, v> t = {};
//         return t;
//     }

//     template <typename k, typename v>
//     bool hashtable_insert(HashTable<k, v> &t, HashNode<k, v> node)
//     {
//         int index = hash(node.key);
//         if (t.internal_array[index] != nullptr)
//         {
//             return false;
//         }
//         auto *new_node = internal::init_node(node);

//         t.internal_array[index] = new_node;
//         return true;
//     }

//     template <typename k, typename v>
//     bool hashtable_exists(const HashTable<k, v> &t, k key)
//     {
//         int index = hash(key);
//         if (t.internal_array[index] != nullptr)
//         {
//             return generic_equals(t.internal_array[index], key);
//         }
//         else
//         {
//             return false;
//         }
//     }

//     //
//     //
//     //
//     // Default hash functions
//     //
//     //
//     //
//     template <>
//     inline u32 hash<const char *>(const char *key)
//     {
//         int length = strlen(key);
//         u32 hash_value = 0;
//         for (int i = 0; i < length; i++)
//         {
//             hash_value += key[i];
//             hash_value = (key[i] * hash_value) % TABLE_SIZE;
//         }
//         return hash_value;
//     }

// } // namespace vtl
