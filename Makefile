CC = g++
CPPFLAGS = -g -Wall -O3 -march=i686
#CPPFLAGS = -g -Wall

tests = perf_test trie_test boggler_test buckets_test multiboggle_test multiboggle_perftest ibuckets_test ibuckets_perftest
progs = $(tests) bucket_boggle bucket_descent ibucket_breaker ibucket_boggle solve
all: $(progs)

test: trie_test boggler_test multiboggle_test buckets_test
	./trie_test && ./boggler_test && ./buckets_test && ./multiboggle_test

mperf: multiboggle_perftest
	./multiboggle_perftest

perf: perf_test
	./perf_test

BOGGLE=trie.o boggler.o
MBOGGLE=multiboggle.o $(BOGGLE)
BUCKETS=buckets.o $(BOGGLE)
IBUCKETS=ibuckets.o $(BOGGLE)

solve: solve.o $(BOGGLE)
anneal: anneal.o $(BOGGLE) mtrandom/mersenne.o
perf_test: perf_test.o $(BOGGLE)
trie_test: trie.o trie_test.o
boggler_test: boggler_test.o $(BOGGLE)
multiboggle_test: multiboggle_test.cc $(MBOGGLE)
multiboggle_perftest: multiboggle_perftest.cc $(MBOGGLE)
buckets_test: buckets_test.o $(BUCKETS)
bucket_boggle: bucket_boggle.o $(BUCKETS)
bucket_descent: bucket_descent.o $(BUCKETS)
bucket_trie: bucket_trie.o $(BUCKETS)
ibucket_breaker: ibucket_breaker.o $(IBUCKETS)
ibucket_boggle: ibucket_boggle.o $(IBUCKETS)
ibuckets_test: ibuckets_test.o $(IBUCKETS)
ibuckets_perftest: ibuckets_perftest.o $(IBUCKETS)

trie.o: trie.h trie.cc
boggler.o: trie.h boggler.h boggler.cc
multiboggle.o: multiboggle.h multiboggle.cc trie.h
buckets.o: buckets.h buckets.cc boggler.h trie.h

clean:
	rm -f -r *.o $(progs) $(tests) *.dSYM
