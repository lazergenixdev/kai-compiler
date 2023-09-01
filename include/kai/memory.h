#ifndef KAI_MEMORY_H
#define KAI_MEMORY_H
#include <kai/core.h>
__KAI_BEGIN_API__

KAI_API(void)
	kai_create_memory(kai_Memory* Memory);

KAI_API(void)
	kai_reset_memory(kai_Memory* Memory);

KAI_API(void)
	kai_destroy_memory(kai_Memory* Memory);

KAI_API(kai_u64)
	kai_memory_usage(kai_Memory* Memory);

__KAI_END_API__
#ifdef KAI_CPP_API
namespace kai {
	struct Memory : public kai_Memory {
		inline Memory()  { kai_create_memory(this); }
		inline ~Memory() { kai_destroy_memory(this); }
	};
}
#endif
#endif//KAI_MEMORY_H