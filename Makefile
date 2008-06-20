CC = g++
CPPFLAGS = -g -Wall -O3 -march=i686
#CPPFLAGS = -g -Wall

tests = perf_test trie_test multiboggle_test multiboggle_perftest
progs = $(tests)
all: $(progs)

test: trie_test boggler_test multiboggle_test buckets_test
	./trie_test && ./boggler_test && ./buckets_test && ./multiboggle_test

mperf: multiboggle_perftest
	./multiboggle_perftest

perf: perf_test
	./perf_test

perf_test: perf_test.o trie.o boggler.o
trie_test: trie.o trie_test.o
boggler_test: trie.o boggler.o boggler_test.o
multiboggle_test: trie.o boggler.o multiboggle.o multiboggle_test.cc
multiboggle_perftest: trie.o boggler.o multiboggle.o multiboggle_perftest.cc
buckets_test: trie.o buckets.o boggler.o buckets_test.o
bucket_boggle: bucket_boggle.o trie.o buckets.o boggler.o

trie.o: trie.h trie.cc
boggler.o: trie.h boggler.h boggler.cc
multiboggle.o: multiboggle.h multiboggle.cc trie.h
buckets.o: buckets.h buckets.cc boggler.h trie.h

clean:
	rm -r *.o $(progs) *.dSYM
