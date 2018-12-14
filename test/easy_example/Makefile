CXXFLAGS = -g -Wall

test_tbus_send:test_main_send.c test_main_recv.c ../../src/mytbus/mytbus.c
	g++ -g test_main_send.c $(CXXFLAGS) -o ./test_tbus_send ../../src/mytbus/mytbus.c
	g++ -g test_main_recv.c $(CXXFLAGS) -o ./test_tbus_recv ../../src/mytbus/mytbus.c	