
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

		mem_cell* p_next_cell = nullptr;
#pragma warning(disable : 4200)
		uint8_t user_mem[0];

#if COMPACT_CELL
	};
#endif // COMPACT_CELL

#if COMPACT_CELL
	enum { USER_MEM_OFFSET = sizeof(head_type) };
#else
	enum { USER_MEM_OFFSET = sizeof(head_type) + sizeof(mem_cell*) };
#endif // COMPACT_CELL

	static mem_cell& get_cell(void* user_mem)
	{
		return *(mem_cell*)((intptr_t)user_mem - USER_MEM_OFFSET);
	}
};
#pragma pack(pop)

CORE_NAMESPACE_END

#endif
