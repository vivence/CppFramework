#include "mem_pool_printer.h"
#include <sstream>
#include <iomanip>

CORE_NAMESPACE_BEG

void mem_pool_printer::print_global_info(std::ostream& out)
{
    out << std::endl;
    out << "---------mem pool info [for global]-------------->" << std::endl;
    out << " ** cell_unit_size: " << _format_size(mem_pool::info_for_global::cell_unit_size) << std::endl;
    out << " ** block_max_size: " << _format_size(mem_pool::info_for_global::block_max_size) << std::endl;
    out << " ** pool_max_count: " << _format_count(mem_pool::info_for_global::pool_max_count) << std::endl;
    out << " ** cell_raw_size_min: " << _format_size(mem_pool::info_for_global::cell_raw_size_min) << std::endl;
    out << " ** cell_head_size: " << _format_size(mem_pool::info_for_global::cell_head_size) << std::endl;
    out << " ** user_mem_offset_in_cell: " << mem_pool::info_for_global::user_mem_offset_in_cell << std::endl;
    out << "-------------------------------------------------<" << std::endl;
}

size_t mem_pool_printer::_format_size(std::stringstream& ss, size_t size, size_t formatSize, const char* suffix)
{
    if (formatSize <= size)
    {
        size_t fsz = size / formatSize;
        ss << fsz << suffix;
        return fsz * formatSize;
    }
    return 0;
}

std::string mem_pool_printer::_format_size(size_t size)
{
    static const size_t B = 1;
    static const size_t KB = 1024 * B;
    static const size_t MB = 1024 * KB;
    static const size_t GB = 1024 * MB;

    size_t origin = size;

    std::stringstream ss;
    size = size - _format_size(ss, size, GB, "G");
    size = size - _format_size(ss, size, MB, "M");
    size = size - _format_size(ss, size, KB, "K");
    _format_size(ss, size, B, "B");

    if (size != origin)
    {
        ss << "(" << origin << ")";
    }
    return std::move(ss.str());
}

void mem_pool_printer::_format_count(std::stringstream& ss, size_t size)
{
    static const size_t K = 1000;
    if (K > size)
    {
        ss << size;
    }
    else
    {
        _format_count(ss, size / K);
        ss << ',';
        ss << std::setw(3) << std::setfill('0') << size % K;
    }
}
std::string mem_pool_printer::_format_count(size_t size)
{
    std::stringstream ss;
    _format_count(ss, size);
    return std::move(ss.str());
}

CORE_NAMESPACE_END