
#ifndef REF_H
#define REF_H

#include "core.h"
#include "dis_new.h"

CORE_NAMESPACE_BEG

template<typename _T>
class ref : dis_new {
protected:
    _T* _p;

public:
    explicit ref(_T* p = nullptr) : _p(p) {}
    virtual ~ref() { _p = nullptr; }

public:
    virtual const _T* operator->() const { return _p; }
    virtual _T* operator->() { return _p; }

public:
    inline bool operator ==(const _T* p) const { return _p == p; }
    inline bool operator !=(const _T* p) const { return _p != p; }
    inline bool operator ==(const ref<_T>& rhs) const { return _p == rhs._p; }
    inline bool operator !=(const ref<_T>& rhs) const { return _p != rhs._p; }
    inline bool operator ==(std::nullptr_t p) const { return p == _p; }
    inline bool operator !=(std::nullptr_t p) const { return p != _p; }
    template<typename _TT>
    friend bool operator ==(std::nullptr_t p, const ref<_TT>&wp);
    template<typename _TT>
	friend bool operator !=(std::nullptr_t p, const ref<_TT>& wp);

private:
	ref* operator&() = delete;
};

template<typename _T>
inline bool operator ==(std::nullptr_t p, const ref<_T>& wp) { return p == wp._p; }
template<typename _T>
inline bool operator !=(std::nullptr_t p, const ref<_T>& wp) { return p != wp._p; }

CORE_NAMESPACE_END

#endif
