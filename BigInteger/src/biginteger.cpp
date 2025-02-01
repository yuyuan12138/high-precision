#include <Biginteger/biginteger.h>

namespace Biginteger{

    void pad_zeros(BigInteger& num, size_t target_len) {
        num.digits.resize(target_len, 0);
    }

    void add_with_avx512(BigInteger& result, const BigInteger& a, const BigInteger& b) {
        size_t max_len = std::max(a.digits.size(), b.digits.size());
        result.digits.resize(max_len + 1, 0);
        
        size_t i = 0;
        __m512i carry = _mm512_setzero_si512();
        
        for (; i + 16 <= max_len; i += 16) {
            __m512i va = _mm512_loadu_si512((__m512i*)&a.digits[i]);
            __m512i vb = _mm512_loadu_si512((__m512i*)&b.digits[i]);
            
            __m512i sum = _mm512_add_epi32(va, vb);
            sum = _mm512_add_epi32(sum, carry);
            
            __mmask16 carry_mask = _mm512_cmpgt_epi32_mask(sum, _mm512_set1_epi32(9));
            sum = _mm512_mask_sub_epi32(sum, carry_mask, sum, _mm512_set1_epi32(10));
            carry = _mm512_maskz_set1_epi32(carry_mask, 1);
            
            _mm512_storeu_si512((__m512i*)&result.digits[i], sum);
        }
        
        int carry_scalar = _mm512_mask_reduce_add_epi32(1, carry);
        for (; i < max_len; ++i) {
            int sum = carry_scalar;
            if (i < a.digits.size()) sum += a.digits[i];
            if (i < b.digits.size()) sum += b.digits[i];
            
            carry_scalar = sum / 10;
            result.digits[i] = sum % 10;
        }
        
        if (carry_scalar)
            result.digits[max_len] = carry_scalar;
        
        remove_leading_zeros(result);
    }

    void multiply_avx512(BigInteger& result, const BigInteger& a, const BigInteger& b) {
        result.digits.resize(a.digits.size() + b.digits.size(), 0);
        
        for (size_t i = 0; i < a.digits.size(); ++i) {
            __m512i carry = _mm512_setzero_si512();
            __m512i va = _mm512_set1_epi32(a.digits[i]);
            
            size_t j = 0;
            for (; j + 16 <= b.digits.size(); j += 16) {
                __m512i vb = _mm512_loadu_si512((__m512i*)&b.digits[j]);
                __m512i res = _mm512_loadu_si512((__m512i*)&result.digits[i + j]);
                
                __m512i product = _mm512_mullo_epi32(va, vb);
                res = _mm512_add_epi32(res, product);
                res = _mm512_add_epi32(res, carry);
                
                __mmask16 carry_mask = _mm512_cmpgt_epi32_mask(res, _mm512_set1_epi32(9));
                res = _mm512_mask_sub_epi32(res, carry_mask, res, _mm512_set1_epi32(10));
                carry = _mm512_maskz_set1_epi32(carry_mask, 1);
                
                _mm512_storeu_si512((__m512i*)&result.digits[i + j], res);
            }
            
            int carry_scalar = _mm512_mask_reduce_add_epi32(1, carry);
            for (; j < b.digits.size(); ++j) {
                int product = a.digits[i] * b.digits[j] + result.digits[i + j] + carry_scalar;
                carry_scalar = product / 10;
                result.digits[i + j] = product % 10;
            }
            
            if (carry_scalar)
                result.digits[i + b.digits.size()] = carry_scalar;
        }
        
        remove_leading_zeros(result);
    }

    BigInteger get_lower(const BigInteger& num, size_t n) {
        BigInteger result;
        result.digits.resize(std::min(n, num.digits.size()));
        std::copy(num.digits.begin(), 
                num.digits.begin() + result.digits.size(),
                result.digits.begin());
        return result;
    }

    BigInteger get_upper(const BigInteger& num, size_t n) {
        BigInteger result;
        if (num.digits.size() > n) {
            result.digits.resize(num.digits.size() - n);
            std::copy(num.digits.begin() + n,
                    num.digits.end(),
                    result.digits.begin());
        }
        return result;
    }

