CC = g++
CPPFLAGS = -g -Wall -O3 -march=i686
#CPPFLAGS = -g -Wall

progs = trie_test boggler_test perf_test perfect-trie_test perfect-boggler_test
all: $(progs)

test: trie_test boggler_test perfect-trie_test perfect-boggler_test
	./trie_test && ./boggler_test && ./perfect-trie_test && ./perfect-boggler_test

perf: perf_test
	./perf_test

trie_test: trie.o trie_test.o
boggler_test: boggler.o trie.o boggler_test.o
perf_test: trie.o boggler.o perf_test.o perfect-trie.o perfect-boggler.o
perfect-trie_test: trie.o perfect-trie.o perfect-trie_test.o
perfect-boggler_test: trie.o perfect-trie.o perfect-boggler.o perfect-boggler_test.o

trie.o: trie.h trie.cc
boggler.o: boggler.h boggler.cc
perfect-trie.o: perfect-trie.h perfect-trie.cc trie.h
perfect-boggler.o: perfect-trie.h perfect-boggler.h perfect-boggler.cc

clean:
	rm *.o $(progs)
