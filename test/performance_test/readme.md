# 测试

1. performance_test_case1.c 文件里面

    case1：发送一条消息，之后读取一条消息，并且检查消息的正确性

    case2：发送方发送1000条消息，然后接受方接受1000条消息

    case3：两个子线程，分别创建发送消息和接收消息

    (注意：在启动进程之前 首先要启动 /debug/tbus_mgr/bin/start.sh)

2. performance_test_case2_recv.c 和 performance_test_case2_send.c 文件

    分别是两个进程来接受和发送消息,发送20w条，12字节的消息，需要的时间。

    ```
    time ./performance_test_case2_send 200000
    [send thread] send msg num 200000

    real    0m0.310s
    user    0m0.156s
    sys     0m0.016s
    ```

    ```
    time ./performance_test_case2_send 200000
    [send thread] send msg num 200000

    real    0m0.310s
    user    0m0.156s
    sys     0m0.016s
    ```

