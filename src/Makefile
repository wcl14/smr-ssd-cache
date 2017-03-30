CC = gcc -g

ifndef SMR_SSD_CACHE_DIR
SMR_SSD_CACHE_DIR = .
endif

CFLAGS += -Wall -pthread
CPPFLAGS += -I$(SMR_SSD_CACHE_DIR) -I$(SMR_SSD_CACHE_DIR)/smr-simulator -I$(SMR_SSD_CACHE_DIR)/strategy

RM = rm -rf
OBJS = global.o ssd_buf_table.o ssd-cache.o inner_ssd_buf_table.o smr-simulator.o trace2call.o main.o clock.o lru.o scan.o lruofband.o band_table.o most.o WA.o ssd_buf_table_for_coldmax_history.o maxcold.o fourquadrant.o

all: $(OBJS) smr-ssd-cache
	@echo 'Successfully built smr-ssd-cache...'

smr-ssd-cache:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(OBJS) -o $@

global.o: global.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

ssd_buf_table.o: ssd_buf_table.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

ssd-cahce.o: sdd-cache.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

inner_ssd_buf_table.o: smr-simulator/inner_ssd_buf_table.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

smr-simulator.o: smr-simulator/smr-simulator.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

trace2call.o: trace2call.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

main.o: main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

clock.o: strategy/clock.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

lru.o: strategy/lru.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

scan.o: strategy/scan.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?


lruofband.o: strategy/lruofband.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

band_table.o: strategy/band_table.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

most.o: strategy/most.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

WA.o: strategy/WA.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

ssd_buf_table_for_coldmax_history.o: strategy/ssd_buf_table_for_coldmax_history.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

maxcold.o: strategy/maxcold.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

fourquadrant.o: strategy/fourquadrant.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $?

clean:
	$(RM) *.o
	$(RM) $(SMR_SSD_CACHE_DIR)/smr-ssd-cache