    BigInteger shift_left(const BigInteger& num, size_t shift) {
        if (num.digits.size() == 1 && num.digits[0] == 0)
            return num;

        BigInteger result;
        result.digits.resize(num.digits.size() + shift);
        std::copy(num.digits.begin(), num.digits.end(), result.digits.begin() + shift);
        return result;
    }

    BigInteger karatsuba(const BigInteger& a, const BigInteger& b) {
        size_t len = std::max(a.digits.size(), b.digits.size());
        
        if (len < KARATSUBA_THRESHOLD)
            return multiply_abs(a, b);

        // 补零对齐
        BigInteger a_copy = a;
        BigInteger b_copy = b;
        pad_zeros(a_copy, len);
        pad_zeros(b_copy, len);

        size_t m = len / 2;
        size_t m2 = len - m;

        BigInteger a_low = get_lower(a_copy, m);
        BigInteger a_high = get_upper(a_copy, m);
        BigInteger b_low = get_lower(b_copy, m);
        BigInteger b_high = get_upper(b_copy, m);

        BigInteger z0 = karatsuba(a_low, b_low);
        BigInteger z2 = karatsuba(a_high, b_high);
        
        BigInteger a_sum = a_low + a_high;
        BigInteger b_sum = b_low + b_high;
        BigInteger z1 = karatsuba(a_sum, b_sum) - z0 - z2;

        // 合并结果
        return shift_left(z2, 2*m) + shift_left(z1, m) + z0;
    }

    BigInteger karatsuba_avx512(const BigInteger& a, const BigInteger& b) {
        size_t len = std::max(a.digits.size(), b.digits.size());
        
        if (len < KARATSUBA_THRESHOLD) {
            BigInteger result;
            multiply_avx512(result, a, b);
            return result;
        }
        
        BigInteger a_copy = a;
        BigInteger b_copy = b;
        pad_zeros(a_copy, len);
        pad_zeros(b_copy, len);
        
        size_t m = len / 2;
        size_t m2 = len - m;
        
        BigInteger a_low = get_lower(a_copy, m);
        BigInteger a_high = get_upper(a_copy, m);
        BigInteger b_low = get_lower(b_copy, m);
        BigInteger b_high = get_upper(b_copy, m);
        
        BigInteger z0 = karatsuba_avx512(a_low, b_low);
        BigInteger z2 = karatsuba_avx512(a_high, b_high);
        
        BigInteger a_sum, b_sum;
        add_with_avx512(a_sum, a_low, a_high);
        add_with_avx512(b_sum, b_low, b_high);
        
        BigInteger z1 = karatsuba_avx512(a_sum, b_sum) - z0 - z2;
        
        return shift_left(z2, 2*m) + shift_left(z1, m) + z0;
    }

    void remove_leading_zeros(BigInteger& num)
    {
        while (num.digits.size() > 1 && num.digits.back() == 0)
            num.digits.pop_back();
        if (num.digits.empty()) num.digits.push_back(0);
    }

    int compare_abs(const BigInteger& a, const BigInteger& b) {
        if (a.digits.size() != b.digits.size())
            return a.digits.size() - b.digits.size();
        for (int i = a.digits.size()-1; i >= 0; --i) {
            if (a.digits[i] != b.digits[i])
                return a.digits[i] - b.digits[i];
        }
        return 0;
    }

    BigInteger create_from_string(const std::string& s) {
        BigInteger num;
        if (s.empty()) throw std::invalid_argument("Empty string");
        
        size_t start = 0;
        if (s[0] == '-' || s[0] == '+') {
            num.is_negative = (s[0] == '-');
            start = 1;
        }
        
        num.digits.resize(s.length() - start);
        for (size_t i = start; i < s.length(); ++i) {
            if (!isdigit(s[i])) throw std::invalid_argument("Invalid character");
            num.digits[s.length()-1 - i] = s[i] - '0';  
        }
        
        remove_leading_zeros(num);
        return num;
    }

    std::string to_string(const BigInteger& num) {
        std::string s;
        if (num.is_negative && !(num.digits.size() == 1 && num.digits[0] == 0))
            s += '-';
        for (auto it = num.digits.rbegin(); it != num.digits.rend(); ++it)
            s += std::to_string(*it);
        return s;
    }

