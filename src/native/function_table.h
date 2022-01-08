#ifndef MIGI_FUNCTION_TABLE_H_
#define MIGI_FUNCTION_TABLE_H_

#include <stdint.h>
#include <set>
#include <unordered_map>

namespace migi {

class FunctionTable {
public:
    typedef uintptr_t (*PreCallFunctionPtr)(uintptr_t);
    typedef void (*PostCallFunctionPtr)(uintptr_t);

    FunctionTable(const uintptr_t* bridges, uintptr_t* entries, PreCallFunctionPtr* preCallPtrs, PostCallFunctionPtr* postCallPtrs, uint32_t tableLength)
        : _bridges(bridges), _entries(entries), _pre_call_ptrs(preCallPtrs), _post_call_ptrs(postCallPtrs) {
        for(uint32_t i = 0; i < tableLength; ++i)
            _available_slots.insert(i);
    }

    uintptr_t convert(uintptr_t stdcallFunction, PreCallFunctionPtr preCallPtr = nullptr, PostCallFunctionPtr postCallPtr = nullptr) {
        if(_available_slots.empty())
            return 0;
        const auto iter = _available_slots.begin();
        uint32_t slot = *iter;
        _available_slots.erase(iter);
        _occupied_slots[stdcallFunction] = slot;
        _entries[slot] = stdcallFunction;
        _pre_call_ptrs[slot] = preCallPtr;
        _post_call_ptrs[slot] = postCallPtr;
        return _bridges[slot];
    }

    void recycle(uintptr_t fastcallFunction) {
        const auto iter = _occupied_slots.find(fastcallFunction);
        if(iter != _occupied_slots.end()) {
            _available_slots.insert(iter->second);
            _occupied_slots.erase(iter);
        }
    }

protected:
    std::set<uint32_t> _available_slots;
    std::unordered_map<uintptr_t, uint32_t> _occupied_slots;
    const uintptr_t* _bridges;
    uintptr_t* _entries;
    PreCallFunctionPtr* _pre_call_ptrs;
    PostCallFunctionPtr* _post_call_ptrs;
};

}

#endif
