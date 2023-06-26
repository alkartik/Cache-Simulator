cache_simulate: ./cache.cpp
	g++ -o ./cache_simulate cache.cpp

run1:
	./cache_simulate 64 1024 2 65536 8 ./memory_trace_files/trace1.txt

run_all:
	./cache_simulate 64 1024 2 65536 8 ./memory_trace_files/trace1.txt
	./cache_simulate 64 1024 2 65536 8 ./memory_trace_files/trace2.txt
	./cache_simulate 64 1024 2 65536 8 ./memory_trace_files/trace3.txt
	./cache_simulate 64 1024 2 65536 8 ./memory_trace_files/trace4.txt
	./cache_simulate 64 1024 2 65536 8 ./memory_trace_files/trace5.txt
	./cache_simulate 64 1024 2 65536 8 ./memory_trace_files/trace6.txt
	./cache_simulate 64 1024 2 65536 8 ./memory_trace_files/trace7.txt
	./cache_simulate 64 1024 2 65536 8 ./memory_trace_files/trace8.txt

graph:
	./cache_simulate 

