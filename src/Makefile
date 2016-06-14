all: global.o ssd_buf_table.o ssd-cache.o inner_ssd_buf_table.o smr-simulator.o strategy.o trace2call.o main.o
	gcc -o smr-ssd-cache global.o ssd_buf_table.o ssd-cache.o inner_ssd_buf_table.o smr-simulator.o strategy.o trace2call.o main.o -pthread

global.o:
	gcc -c global.c -o global.o
ssd_buf_table.o:
	gcc -c ssd_buf_table.c -o ssd_buf_table.o
ssd-cahce.o:
	gcc -c ssd-cache.c -o ssd-cache.o
inner_ssd_buf_table.o:
	gcc -c inner_ssd_buf_table.c -o inner_ssd_buf_table.o
smr-simulator.o:
	gcc -c smr-simulator.c -o smr-simulator.o -pthread
strategy.o:
	gcc -c strategy.c -o strategy.o
trace2call.o:
	gcc -c trace2call.c -o trace2call.o
main.o:
	gcc -c main.c -o main.o

clean:
	rm smr-ssd-cache global.o ssd_buf_table.o ssd-cache.o inner_ssd_buf_table.o smr-simulator.o strategy.o trace2call.o main.o
