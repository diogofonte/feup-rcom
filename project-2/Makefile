NAME = download
DEPS = download.h
OBJ= download.o main.o

CC = gcc
CFLAGS = -Wall

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

download: ${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o $@ 

clean:
	rm -f ${NAME} 
	rm -f ${OBJ}
