#include <iostream>
#include <functional>
#include <stdio.h>
#include <memory.h>


class MemoryManager
{
	struct MemoryChunk
	{
		size_t magic = 0xBABE7777;
		void * mem = 0;
		struct MemoryChunk* next = nullptr;
	};
public:
	void init(void *mem, size_t size)
	{
		firstMemoryChunck = nullptr;
		m_memory = mem;
		m_freesize = m_totalsize = size;
	}

	void* alloc(size_t size)
	{
		void* mem = m_memory;
		m_freesize -= size;
		m_memory = ((char*)m_memory) + size;
		return mem;
	}

	void free(void * ptr)
	{
		// not used/finished
	}

private:
	struct MemoryChunk* firstMemoryChunck = nullptr;
	void * m_memory = nullptr;
	size_t m_freesize = 0;
	size_t m_totalsize = 0;
};

static MemoryManager defaultMemmgr;
static MemoryManager* memmgr = &defaultMemmgr;

void* operator new(size_t size)
{
	return memmgr->alloc(size);
}

void operator delete(void* ptr)
{
	memmgr->free(ptr);
}

int main(int argc, char ** argv)
{
	const size_t size_max = 8 * 1024 *1024;
	static char mem[size_max];
	memmgr->init(mem, size_max);
	int * i = new int;
	int * j = new int;

	std::cout << "--- mem ---" << std::endl;
	std::cout << "addr=" << (int*)(mem) << " offset=" << int64_t(mem) - int64_t(mem)<<std::endl;
	std::cout << "--- i ---" << std::endl;
	std::cout << "addr=" << i << " offset=" << int64_t(i) - int64_t(mem)<<std::endl;
	std::cout << "--- j ---" << std::endl;
	std::cout << "addr=" << j << " offset=" <<  int64_t(j) - int64_t(mem) <<std::endl;
	return 0;
}

/* output :
--- mem ---
addr=0x6021e0 offset=0
--- i ---
addr=0x6021e0 offset=0
--- j ---
addr=0x6021e4 offset=4 
*/
