1、WORKSPACE：工作区当前Job构建目录的绝对路径
2、SVN_REVISION：SVN版本当前工作区的Subversion版本号；
3、SVN_URL当前工作区的Svn URL
4、BUILD_NUMBER：构建编号当前构建的编号，例如“4674”等
5、BUILD_ID：构建ID与构建的BUILD_NUMBER相同
6、BUILD_DISPLAY_NAME当前版本的显示名称，默认为“# 4674”，即BUILD_NUMBER；
7、JOB_NAME：项目名称即此版本的项目名称，例如“foo”或“foo / bar”；
8、JENKINS_HOMEJenkins用于存储数据的主节点上分配的目录的绝对路径；
9、JENKINS_URL：Jenkins的完整URL如http：// server：port / jenkins /（注意：仅在系统配置中设置Jenkins URL时可用）；
10、BUILD_URL：此版本的完整URL例如http：// server：port / jenkins / job / foo / 15 /（必须设置Jenkins URL）
11、JOB_URL：该项目的完整URL，例如http：// server：port / jenkins / job / foo /（必须设置Jenkins URL)
12、BRANCH_NAME对于多分支项目，这将被设置为正在构建的分支的名称，
例如，如果您希望从master部署到生产环境而不是从feature分支部署；如果对应某种更改请求，则该名称通常是任意的（请参阅下面的CHANGE_ID和CHANGE_TARGET）
13、CHANGE_ID对于与某种更改请求相对应的多分支项目，这将被设置为更改ID，例如拉取请求编号（如果支持）;其他未设置；
14、CHANGE_URL对于与某种更改请求相对应的多分支项目，这将被设置为更改URL(如果支持)；其他未设置；
15、CHANGE_TITLE对于与某种更改请求相对应的多分支项目，这将被设置为更改的标题（如果支持）;其他未设置；
16、CHANGE_AUTHOR对于与某种更改请求相对应的多分支项目，这将被设置为建议更改的作者的用户名（如果支持）;其他未设置；
17、CHANGE_AUTHOR_DISPLAY_NAME：对于与某种更改请求相对应的多分支项目，这将被设置为建议更改的作者的人名（如果支持）;其他未设置；
18、CHANGE_AUTHOR_EMAIL：对于与某种更改请求相对应的多分支项目，这将被设置为建议更改的作者的Email地址（如果支持）;其他未设置；
19、CHANGE_TARGET：对于与某种更改请求相对应的多分支项目，这将被设置为合并到的目标或者基础分支（如果支持）;其他未设置；
20、JOB_BASE_NAME：此构建的项目的短名称剥离文件夹路径，例如“bar / foo”的“foo”；
21、BUILD_TAG：“jenkins - $ {JOB_NAME} - $ {BUILD_NUMBER}”的字符串。 JOB_NAME中的所有正斜杠（/）都用破折号（ - ）替换。方便地放入资源文件，jar文件等，以便于识别。
22、EXECUTOR_NUMBER：唯一编号，用于标识执行此构建的当前执行程序（在同一台计算机的执行程序中）。这是您在“构建执行程序状态”中看到的数字，但数字从0开始，而不是从1开始。
23、NODE_NAME：如果构建在代理上，则代理的名称; 如果在主版本上运行，则为“MASTER”；
24、NODE_LABELS：节点分配的空白分隔的标签列表。
25、GIT_COMMITgit提交的hash码
26、GIT_PREVIOUS_COMMIT：当前分支上次提交的hash码
27、GIT_PREVIOUS_SUCCESSFUL_COMMIT：当前分支上次成功构建时提交的hash码
28、GIT_BRANCH远程分支名称，如果有的话；
29、GIT_LOCAL_BRANCH本地分支名称，如果有的话；
30、GIT_URL：远程git仓库的URL。如果有多个，将会是GIT_URL_1，GIT_URL_2等；
31、GIT_COMMITTER_NAME配置的Git提交者名称（如果有的话）
32、GIT_AUTHOR_NAME：配置的Git作者姓名（如果有的话）
33、GIT_COMMITTER_EMAIL配置的Git提交者电子邮件（如果有的话）
34、GIT_AUTHOR_EMAIL：已配置的Git作者电子邮件（如果有）
35、JOB_DESCRIPTION：显示项目描述；
36、CAUSE：显示谁、通过什么渠道触发这次构建；
37、CHANGES显示上一次构建之后的变化
 showPaths 如果为 true,显示提交修改后的地址。默认false。
 showDependencies 如果为true，显示项目构建依赖。默认为false
 format 遍历提交信息，一个包含%X的字符串，其中%a表示作者，%d表示日期，%m表示消息，%p表示路径，%r表示版本。注意，并不是所有的版本系统都支持%d和%r。如果指定showPaths将被忽略。默认“[%a] %m\\n”。
 pathFormat 一个包含“%p”的字符串，用来标示怎么打印路径。
