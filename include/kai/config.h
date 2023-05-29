#ifndef KAI_CONFIG_H
#define KAI_CONFIG_H

// I literally don't care that `__` identifiers are reserved,
//   use them anyway.
#ifdef __cplusplus
#   define __KAI_BEGIN_API__ extern "C" {
#   define __KAI_END_API__ }

#	ifndef KAI_NO_CPP_API
#		define KAI_CPP_API
#	endif
#elif
#   define __KAI_BEGIN_API__
#   define __KAI_END_API__
#endif

#define KAI_API extern

#define KAI_VERSION_MAJOR 0
#define KAI_VERSION_MINOR 1
#define KAI_VERSION_PATCH 0
#define _KAI_MAKE_VERSION_STRINGV(X,Y,Z) "v" #X "." #Y "." #Z
#define _KAI_MAKE_VERSION_STRINGV2(X,Y,Z) _KAI_MAKE_VERSION_STRINGV(X,Y,Z) // garbage helper macro because macros suck.

#define KAI_VERSION_STR _KAI_MAKE_VERSION_STRINGV2(KAI_VERSION_MAJOR,KAI_VERSION_MINOR,KAI_VERSION_PATCH)

#define KAI_BIT(X) (1 << X)

#endif//KAI_CONFIG_H