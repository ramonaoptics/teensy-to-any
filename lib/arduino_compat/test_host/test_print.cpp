// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Ramona Optics, Inc.
//
// Host unit test for the clean-room Print number formatting. Compares output
// against independently hand-computed Arduino-equivalent strings. Runs on the
// host compiler (no hardware):
//
//   c++ -std=gnu++14 -I../src test_print.cpp ../src/Print.cpp -o /tmp/tp && /tmp/tp
//
// Cases are chosen to be integer-width independent so the result matches the
// 32-bit target regardless of host `long` width.

#include "Print.h"
#include <string>
#include <cstdio>
#include <cstring>

class StringPrint : public Print {
public:
    std::string out;
    size_t write(uint8_t b) override { out.push_back((char)b); return 1; }
    using Print::write;
};

static int failures = 0;

template <typename Fn>
static void check(const char *name, Fn fn, const char *expected)
{
    StringPrint p;
    fn(p);
    if (p.out != expected) {
        std::printf("FAIL %-28s got \"%s\"  expected \"%s\"\n",
                    name, p.out.c_str(), expected);
        failures++;
    } else {
        std::printf("ok   %-28s \"%s\"\n", name, expected);
    }
}

int main()
{
    check("print(255)",        [](Print &p){ p.print(255); }, "255");
    check("print(0)",          [](Print &p){ p.print(0); }, "0");
    check("print(-42)",        [](Print &p){ p.print(-42); }, "-42");
    check("print(255,HEX)",    [](Print &p){ p.print(255, HEX); }, "ff");
    check("print(255u,HEX)",   [](Print &p){ p.print((unsigned int)255, HEX); }, "ff");
    check("print(8,BIN)",      [](Print &p){ p.print(8, BIN); }, "1000");
    check("print(64,OCT)",     [](Print &p){ p.print(64, OCT); }, "100");
    check("print('A')",        [](Print &p){ p.print('A'); }, "A");
    check("print(str)",        [](Print &p){ p.print("hi"); }, "hi");

    check("print(3.14159,2)",  [](Print &p){ p.print(3.14159, 2); }, "3.14");
    check("print(3.14159,4)",  [](Print &p){ p.print(3.14159, 4); }, "3.1416");
    check("print(-2.5,1)",     [](Print &p){ p.print(-2.5, 1); }, "-2.5");
    check("print(2.0,0)",      [](Print &p){ p.print(2.0, 0); }, "2");
    check("print(0.005,2)",    [](Print &p){ p.print(0.005, 2); }, "0.01");
    check("print(123.456,3)",  [](Print &p){ p.print(123.456, 3); }, "123.456");
    check("print(default 2dp)",[](Print &p){ p.print(1.5); }, "1.50");

    check("println(7)",        [](Print &p){ p.println(7); }, "7\r\n");
    check("println()",         [](Print &p){ p.println(); }, "\r\n");

    check("printf int+str",    [](Print &p){ p.printf("%d-%s", 5, "x"); }, "5-x");
    check("printf hex",        [](Print &p){ p.printf("%02X", 0xab); }, "AB");

    std::printf("\n%s (%d failure%s)\n",
                failures ? "TESTS FAILED" : "ALL TESTS PASSED",
                failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
