CC = g++
CPPFLAGS = -g -Wall -O3 -I.
#CPPFLAGS = -g -Wall

tests = perf_test trie_test boggler_test buckets_test multiboggle_test multiboggle_perftest ibuckets_test ibuckets_perftest 3x3/boggler_test 3x3/ibuckets_test
progs = $(tests) bucket_boggle bucket_descent ibucket_breaker ibucket_boggle solve neighbors random_boards anneal 3x3/solve 3x3/anneal 3x3/ibucket_boggle 3x3/ibucket_breaker
all: $(progs)

test: trie_test boggler_test multiboggle_test buckets_test
	./trie_test && ./boggler_test && ./buckets_test && ./multiboggle_test

mperf: multiboggle_perftest
	./multiboggle_perftest

perf: perf_test
	./perf_test

test3: 3x3/ibuckets_test
	./3x3/ibuckets_test

BOGGLE=trie.o boggler.o
MBOGGLE=multiboggle.o $(BOGGLE)
BUCKETS=buckets.o $(BOGGLE)
IBUCKETS=ibuckets.o $(BOGGLE)
BOGGLE3=trie.o 3x3/boggler.o
IBUCKETS3=3x3/ibuckets.o $(BOGGLE3)
GFLAGS=gflags/gflags.o gflags/gflags_reporting.o gflags/gflags_completions.o

solve: solve.o $(BOGGLE) $(GFLAGS)
anneal: anneal.o $(BOGGLE) mtrandom/mersenne.o $(GFLAGS)
normalize: normalize.o $(GFLAGS)
neighbors: neighbors.o $(GFLAGS)
random_boards: random_boards.o mtrandom/mersenne.o $(GFLAGS)
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

3x3/solve: 3x3/solve.o $(BOGGLE3) $(GFLAGS)
3x3/boggler_test: 3x3/boggler_test.o $(BOGGLE3) $(GFLAGS)
3x3/anneal: 3x3/anneal.o $(BOGGLE3) mtrandom/mersenne.o $(GFLAGS)
3x3/ibucket_boggle: 3x3/ibucket_boggle.o $(IBUCKETS3) $(GFLAGS)
3x3/ibucket_breaker: 3x3/ibucket_breaker.o 3x3/ibucket_breaker_main.o mtrandom/mersenne.o $(IBUCKETS3) $(GFLAGS)
3x3/ibuckets_test: 3x3/ibuckets_test.o $(IBUCKETS3)

trie.o: trie.h trie.cc
boggler.o: trie.h boggler.h boggler.cc
multiboggle.o: multiboggle.h multiboggle.cc trie.h
buckets.o: buckets.h buckets.cc boggler.h trie.h
3x3/boggler.o: trie.h 3x3/boggler.h 3x3/boggler.cc

clean:
	rm -f -r *.o $(progs) $(tests) *.dSYM mtrandom/*.o gflags/*.o 3x3/*.o
