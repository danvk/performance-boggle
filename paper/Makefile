CC = g++
CPPFLAGS = -g -Wall -O3 -I.

tests=trie_test ibuckets_test
progs=

test: trie_test ibuckets_test

trie.o: trie.h trie.cc
trie_test: trie.o trie_test.o

IBUCKETS=ibuckets.o trie.o
ibuckets_test: $(IBUCKETS) ibuckets_test.o

clean:
	rm -f -r *.o $(progs) $(tests) *.dSYM
