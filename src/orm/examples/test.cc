#include "types.h"

#include <iostream>
#include <cstdint>
#include <cstdio>
#include <climits>

using namespace std;

int main() {
    flame::orm::col<uint64_t> csd_id("csd_id", 0);
    cout << csd_id << endl;
    uint32_t n = 0xFFFFFFFF;
    printf("%u\n", n);
    printf("%d\n", n);
    int32_t m = 0x80000000;
    printf("%u\n", -INT_MIN);
    printf("%d\n", -INT_MIN);
    printf("%u\n", 0x70000000);
    printf("%d\n", 0xF0000000);
}