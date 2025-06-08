#ifndef TEST_H
#define TEST_H

#include "kai_dev.h"
#include <stdio.h>

extern void begin_test(const char* name);

#define PASS   (printf("\x1b[92mPASS\x1b[0m\n"), 1)
#define FAIL   (printf("\x1b[91mFAIL\x1b[0m (Line %i)\n", __LINE__), 0)
#define TEST() begin_test(__FUNCTION__)

extern Kai_String_Writer error_writer;

#endif
