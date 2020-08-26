# HttpInterface：Windows上C++封装的HTTP库，包含三种实现模式（WinInet、WinHttp、socket）  

主要实现了HTTP的get\post方法，下载到内存、下载到本地文件，回调下载进度等接口  测试程序中展现了常用的几个方法。 

## 更新记录
	2019-09-10
	（1）修复WinInet实现库中的若干bug（下载到缓存中数据不对、http请求返回字符串\0截断……）；
	（2）WinInet & WinHttp实现库支持HTTPS协议；
	（3）测试HTTPS下载、请求。

	2020-03-07
	（1）抽离出初始化winsocket库接口；
	（2）增加注释，更新部分枚举命名。

	2020-05-20
	（1）规范化定义HTTP请求头相关字段
	（2）修复socket实现HTTP重定向的一些小问题

	2020-08-26 22:41:43
	（1）支持添加http头接口
	（2）增加post测试代码

	
## 问题反馈
	能力有限，有bug欢迎指正。
