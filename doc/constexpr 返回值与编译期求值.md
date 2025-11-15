如果函数返回值修饰**`constexpr`** ，那么如果函数参数传入的是常量就会在编译期运算并替换掉原来的位置，如果是变量，那么不会在编译期求值



# 关于编译期求值



**这取决于循环和条件语句的具体情况**。C++标准对 `constexpr` 函数中的控制流有明确规则：

## C++标准演进

### C++11 限制严格
```cpp
// C++11: constexpr 函数只能包含单个return语句
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);  // 只能递归，不能循环
}
```

### C++14+ 放宽限制
```cpp
// C++14+: 允许循环、if等
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 1; i <= n; ++i) {  // ✅ 允许循环
        result *= i;
    }
    return result;
}
```

## 编译期计算的条件

### ✅ 可以编译期计算的情况
```cpp
constexpr int compute(int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {          // ✅ if语句
            sum += i;
        }
    }
    return sum;
}

// 编译期计算 - 参数是编译期常量
constexpr int result = compute(10);  // ✅ 编译期计算
```

### ❌ 不能编译期计算的情况
```cpp
constexpr int problematic(int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        // 如果包含不能在编译期求值的操作
        static int counter = 0;    // ❌ 静态变量
        counter++;                 // ❌ 修改静态变量
        
        sum += i;
    }
    return sum;
}

constexpr int bad_example(int* ptr) {
    return *ptr;  // ❌ 解引用指针（除非指针指向编译期已知内存）
}
```

## 你的 `_Mod` 函数分析

```cpp
ALWAYS_INLINE constexpr size_t _Mod(size_t i) const noexcept { 
    return i & (CapacitySize - 1); 
}
```

这个函数**肯定可以在编译期计算**，因为：
- 只有简单的位运算
- 没有循环或条件语句
- 所有操作都是编译期可求值的

## 带循环/if的constexpr函数示例

```cpp
// ✅ 有效的constexpr函数（C++14+）
constexpr size_t find_index(const char* str, char target) {
    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (str[i] == target) {
            return i;
        }
    }
    return static_cast<size_t>(-1);
}

// 编译期使用
constexpr size_t idx = find_index("hello", 'e');  // ✅ 编译期计算：idx = 1

// 运行时使用  
char input[] = "runtime";
size_t idx2 = find_index(input, 't');  // ✅ 运行时计算
```

## 关键限制

`constexpr` 函数中**不允许**的操作：
- **静态变量**（static variables）
- **动态内存分配**（new/delete）
- **异常**（throw，除非被捕获且不抛出）
- **虚函数**
- **goto语句**
- **非constexpr函数调用**
- **非常量表达式的变量修改**

## 总结

**有循环和if的constexpr函数仍然可以在编译期计算，只要：**

1. **所有参数都是编译期常量**
2. **循环边界在编译期可知**
3. **不违反constexpr函数的限制规则**
4. **所有操作都是编译期可求值的**

你的 `_Mod` 函数非常简单，完全符合这些条件，所以 `constexpr` 会正常发挥作用。