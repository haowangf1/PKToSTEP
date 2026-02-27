
#ifndef EXCHANGE_BASE_API_H
#define EXCHANGE_BASE_API_H

#ifdef EXCHANGE_BASE_STATIC_DEFINE
#  define EXCHANGE_BASE_API
#  define EXCHANGE_BASE_NO_EXPORT
#else
#  ifndef EXCHANGE_BASE_API
#    ifdef AMCAXExchangeBase_EXPORTS
        /* We are building this library */
#      define EXCHANGE_BASE_API __declspec(dllexport)
#    else
        /* We are using this library */
#      define EXCHANGE_BASE_API __declspec(dllimport)
#    endif
#  endif

#  ifndef EXCHANGE_BASE_NO_EXPORT
#    define EXCHANGE_BASE_NO_EXPORT 
#  endif
#endif

#ifndef EXCHANGE_BASE_DEPRECATED
#  define EXCHANGE_BASE_DEPRECATED __declspec(deprecated)
#endif

#ifndef EXCHANGE_BASE_DEPRECATED_EXPORT
#  define EXCHANGE_BASE_DEPRECATED_EXPORT EXCHANGE_BASE_API EXCHANGE_BASE_DEPRECATED
#endif

#ifndef EXCHANGE_BASE_DEPRECATED_NO_EXPORT
#  define EXCHANGE_BASE_DEPRECATED_NO_EXPORT EXCHANGE_BASE_NO_EXPORT EXCHANGE_BASE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef EXCHANGE_BASE_NO_DEPRECATED
#    define EXCHANGE_BASE_NO_DEPRECATED
#  endif
#endif

#endif /* EXCHANGE_BASE_API_H */
