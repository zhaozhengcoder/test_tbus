## easy_example

这个是一个简单的例子，进程A(test_tbus_send)发送3条消息，进程B(test_tbus_recv)接受三条消息。


在进程A和B运行之前，首先需要执行tbus_mgr进程，读取配置和创建共享内存和消息通道。
```
debug/tbus_mgr/bin/start.sh 
```