
#ifndef MEM_CELL_H
#define MEM_CELL_H

#include "core.h"

#define COMPACT_CELL 1

CORE_NAMESPACE_BEG

#pragma pack(push, 1)
struct mem_cell {
	typedef uint8_t head_type;
	/// <summary>
	/// 最高位1位位0则表示used，为1则表示unused，剩余的低位由用户决定如何使用
	/// </summary>
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

private:
	static const head_type _UsedMark = 0;
	static const head_type _UnuseMark = ~((head_type)~0 >> 1);

public:
	enum {
		PoolCount = ((head_type)~0 >> 1) +1,
#if COMPACT_CELL
		UserMemOffset = sizeof(head_type) 
#else
		UserMemOffset = sizeof(head_type) + sizeof(mem_cell*)
#endif // COMPACT_CELL
	};

	void mark_unused()
	{
		head = _UnuseMark;
	}
	bool is_unused() const
	{
		return _UnuseMark == head;
	}
	void mark_used()
	{
		head = _UsedMark;
	}
	bool is_used() const 
	{
		return !is_unused();
	}

	static mem_cell& get_cell(void* user_mem)
	{
		return *(mem_cell*)((intptr_t)user_mem - UserMemOffset);
	}
};
#pragma pack(pop)

CORE_NAMESPACE_END

#endif
