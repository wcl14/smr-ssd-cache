all: gloable.o ssd_buf_table.o ssd-cache.o inner_ssd_buf_table.o smr-simulator.o main.o
	gcc -o smr-ssd-cache gloable.o ssd_buf_table.o ssd-cache.o inner_ssd_buf_table.o smr-simulator.o main.o

gloable.o:
	gcc -c gloable.c -o gloable.o
ssd_buf_table.o:
	gcc -c ssd_buf_table.c -o ssd_buf_table.o
ssd-cahce.o:
	gcc -c ssd-cache.c -o ssd-cache.o
inner_ssd_buf_table.o:
	gcc -c inner_ssd_buf_table.c -o inner_ssd_buf_table.o
smr-simulator.o:
	gcc -c smr-simulator.c -o smr-simulator.o
main.o:
	gcc -c main.c -o main.o

clean:
	rm smr-ssd-cache gloable.o ssd_buf_table.o ssd-cache.o inner_ssd_buf_table.o smr-simulator.o main.o