38、PROJECT_NAME：显示项目的全名
39、PROJECT_DISPLAY_NAME：显示项目的显示名称。（见AbstractProject.getDisplayName）
40、SCRIPT：从一个脚本生成自定义消息内容。自定义脚本应该放在"$JENKINS_HOME/email-templates"。当使用自定义脚本时会默认搜索$JENKINS_HOME/email-templatesdirectory目录。其他的目录将不会被搜索。
 script 当其使用的时候，仅仅只有最后一个值会被脚本使用（不能同时使用script和template）。
 template常规的simpletemplateengine格式模板。
41.BUILD_LOG_MULTILINE_REGEX：按正则表达式匹配并显示构建日志。
 regex java.util.regex.Pattern 生成正则表达式匹配的构建日志。无默认值，可为空。
    maxMatches 匹配的最大数量。如果为0，将匹配所有。默认为0。
    showTruncatedLines 如果为true，包含[...truncated ### lines...]行。默认为true。
    substText 如果非空，就把这部分文字（而不是整行）插入该邮件。默认为空。
    escapeHtml 如果为true，格式化HTML。默认为false。
    matchedSegmentHtmlStyle 如果非空，输出HTML。匹配的行数将变为<b style="your-style-value"> html escaped matched line </b>格式。默认为空。
42、BUILD_LOG：显示最终构建日志。显示最终构建日志。
 maxLines 日志最多显示的行数，默认250行。
 escapeHtml 
  true:格式化HTML。
  false:默认，不格式化
43、PROJECT_URL：显示项目的URL地址。
44、BUILD_STATUS：显示当前构建的状态(失败、成功等等)
45、BUILD_URL：显示当前构建的URL地址。
46、CHANGES_SINCE_LAST_SUCCESS：显示上一次成功构建之后的变化。
 reverse在顶部标示新近的构建。
  true
  false(默认)
 format遍历构建信息，一个包含%X的字符串，其中%c为所有的改变，%n为构建编号。默认”Changes for Build #%n\n%c\n”。
 showPaths,changesFormat,pathFormat分别定义如${CHANGES}的showPaths、format和pathFormat参数。
47、CHANGES_SINCE_LAST_UNSTABLE：显示显示上一次不稳固或者成功的构建之后的变化。
 reverse在顶部标示新近的构建。默认false。
 format遍历构建信息，一个包含%X的字符串，其中%c为所有的改变，%n为构建编号。默认”Changes for Build #%n\n%c\n”。
 showPaths,changesFormat,pathFormat分别定义如${CHANGES}的showPaths、format和pathFormat参数。
48、ENV：显示一个环境变量。
 var– 显示该环境变量的名称。如果为空，显示所有，默认为空。
49、FAILED_TESTS：如果有失败的测试，显示这些失败的单元测试信息。
50、HUDSON_URL：不推荐，请使用$JENKINS_URL
51、JELLY_SCRIPT：从一个Jelly脚本模板中自定义消息内容。有两种模板可供配置：HTML和TEXT。你可以在$JENKINS_HOME/email-templates下自定义替换它。当使用自动义模板时，”template”参数的名称不包含“.jelly”。
52、TEST_COUNTS：显示测试的数量。
    var– 默认“total”。   ${TEST_COUNTS,var="fail"}
        total -所有测试的数量。
        fail -失败测试的数量。
        skip -跳过测试的数量。