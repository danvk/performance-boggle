CC = g++
CPPFLAGS = -g -Wall -O3 -march=i686
#CPPFLAGS = -g -Wall

progs = perf_test perfect-trie_test perfect-boggler_test
all: $(progs)

test: perfect-trie_test perfect-boggler_test
	./perfect-trie_test && ./perfect-boggler_test

perf: perf_test
	./perf_test

perf_test: trie.o perf_test.o perfect-trie.o perfect-boggler.o
perfect-trie_test: trie.o perfect-trie.o perfect-trie_test.o
perfect-boggler_test: trie.o perfect-trie.o perfect-boggler.o perfect-boggler_test.o

trie.o: trie.h trie.cc
perfect-trie.o: perfect-trie.h perfect-trie.cc trie.h
perfect-boggler.o: perfect-trie.h perfect-boggler.h perfect-boggler.cc

clean:
	rm *.o $(progs)
