chess: chess.c chdatabase.c chdatabase.h
	#gcc -g -Wall -std=c11 -DDD_DEBUG -o chess chess.c chdatabase.c -lreadline -lddutil-dbg
	gcc -O3 -Wall -std=c11 -o chess chess.c chdatabase.c -lreadline -lddutil

chdatabase.c: chdatabase.h

chdatabase.h: Chess.dd
	datadraw Chess.dd

clean:
	rm -f chdatabase.[ch] chess
