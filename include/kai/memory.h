#ifndef KAI_MEMORY_H
#define KAI_MEMORY_H
#include "core.h"
__KAI_BEGIN_API__

KAI_API(void)
	kai_create_memory(Kai_Memory* Memory);

KAI_API(void)
	kai_reset_memory(Kai_Memory* Memory);

KAI_API(void)
	kai_destroy_memory(Kai_Memory* Memory);

KAI_API(Kai_u64)
	kai_memory_usage(Kai_Memory* Memory);

__KAI_END_API__
#ifdef KAI_CPP_API
namespace kai {
	struct Memory : public Kai_Memory {
		inline  Memory() { kai_create_memory(this); }
		inline ~Memory() { kai_destroy_memory(this); }
	};
}
#endif
#endif//KAI_MEMORY_H