    BigInteger add_abs(const Biginteger::BigInteger& a, const Biginteger::BigInteger& b) {
        BigInteger result;
        size_t max_len = std::max(a.digits.size(), b.digits.size());
        int carry = 0;

        for (size_t i = 0; i < max_len || carry; ++i) {
            int sum = carry;
            if (i < a.digits.size()) sum += a.digits[i];
            if (i < b.digits.size()) sum += b.digits[i];
            
            carry = sum / 10;
            result.digits.push_back(sum % 10);
        }
        return result;
    }

    BigInteger absolute(const BigInteger& num){
        BigInteger result = num;
        result.is_negative = false;
        return result;
    }

    BigInteger negate(const BigInteger& num){
        BigInteger result = num;
        if (!(result.digits.size() == 1 && result.digits[0] == 0)) {
            result.is_negative = !result.is_negative;
        }
        return result;
    }

    // abs(a) must >= abs(b)
    BigInteger sub_abs(const BigInteger& a, const BigInteger& b) {
        BigInteger result;
        int borrow = 0;

        for (size_t i = 0; i < a.digits.size(); ++i) {
            int sub = a.digits[i] - borrow;
            borrow = 0;
            
            if (i < b.digits.size()) sub -= b.digits[i];
            
            if (sub < 0) {
                sub += 10;
                borrow = 1;
            }
            result.digits.push_back(sub);
        }
        
        remove_leading_zeros(result);
        return result;
    }

    BigInteger create_from_longlong(long long x) {
        BigInteger num;
        if (x < 0) {
            num.is_negative = true;
            x = -x;
        }
        do {
            num.digits.push_back(x % 10);
            x /= 10;
        } while (x > 0);
        return num;
    }

    BigInteger operator+(const BigInteger& a, const BigInteger& b) {
        if (a.is_negative == b.is_negative) {
            BigInteger result = add_abs(a, b);
            result.is_negative = a.is_negative;
            return result;
        }
        
        // 异号处理：转换为减法
        const BigInteger& pos = a.is_negative ? b : a;
        const BigInteger& neg = a.is_negative ? a : b;
        
        int cmp = compare_abs(a, b);
        if (cmp == 0) return create_from_longlong(0);  // 相等时返回0
        
        if (cmp > 0) {
            BigInteger result = sub_abs(pos, absolute(neg));
            result.is_negative = false;
            return result;
        } else {
            BigInteger result = sub_abs(absolute(neg), pos);
            result.is_negative = true;
            return result;
        }
    }

    BigInteger operator-(const BigInteger& a, const BigInteger& b) {
        return a + negate(b);  // a - b = a + (-b)
    }

    bool operator<(const BigInteger& a, const BigInteger& b) {
        if (a.is_negative != b.is_negative)
            return a.is_negative;
        
        if (a.is_negative) {
            return compare_abs(b, a) < 0;  // 负数比较取反
        }
        return compare_abs(a, b) < 0;
    }

    bool operator==(const BigInteger& a, const BigInteger& b) {
        return a.is_negative == b.is_negative && a.digits == b.digits;
    }

    BigInteger multiply_abs(const BigInteger& a, const BigInteger &b){
        BigInteger result;
        result.digits.resize(a.digits.size() + b.digits.size(), 0);

        for (size_t i = 0; i < a.digits.size(); ++i) {
            int carry = 0;
            for (size_t j = 0; j < b.digits.size() || carry; ++j) {
                long long product = result.digits[i + j] + a.digits[i] * (j < b.digits.size() ? b.digits[j] : 0) + carry;
                result.digits[i + j] = product % 10;
                carry = product / 10;
            }
        }

        remove_leading_zeros(result);
        return result;
    }

    BigInteger operator*(const BigInteger& a, const BigInteger& b) {
        if ((a.digits.size() == 1 && a.digits[0] == 0) ||
            (b.digits.size() == 1 && b.digits[0] == 0)) {
            return create_from_longlong(0);
        }

        BigInteger result;
        if (a.digits.size() < KARATSUBA_THRESHOLD || 
            b.digits.size() < KARATSUBA_THRESHOLD) {
            result = multiply_abs(a, b);
        } else {
            result = karatsuba(a, b);
        }

        result.is_negative = a.is_negative != b.is_negative;
        remove_leading_zeros(result);
        
        if (result.digits.size() == 1 && result.digits[0] == 0) {
            result.is_negative = false;
        }
        return result;
    }

}