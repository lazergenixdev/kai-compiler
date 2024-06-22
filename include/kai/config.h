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
#else
#   define __KAI_BEGIN_API__
#   define __KAI_END_API__
#endif

#define KAI_CALL     
#define KAI_API(RET) extern RET

#define KAI_VERSION_MAJOR 0
#define KAI_VERSION_MINOR 1
#define KAI_VERSION_PATCH 0
#define _KAI_MAKE_VERSION_STRING(X,Y,Z) #X "." #Y "." #Z
#define _KAI_MAKE_VERSION_STRING2(X,Y,Z) _KAI_MAKE_VERSION_STRING(X,Y,Z) // garbage helper macro because macros suck.

#define KAI_VERSION_STRING       _KAI_MAKE_VERSION_STRING2(KAI_VERSION_MAJOR,KAI_VERSION_MINOR,KAI_VERSION_PATCH)
#define KAI_VERSION_STRING_V "v" _KAI_MAKE_VERSION_STRING2(KAI_VERSION_MAJOR,KAI_VERSION_MINOR,KAI_VERSION_PATCH)

#ifdef __cplusplus
#    define KAI_STRUCT(X) X
#else
#    define KAI_STRUCT(X) (X)
#endif

#endif//KAI_CONFIG_H
