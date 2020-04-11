#ifndef _ALLOCTOR_H_
#define _ALLOCTOR_H_

#include"MemoryMgr.hpp"

void* operator new(size_t nSize)
{
	return MemoryMgr::Instance().allocMem(nSize);
}

void operator delete(void* p)
{
	MemoryMgr::Instance().freeMem(p);
}

void* operator new[](size_t nSize)
{
	return MemoryMgr::Instance().allocMem(nSize);
}

void operator delete[](void* p)
{
	MemoryMgr::Instance().freeMem(p);
}

void* mem_alloc(size_t size)
{
	return malloc(size);
}

void mem_free(void* p)
{
	free(p);
}
#endif // !_ALLOCTOR_H_
