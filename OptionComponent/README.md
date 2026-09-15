# 可选组件

可选组件可以按需加到工程中



# 要求

1. 所有组件均继承于:KERNEL_NS::CompHostObject或KERNEL_NS::CompObject

2. 组件代码目录结构:/Impl, /Interface

   a. Impl: 组件实现目录, 必须包含组件对象实现, 组件对象工厂类

   b. Interface: 接口目录, 必须包含组件接口类

   c. 组件的根目录下必须向外暴露组件对外公开的头文件, 该头文件必须include组件的接口类路径, 以及组件的工厂类路径

3. 所有组件的文件相对路径都是如下开始: OptionComp/

