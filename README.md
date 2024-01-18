#### 题目（均已完成，分别对应ex01，ex02，ex03）：

***\*1）\*******\*设计并实现一个简单的\*******\*聊天室\*******\*程序\*******\*,实现如下功能：\****

（1）、终端字符界面，支持用户管理，用户名/密码注册和登录（3分）

（2）、客户端登陆到服务器聊天室后，可以在聊天室内与其他多个用户交流，聊天室中的任何一个用户输入一段字符后，室内的其他用户都可以看到这句话；（3分）

（3）、客户端对应每一个参加聊天的用户，完成从终端上输入采集并传递到服务器端和从服务器端接收信息输出显示的功能。（3分）



***\*2)\**** ***\*很多公共服务场所都有取号机，用来给用户进行编号，如医院，银行等。\*******\*选定一个你感兴趣的应用场景，为其\*******\*编写一个socket客户机/服务器程序，用来模拟取号机。实现的功能如下：\****

(1)、 客户机连接服务器（3分）

(2)、 服务器收到客户机连接请求后，返回服务选项给客户机，包括（也可以是自己定义的其他选项）：(a) 购买；(b) 维修；(c) 其他；（3分） 

(3)、 客户机接收到服务选项信息，在屏幕打印，并提示用户按服务编号进行选择（3分）

(4)、 用户输入自己想选择的服务编号，如果合法，客户机将客户的选择传送给服务器，如果不合法，则打印错误提示信息要求用户重新输入；（5分）

(5)、 服务器为该服务请求分配一个新的流水号，将时间、服务项目、流水号传递给客户机，并将此次请求的所有信息存入文件备案（5分）

(6)、 客户机收到服务器返回的信息后在屏幕输出，告知用户有关此次服务请求的相关内容（5分）

(7)、 客户机退出，服务器继续等待新请求（5分）

 

***\*3\*******\*)\**** ***\*在Linux操作系统上实现的一个简单的HTTP服务器，要求实现如下功能：\****

（1）、主线程只负责监听文件描述符上是否有事件发生，有的话立即将该事件通知工作线程（3分）

（2）、读写数据，接受新的连接，以及处理客户请求均在工作线程中完成（5分）

（3）、能接收客户端的GET请求；（5分）

（4）、能够解析客户端的请求报文，根据客户端要求找到相应的资源；（5分）

（5）、能够回复http应答报文；（5分）

（6）、能够读取服务器中存储的文件，并返回给请求客户端，实现对外发布静态资源；（5分）

（7）、使用I/O复用来提高处理请求的并发度；（5分）

（8）、服务器端支持错误处理，如要访问的资源不存在时回复404错误等。（5分）
