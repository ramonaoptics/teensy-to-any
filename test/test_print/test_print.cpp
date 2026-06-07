// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Ramona Optics, Inc.
//
// Host unit test for the clean-room Print number formatting. Compares output
// against independently hand-computed Arduino-equivalent strings. Runs on the
// host with no hardware via PlatformIO + Unity:
//
//   pio test -e native
//
// Cases are chosen to be integer-width independent so the result matches the
// 32-bit target regardless of host `long` width.

#include <unity.h>
#include "Print.h"
#include <string>

class StringPrint : public Print {
public:
    std::string out;
    size_t write(uint8_t b) override { out.push_back((char)b); return 1; }
    using Print::write;
};

void setUp(void) {}
void tearDown(void) {}

static void test_integers(void)
{
    StringPrint p;
    p.print(255);                  TEST_ASSERT_EQUAL_STRING("255", p.out.c_str()); p.out.clear();
    p.print(0);                    TEST_ASSERT_EQUAL_STRING("0", p.out.c_str());   p.out.clear();
    p.print(-42);                  TEST_ASSERT_EQUAL_STRING("-42", p.out.c_str()); p.out.clear();
    p.print(255, HEX);             TEST_ASSERT_EQUAL_STRING("ff", p.out.c_str());  p.out.clear();
    p.print((unsigned int)255, HEX); TEST_ASSERT_EQUAL_STRING("ff", p.out.c_str()); p.out.clear();
    p.print(8, BIN);               TEST_ASSERT_EQUAL_STRING("1000", p.out.c_str()); p.out.clear();
    p.print(64, OCT);              TEST_ASSERT_EQUAL_STRING("100", p.out.c_str());  p.out.clear();
}

static void test_strings_chars(void)
{
    StringPrint p;
    p.print('A');                  TEST_ASSERT_EQUAL_STRING("A", p.out.c_str());  p.out.clear();
    p.print("hi");                 TEST_ASSERT_EQUAL_STRING("hi", p.out.c_str()); p.out.clear();
}

static void test_floats(void)
{
    StringPrint p;
    p.print(3.14159, 2);           TEST_ASSERT_EQUAL_STRING("3.14", p.out.c_str());    p.out.clear();
    p.print(3.14159, 4);           TEST_ASSERT_EQUAL_STRING("3.1416", p.out.c_str());  p.out.clear();
    p.print(-2.5, 1);              TEST_ASSERT_EQUAL_STRING("-2.5", p.out.c_str());    p.out.clear();
    p.print(2.0, 0);               TEST_ASSERT_EQUAL_STRING("2", p.out.c_str());       p.out.clear();
    p.print(0.005, 2);             TEST_ASSERT_EQUAL_STRING("0.01", p.out.c_str());    p.out.clear();
    p.print(123.456, 3);           TEST_ASSERT_EQUAL_STRING("123.456", p.out.c_str()); p.out.clear();
    p.print(1.5);                  TEST_ASSERT_EQUAL_STRING("1.50", p.out.c_str());    p.out.clear();
}

static void test_println_printf(void)
{
    StringPrint p;
    p.println(7);                  TEST_ASSERT_EQUAL_STRING("7\r\n", p.out.c_str()); p.out.clear();
    p.println();                   TEST_ASSERT_EQUAL_STRING("\r\n", p.out.c_str());  p.out.clear();
    p.printf("%d-%s", 5, "x");     TEST_ASSERT_EQUAL_STRING("5-x", p.out.c_str());   p.out.clear();
    p.printf("%02X", 0xab);        TEST_ASSERT_EQUAL_STRING("AB", p.out.c_str());    p.out.clear();
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_integers);
    RUN_TEST(test_strings_chars);
    RUN_TEST(test_floats);
    RUN_TEST(test_println_printf);
    return UNITY_END();
}
