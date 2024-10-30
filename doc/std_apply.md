`std::apply` 是 C++17 引入的一个函数模板，定义在头文件 `<tuple>` 中。它用于将一个可调用对象（如函数、函数对象、lambda 表达式等）应用于一个 `std::tuple` 中的所有元素。换句话说，`std::apply` 可以将 `std::tuple` 展开为参数列表，并传递给一个可调用对象。

```
template< class F, class Tuple >  
constexpr auto apply( F&& f, Tuple&& t );

```

- `F` 是可调用对象的类型。
- `Tuple` 是 `std::tuple` 或其他包含多个元素的类似容器类型。
- `f` 是要调用的可调用对象。
- `t` 是包含参数的 `std::tuple`。

`std::apply` 返回调用 `f` 的结果。