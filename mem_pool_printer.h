
#ifndef MEM_POOL_PRINTER_H
#define MEM_POOL_PRINTER_H

#include "core.h"
#include "mem_pool.h"
#include "utils.h"
#include <ostream>

CORE_NAMESPACE_BEG

struct mem_pool_printer {
    static void print_global_info(std::ostream& out) { _print_global_info<mem_pool>(out); }
    template<typename _T>
    static void print_type_info(std::ostream& out) { _print_type_info<mem_pool, _T>(out); }


    static void print_large_global_info(std::ostream& out) { _print_global_info<large_mem_pool>(out); }
    template<typename _T>
    static void print_large_type_info(std::ostream& out) { _print_type_info<large_mem_pool, _T>(out); }

private:
    template<typename _M>
    static void _print_global_info(std::ostream& out);

    template<typename _M, typename _T>
    static void _print_type_info(std::ostream& out);
};

template<typename _M>
void mem_pool_printer::_print_global_info(std::ostream& out)
{
    out << std::endl;
    out << "---------mem pool info [for global]-------------->" << std::endl;
    out << " ** cell_unit_size: " << string_format_utils::format_size(_M::info_for_global::cell_unit_size) << std::endl;
    out << " ** block_max_size: " << string_format_utils::format_size(_M::info_for_global::block_max_size) << std::endl;
    out << " ** pool_max_count: " << string_format_utils::format_count(_M::info_for_global::pool_max_count) << std::endl;
    out << " ** cell_raw_size_min: " << string_format_utils::format_size(_M::info_for_global::cell_raw_size_min) << std::endl;
    out << " ** cell_head_size: " << string_format_utils::format_size(_M::info_for_global::cell_head_size) << std::endl;
    out << " ** user_mem_offset_in_cell: " << _M::info_for_global::user_mem_offset_in_cell << std::endl;
    out << " ** min_cell: " << "size = " << string_format_utils::format_size(_M::info_for_global::min_cell_size) << ", user_mem = " << string_format_utils::format_size(_M::info_for_global::min_cell_user_mem_size) << ", count = " << string_format_utils::format_count(_M::info_for_global::min_cell_count) << ", pool_index = " << _M::info_for_global::min_cell_pool_index << std::endl;
    out << " ** max_cell: " << "size = " << string_format_utils::format_size(_M::info_for_global::max_cell_size) << ", user_mem = " << string_format_utils::format_size(_M::info_for_global::max_cell_user_mem_size) << ", count = " << string_format_utils::format_count(_M::info_for_global::max_cell_count) << std::endl;
    out << "-------------------------------------------------<" << std::endl;
}

template<typename _M, typename _T>
static void mem_pool_printer::_print_type_info(std::ostream& out)
{
    out << std::endl;
    out << "---------mem pool info [for <" << typeid(_T).name() << ">]-------------->" << std::endl;
    out << " ** type_size: " << string_format_utils::format_size(_M::template info_for_type<_T>::type_size) << std::endl;
    out << " ** cell_size: " << string_format_utils::format_size(_M::template info_for_type<_T>::cell_size) << std::endl;
    out << " ** cell_count_in_block: " << string_format_utils::format_count(_M::template info_for_type<_T>::cell_count_in_block) << std::endl;
    out << " ** pool_index: " << _M::template info_for_type<_T>::pool_index << std::endl;
    out << "-------------------------------------------------<" << std::endl;
}

CORE_NAMESPACE_END

#endif
