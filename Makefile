customer_dir = customer/
daemon_dir    = daemon_/
Log_dir       = Log/
luncher_dir   = luncher/
file_synch_dir = file_synch/
CC = gcc
CFLAGS = -std=c18 \
  -Wall -Wconversion -Werror -Wextra -Wpedantic -Wwrite-strings  \
  -fpie -fstack-protector-all -D_XOPEN_SOURCE=500           \
  -I$(customer_dir) -I$(daemon_dir) -I$(file_synch_dir)   \
  -I$(Log_dir) -I$(luncher_dir)

LDFLAGS = -L. -pthread
LDLIBS = -lrt

vpath %.c $(customer_dir) $(Log_dir) $(daemon_dir) $(luncher_dir) $(file_synch_dir)
vpath %.h $(customer_dir) $(Log_dir) $(luncher_dir) $(file_synch_dir)

lanceur_o = luncher.o log.o file_synch.o
client_o  = customer.o file_synch.o
daemon_o  = daemon.o log.o

objects = customer.o daemon.o log.o luncher.o file_synch.o
executables = lanceur client daemon

.PHONY: all clean

all: $(executables)

clean:
	$(RM) $(objects) $(executables)
	# @$(RM) $(makefile_indicator)

client: $(client_o)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

lanceur: $(lanceur_o)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

daemon: $(daemon_o)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
