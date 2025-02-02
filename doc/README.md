<!--
 * @Author: yuyuan 115848824+yuyuan12138@users.noreply.github.com
 * @Date: 2025-02-02 22:38:47
 * @LastEditors: yuyuan 115848824+yuyuan12138@users.noreply.github.com
 * @LastEditTime: 2025-02-02 22:39:05
 * @FilePath: \high-precision\doc\README.md
 * @Description: 
 * 
 * Copyright (c) ${Date} by ${115848824+yuyuan12138@users.noreply.github.com}, All Rights Reserved. 
-->

## Include
You need and only need to include the header like it:

    #include <BigInteger/biginteger.h>

## API

### Create BigIntegers
    std::string num1 = "1234";
    std::string num2 = "1234";
    Biginteger::BigInteger big_num1 = Biginteger::from_string(num1);
    Biginteger::BigInteger big_num2 = Biginteger::from_string(num2);

### Calculate
    