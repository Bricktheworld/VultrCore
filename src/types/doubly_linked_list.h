// TODO(Brandon): Replace with custom allocator.
#pragma once
#include <memory>
#include "types.h"

namespace vtl
{
	template <typename T>
	struct ListNode
	{
		T value;

	  private:
		ListNode<T> *next = nullptr;
		ListNode<T> *prev = nullptr;
	};

	template <typename T>
	struct DoublyLinkedList
	{
		ListNode<T> *head;

		struct Iterator
		{
			typedef std::bidirectional_iterator_tag IteratorCategory;
			typedef T ValueType;
			typedef ListNode<T> *Pointer;
			typedef ValueType &Reference;
			Iterator(Pointer p_ptr) : ptr(p_ptr) {}

			Iterator &operator++()
			{
				assert(ptr != nullptr && "Out of bounds!");
				ptr = ptr->next;
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
				assert(ptr != nullptr && "Out of bounds!");
				ptr = ptr->prev;
				return *this;
			}

			Iterator operator--(int)
			{
				Iterator iterator = *this;
				--(*this);
				return iterator;
			}

			Pointer operator->() { return ptr; }

			Reference operator*() { return ptr->value; }

			bool operator==(const Iterator &other) const { return ptr == other.ptr; }

			bool operator!=(const Iterator &other) const { return !(*this == other); }

		  private:
			Pointer ptr;
		};

		Iterator begin() { return Iterator(head); }

		Iterator end()
		{
			// if (head == nullptr)
			//     return Iterator(nullptr);
			// auto *iterator = head;
			// while (iterator->next != nullptr)
			// {
			//     iterator = iterator->next;
			// }
			return Iterator(nullptr);
		}

		T &operator[](int index)
		{
			assert(index >= 0 && "Index out of bounds!");
			assert(head != nullptr && "List is empty!");
			auto *iterator = head;
			for (int i = 0; i < index; i++)
			{
				if (iterator->next == nullptr)
					throw("Index out of bounds!");
				iterator = iterator->next;
			}
			return iterator->value;
		}

	  private:
		uint count = 0;
	};

	template <typename T>
	DoublyLinkedList<T> new_doubly_linked_list()
	{
		return {.head = nullptr};
	}

	template <typename T>
	ListNode<T> *doubly_linked_list_push_back(DoublyLinkedList<T> &l, T value)
	{
		ListNode<T> *n = nullptr; //(ListNode<T> *)malloc(sizeof(ListNode<T>));
		n->value       = value;
		n->next        = nullptr;
		if (l.head == nullptr)
		{
			l.head  = n;
			n->prev = nullptr;
		}
		else
		{
			ListNode<T> *iterator = l.head;
			while (iterator->next != nullptr)
			{
				iterator = iterator->next;
			}
			iterator->next = n;
			n->prev        = iterator;
		}
		return n;
	}

	template <typename T>
	ListNode<T> *doubly_linked_list_insert(DoublyLinkedList<T> &l, T value, int index)
	{
		assert(index >= 0 && "Index out of bounds!");
		ListNode<T> *n = nullptr; // (ListNode<T> *)malloc(sizeof(ListNode<T>));
		n->value       = value;

		if (index > 0)
		{
			ListNode<T> *iterator = l.head;
			for (int i = 0; i < index - 1; i++)
			{
				if (iterator->next == nullptr)
				{
					throw("Index out of bounds!");
				}
				iterator = iterator->next;
			}
			auto *prev = iterator;
			auto *next = iterator->next;
			if (next != nullptr)
			{
				next->prev = n;
				n->next    = next;
			}
			prev->next = n;
			n->prev    = prev;
		}
		else
		{
			l.head->prev = n;
			n->next      = l.head;
			l.head       = n;
		}
		return n;
	}

	template <typename T>
	T doubly_linked_list_delete(DoublyLinkedList<T> &l, int index)
	{
		ListNode<T> *iterator = l.head;
		for (int i = 0; i < index; i++)
		{
			if (iterator->next == nullptr)
				throw("Index out of bounds!");
			iterator = iterator->next;
		}
		auto *deleted = iterator;
		auto *next    = iterator->next;
		auto *prev    = iterator->prev;
		if (next != nullptr)
		{
			next->prev = prev;
		}
		if (prev != nullptr)
		{
			prev->next = next;
		}
		else
		{
			l.head = next;
		}
		T temp = deleted->value;
		// free(deleted);
		return temp;
	}

	template <typename T>
	bool doubly_linked_list_exists(DoublyLinkedList<T> &l, T value)
	{
		for (T element : l)
		{
			if (generic_equals(element, value))
				return true;
		}
		return false;
	}

	template <typename T>
	int doubly_linked_list_find(DoublyLinkedList<T> &l, T value)
	{
		uint index = 0;
		for (T element : l)
		{
			if (generic_equals(element, value))
				return index;
			index++;
		}
		return -1;
	}

	template <typename T>
	T doubly_linked_list_delete(DoublyLinkedList<T> &l, T value)
	{
		int index = doubly_linked_list_find(l, value);
		assert(index >= 0 && "Cannot find value");
		return doubly_linked_list_delete(l, index);
	}

	template <typename T>
	T &doubly_linked_list_get(const DoublyLinkedList<T> &l, int index)
	{
		return l[index];
	}

	template <typename T>
	uint doubly_linked_list_length(DoublyLinkedList<T> &l)
	{
		uint count = 0;
		for (T element : l)
		{
			count++;
		}
		return count;
	}

} // namespace vtl
