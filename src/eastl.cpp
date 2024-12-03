#include <EASTL/allocator.h>
#include <stdio.h>
#include <wchar.h>
#include "sysconfig.h"


void* operator new[](size_t size, const char* /*pName*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/,
                     int /*line*/) {
    return malloc(size);
}

void* operator new[](size_t size, size_t alignment, size_t /*alignmentOffset*/, const char* /*pName*/, int /*flags*/,
                     unsigned /*debugFlags*/, const char* /*file*/, int /*line*/) {
    // this allocator doesn't support alignment
    EASTL_ASSERT(alignment <= 8);
    return malloc(size);
}

// EASTL also wants us to define this (see string.h line 217)
int Vsnprintf8(char* pDestination, size_t n, const char* pFormat, va_list arguments) {
#ifdef _MSC_VER
    return _vsnprintf(pDestination, n, pFormat, arguments);
#else
    return vsnprintf(pDestination, n, pFormat, arguments);
#endif
}
int Vsnprintf16(char8_t* pDestination, size_t n, const char16_t* pFormat, va_list arguments) {
#ifdef _MSC_VER
    return _vsnwprintf((wchar_t*)pDestination, n, (const wchar_t*)pFormat, arguments);
#else
    return vsnprintf(pDestination, n, (char*)pFormat, arguments);
#endif
}
int Vsnprintf16(wchar_t* pDestination, size_t n, const wchar_t* pFormat, va_list arguments) {
#ifdef _MSC_VER
    return _vsnwprintf(pDestination, n, pFormat, arguments);
#else
    return vswprintf(pDestination, n, pFormat, arguments);
#endif
}
int Vsnprintf32(char8_t* pDestination, size_t n, const char32_t* pFormat, va_list arguments) {
#ifdef _MSC_VER
    return _vsnprintf((char*)pDestination, n, (char*)pFormat, arguments);
#else
    return vsnprintf(pDestination, n, (char*)pFormat, arguments);
#endif
}
