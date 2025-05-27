#ifndef TEST_H
#define TEST_H

#define KAI_USE_MEMORY_API
#define KAI_USE_DEBUG_API
#include "../include/kai.h"
#include <stdio.h>

extern void begin_test(const char* name);

#define PASS   (printf("\x1b[92mPASS\x1b[0m\n"), 1)
#define FAIL   (printf("\x1b[91mFAIL\x1b[0m (Line %i)\n", __LINE__), 0)
#define TEST() begin_test(__FUNCTION__)

extern Kai_Debug_String_Writer error_writer;

#endif
