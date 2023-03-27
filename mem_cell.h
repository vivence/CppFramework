
#ifndef MEM_CELL_H
#define MEM_CELL_H

#include "core.h"

#define COMPACT_CELL 1

CORE_NAMESPACE_BEG

#pragma pack(push, 1)
struct mem_cell {
	typedef uint8_t head_type;
	/// <summary>
	/// ���λ1λλ0���ʾused��Ϊ1���ʾunused��ʣ��ĵ�λ���û��������ʹ��
	/// </summary>
	head_type head = 0;

#if COMPACT_CELL
	union
	{
#endif // COMPACT_CELL

		/// <summary>
		/// �����Ƶ�ʡ���������������С��sizeof(mem_cell)���ڴ棬���Կ��ǽ�p_next_cell�Ż�Ϊһ�ֽڵ�block������������΢������������Ŀ�����
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
