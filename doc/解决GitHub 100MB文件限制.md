* GitHub文件有100MB限制

* 目前解决办法：

  ```
  1.把原文件xxx.xx 拆成xxx.xx.mainpart, xxx.xx.part1, xxx.xx.part2, 
  由拆分工具进行拆分
  2.在编译的时候会自动运行拆分工具，将mainpart part1, part2,...按顺序进行合并然后去掉mainpart后缀(先删除原来的文件)
  3.进行新文件生成后由拆分工具自动拆分成mainpart, part1, part2
  
  ```

  