CC = gcc
RM = rm
CFLAGS = -std=c18 \
  -lpthread -Wall -Wconversion -Werror -Wextra -Wpedantic -Wwrite-strings \
  -O2
objects = client.o 
executable = client

all: $(executable)


$(executable): $(objects)
	$(CC) $(objects) -o $(executable)

%.o : %.c %.h

clean:
	$(RM) $(objects) $(executable)