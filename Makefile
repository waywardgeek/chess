#CFLAGS=-O3 -Wall -std=c11 -fsanitize-undefined-trap-on-error -fsanitize=signed-integer-overflow,unsigned-integer-overflow
CFLAGS=-O3 -Wall -std=c11
CC=clang

chess: chess.c chdatabase.c chdatabase.h
	#gcc $(CFLAGS) -DDD_DEBUG -o chess chess.c chdatabase.c -lreadline -lddutil-dbg
	$(CC) $(CFLAGS) -o chess chess.c chdatabase.c -lreadline -lddutil

chdatabase.c: chdatabase.h

chdatabase.h: Chess.dd
	datadraw Chess.dd

clean:
	rm -f chdatabase.[ch] chess
