#include "utils.h"
#include <iomanip>

CORE_NAMESPACE_BEG

const char* const console_text::RESET   = "\033[0m";
const char* const console_text::BOLD    = "\033[1m";

const char* const console_text::BLACK   = "\033[30m";
const char* const console_text::RED     = "\033[31m";
const char* const console_text::GREEN   = "\033[32m";
const char* const console_text::YELLOW  = "\033[33m";
const char* const console_text::BLUE    = "\033[34m";
const char* const console_text::PURPLE  = "\033[35m";
const char* const console_text::CYAN    = "\033[36m";
const char* const console_text::WHITE   = "\033[37m";

const char* const console_text::BKG_BLACK  = "\033[40m";
const char* const console_text::BKG_RED    = "\033[41m";
const char* const console_text::BKG_GREEN  = "\033[42m";
const char* const console_text::BKG_YELLOW = "\033[43m";
const char* const console_text::BKG_BLUE   = "\033[44m";
const char* const console_text::BKG_PURPLE = "\033[45m";
const char* const console_text::BKG_CYAN   = "\033[46m";
const char* const console_text::BKG_WHITE  = "\033[47m";

size_t string_format_utils::format_size(std::stringstream& ss, size_t size, size_t formatSize, const char* suffix)
{
    if (formatSize <= size)
    {
        size_t fsz = size / formatSize;
        ss << fsz << suffix;
        return fsz * formatSize;
    }
    return 0;
}

std::string string_format_utils::format_size(size_t size)
{
    static const size_t B = 1;
    static const size_t KB = 1024 * B;
    static const size_t MB = 1024 * KB;
    static const size_t GB = 1024 * MB;
    if (0 == size)
    {
        std::stringstream ss;
        ss << size;
        return std::move(ss.str());
    }

    size_t origin = size;

    std::stringstream ss;
    size = size - format_size(ss, size, GB, "G");
    size = size - format_size(ss, size, MB, "M");
    size = size - format_size(ss, size, KB, "K");
    format_size(ss, size, B, "B");

    if (size != origin)
    {
        ss << "(" << origin << ")";
    }
    return std::move(ss.str());
}

void string_format_utils::format_count(std::stringstream& ss, size_t size)
{
    static const size_t K = 1000;
    if (K > size)
    {
        ss << size;
    }
    else
    {
        format_count(ss, size / K);
        ss << ',';
        ss << std::setw(3) << std::setfill('0') << size % K;
    }
}
std::string string_format_utils::format_count(size_t size)
{
    std::stringstream ss;
    format_count(ss, size);
    return std::move(ss.str());
}

CORE_NAMESPACE_END