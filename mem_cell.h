
#ifndef MEM_CELL_H
#define MEM_CELL_H

#include "core.h"
#include <utility>

#define USER_MEM_ALIGN 1
#define COMPACT_CELL 1

CORE_NAMESPACE_BEG

#pragma pack(push, 1)
struct mem_cell {
	typedef uint8_t head_type; // sizeof(head_type) can't bigger than sizeof(size_t)
#if USER_MEM_ALIGN
	typedef size_t align_type;
#else
	typedef head_type align_type;
#endif // USER_MEM_ALIGN
	union
	{
		/// <summary>
		/// 最高位1位位0则表示used，为1则表示unused，剩余的低位由用户决定如何使用
		/// </summary>
		head_type head = 0;
		align_type __align;
	};

#if COMPACT_CELL
	union
	{
#endif // COMPACT_CELL

		/// <summary>
		/// 这里没必要再优化p_next_cell的大小了，如果分配的对象尺寸都小于一个指针了，那么应该考虑值类型
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
		UserMemOffset = sizeof(__align)
#else
		UserMemOffset = sizeof(__align) + sizeof(mem_cell*)
#endif // COMPACT_CELL
	};

	inline void mark_unused()
	{
		head = _UnuseMark;
	}
	inline bool is_unused() const
	{
		return _UnuseMark == head;
	}
	inline void mark_used()
	{
		head = _UsedMark;
	}
	inline bool is_used() const
	{
		return !is_unused();
	}

	inline static mem_cell& get_cell(void* user_mem)
	{
		return *(mem_cell*)((intptr_t)user_mem - UserMemOffset);
	}
};
#pragma pack(pop)

CORE_NAMESPACE_END

#endif
