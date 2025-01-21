# windows

使用cmake直接默认配置与编译

# Linux

需要在cmakelist中添加：

```
# 添加 -fPIC 编译选项
target_compile_options(miniz PRIVATE -fPIC)
```

生成位置无关代码

