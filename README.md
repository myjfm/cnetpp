What is cnetpp?
---------------

一个轻量级的网络通信框架，专注于现代c++的后端开发。

Dependancies:
-------------

* linux2.6 或更高版本（捡来会支持跨平台）
* 支持c++11或更高版本c++的编译器

Includes:
---------

* 一个简单的json解析器，命名为csonpp
* 一个简单的线程及线程池管理框架
* 基于epoll（将来会集成select，poll，kqueue，/dev/poll等）的异步事件通知的TCP+HTTP网络通信框架。
  
Install:
--------

* 进入到cnetpp根目录
* mkdir build/
* cd build/
* cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..  修改CMAKE_INSTALL_PREFIX变量的值来改变安装目录
* make
* sudo make install

