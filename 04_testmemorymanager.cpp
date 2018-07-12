#include <iostream>
#include <functional>
#include <stdio.h>
#include <memory.h>

#define MEM_ALIGN(size, align) (size & ~(align - 1))
#define COUNT(total, one) (total/one)

class MemoryManager
{
	struct AllocInfo
	{
		size_t magic = 0xBABE7777;
		void * mem = 0;
		size_t size = 0;
	};
public:
	void init(void *mem, size_t size)
	{
		// 10% of memory is used for the alloc info array
		size_t alloc_info_array_size_in_bytes = MEM_ALIGN(size / 10, sizeof(struct AllocInfo));
		m_allocs_count = 0;
		m_max_allocs = COUNT(alloc_info_array_size_in_bytes, sizeof(struct AllocInfo));
		m_memory = mem;
		m_freesize = m_totalsize = size - alloc_info_array_size_in_bytes;
		m_alloc_info_array = (struct AllocInfo*) (((char*)mem) + m_freesize);
	}

	void* alloc(size_t size)
	{
		//size = MEM_ALIGN(size, 4);

		// not finished : need to be more efficient
		return _addElementInAllocInfoArray(size);
	}

	void free(void * ptr)
	{
		// not finished : need to be more efficient
		_removeElementInAllocInfoArray(ptr);
	}

	size_t freesize()
	{
		return m_freesize;
	}

	size_t usedmem()
	{
		return m_totalsize - m_freesize;
	}

	size_t alloccount()
	{
		return m_allocs_count;
	}

	void dump()
	{
		std::cout << "--- Memory Manager Info ---" << std::endl;
		std::cout << "used mem : " << usedmem() <<std::endl;
		std::cout << "alloc count : " << alloccount() <<std::endl;
		if (m_allocs_count == 0)
		{
			return;
		}

		std::cout << "--- Memory Alloc Info ---" << std::endl;
		struct AllocInfo* current = m_alloc_info_array;
		for(size_t i = 0; i < m_allocs_count; i++, current++)
		{
			std::cout
			<< "addr=" << current->mem
			<< ", offset="<< int64_t(current->mem) - int64_t(m_memory)
			<< ", size=" << current->size
			<< std::endl;
		}
	}

private:
	void* _searchFreeMemChunck(size_t size)
	{
		if (size > m_freesize)
		{
			return nullptr;
		}

		if (m_allocs_count == 0)
		{
			return m_memory;
		}

		struct AllocInfo* last = m_alloc_info_array + m_allocs_count - 1;
		if (((char*)last->mem) + last->size + size > ((char*)m_memory) + m_totalsize)
		{
			return nullptr;
		}

		return ((char*)last->mem) + last->size;
	}

	void* _addElementInAllocInfoArray(size_t size)
	{
		if (m_allocs_count == m_max_allocs)
		{
			return nullptr;
		}
		void* mem = _searchFreeMemChunck(size);
		if (mem)
		{
			struct AllocInfo* current = m_alloc_info_array + m_allocs_count;
			current->mem = mem;
			current->size = size;
			m_freesize -= size;
			m_allocs_count++;
		}
		return mem;
	}

	struct AllocInfo* _findAllocInfo(void * mem)
	{
		struct AllocInfo* current = m_alloc_info_array;
		for (size_t idx = 0; idx < m_allocs_count; idx++, current++)
		{
			if (current->mem == mem)
			{
				return current;
			}
		}

		return nullptr;
	}

	void _removeElementInAllocInfoArray(void * mem)
	{
		if (m_allocs_count == 0)
		{
			return;
		}

		struct AllocInfo* current = _findAllocInfo(mem);
		if (current == nullptr)
		{
			return;
		}
		m_freesize += current->size;
		struct AllocInfo* last = m_alloc_info_array + m_allocs_count - 1;
		if (current != last)
		{
			size_t idx = current - m_alloc_info_array;
			size_t n = m_allocs_count - 1 - idx;
			memcpy(current, current + 1,  n * sizeof(struct AllocInfo));
		}
		m_allocs_count--;
	}

private:
	void* m_memory = nullptr;
	size_t m_freesize = 0;
	size_t m_totalsize = 0;

	struct AllocInfo* m_alloc_info_array = nullptr;
	size_t m_allocs_count = 0;
	size_t m_max_allocs = 0;
};

static MemoryManager defaultMemmgr;
static MemoryManager* memmgr = &defaultMemmgr;

void* operator new(size_t size)
{
	return memmgr->alloc(size);
}

void* operator new[](size_t size)
{
	return memmgr->alloc(size);
}

void operator delete(void* ptr)
{
	memmgr->free(ptr);
}

void operator delete[](void* ptr)
{
	memmgr->free(ptr);
}

int main(int argc, char ** argv)
{
	const size_t size_max = 8 * 1024 *1024;
	static char mem[size_max];

	memmgr->init(mem, size_max);

	std::cout << "--- init ---" << std::endl;
	memmgr->dump();

	std::cout << "--- alloc : 2 ints + 1 int ---" << std::endl;
	int * i = new int[2];
	int * j = new int;
	memmgr->dump();

	std::cout << "--- free : 2 ints + 1 int ---" << std::endl;
	delete [] i;
	delete j;
	memmgr->dump();

	return 0;
}

/* output :
--- init ---
--- Memory Manager Info ---
used mem : 0
alloc count : 0
--- alloc : 2 ints + 1 int ---
--- Memory Manager Info ---
used mem : 12
alloc count : 2
--- Memory Alloc Info ---
addr=0x602220, offset=0, size=8
addr=0x602228, offset=8, size=4
--- free : 2 ints + 1 int ---
--- Memory Manager Info ---
used mem : 0
alloc count : 0
*/
