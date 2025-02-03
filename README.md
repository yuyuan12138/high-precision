# BigInteger Library Documentation

## Overview
The `BigInteger` library provides arbitrary-precision integer arithmetic operations, supporting basic arithmetic, fast multiplication algorithms (FFT), division, and modular operations. Numbers are stored in base-10 with **least-significant digit first** (e.g., "123" is stored as `[3, 2, 1]`). Negative numbers are fully supported.

---

## Data Structure
### `BigInteger`
```cpp
struct BigInteger {
    std::vector<int> digits;  // Least-significant digit first (e.g., "123" → [3,2,1])
    bool is_negative = false; // Sign flag (true for negative numbers)
};
```

---

## Core API Reference

### Constructors and Conversion

#### `from_string`
- **Description**: Converts a string to a `BigInteger`.
- **Parameters**: `const std::string& s` (valid numeric string, e.g., `"-123"`).
- **Exceptions**: Throws `std::invalid_argument` if the string contains non-digit characters.
- **Example**:
  ```cpp
  auto num = Biginteger::from_string("123456");     // Positive number
  auto neg_num = Biginteger::from_string("-789");   // Negative number
  ```

#### `to_string`
- **Description**: Converts a `BigInteger` to its string representation.
- **Returns**: Decimal string with optional sign.
- **Example**:
  ```cpp
  Biginteger::BigInteger num = ...;
  std::string s = Biginteger::to_string(num); // e.g., "-123456"
  ```

---

### Arithmetic Operators

#### `operator+`
- **Description**: Performs addition, supporting mixed positive/negative numbers.
- **Example**:
  ```cpp
  auto a = Biginteger::from_string("123");
  auto b = Biginteger::from_string("-456");
  auto result = a + b;  // Result: -333
  ```

#### `operator-`
- **Description**: Performs subtraction.
- **Example**:
  ```cpp
  auto a = Biginteger::from_string("579");
  auto b = Biginteger::from_string("123");
  auto result = a - b;  // Result: 456
  ```

#### `operator*`
- **Description**: Performs multiplication. Use FFT algorithm.
- **Example**:
  ```cpp
  auto a = Biginteger::from_string("123456789");
  auto b = Biginteger::from_string("987654321");
  auto result = a * b;  // Result: 121932631137021795
  ```

#### `operator/` and `operator%`
- **Description**: Division and modulo operations.
- **Example**:
  ```cpp
  auto dividend = Biginteger::from_string("12345");
  auto divisor = Biginteger::from_string("67");
  auto quotient = dividend / divisor;   // Result: 184
  auto remainder = dividend % divisor;  // Result: 17
  ```

---

### Fast Multiplication Algorithms

#### `FFT_multiply`
- **Description**: Multiplies two large integers using the Fast Fourier Transform (FFT) algorithm for optimal performance with very large numbers.
- **Example**:
  ```cpp
  auto a = Biginteger::from_string("12345678901234567890");
  auto b = Biginteger::from_string("98765432109876543210");
  auto result = Biginteger::FFT_multiply(a, b); // Direct FFT-based multiplication
  ```

#### `karatsuba`
- **Description**: Recursively multiplies two integers using the Karatsuba divide-and-conquer algorithm.
- **Example**:
  ```cpp
  auto a = ...; // Number with >32 digits
  auto b = ...;
  auto result = Biginteger::karatsuba(a, b); // Uses Karatsuba automatically
  ```

---

### Utility Functions

#### `compare_abs`
- **Description**: Compares the absolute values of two `BigInteger` objects.
- **Returns**: 
  - Positive value if `|a| > |b|`.
  - Negative value if `|a| < |b|`.
  - `0` if `|a| == |b|`.
- **Example**:
  ```cpp
  auto a = Biginteger::from_string("123");
  auto b = Biginteger::from_string("-456");
  int cmp = Biginteger::compare_abs(a, b); // Returns -333 (|123| < |456|)
  ```

#### `absolute`
- **Description**: Returns the absolute value of a `BigInteger`.
- **Example**:
  ```cpp
  auto num = Biginteger::from_string("-123");
  auto abs_num = Biginteger::absolute(num); // Result: 123
  ```

---

## Examples

### Example 1: Basic Arithmetic
```cpp
auto a = Biginteger::from_string("123");
auto b = Biginteger::from_string("-456");
std::cout << "a + b = " << Biginteger::to_string(a + b) << "\n"; // Output: -333
std::cout << "a * b = " << Biginteger::to_string(a * b) << "\n"; // Output: -56088
```

### Example 2: Division and Modulo
```cpp
auto dividend = Biginteger::from_string("12345");
auto divisor = Biginteger::from_string("67");
auto quotient = dividend / divisor;
auto remainder = dividend % divisor;
std::cout << "Quotient: " << Biginteger::to_string(quotient)   // 184
          << ", Remainder: " << Biginteger::to_string(remainder); // 17
```

### Example 3: Performance Benchmark
```cpp
void benchmark() {
    auto a = Biginteger::from_string(std::string(1000, '9')); // 1000-digit number
    auto b = Biginteger::from_string(std::string(1000, '9'));
    auto start = std::chrono::high_resolution_clock::now();
    auto result = Biginteger::FFT_multiply(a, b); // Use FFT multiplication
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "FFT time: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
              << "ms\n";
}
```

---

## Notes
1. **Negative Numbers**: Multiplication and division follow standard sign rules (e.g., `-a * -b = a * b`).
2. **Leading Zeros**: All operations automatically remove leading zeros from results.
3. **Performance**: 
   - For numbers with fewer than 32 digits, standard multiplication is used.
   - For larger numbers, Karatsuba or FFT algorithms are prioritized for efficiency.
4. **AVX-512 Support**: The `karatsuba_avx512` function (commented in code) leverages SIMD instructions for further optimization (requires compatible hardware).