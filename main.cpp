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
    Biginteger::BigInteger c1 = a * b;
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Normal: " 
            << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
            << "ms\n";

    start = std::chrono::high_resolution_clock::now();
    Biginteger::BigInteger c4 = FFT_multiply(a, b);  // FFT
    end = std::chrono::high_resolution_clock::now();
    std::cout << "FFT_multiply: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
              << "ms\n";
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
    // 测试小数除法
    Biginteger::BigInteger five = Biginteger::from_string("10");
    Biginteger::BigInteger two = Biginteger::from_string("3");
    std::cout << "5 / 2 = " << Biginteger::divide_decimal(five, two, 3) << "\n"; // 输出2.5

    Biginteger::BigInteger neg = Biginteger::from_string("-123");
    Biginteger::BigInteger ten = Biginteger::from_string("10");
    std::cout << "-123 / 10 = " << Biginteger::divide_decimal(neg, ten, 1) << "\n"; // 输出-12.3
    // 表达式解析测试
    std::cout << "Parsing and evaluating expressions:" << std::endl;
    std::cout << "5 / 2 = " << Biginteger::evaluate_expression("5 / 2") << std::endl;       // 输出2.5
    std::cout << "123 + 456 = " << Biginteger::evaluate_expression("123 + 456") << std::endl; // 输出579
    std::cout << "-123 * 10 = " << Biginteger::evaluate_expression("-123 * 10") << std::endl; // 输出-1230
    std::cout << "100 - 50 = " << Biginteger::evaluate_expression("100 - 50") << std::endl;   // 输出50
    using namespace Biginteger;

    // 复杂表达式测试
    std::cout << "Test 1: 2 + 3 * 4 = "
              << evaluate_expression("2 + 3 * 4") << "\n"; // 输出14

    std::cout << "Test 2: (5 + 3) // 2 = "
              << evaluate_expression("(5 + 3) // 2") << "\n"; // 输出4

    std::cout << "Test 3: sum(10, 20, 30) = "
              << evaluate_expression("sum(10, 20, 30)") << "\n"; // 输出60

    std::cout << "Test 4: max(5, 3*4, 10-2) = "
              << evaluate_expression("max(5, 3*4, 10-2)") << "\n"; // 输出8

    std::cout << "Test 5: (100 // 3) / 4 = "
              << evaluate_expression("(100 // 3) / 4") << "\n"; // 输出8.3333333333

    return 0;
    return 0;
    // benchmark();
    // test_division();
    return 0;
}