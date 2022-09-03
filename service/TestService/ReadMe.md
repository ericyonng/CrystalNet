# 系统结构

* Service（服务文件，服务的入口，所有功能模块都是它的组件）
* SessionType(定义会话的类型)
* Comps
  1. 基础组件：EventMgr(全局事件派发器)，EventMgrToEventMgr(跨事件派发器派发事件（比如按照玩家id派发到玩家的事件管理器）)，IPlayerSys(玩家子系统)，IGlobalSys(全局子系统)
  2. 其他功能组件
* 

