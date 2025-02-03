#include <BigInteger/biginteger.h>
#include <chrono>
#include <cassert>

void benchmark() {

    std::string big_num1(1000, '0');
    std::string big_num2(1000, '0');
    for (auto& c : big_num1) c = '0' + rand()%10;
    for (auto& c : big_num2) c = '0' + rand()%10;
    
    Biginteger::BigInteger a = Biginteger::from_string(big_num1);
    Biginteger::BigInteger b = Biginteger::from_string(big_num2);
    
    auto start = std::chrono::high_resolution_clock::now();
    // Biginteger::BigInteger c1 = Biginteger::multiply_abs(a, b);  // 普通乘法
    // Biginteger::BigInteger c1 = a * b;
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Normal: " 
            << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
            << "ms\n";
    
    // start = std::chrono::high_resolution_clock::now();
    // Biginteger::BigInteger c2 = Biginteger::karatsuba(a, b);    // Karatsuba
    // end = std::chrono::high_resolution_clock::now();
    // std::cout << "Karatsuba: " 
    //           << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
    //           << "ms\n";

    // start = std::chrono::high_resolution_clock::now();
    // Biginteger::BigInteger c3 = karatsuba_avx512(a, b);  // Karatsuba + AVX-512
    // end = std::chrono::high_resolution_clock::now();
    // std::cout << "Karatsuba+AVX512: " 
    //           << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
    //           << "ms\n";

    start = std::chrono::high_resolution_clock::now();
    Biginteger::BigInteger c4 = FFT_multiply(a, b);  // FFT
    end = std::chrono::high_resolution_clock::now();
    std::cout << "FFT_multiply: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
              << "ms\n";
    
    // for(auto& x: c1.digits){
    //     std::cout << x;
    // }
    // std::cout << std::endl;
    // for(auto& x: c2.digits){
    //     std::cout << x;
    // }
    // std::cout << std::endl;
    // for(auto& x: c4.digits){
    //     std::cout << x;
    // }
    std::cout << std::endl;
    // std::cout << Biginteger::to_longlong(c1);
}

void test_division() {

    // 测试1: 12345 ÷ 67 = 184 余 17
    Biginteger::BigInteger a = Biginteger::from_string("12345");
    Biginteger::BigInteger b = Biginteger::from_string("67");
    Biginteger::BigInteger rem;
    Biginteger::BigInteger quo = Biginteger::divide(a, b, rem);
    
    assert(Biginteger::to_string(quo) == "184");
    assert(Biginteger::to_string(rem) == "17");

    // 测试2: 100 ÷ 2 = 50 余 0
    Biginteger::BigInteger c = Biginteger::from_string("100");
    Biginteger::BigInteger d = Biginteger::from_string("2");
    quo = Biginteger::divide(c, d, rem);
    
    assert(Biginteger::to_string(quo) == "50");
    assert(Biginteger::to_string(rem) == "0");

    // 测试3: 999 ÷ 3 = 333 余 0
    Biginteger::BigInteger e = Biginteger::from_string("999");
    Biginteger::BigInteger f = Biginteger::from_string("3");
    quo = Biginteger::divide(e, f, rem);
    assert(Biginteger::to_string(quo) == "333");
    assert(Biginteger::to_string(rem) == "0");

    // 测试4: 符号处理 (-123) ÷ 10 = -12 余 -3
    Biginteger::BigInteger g = Biginteger::from_string("-123");
    Biginteger::BigInteger h = Biginteger::from_string("10");
    quo = Biginteger::divide(g, h, rem);
    
    assert(Biginteger::to_string(quo) == "-12");
    assert(Biginteger::to_string(rem) == "-3");
    // 测试5: 0 ÷ 5 = 0 余 0
    Biginteger::BigInteger zero = Biginteger::from_string("0");
    Biginteger::BigInteger five = Biginteger::from_string("5");
    quo = Biginteger::divide(zero, five, rem);
    assert(Biginteger::to_string(quo) == "0");
    assert(Biginteger::to_string(rem) == "0");


}

int main() {
    // 测试用例
    Biginteger::BigInteger a = Biginteger::from_string("123");
    Biginteger::BigInteger b = Biginteger::from_string("-456");
    Biginteger::BigInteger c = Biginteger::from_string("-123");
    Biginteger::BigInteger d = Biginteger::from_string("456");

    // 测试异号加法
    std::cout << "123 + (-456) = " << Biginteger::to_string(a + b) << "\n";  // 应输出-333
    std::cout << "-123 + 456 = " << Biginteger::to_string(c + d) << "\n";    // 应输出333

    // 测试异号减法
    std::cout << "123 - (-456) = " << Biginteger::to_string(a - b) << "\n";  // 应输出579
    std::cout << "-123 - 456 = " << Biginteger::to_string(c - d) << "\n";    // 应输出-579

    // 测试边界情况
    Biginteger::BigInteger e = Biginteger::from_string("-100");
    Biginteger::BigInteger f = Biginteger::from_string("100");
    std::cout << "-100 + 100 = " << Biginteger::to_string(e + f) << "\n";  // 应输出0
    std::cout << "100 - (-100) = " << Biginteger::to_string(f - e) << "\n"; // 应输出200

    std::cout << "-100 * 100 = " << Biginteger::to_string(e * f) << "\n";

    Biginteger::BigInteger g = Biginteger::from_string("12345678901234567890");
    Biginteger::BigInteger h = Biginteger::from_string("98765432109876543210");
    std::cout << "Karatsuba: " 
              << Biginteger::to_string(g * h) << "\n";
    benchmark();
    test_division();
    return 0;
}