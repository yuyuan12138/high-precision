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
        std::copy_n(num.digits.begin(),
                result.digits.size(),
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
        const size_t len = std::max(a.digits.size(), b.digits.size());
        
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
        const size_t len = std::max(a.digits.size(), b.digits.size());
        
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

    BigInteger from_string(const std::string& s) {
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
        
        // std::reverse(s.begin(), s.end());
        return s;
    }

    BigInteger add_abs(const BigInteger& a, const BigInteger& b) {
        BigInteger result;
        size_t max_len = std::max(a.digits.size(), b.digits.size());
        result.digits.resize(max_len + 1, 0); // 预分配
        
        int carry = 0;
        for (size_t i = 0; i < max_len || carry; ++i) {
            int sum = carry;
            if (i < a.digits.size()) sum += a.digits[i];
            if (i < b.digits.size()) sum += b.digits[i];
            
            carry = sum / 10;
            result.digits[i] = sum % 10; // 直接通过索引赋值
        }
        
        remove_leading_zeros(result);
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

    BigInteger from_longlong(long long x) {
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
        if (cmp == 0) return from_longlong(0);  // 相等时返回0
        
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
            return from_longlong(0);
        }

        BigInteger result;
        const size_t n = a.digits.size() + b.digits.size();
        // if (a.digits.size() < KARATSUBA_THRESHOLD || 
        //     b.digits.size() < KARATSUBA_THRESHOLD) {
        //     result = multiply_abs(a, b);
        // } else if(a.digits.size() >= FFT_THRESHOLD &&
        //             b.digits.size() >= FFT_THRESHOLD){
        //     result = FFT_multiply(a, b);
        // }else{
        //     // result = karatsuba(a, b);
        //     result = FFT_multiply(a, b);
        // }
        if (n < FFT_THRESHOLD) {
            result = multiply_abs(a, b);
        }else {
            result = FFT_multiply(a, b);
        }


        result.is_negative = a.is_negative != b.is_negative;
        remove_leading_zeros(result);
        
        if (result.digits.size() == 1 && result.digits[0] == 0) {
            result.is_negative = false;
        }
        return result;
    }

    void fft(std::vector<std::complex<double>>& a, bool inv){
        const int n = a.size();
        if (n == 1) {
            return;
        }

        std::vector<std::complex<double>> a0(n / 2), a1(n / 2);
        for (int i = 0, j = 0; i < n; i += 2, j++) {
            a0[j] = a[i];
            a1[j] = a[i + 1];
        }
        fft(a0, inv);
        fft(a1, inv);
        
        //FFT
        const double angle = 2 * PI / n * (inv ? -1 : 1);
        std::complex<double> w(1.0, 0.0);
        const std::complex<double> wn(cos(angle), sin(angle));
        for (int i = 0; i < n / 2; i++) {
            a[i] = a0[i] + w * a1[i];
            a[i + n / 2] = a0[i] - w * a1[i];
            w *= wn;
        }
    }

    BigInteger FFT_multiply(BigInteger a, BigInteger b){
        if ((a.digits.size() == 1 && a.digits[0] == 0) ||
            (b.digits.size() == 1 && b.digits[0] == 0)) {
            return from_longlong(0);
        }

        int n = 1;
        while (n < a.digits.size() + b.digits.size()) {
            n *= 2;
        }

        a.digits.resize(n); b.digits.resize(n);
        std::vector<std::complex<double>> c(n), d(n);

        for (int i = 0; i < n; i++) {
            c[i] = std::complex<double>(a.digits[i], 0);
            d[i] = std::complex<double>(b.digits[i], 0);
        }

        fft(c, false), fft(d, false);
        for (int i = 0; i < n; i++) {
            c[i] *= d[i];
        }
        
        fft(c, true);
        
        std::vector<int> res(n);
        int carry = 0;
        for (int i = 0; i < n; i++) {
            res[i] = (int)(c[i].real() / n + 0.5);
            res[i] += carry;
            carry = res[i] / 10;
            res[i] %= 10;
        }
        
        // int carry = 0;
        // for (int i = 0; i < n; i++) {
        //     res[i] += carry;
        //     carry = res[i] / 10;
        //     res[i] %= 10;
        // }
        
        while (res.size() > 1 && res.back() == 0) {
            res.pop_back();
        }
        BigInteger result;
        result.digits = res;
        if (a.is_negative + b.is_negative == 1){
            result.is_negative = true;
        }else{
            result.is_negative = false;
        }
        return result;
    }

    long long to_longlong(BigInteger& a){
        remove_leading_zeros(a);
        long long result = 0;

        std::string s_a = to_string(a);
        try
        {
            long long tmp = std::stoll(s_a);
            if ( std::numeric_limits<long long>::max() < tmp )
            {
                throw std::out_of_range( "Too big number!" );
            }
            result = tmp;
        }
        catch ( const std::out_of_range &e )
        {
            std::cout << e.what() << '\n';
        }

        return result;
    }

    // std::string to_string(BigInteger& a){
    //     std::string result;
        
    //     for(auto& x: a.digits){
    //         result.push_back(char(x) + '0');
    //     }
    //     if(a.is_negative){
    //         result.push_back('-');
    //     }
    //     std::reverse(result.begin(), result.end());
    //     return result;
    // }

    const BigInteger BigIntegerZero = from_longlong(0);

    BigInteger divide(const BigInteger& dividend, const BigInteger& divisor, BigInteger& remainder) {
        if (compare_abs(divisor, BigIntegerZero) == 0) {
            throw std::invalid_argument("Division by zero");
        }

        bool quotient_negative = dividend.is_negative != divisor.is_negative;
        BigInteger a = absolute(dividend);
        BigInteger b = absolute(divisor);

        if (compare_abs(a, b) < 0) {
            remainder = a;
            remainder.is_negative = dividend.is_negative;
            return BigIntegerZero;
        }

        std::vector<int> a_digits_high(a.digits.rbegin(), a.digits.rend());
        std::vector<int> b_digits_high(b.digits.rbegin(), b.digits.rend());

        std::vector<int> quotient_digits_high;
        remainder = BigIntegerZero;

        for (int digit : a_digits_high) {
            remainder.digits.insert(remainder.digits.begin(), 0); // 乘以10
            remainder.digits[0] = digit; // 加当前位
            remove_leading_zeros(remainder);

            int q = 0;
            if (compare_abs(remainder, b) >= 0) {
                int low = 0, high = 9;
                while (low <= high) {
                    int mid = (low + high) / 2;
                    BigInteger product = multiply_abs(b, from_longlong(mid));
                    if (compare_abs(product, remainder) <= 0) {
                        q = mid;
                        low = mid + 1;
                    } else {
                        high = mid - 1;
                    }
                }
                BigInteger product = multiply_abs(b, from_longlong(q));
                remainder = sub_abs(remainder, product);
            }
            quotient_digits_high.push_back(q);
        }

        std::ranges::reverse(quotient_digits_high);
        // while (quotient_digits_high.size() > 1 && quotient_digits_high.back() == 0) {
        //     quotient_digits_high.pop_back();
        // }
        // reverse(quotient_digits_high.begin(), quotient_digits_high.end());
        
        BigInteger quotient;
        quotient.digits = quotient_digits_high;
        quotient.is_negative = quotient_negative;
        remove_leading_zeros(quotient);

        // 设置余数的符号
        remainder.is_negative = dividend.is_negative;
        remove_leading_zeros(remainder);

        return quotient;
    }

    BigInteger operator/(const BigInteger& a, const BigInteger& b) {
        BigInteger remainder;
        return divide(a, b, remainder);
    }
    BigInteger operator%(const BigInteger& a, const BigInteger& b) {
        BigInteger remainder;
        divide(a, b, remainder);
        return remainder;
    }

    BigInteger multiply_by_10(const BigInteger& num) {
        BigInteger result = num;
        result.digits.insert(result.digits.begin(), 0);
        remove_leading_zeros(result);
        return result;
    }

    std::string divide_decimal(const BigInteger& a, const BigInteger& b, int precision) {
        if (compare_abs(b, BigIntegerZero) == 0) {
            throw std::invalid_argument("Division by zero");
        }

        bool result_negative = a.is_negative != b.is_negative;

        BigInteger a_abs = absolute(a);
        BigInteger b_abs = absolute(b);

        BigInteger quotient, remainder;
        quotient = divide(a_abs, b_abs, remainder);

        std::string result;

        if (result_negative && !(quotient.digits.size() == 1 && quotient.digits[0] == 0 && remainder.digits.size() == 1 && remainder.digits[0] == 0)) {
            result += "-";
        }

        result += to_string(quotient);

        if (!(remainder.digits.size() == 1 && remainder.digits[0] == 0) && precision > 0) {
            result += ".";

            BigInteger remainder_abs = absolute(remainder);

            for (int i = 0; i < precision; ++i) {
                remainder_abs = multiply_by_10(remainder_abs);
                BigInteger temp_quotient, temp_remainder;
                temp_quotient = divide(remainder_abs, b_abs, temp_remainder);
                result += to_string(temp_quotient);
                remainder_abs = temp_remainder;

                if (remainder_abs.digits.size() == 1 && remainder_abs.digits[0] == 0) {
                    break;
                }
            }
        }

        return result;
    }
    BigInteger integer_divide(const BigInteger& a, const BigInteger& b) {
        BigInteger rem;
        return divide(a, b, rem); // 直接返回商部分
    }

    // 求和函数
    BigInteger sum(const std::vector<BigInteger>& nums) {
        if (nums.empty()) return from_longlong(0);
        BigInteger total = nums[0];
        for (size_t i = 1; i < nums.size(); ++i) {
            total = total + nums[i];
        }
        return total;
    }

    // 最大值函数
    BigInteger max(const std::vector<BigInteger>& nums) {
        if (nums.empty()) throw std::invalid_argument("max() requires at least one argument");
        BigInteger current_max = nums[0];
        for (const auto& num : nums) {
            if (num > current_max) current_max = num;
        }
        return current_max;
    }

    // 解析参数列表（支持嵌套表达式）
    std::vector<BigInteger> parse_argument_list(const std::vector<std::string>& tokens, size_t& index) {
        std::vector<BigInteger> args;
        if (tokens[index] != "(") throw std::invalid_argument("Expected '('");
        ++index;
        while (tokens[index] != ")") {
            args.push_back(eval(tokens, index));
            if (tokens[index] == ",") ++index;
        }
        ++index; // 跳过 ")"
        return args;
    }

    // 解析函数调用（如 sum(1, 2)）
    BigInteger parse_function_call(const std::string& func_name, const std::vector<std::string>& tokens, size_t& index) {
        if (func_name == "sum") {
            auto args = parse_argument_list(tokens, index);
            return sum(args);
        } else if (func_name == "max") {
            auto args = parse_argument_list(tokens, index);
            return max(args);
        } else {
            throw std::invalid_argument("Unknown function: " + func_name);
        }
    }

    // 解析基本元素（数字、括号、函数）
    BigInteger parse_primary(const std::vector<std::string>& tokens, size_t& index) {
        if (tokens[index] == "(") {
            ++index;
            auto val = eval(tokens, index);
            if (tokens[index] != ")") throw std::invalid_argument("Expected ')'");
            ++index;
            return val;
        } else if (isdigit(tokens[index][0]) || tokens[index][0] == '-') { // 数字
            return from_string(tokens[index++]);
        } else if (isalpha(tokens[index][0])) { // 函数调用
            std::string func_name = tokens[index++];
            return parse_function_call(func_name, tokens, index);
        } else {
            throw std::invalid_argument("Unexpected token: " + tokens[index]);
        }
    }

    // 解析乘除、整除
    BigInteger parse_term(const std::vector<std::string>& tokens, size_t& index) {
        auto left = parse_primary(tokens, index);
        while (index < tokens.size()) {
            std::string op = tokens[index];
            if (op == "*" || op == "/" || op == "//") {
                ++index;
                auto right = parse_primary(tokens, index);
                if (op == "*") {
                    left = left * right;
                } else if (op == "/") {
                    left = divide_decimal(left, right, 10); // 默认保留10位小数
                } else if (op == "//") {
                    left = integer_divide(left, right);
                }
            } else {
                break;
            }
        }
        return left;
    }

    // 解析加减
    BigInteger parse_expr(const std::vector<std::string>& tokens, size_t& index) {
        auto left = parse_term(tokens, index);
        while (index < tokens.size()) {
            std::string op = tokens[index];
            if (op == "+" || op == "-") {
                ++index;
                auto right = parse_term(tokens, index);
                left = (op == "+") ? (left + right) : (left - right);
            } else {
                break;
            }
        }
        return left;
    }

    // 主解析函数
    BigInteger eval(const std::vector<std::string>& tokens, size_t& index) {
        return parse_expr(tokens, index);
    }

    // 公开的表达式解析接口
    BigInteger evaluate_expression(const std::string& expr) {
        // 分词处理（支持复杂表达式）
        std::vector<std::string> tokens;
        std::string token;
        for (char c : expr) {
            if (isspace(c)) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else if (c == '(' || c == ')' || c == ',') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
                tokens.push_back(std::string(1, c));
            } else if (c == '+' || c == '-' || c == '*' || c == '/') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
                tokens.push_back(std::string(1, c));
            } else {
                token += c;
            }
        }
        if (!token.empty()) tokens.push_back(token);

        size_t index = 0;
        return eval(tokens, index);
    }
}