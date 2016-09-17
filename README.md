# Thread-Example
Basic thread example that is written in C

Finds prime numbers up to specified limit, by using specified number of threads maximum. 

gcc -o a.o prime2.c -lpthread
./a.o -n <UPPER_LIMIT> -t <NUM_OF_THREADS> -o file.txt
