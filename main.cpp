#include <BigInteger/biginteger.h>
#include <chrono>

void benchmark() {

    std::string big_num1(15, '0');
    std::string big_num2(15, '0');
    for (auto& c : big_num1) c = '0' + rand()%10;
    for (auto& c : big_num2) c = '0' + rand()%10;
    
    Biginteger::BigInteger a = Biginteger::from_string(big_num1);
    Biginteger::BigInteger b = Biginteger::from_string(big_num2);
    
    auto start = std::chrono::high_resolution_clock::now();
    Biginteger::BigInteger c1 = Biginteger::multiply_abs(a, b);  // 普通乘法
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Normal: " 
            << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
            << "ms\n";
    
    start = std::chrono::high_resolution_clock::now();
    Biginteger::BigInteger c2 = Biginteger::karatsuba(a, b);    // Karatsuba
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Karatsuba: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
              << "ms\n";

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
    
    for(auto& x: c1.digits){
        std::cout << x;
    }
    std::cout << std::endl;
    for(auto& x: c2.digits){
        std::cout << x;
    }
    std::cout << std::endl;
    for(auto& x: c4.digits){
        std::cout << x;
    }
    std::cout << std::endl;
    std::cout << Biginteger::to_longlong(c1);
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
    return 0;
}