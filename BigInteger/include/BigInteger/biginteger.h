#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <immintrin.h>
#include <cmath>
#include <complex>
#include <limits>

namespace Biginteger{
    
    
    struct BigInteger {
        std::vector<int> digits;  // 低位在前存储
        bool is_negative = false;
    };

    const size_t KARATSUBA_THRESHOLD = 32;
    const size_t FFT_THRESHOLD = 64;
    const double PI = acos(-1.0);

    void fft(std::vector<std::complex<double>>& a, bool inv);
    void remove_leading_zeros(BigInteger& num);
    void pad_zeros(BigInteger& num, size_t target_len);
    BigInteger get_lower(const BigInteger& num, size_t n);
    BigInteger get_upper(const BigInteger& num, size_t n);
    BigInteger shift_left(const BigInteger& num, size_t shift);

    int compare_abs(const BigInteger& a, const BigInteger& b);
    BigInteger from_longlong(long long x = 0);

    BigInteger absolute(const BigInteger& num);
    BigInteger negate(const BigInteger& num);

    BigInteger from_string(const std::string& s);
    // std::string to_string(const BigInteger& num);

    BigInteger add_abs(const Biginteger::BigInteger& a, const Biginteger::BigInteger& b);
    BigInteger sub_abs(const BigInteger& a, const BigInteger& b);
    BigInteger multiply_abs(const BigInteger& a, const BigInteger &b);
    BigInteger karatsuba(const BigInteger& a, const BigInteger& b);
    BigInteger karatsuba_avx512(const BigInteger& a, const BigInteger& b);
    BigInteger FFT_multiply(BigInteger a, BigInteger b);
    BigInteger divide(const BigInteger& dividend, const BigInteger& divisor, BigInteger& remainder);

    BigInteger operator+(const BigInteger& a, const BigInteger& b);
    BigInteger operator-(const BigInteger& a, const BigInteger& b);
    BigInteger operator*(const BigInteger& a, const BigInteger& b);
    bool operator<(const BigInteger& a, const BigInteger& b);
    bool operator==(const BigInteger& a, const BigInteger& b);
    BigInteger operator/(const BigInteger& a, const BigInteger& b);
    BigInteger operator%(const BigInteger& a, const BigInteger& b);

    void add_with_avx512(BigInteger& result, const BigInteger& a, const BigInteger& b);
    void multiply_avx512(BigInteger& result, const BigInteger& a, const BigInteger& b);
    
    long long to_longlong(BigInteger& a);
    std::string to_string(const BigInteger& num);
}

