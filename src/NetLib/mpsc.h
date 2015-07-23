#pragma once

class NodeEntry
{
public:
	NodeEntry() : mNext(nullptr) {}
	NodeEntry* volatile mNext;
};

//////////////////////////////////////////////////////////////////////////

class QueueNode
{
public:
	QueueNode()
		: _node_entry()
	{

	}

	virtual	~QueueNode()
	{

	}

public:
	NodeEntry	_node_entry; ///< 반드시 NodeEntry를 포함해야 함
};

//////////////////////////////////////////////////////////////////////////

template <class T>
class QueueMpsc
{
public:
	QueueMpsc() : mHead(&mStub), mTail(&mStub)
	{
		mOffset = reinterpret_cast<__int64>(&((reinterpret_cast<T*>(0))->_node_entry));
	}
	~QueueMpsc() {}

	void Push(T* newData)
	{
		newData->_node_entry.mNext = nullptr;

#ifdef __ATLCONV_H__
		NodeEntry* prevNode = (NodeEntry*)InterlockedExchangePointer((void**)&mHead, (void*)&(newData->_node_entry));
#else
		NodeEntry* prevNode = (NodeEntry*)InterlockedExchangePointer((void*volatile*)&mHead, (void*)&(newData->_node_entry));
#endif
		prevNode->mNext = &(newData->_node_entry);
	}

	T* Pop()
	{
		NodeEntry* tail = mTail;
		NodeEntry* next = tail->mNext;

		if (tail == &mStub)
		{
			/// 데이터가 없을 때
			if (nullptr == next)
				return nullptr;

			/// 처음 꺼낼 때
			mTail = next;
			tail = next;
			next = next->mNext;
		}

		/// 대부분의 경우에 데이터를 빼낼 때
		if (next)
		{
			mTail = next;

			return reinterpret_cast<T*>(reinterpret_cast<__int64>(tail)-mOffset);
		}

		NodeEntry* head = mHead;
		if (tail != head)
			return nullptr;

		/// 마지막 데이터 꺼낼 때
		mStub.mNext = nullptr;
#ifdef __ATLCONV_H__
		NodeEntry* prev = (NodeEntry*)InterlockedExchangePointer((void**)&mHead, (void*)&mStub);
#else
		NodeEntry* prev = (NodeEntry*)InterlockedExchangePointer((void*volatile*)&mHead, (void*)&mStub);
#endif
		prev->mNext = &mStub;

		next = tail->mNext;
		if (next)
		{
			mTail = next;

			return reinterpret_cast<T*>(reinterpret_cast<__int64>(tail)-mOffset);
		}

		return nullptr;
	}


private:
#ifdef __ATLCONV_H__
	NodeEntry*  	    mHead;
#else
	NodeEntry* volatile	mHead;
#endif
	NodeEntry*			mTail;
	NodeEntry			mStub;

	__int64				mOffset;
};



using namespace std;

/////////////////////////
// Array based lock free
// queue 
/////////////////////////
template<class T>
class ArrayQ
{
private:
	T* pData;
	volatile LONG nWrite;
	volatile LONG nRead;
	volatile LONG nSize;
	// size of array at creation
	enum SizeEnum{ InitialSize = 240 };
	// Lock pData to copy
	void Resize()
	{
		// Declare temporary size variable
		LONG nNewSize = 0;
		CRITICAL_SECTION cs;

		// double the size of our queue
		InterlockedExchangeAdd(&nNewSize, 2 * nSize);

		// allocate the new array
		T* pNewData = new T[nNewSize];
		const ULONG uiTSize = sizeof(T);

		// Initialize the critical section to protect the copy
		InitializeCriticalSection(&cs);

		// Enter the critical section
		EnterCriticalSection(&cs);

		// copy the old data
		memcpy_s((void*)pNewData, nNewSize*uiTSize, (void*)pData, nSize*uiTSize);

		// dump the old array
		delete[] pData;

		// save the new array
		pData = pNewData;

		// save the new size
		nSize = nNewSize;

		// Leave the critical section
		LeaveCriticalSection(&cs);

		// Delete the critical section
		DeleteCriticalSection(&cs);
	}
public:
	ArrayQ() : nWrite(0), nRead(0), pData(new T[InitialSize]), nSize(InitialSize)
	{

	}

