CC = g++
CPPFLAGS = -g -Wall -O3 -I.

tests=trie_test ibuckets_test breaker_test
progs=break_all

test: trie_test ibuckets_test breaker_test

IBUCKETS=ibuckets.o trie.o
BREAKER=$(IBUCKETS) breaker.o

trie_test: trie.o trie_test.o
ibuckets_test: $(IBUCKETS) ibuckets_test.o
breaker_test: $(BREAKER) breaker_test.o
break_all: break_all.o $(BREAKER)

clean:
	rm -f -r *.o $(progs) $(tests) *.dSYM
