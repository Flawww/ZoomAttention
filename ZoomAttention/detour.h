#pragma once
#include "pch.h"

class detour_hook {
public:
    detour_hook() : m_target_addr(0), m_shellcode(0), m_len(0) {};

    /* len is amount of bytes we need to nop out to reach 5 bytes and still have valid instructions afterwards */
    detour_hook(void* target, void* new_func, size_t len) : m_target_addr(target), m_len(len) {
        static auto base_virtual_protect = reinterpret_cast<BOOL(__stdcall*)(void*, size_t, int, PDWORD)>(GetProcAddress(GetModuleHandleA("KernelBase.dll"), "VirtualProtect"));

        if (len < 5 || !target || !new_func)
            throw std::runtime_error("failed detouring function, invalid param");

        m_shellcode = VirtualAlloc(nullptr, len + 5, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

        if (!m_shellcode)
            throw std::runtime_error("failed allocating buffer for detour shellcode");

        DWORD old;
        base_virtual_protect(target, len, PAGE_EXECUTE_READWRITE, &old); // make function writeable
        memcpy_s(m_shellcode, len + 5, target, len);
        memset(target, 0x90, len); // nop out length

        /* write hook into function */
        uint32_t relative = uintptr_t(new_func) - uintptr_t(target) - 5;
        *(BYTE*)target = 0xE9;
        *(uint32_t*)(uintptr_t(target) + 1) = relative;

        base_virtual_protect(target, len, old, &old); // restore function protection

        /* write jump back shellcode */
        uint32_t jump_back = (uintptr_t(target) + len) - uintptr_t(m_shellcode) - len - 5;
        *(BYTE*)(uintptr_t(m_shellcode) + len) = 0xE9;
        *(uint32_t*)(uintptr_t(m_shellcode) + len + 1) = jump_back;
    };

    ~detour_hook() {
        static auto base_virtual_protect = reinterpret_cast<BOOL(__stdcall*)(void*, size_t, int, PDWORD)>(GetProcAddress(GetModuleHandleA("KernelBase.dll"), "VirtualProtect"));

        DWORD old;
        base_virtual_protect(m_target_addr, m_len, PAGE_EXECUTE_READWRITE, &old); // make function writeable
        memcpy(m_target_addr, m_shellcode, m_len); // restore shellcode in function 
        base_virtual_protect(m_target_addr, m_len + 5, old, &old); // protect shellcode from writing

        VirtualFree(m_shellcode, 0, MEM_RELEASE);
    }

    template<class t>
    inline t get_orig() {
        return (t)m_shellcode;
    }

private:
    void* m_target_addr;
    void* m_shellcode;
    size_t m_len;
};