	~ArrayQ()
	{
		delete[] pData;
	}


	void enqueue(const T& t)
	{
		// temporary write index and size
		volatile LONG nTempWrite, nTempSize;

		// atomic copy of the originals to temporary storage
		InterlockedExchange(&nTempWrite, nWrite);
		InterlockedExchange(&nTempSize, nSize);

		// increment before bad things happen
		InterlockedIncrement(&nWrite);

		// check to make sure we haven't exceeded our storage 
		if (nTempWrite == nTempSize)
		{
			// we should resize the array even if it means using a lock
			Resize();
		}

		pData[nTempWrite] = t;
	}

	// returns false if queue is empty
	bool dequeue(T& t)
	{
		// temporary write index and size
		volatile LONG nTempWrite, nTempRead;

		// atomic copy of the originals to temporary storage
		InterlockedExchange(&nTempWrite, nWrite);
		InterlockedExchange(&nTempRead, nRead);

		// increment before bad things happen
		InterlockedIncrement(&nRead);

		// check to see if queue is empty
		if (nTempRead == nTempWrite)
		{
			// reset both indices
			InterlockedCompareExchange(&nRead, 0, nTempRead + 1);
			InterlockedCompareExchange(&nWrite, 0, nTempWrite);
			return false;
		}

		t = pData[nTempRead];
		return true;
	}

};


//////////////////////////////
// queue based on work of 
// Maged M. Michael &
// Michael L. Scott
//////////////////////////////

template< class T >
class MSQueue
{
private:

	// pointer structure
	struct node_t;

	struct pointer_t
	{
		node_t* ptr;
		LONG count;
		// default to a null pointer with a count of zero
		pointer_t() : ptr(NULL), count(0){}
		pointer_t(node_t* node, const LONG c) : ptr(node), count(c){}
		pointer_t(const pointer_t& p)
		{
			InterlockedExchange(&count, p.count);
			InterlockedExchangePointer((void**)&ptr, p.ptr);
		}

		pointer_t(const pointer_t* p) : ptr(NULL), count(0)
		{
			if (NULL == p)
				return;

			InterlockedExchange(&count, const_cast< LONG >(p->count));
			InterlockedExchangePointer((void**)&ptr, const_cast< node_t* >(p->ptr));
		}

	};

	// node structure
	struct node_t
	{
		T value;
		pointer_t next;
		// default constructor
		node_t(){}
	};

	pointer_t Head;
	pointer_t Tail;
	bool CAS(pointer_t& dest, pointer_t& compare, pointer_t& value)
	{
		if (compare.ptr == InterlockedCompareExchangePointer((PVOID volatile*)&dest.ptr, value.ptr, compare.ptr))
		{
			InterlockedExchange(&dest.count, value.count);
			return true;
		}

		return false;
	}
public:
	// default constructor
	MSQueue()
	{
		node_t* pNode = new node_t();
		Head.ptr = Tail.ptr = pNode;

		casFailCount = 0;

	}
	~MSQueue()
	{
		// remove the dummy head
		delete Head.ptr;
	}

	int casFailCount;



	// insert items of class T in the back of the queue
	// items of class T must implement a default and copy constructor
	// Enqueue method
	void enqueue(const T& t)
	{
		// Allocate a new node from the free list
		node_t* pNode = new node_t();

		// Copy enqueued value into node
		pNode->value = t;

		// Keep trying until Enqueue is done
		bool bEnqueueNotDone = true;

		while (bEnqueueNotDone)
		{
			// Read Tail.ptr and Tail.count together
			pointer_t tail(Tail);

			bool nNullTail = (NULL == tail.ptr);
			// Read next ptr and count fields together
			pointer_t next( // ptr 
				(nNullTail) ? NULL : tail.ptr->next.ptr,
				// count
				(nNullTail) ? 0 : tail.ptr->next.count
				);


			// Are tail and next consistent?
			if (tail.count == Tail.count && tail.ptr == Tail.ptr)
			{
				if (NULL == next.ptr) // Was Tail pointing to the last node?
				{
					// Try to link node at the end of the linked list										
					if (CAS(tail.ptr->next, next, pointer_t(pNode, next.count + 1)))
					{
						bEnqueueNotDone = false;
					} // endif


				} // endif

				else // Tail was not pointing to the last node
				{
					// Try to swing Tail to the next node
					CAS(Tail, tail, pointer_t(next.ptr, tail.count + 1));

				}

			} // endif

			if (bEnqueueNotDone == true)
				++casFailCount;

		} // endloop
	}

