#ifndef KAI_MEMORY_H
#define KAI_MEMORY_H
#include <kai/core.h>
__KAI_BEGIN_API__

KAI_API(void)
	kai_memory_create(kai_Memory* Memory);

KAI_API(void)
	kai_memory_reset(kai_Memory* Memory);

KAI_API(void)
	kai_memory_destroy(kai_Memory* Memory);

KAI_API(kai_u64)
	kai_memory_usage(kai_Memory* Memory);

__KAI_END_API__
#endif//KAI_MEMORY_H