# 导表工具说明

* 使用说明

  * Xlsx格式

    * 文件名:平台的文件名

    * xlsx内的sheet名: 配置的描述内容|生成的配置名称，且生成的配置名称首字母必须大写，例如:样例配置|Example

    * xlsx内容格式：

      * 第一行是选择导出的服务端/客户端版本

      * 第二行是字段名

      * 第三行是数据类型

      * 第四行是校验

      * 第五行是unique/index等方便构建索引以及相关索引的配置接口

      * 第六行是默认值

      * 第七行是描述信息

      * 第八行开始都是数据

      * 第一列是用来做特殊功能的，比如#表示注释单行（本行不导出数据），### ### 是第一个###开始到第二个###所在行结束都不导出数据

      * ```
        客户端/服务端		C|S	C|S	C|S
        字段名			  Id	Type	title
        数据类型	     int32	int32	string
        校验	          check_not_empty()		
        flags	        unique	index	
        默认值	          0		
        描述信息	     id	类型:1, 2, 3	称号，文字id
        #	11	1	"CHU_JI"
        	12	1	"ZHONG_JI"
        	13	1	"GAO_JI"
        ###	14	2	"DA_WANG"
        	15	2	"QIAN_NIAN_LAO_LIU"
        	16	2	"WEI_SUO_XIAN_SHENG"
        	17	2	"MAI_SE_QING_DE"
        ###	18	2	"LA_PI_TIAO_DE"
        	19	3	"BAO_AN"
        	20	3	"GUO_JIA_LING_XIU"
        
        ```

    * 支持的数据类型：string, int8, uint8, int16, uint16, int32, uint32, int64, uint64, array[int],  array[string], array[dict<int, int>] dict<int, int>

      * array[xxx]:xxx表示array数组的元素类型, array数据需要符合json格式
      * dict<key, value> key:表示字典类型的键数据类型， value是字典值类型，key只支持简单的数据类型，如:string, number等，且dict的数据需要符合json格式

  * 工具参数

    * --config 参数用来指定导出的配置文件的扩展名

    * --lang参数用来指定S:服务端导出的语言版本，C:客户端导出的语言版本

    * --source_dir:用来指定配置文件的根路径

    * --target_dir:用来指定生成的代码的根路径

    * --data:用来指定生成的数据的根路径

    * --meta:用来指定各个xlsx对应的meta文件的路径

    * ```
      --config=xlsx --lang=S:cpp|C:C#,lua --source_dir=../../service_common/config/xlsx --target_dir=./code --data=./data --meta=./meta
      ```

  * 待实现的功能

    * 导表工具中校验语法解析的功能尚未实现，后面会抽空实现

* 测试
  * 多个表合并表头必须一致
    * owntype是否一样
    * fieldName是否一样
    * dataType数据类型是否一样
    * 校验是否一样
    * flags是否一样
    
  * 测试参数

    ```
    --config=xlsx --lang=S:cpp|C:C#,lua --source_dir=../../service_common/config/xlsx --target_dir=./code --data=./data --meta=./meta
    ```

    

