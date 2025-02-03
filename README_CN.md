# BigInteger库文档

## 概述
该库提供任意精度整数（`BigInteger`）的实现，支持基本算术运算、快速乘法算法（FFT）和除法运算。数字以十进制存储，低位在前，支持负数操作。

---

## 数据结构
### `BigInteger`
```cpp
struct BigInteger {
    std::vector<int> digits;  // 低位在前，例如数字"123"存储为[3,2,1]
    bool is_negative = false; // 是否为负数
};
```

---

## API 详解

### 构造函数与转换函数

#### `from_string`
- **功能**：将字符串转换为`BigInteger`。
- **参数**：`const std::string& s`（有效数字字符串，如`"-123"`）。
- **异常**：若字符串包含非数字字符，抛出`std::invalid_argument`。
- **示例**：
  ```cpp
  auto num = Biginteger::from_string("123456");  // 正数
  auto neg_num = Biginteger::from_string("-789");// 负数
  ```

#### `to_string`
- **功能**：将`BigInteger`转换为字符串。
- **返回**：十进制字符串。
- **示例**：
  ```cpp
  Biginteger::BigInteger num = ...;
  std::string s = Biginteger::to_string(num); // 如 "-123456"
  ```

---

### 算术运算符

#### `operator+`
- **功能**：加法运算，支持正负数混合计算。
- **示例**：
  ```cpp
  auto a = Biginteger::from_string("123");
  auto b = Biginteger::from_string("-456");
  auto result = a + b;  // 结果为-333
  ```

#### `operator-`
- **功能**：减法运算。
- **示例**：
  ```cpp
  auto a = Biginteger::from_string("579");
  auto b = Biginteger::from_string("123");
  auto result = a - b;  // 结果为456
  ```

#### `operator*`
- **功能**：乘法运算，使用FFT算法。
- **示例**：
  ```cpp
  auto a = Biginteger::from_string("123456789");
  auto b = Biginteger::from_string("987654321");
  auto result = a * b;  // 结果为121932631137021795
  ```

#### `operator/` 和 `operator%`
- **功能**：除法和取余运算。
- **示例**：
  ```cpp
  auto dividend = Biginteger::from_string("12345");
  auto divisor = Biginteger::from_string("67");
  auto quotient = dividend / divisor;  // 184
  auto remainder = dividend % divisor; // 17
  ```

---

### 快速乘法算法

#### `FFT_multiply`
- **功能**：使用快速傅里叶变换（FFT）实现的高效乘法，适用于超大数。
- **示例**：
  ```cpp
  auto a = Biginteger::from_string("12345678901234567890");
  auto b = Biginteger::from_string("98765432109876543210");
  auto result = Biginteger::FFT_multiply(a, b); // 直接调用FFT乘法
  ```

#### `karatsuba`
- **功能**：Karatsuba分治算法，递归实现。
- **示例**：
  ```cpp
  auto a = ...; // 超过32位的数
  auto b = ...;
  auto result = Biginteger::karatsuba(a, b); // 自动调用Karatsuba
  ```

---

### 工具函数

#### `compare_abs`
- **功能**：比较两个`BigInteger`的绝对值大小。
- **返回**：正数表示`a > b`，负数表示`a < b`，0表示相等。
- **示例**：
  ```cpp
  auto a = Biginteger::from_string("123");
  auto b = Biginteger::from_string("-456");
  int cmp = Biginteger::compare_abs(a, b); // 结果-333（绝对值123 < 456）
  ```

---

## 示例代码

### 示例1：基本运算
```cpp
auto a = Biginteger::from_string("123");
auto b = Biginteger::from_string("-456");
std::cout << "a + b = " << Biginteger::to_string(a + b) << "\n"; // 输出-333
std::cout << "a * b = " << Biginteger::to_string(a * b) << "\n"; // 输出-56088
```

### 示例2：除法与取余
```cpp
auto dividend = Biginteger::from_string("12345");
auto divisor = Biginteger::from_string("67");
auto quotient = dividend / divisor;
auto remainder = dividend % divisor;
std::cout << "商: " << Biginteger::to_string(quotient)   // 184
          << ", 余数: " << Biginteger::to_string(remainder); // 17
```

### 示例3：性能测试
```cpp
void benchmark() {
    auto a = Biginteger::from_string(std::string(1000, '9')); // 1000位大数
    auto b = Biginteger::from_string(std::string(1000, '9'));
    auto start = std::chrono::high_resolution_clock::now();
    auto result = Biginteger::FFT_multiply(a, b); // 使用FFT乘法
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "FFT耗时: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
              << "ms\n";
}
```

---

## 注意事项
1. **负数处理**：乘法和除法的符号遵循数学规则（异号得负，同号得正）。
2. **前导零**：所有运算后会自动去除前导零。
3. **性能**：对于超过32位的乘法，优先使用Karatsuba或FFT算法以提升速度。