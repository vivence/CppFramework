
#ifndef MEM_CELL_H
#define MEM_CELL_H

#include "core.h"

#define COMPACT_CELL 1

CORE_NAMESPACE_BEG

#pragma pack(push, 1)
struct mem_cell {
	typedef uint8_t head_type;
	head_type head = 0;

#if COMPACT_CELL
	union
	{
#endif // COMPACT_CELL

		/// <summary>
		/// 如果高频率、高数量分配明显小于sizeof(mem_cell)的内存，可以考虑将p_next_cell优化为一字节的block内索引（会略微增加链表操作的开销）
		/// </summary>
		mem_cell* p_next_cell = nullptr;
#pragma warning(disable : 4200)
		uint8_t user_mem[0];

#if COMPACT_CELL
	};
#endif // COMPACT_CELL

//private:
//	enum { _HalfBytesCountOfHeadType = (sizeof(head_type) * 8 / 2) };
//	static const head_type _BlockIndexMask = (head_type)~0 >> _HalfBytesCountOfHeadType;
//	static const head_type _PoolIndexMask = ~_BlockIndexMask;

public:
	enum {
		//PoolCount = _BlockIndexMask + 1,
		//BlockCount = _BlockIndexMask + 1,
		PoolCount = (head_type)~0 +1,
#if COMPACT_CELL
		UserMemOffset = sizeof(head_type) 
#else
		UserMemOffset = sizeof(head_type) + sizeof(mem_cell*)
#endif // COMPACT_CELL
	};

	void set_pool_index(size_t i)
	{
		// assert(PoolCount > i)
		//head = (head & ~_PoolIndexMask) | ((head_type)i << _HalfBytesCountOfHeadType);
		head = (head_type)i;
	}
	size_t get_pool_index()
	{
		//return head >> _HalfBytesCountOfHeadType;
		return head;
	}

	//void set_block_index(size_t i)
	//{
	//	// assert(BlockCount > i)
	//	head = (head & ~_BlockIndexMask) | ((head_type)i & _BlockIndexMask);
	//}
	//size_t get_block_index()
	//{
	//	return head & _BlockIndexMask;
	//}

	static mem_cell& get_cell(void* user_mem)
	{
		return *(mem_cell*)((intptr_t)user_mem - UserMemOffset);
	}
};
#pragma pack(pop)

CORE_NAMESPACE_END

#endif
