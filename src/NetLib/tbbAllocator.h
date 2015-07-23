#pragma once

#include "tbb/scalable_allocator.h"

class tbbAllocator
{
public:
	inline static void* operator new (size_t size)
	{
		return scalable_malloc(size);
	}


	inline static void* operator new[](size_t size)
	{
		return scalable_malloc(size);
	}


	inline static void operator delete (void* ptr, size_t size)
	{
		scalable_free(ptr);
	}


	inline  static void operator delete[](void* ptr, size_t size)
	{
		scalable_free(ptr);
	}
	
public:
	tbbAllocator() {}
	virtual ~tbbAllocator() {}
};

