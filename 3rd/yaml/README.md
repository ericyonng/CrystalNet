* yaml库需要在引入的项目中添加 YAML_CPP_STATIC_DEFINE 宏，否则会出现链接错误
* 测试用例见testsuit\testsuit\testinst\TestYaml.h,  testsuit\testsuit\testinst\TestYaml.cpp

```
void TestYaml::Run()
{
    auto &&progPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    auto testYamlPath = progPath + "/" + "test.yaml";
    auto config = YAML::LoadFile(testYamlPath.c_str());
}
```

* 编译yaml库

* ```
  windows下：
  cmake设置代码路径和build路径
  直接config, generate, build
  
  linux 下:
  mkdir build
  cd build
  cmake [-G generator] [-DYAML_BUILD_SHARED_LIBS=OFF] ..
  make && make install
  ```

  