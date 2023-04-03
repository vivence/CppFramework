
#ifndef CONTAINERS_H
#define CONTAINERS_H

#include "core.h"
#include "mem_pool.h"
#include <memory>
#include <limits>
#include <type_traits>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <sstream>

CORE_NAMESPACE_BEG

template<typename _T>
class allocator
{
public:
    using value_type = _T;
    using pointer = _T*;
    using const_pointer = const _T*;
    using void_pointer = void*;
    using const_void_pointer = const void*;
    using reference = _T&;
    using const_reference = const _T&;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;

    template<typename _U>
    struct rebind { using other = allocator<_U>; };

    allocator() = default;
    ~allocator() = default;
    allocator(const allocator&) noexcept {};
    template<typename _U>
    allocator(const allocator<_U>&) noexcept {};
    allocator& operator =(const allocator&) = delete;

    pointer allocate(size_type count) 
    {
        return static_cast<pointer>(mem_pool_utils::alloc(sizeof(_T) * count));
    }

    pointer allocate(size_type count, const_void_pointer hit)
    {
        hit;
        return allocate(count);
    }

    void deallocate(pointer p, size_type count)
    {
        count;
        mem_pool_utils::free(p);
    }

    template<typename _U, typename ...Args>
    void construct(_U* const p, Args&&... args)
    {
        ::new(p) _U(std::forward<Args>(args)...);
    }

    template <typename _U>
    void destroy(_U* const p)
    {
        p->~_U();
    }

    pointer address(reference x) 
    {
        return (pointer)&x;
    }

    const_pointer address(const_reference x) 
    {
        return (const_pointer)&x;
    }

    size_type max_size() const 
    {
        return static_cast<size_type>(-1) / sizeof(_T);
    }
};
template <class _T, class _P>
bool operator==(const allocator<_T>&, const allocator<_P>&) noexcept 
{
    return true;
}
template <class _T, class _P>
bool operator!=(const allocator<_T>&, const allocator<_P>&) noexcept 
{
    return false;
}

// -------------------------------------

using string = std::basic_string<char, std::char_traits<char>, allocator<char>>;
using wstring = std::basic_string<wchar_t, std::char_traits<wchar_t>, allocator<wchar_t>>;

using stringbuf = std::basic_stringbuf<char, std::char_traits<char>, allocator<char>>;
using istringstream = std::basic_istringstream<char, std::char_traits<char>, allocator<char>>;
using ostringstream = std::basic_ostringstream<char, std::char_traits<char>, allocator<char>>;
using stringstream = std::basic_stringstream<char, std::char_traits<char>, allocator<char>>;

using wstringbuf = std::basic_stringbuf<wchar_t, std::char_traits<wchar_t>, allocator<wchar_t>>;
using wistringstream = std::basic_istringstream<wchar_t, std::char_traits<wchar_t>, allocator<wchar_t>>;
using wostringstream = std::basic_ostringstream<wchar_t, std::char_traits<wchar_t>, allocator<wchar_t>>;
using wstringstream = std::basic_stringstream<wchar_t, std::char_traits<wchar_t>, allocator<wchar_t>>;

template<typename _T>
using vector = std::vector<_T, allocator<_T>>;

template<typename _T>
using list = std::list<_T, allocator<_T>>;

template<typename _T>
using deque = std::deque<_T, allocator<_T>>;

template<typename _T>
using stack = std::stack<_T, deque<_T>>;

template<typename _T>
using queue = std::queue<_T, deque<_T>>;

template<typename _T>
using set = std::set<_T, std::less<_T>, allocator<_T>>;

template<typename _T>
using multiset = std::multiset<_T, std::less<_T>, allocator<_T>>;

template<typename _K, typename _V>
using map = std::map<_K, _V, std::less<_K>, allocator<std::pair<const _K, _V>>>;

template<typename _K, typename _V>
using multimap = std::multimap<_K, _V, std::less<_K>, allocator<std::pair<const _K, _V>>>;

template<typename _T>
using unordered_set = std::unordered_set<_T, std::hash<_T>, std::equal_to<_T>, allocator<_T>>;

template<typename _T>
using unordered_multiset = std::unordered_multiset<_T, std::hash<_T>, std::equal_to<_T>, allocator<_T>>;

template<typename _K, typename _V>
using unordered_map = std::unordered_map<_K, _V, std::hash<_K>, std::equal_to<_K>, allocator<std::pair<const _K, _V>>>;

template<typename _K, typename _V>
using unordered_multimap = std::unordered_multimap<_K, _V, std::hash<_K>, std::equal_to<_K>, allocator<std::pair<const _K, _V>>>;

CORE_NAMESPACE_END

#endif