	// remove items of class T from the front of the queue
	// items of class T must implement a default and copy constructor
	// Dequeue method
	bool dequeue(T& t)
	{
		pointer_t head;
		// Keep trying until Dequeue is done
		bool bDequeNotDone = true;
		while (bDequeNotDone)
		{
			// Read Head
			head = Head;
			// Read Tail
			pointer_t tail(Tail);

			if (head.ptr == NULL)
			{
				// queue is empty
				return false;
			}

			// Read Head.ptr->next
			pointer_t next(head.ptr->next);

			// Are head, tail, and next consistent
			if (head.count == Head.count && head.ptr == Head.ptr)
			{
				if (head.ptr == tail.ptr) // is tail falling behind?
				{
					// Is the Queue empty
					if (NULL == next.ptr)
					{
						// queue is empty cannot deque
						return false;
					}
					CAS(Tail, tail, pointer_t(next.ptr, tail.count + 1)); // Tail is falling behind. Try to advance it
				} // endif

				else // no need to deal with tail
				{
					// read value before CAS otherwise another deque might try to free the next node
					t = next.ptr->value;

					// try to swing Head to the next node
					if (CAS(Head, head, pointer_t(next.ptr, head.count + 1)))
					{
						bDequeNotDone = false;
					}
				}

			} // endif

		} // endloop

		// It is now safe to free the old dummy node
		delete head.ptr;

		// queue was not empty, deque succeeded
		return true;
	}
};


//#define _INTRUSIVE_QUEUE_
#ifdef _INTRUSIVE_QUEUE_
//-------------------------------------------------------------
//	Non-intrusive version:
//-------------------------------------------------------------
struct mpscq_node_t
{
	mpscq_node_t* volatile  next;
	void*                   state;

};

struct mpscq_t
{
	mpscq_node_t* volatile  head;
	mpscq_node_t*           tail;

};

void mpscq_create(mpscq_t* self, mpscq_node_t* stub)
{
	stub->next = 0;
	self->head = stub;
	self->tail = stub;

}

void mpscq_push(mpscq_t* self, mpscq_node_t* n)
{
	n->next = 0;
	
	//asm();

	mpscq_node_t* prev = (mpscq_node_t*)InterlockedExchangePointer((void**)&self->head, n); // serialization-point wrt producers
	prev->next = n; // serialization-point wrt consumer

}

mpscq_node_t* mpscq_pop(mpscq_t* self)
{
	mpscq_node_t* tail = self->tail;
	mpscq_node_t* next = tail->next; // serialization-point wrt producers
	if (next)
	{
		self->tail = next;
		tail->state = next->state;
		return tail;
	}
	return 0;

}

#else
//-------------------------------------------------------------
//	And here is intrusive version:
//-------------------------------------------------------------
struct mpscq_node_t
{
	mpscq_node_t* volatile  next;

};

struct mpscq_t
{
	mpscq_node_t* volatile  head;
	mpscq_node_t*           tail;
	mpscq_node_t            stub;

};

#define MPSCQ_STATIC_INIT(self) {&self.stub, &self.stub, {0}}

void mpscq_create(mpscq_t* self)
{
	self->head = &self->stub;
	self->tail = &self->stub;
	self->stub.next = 0;

}

void mpscq_push(mpscq_t* self, mpscq_node_t* n)
{
	n->next = 0;
	mpscq_node_t* prev = (mpscq_node_t*)InterlockedExchangePointer((void**)&self->head, n);
	//(*)
	prev->next = n;

}

mpscq_node_t* mpscq_pop(mpscq_t* self)
{
	mpscq_node_t* tail = self->tail;
	mpscq_node_t* next = tail->next;
	if (tail == &self->stub)
	{
		if (0 == next)
			return 0;
		self->tail = next;
		tail = next;
		next = next->next;
	}
	if (next)
	{
		self->tail = next;
		return tail;
	}
	mpscq_node_t* head = self->head;
	if (tail != head)
		return 0;
	mpscq_push(self, &self->stub);
	next = tail->next;
	if (next)
	{
		self->tail = next;
		return tail;
	}
	return 0;
}

#endif

