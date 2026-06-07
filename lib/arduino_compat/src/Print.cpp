// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Ramona Optics, Inc.
//
// Clean-room MIT implementation of Print. The integer and float formatting
// reproduce Arduino's documented algorithm (digit-by-digit into a reversed
// buffer; printFloat rounds by adding 0.5/10^digits) so protocol output stays
// byte-identical. Written from the algorithm description, not copied.

#include "Print.h"
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

size_t Print::write(const uint8_t *buffer, size_t size)
{
    size_t n = 0;
    while (size--) {
        if (write(*buffer++)) n++;
        else break;
    }
    return n;
}

size_t Print::printSigned(long n, int base)
{
    // Arduino: only base 10 is treated as signed; other bases print the raw
    // (two's-complement) value as unsigned.
    if (base == 10 && n < 0) {
        size_t t = write((uint8_t)'-');
        return t + printNumber((unsigned long)(-n), 10);
    }
    return printNumber((unsigned long)n, (uint8_t)base);
}

size_t Print::printNumber(unsigned long n, uint8_t base)
{
    char buf[8 * sizeof(unsigned long) + 1]; // worst case: base 2
    char *str = &buf[sizeof(buf) - 1];
    *str = '\0';

    if (base < 2) base = 10;

    do {
        unsigned long m = n;
        n /= base;
        char c = (char)(m - base * n);
        *--str = (char)(c < 10 ? c + '0' : c + 'a' - 10);
    } while (n);

    return write(str);
}

size_t Print::printFloat(double number, uint8_t digits)
{
    size_t n = 0;

    if (isnan(number)) return print("nan");
    if (isinf(number)) return print("inf");
    if (number > 4294967040.0) return print("ovf"); // out of long range
    if (number < -4294967040.0) return print("ovf");

    if (number < 0.0) {
        n += write((uint8_t)'-');
        number = -number;
    }

    // Round to the requested number of decimal places.
    double rounding = 0.5;
    for (uint8_t i = 0; i < digits; ++i) rounding /= 10.0;
    number += rounding;

    unsigned long int_part = (unsigned long)number;
    double remainder = number - (double)int_part;
    n += printNumber(int_part, 10);

    if (digits > 0) n += write((uint8_t)'.');

    while (digits-- > 0) {
        remainder *= 10.0;
        unsigned int toPrint = (unsigned int)remainder;
        n += printNumber(toPrint, 10);
        remainder -= toPrint;
    }
    return n;
}

int Print::printf(const char *format, ...)
{
    char buf[128];
    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    if (len <= 0) return 0;

    size_t to_write = (size_t)len < sizeof(buf) ? (size_t)len : sizeof(buf) - 1;
    return (int)write((const uint8_t *)buf, to_write);
}
