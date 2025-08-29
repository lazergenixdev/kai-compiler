#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../3rd-party/nob.h"
#define KAI_IMPLEMENTATION
#include "../kai.h"

#define assert_true(EXPR) if (!(EXPR)) printf("\e[91mTest Failed\e[0m: %s (\e[92m%s:%i\e[0m)\n", #EXPR, __FILE__, __LINE__), exit(1)
