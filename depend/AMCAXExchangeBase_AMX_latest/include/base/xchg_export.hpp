#ifndef XCHG_EXPORT_HPP
#define XCHG_EXPORT_HPP

// Include the generated export header
#include "exchange_base_export.h"

// Alias for easier use throughout the codebase
#define XCHG_API EXCHANGE_BASE_API
#define XCHG_NO_EXPORT EXCHANGE_BASE_NO_EXPORT

// Template export helper (templates can't use dllexport/dllimport)
#define XCHG_TEMPLATE_API

// Deprecated macro (optional, for marking deprecated APIs)
#if defined(__GNUC__) || defined(__clang__)
    #define XCHG_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
    #define XCHG_DEPRECATED __declspec(deprecated)
#else
    #define XCHG_DEPRECATED
#endif

// Suppress MSVC C4251 warning for SmartPtr template members
// This warning occurs when exported classes contain template members (like SmartPtr<T>)
// It's safe to suppress because SmartPtr is a header-only template and doesn't need DLL interface
// This is a systemic design decision: all exported classes may contain SmartPtr members
#ifdef _MSC_VER
    #pragma warning(disable: 4251)  // class 'X' needs to have dll-interface to be used by clients of class 'Y'
#endif


#endif // XCHG_EXPORT_HPP

