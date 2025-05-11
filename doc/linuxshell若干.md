* 如果路径中有空格，需要用双引号包裹

  * ```shell
    TAR_PATH="HELLO xx.tar.gz"
    tar -zxvf "${TAR_PATH}" -C "${TARGET_PATH}"
    ```

* echo -e 会开启'\'反斜杠的转义功能

* 