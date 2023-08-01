# 用户系统设计

* 基本组件：IUser, IUserSys, IUserMgr

#### IUser

​	IUser是User的基类, 提供User基本接口

#### IUserSys

​	IUserSys是User下的组件，实现User下的不同功能，所有User的模块都继承于IUserSys实现其逻辑

#### IUserMgr

​	IUserMgr是管理User的GlobalSys组件

* 

