CC = g++
CPPFLAGS = -g -Wall -O3 -march=i686
#CPPFLAGS = -g -Wall

progs = perf_test trie_test boggler_test
all: $(progs)

test: trie_test boggler_test
	./trie_test && ./boggler_test

perf: perf_test
	./perf_test

perf_test: perf_test.o trie.o boggler.o
trie_test: trie.o trie_test.o
boggler_test: trie.o boggler.o boggler_test.o

trie.o: trie.h trie.cc
boggler.o: trie.h boggler.h boggler.cc

clean:
	rm *.o $(progs)
