all: global.o ssd_buf_table.o ssd-cache.o smr-simulator/inner_ssd_buf_table.o smr-simulator/smr-simulator.o trace2call.o main.o strategy/clock.o strategy/lru.o
	gcc -o smr-ssd-cache global.o ssd_buf_table.o ssd-cache.o smr-simulator/inner_ssd_buf_table.o smr-simulator/smr-simulator.o trace2call.o main.o strategy/clock.o strategy/lru.o -pthread

global.o:
	gcc -c global.c -o global.o
ssd_buf_table.o:
	gcc -c ssd_buf_table.c -o ssd_buf_table.o
ssd-cahce.o:
	gcc -c ssd-cache.c -o ssd-cache.o
inner_ssd_buf_table.o:
	gcc -c smr-simulator/inner_ssd_buf_table.c -o smr-simulator/inner_ssd_buf_table.o
smr-simulator.o:
	gcc -c smr-simulator/smr-simulator.c -o smr-simulator/smr-simulator.o -pthread
trace2call.o:
	gcc -c trace2call.c -o trace2call.o
main.o:
	gcc -c main.c -o main.o
clock.o:
	gcc -c strategy/clock.c -o strategy/clock.o
lru.o:
	gcc -c strategy/lru.c -o strategy/lru.o

clean:
	rm smr-ssd-cache global.o ssd_buf_table.o ssd-cache.o smr-simulator/inner_ssd_buf_table.o smr-simulator/smr-simulator.o trace2call.o main.o strategy/clock.o strategy/lru.o
