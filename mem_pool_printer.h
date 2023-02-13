
#ifndef MEM_POOL_PRINTER_H
#define MEM_POOL_PRINTER_H

#include "core.h"
#include "mem_pool.h"
#include <ostream>

CORE_NAMESPACE_BEG

struct mem_pool_printer {
    static void print_global_info(std::ostream& out);
    template<typename _T>
    static void print_type_info(std::ostream& out);

private:
    static size_t _format_size(std::stringstream& ss, size_t size, size_t formatSize, const char* suffix);
    static std::string _format_size(size_t size);
    static void _format_count(std::stringstream& ss, size_t size);
    static std::string _format_count(size_t size);
};

template<typename _T>
static void mem_pool_printer::print_type_info(std::ostream& out)
{
    out << std::endl;
    out << "---------mem pool info [for <" << typeid(_T).name() << ">]-------------->" << std::endl;
    out << " ** cell_raw_size: " << _format_size(mem_pool::info_for_type<_T>::cell_raw_size) << std::endl;
    out << " ** cell_size: " << _format_size(mem_pool::info_for_type<_T>::cell_size) << std::endl;
    out << " ** cell_count_in_block: " << _format_count(mem_pool::info_for_type<_T>::cell_count_in_block) << std::endl;
    out << " ** pool_index: " << mem_pool::info_for_type<_T>::pool_index << std::endl;
    out << "-------------------------------------------------<" << std::endl;
}

CORE_NAMESPACE_END

#endif
