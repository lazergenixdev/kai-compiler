/*       87654  32109  87654  3210
            &      &            &&
break    00010  10010  01011  0101
continue 00011  01111  00101  1000
else     00101  01100  00101  0100
if       01001  00110  00110  0010
using    10101  10011  00111  0101
while    10111  01000  00101  0101
cast     00011  00001  10100  0100
defer    00100  00101  10010  0101
for      00110  01111  10010  0011
loop     01100  01111  10000  0100
ret      10010  00101  10100  0011
struct   10011  10100  10100  0110

         &&&&
else     0000
defer    0001
loop     0100
using    0101
if       0110
while    1001
cast     1000
struct   1010
ret      1011
continue 1100
break    1101
for      1111

*/
#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
#include <cstdint>

std::vector<std::pair<std::string, uint32_t>> arr;
std::unordered_set<uint32_t> set;

#define for_n(I,N) for (int I = 0; I < N; ++I)

static constexpr uint32_t get_bit(uint32_t n, int bit) {
    return (n >> bit) & 1;
}

#define TEST(S) ((S.size() == 12)? "PASS" : "FAIL")

int main() {
    arr.emplace_back("break"    , 0b0001010010010110101);
    arr.emplace_back("continue" , 0b0001101111001011000);
    arr.emplace_back("else"     , 0b0010101100001010100);
    arr.emplace_back("if"       , 0b0100100110001100010);
    arr.emplace_back("using"    , 0b1010110011001110101);
    arr.emplace_back("while"    , 0b1011101000001010101);
    arr.emplace_back("cast"     , 0b0001100001101000100);
    arr.emplace_back("defer"    , 0b0010000101100100101);
    arr.emplace_back("for"      , 0b0011001111100100011);
    arr.emplace_back("loop"     , 0b0110001111100000100);
    arr.emplace_back("ret"      , 0b1001000101101000011);
    arr.emplace_back("struct"   , 0b1001110100101000110);

    static constexpr int N = 19;

    for_n(a, N)
    for_n(b, N)
    for_n(c, N)
    for_n(d, N) {
        if (a == b || a == c
        ||  a == d || b == c
        ||  b == d || c == d) continue;

        set.clear();

        for (auto&& [s,n]: arr) {
            auto bit_a = get_bit(n, a);
            auto bit_b = get_bit(n, b);
            auto bit_c = get_bit(n, c);
            auto bit_d = get_bit(n, d);

            uint32_t k = (bit_a << 3) | (bit_b << 2) | (bit_c << 1) | (bit_d);

            set.insert(k);
        }

        if (set.size() >= 12) {
            std::cout << "FOUND!" << '\n';
            std::cout << a << " " << b << " " << c << " " << d;
            return 0;
        }
    }

    return 0;
}
