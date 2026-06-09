# 热更指令

```
windows: 
控制台输入fix并回车

linux(pid:进程id):
sh ./scripts/hotfix/hotfix.sh pid

```



# 热更插件集

* 采用数据和逻辑分离，数据定义在TestService中, 逻辑写在Plugin中

# 注意

* windows 下由于dll和exe程序集使用不同堆分配对象, 且项目大量使用static等，所以windows下Plugin是以静态库的形式插入到程序集中，不支持热更
* linux下由于堆空间只有一个所以可以支持通过dlopen方式热更Plugin
* 插件集不能定义任何可能在插件集中会被分配内存的对象, 因为会导致它的对象池在热更的时候无法卸载掉，后面会产生异常
* 插件集面相程序集的头文件不能动, 避免无法热更



# 不可热更的头文件接口（其他接口都可热更）

TestServicePlugin.h(面相程序集)

PluginEntry.h (面相程序集)

test_plugin_export.h(面相程序集)

