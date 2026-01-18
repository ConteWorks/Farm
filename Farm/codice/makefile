CC		=  gcc
CFLAGS	        += -std=c99 -Wall -Werror -g
LIBS            = -lpthread
TARGETS		= boundedqueue.c \
		  masterthread.c \
		  poolworker.c \
		  collector.c \
		  main.c
		  	
farm: $(TARGETS)
	$(CC) $(CFLAGS) $(TARGETS) $(LIBS) -o farm
	
test:
	./test.sh
	
clean:
	rm -f *.dat -rf testdir expected.txt farm
	
debug:
	$(CC) $(CFLAGS) -D DEBUG $(TARGETS) $(LIBS) -o farm
	
file:
	./generafile.sh
	
test2:
	./mytest.sh
