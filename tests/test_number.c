#include "test.h"
#include <stdlib.h>
#include <string.h>

#define assert_eq(A,...) if(kai_number_compare(A,__VA_ARGS__) != 0) return FAIL

int number(void)
{
    TEST();

    // Comparison
    {
        Kai_Number w0 = {0x8000000080000000, 1, 0, 1};
        Kai_Number w1 = {0x8000000080000000, 1, 0, 0};

        if (kai_number_compare(w0, w1) !=  1) return FAIL;
        if (kai_number_compare(w1, w0) != -1) return FAIL;
        
        Kai_Number w2 = {0x8572352300000123, 0x7391253400000001, 0, 23};
        Kai_Number w3 = {0x8572352300000124, 0x7391253400000001, 0, 23};
        
        if (kai_number_compare(w2, w3) != -1) return FAIL;
        if (kai_number_compare(w3, w2) !=  1) return FAIL;

        Kai_Number w4 = {0x20000000000000, 1, 0, -53};
        Kai_Number w5 = {1, 0x20000000000000, 0, 53};

        if (kai_number_compare(w4, w5) != 0) return FAIL;
        if (kai_number_compare(w5, w4) != 0) return FAIL;

        Kai_Number w6 = {0x982763547832123, 1};

        if (kai_number_compare(w6, (Kai_Number){0xFFFFFFFFFFFFFFFF, 1}) != -1) return FAIL;
        if (kai_number_compare(w6, (Kai_Number){0}) != 1) return FAIL;
    }

    // Arithmetic, no overflow
    {
        Kai_Number r0 = kai_number_add((Kai_Number){4324, 32}, (Kai_Number){582, 532});
        assert_eq(r0, (Kai_Number){144937, 1064});

        Kai_Number r1 = kai_number_sub((Kai_Number){4324, 32}, (Kai_Number){582, 532});
        assert_eq(r1, (Kai_Number){142609, 1064});

        Kai_Number r2 = kai_number_mul((Kai_Number){4324, 32}, (Kai_Number){582, 532});
        assert_eq(r2, (Kai_Number){314571, 2128});

        Kai_Number r3 = kai_number_div((Kai_Number){4324, 32}, (Kai_Number){582, 532});
        assert_eq(r3, (Kai_Number){143773, 1164});
    }

    return PASS;
}
