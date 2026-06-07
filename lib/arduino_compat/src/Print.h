// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Ramona Optics, Inc.
//
// Clean-room MIT replacement for Arduino's Print, implementing only the
// overloads this firmware uses (see docs/migration/arduino-compat-surface.md).
// Number formatting reproduces Arduino's algorithm exactly so the serial
// protocol output stays byte-identical. Written from scratch, not copied.

#pragma once

#include <stdint.h>
#include <stddef.h>

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class Print {
public:
    virtual ~Print() {}

    // The one method a concrete sink must implement.
    virtual size_t write(uint8_t b) = 0;

    // Block write; default loops over single-byte write().
    virtual size_t write(const uint8_t *buffer, size_t size);
    size_t write(const char *str) {
        if (!str) return 0;
        return write((const uint8_t *)str, _strlen(str));
    }
    size_t write(const char *buffer, size_t size) {
        return write((const uint8_t *)buffer, size);
    }

    // print()
    size_t print(const char *s) { return write(s); }
    size_t print(char c) { return write((uint8_t)c); }
    size_t print(unsigned char n, int base = DEC) { return printNumber(n, base); }
    size_t print(int n, int base = DEC) { return printSigned(n, base); }
    size_t print(unsigned int n, int base = DEC) { return printNumber(n, base); }
    size_t print(long n, int base = DEC) { return printSigned(n, base); }
    size_t print(unsigned long n, int base = DEC) { return printNumber(n, base); }
    size_t print(double n, int digits = 2) { return printFloat(n, digits); }

    // println()
    size_t println(void) { return write((const uint8_t *)"\r\n", 2); }
    size_t println(const char *s) { size_t n = print(s); return n + println(); }
    size_t println(char c) { size_t n = print(c); return n + println(); }
    size_t println(unsigned char b, int base = DEC) { size_t n = print(b, base); return n + println(); }
    size_t println(int v, int base = DEC) { size_t n = print(v, base); return n + println(); }
    size_t println(unsigned int v, int base = DEC) { size_t n = print(v, base); return n + println(); }
    size_t println(long v, int base = DEC) { size_t n = print(v, base); return n + println(); }
    size_t println(unsigned long v, int base = DEC) { size_t n = print(v, base); return n + println(); }
    size_t println(double v, int digits = 2) { size_t n = print(v, digits); return n + println(); }

    // Teensy extension used by src/: printf over write().
    int printf(const char *format, ...) __attribute__((format(printf, 2, 3)));

private:
    static size_t _strlen(const char *s) { size_t n = 0; while (s[n]) n++; return n; }
    size_t printSigned(long n, int base);
    size_t printNumber(unsigned long n, uint8_t base);
    size_t printFloat(double number, uint8_t digits);
};
