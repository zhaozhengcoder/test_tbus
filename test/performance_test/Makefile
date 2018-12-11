CXXFLAGS = -g 

performance_test:performance_test_case1.c ../../src/mytbus/mytbus.c
	g++ performance_test_case1.c $(CXXFLAGS) -o  performance_test_case1  ../../src/mytbus/mytbus.c -lpthread -std=c++11

performance_test_case2_recv:performance_test_case2_recv.c ../../src/mytbus/mytbus.c
	g++ performance_test_case2_recv.c $(CXXFLAGS) -o performance_test_case2_recv ../../src/mytbus/mytbus.c

performance_test_case2_send:performance_test_case2_send.c ../../src/mytbus/mytbus.c
	g++ performance_test_case2_send.c $(CXXFLAGS) -o performance_test_case2_send ../../src/mytbus/mytbus.c