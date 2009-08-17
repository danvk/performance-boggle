CC = g++
CPPFLAGS = -g -Wall -O3 -I. -Wno-sign-compare
#CPPFLAGS = -g -Wall

tests = perf_test trie_test boggler_test ibuckets_test ibuckets_perftest 3x3/boggler_test 3x3/ibuckets_test board-utils_test
progs = $(tests) ibucket_breaker ibucket_boggle solve neighbors random_boards anneal 3x3/ibucket_boggle 3x3/ibucket_breaker normalize
all: $(progs)

test: trie_test boggler_test
	./trie_test && ./boggler_test

perf: perf_test
	./perf_test

test3: 3x3/ibuckets_test
	./3x3/ibuckets_test

GFLAGS=gflags/gflags.o gflags/gflags_reporting.o gflags/gflags_completions.o

BOGGLE_ALL=trie.o boggle_solver.o 3x3/boggler.o 4x4/boggler.o 3x4/boggler.o
IBUCKETS_ALL=trie.o bucket_solver.o 3x3/ibuckets.o 4x4/ibuckets.o
UTILS=board-utils.o
BREAK=ibucket_breaker.o $(IBUCKETS_ALL) $(UTILS)
RAND=mtrandom/mersenne.o

solve: solve.o $(BOGGLE_ALL) $(GFLAGS)
anneal: anneal.o $(BOGGLE_ALL) $(RAND) $(GFLAGS)

neighbors: neighbors.o $(GFLAGS)
random_boards: random_boards.o $(RAND) $(GFLAGS)
normalize: normalize.o $(GFLAGS) $(UTILS)

ibucket_boggle: ibucket_boggle.o $(IBUCKETS_ALL) $(BOGGLE_ALL) $(GFLAGS)
ibucket_breaker: ibucket_breaker_main.o $(BREAK) $(GFLAGS) $(BOGGLE_ALL) $(UTILS) $(RAND)

# Tests
board-utils_test: board-utils_test.o $(UTILS)

#perf_test: perf_test.o $(BOGGLE)
#trie_test: trie.o trie_test.o
#boggler_test: boggler_test.o $(BOGGLE)
#ibucket_breaker: ibucket_breaker.o $(IBUCKETS)
#ibuckets_test: ibuckets_test.o $(IBUCKETS)
#ibuckets_perftest: ibuckets_perftest.o $(IBUCKETS)
#
#3x3/boggler_test: 3x3/boggler_test.o $(BOGGLE3) $(GFLAGS)
#3x3/ibucket_boggle: 3x3/ibucket_boggle.o $(IBUCKETS3) $(GFLAGS)
#3x3/ibucket_breaker: 3x3/ibucket_breaker.o 3x3/ibucket_breaker_main.o mtrandom/mersenne.o $(IBUCKETS3) $(GFLAGS)
#3x3/ibuckets_test: 3x3/ibuckets_test.o $(IBUCKETS3)

trie.o: trie.h trie.cc

clean:
	rm -f -r *.o $(progs) $(tests) *.dSYM mtrandom/*.o gflags/*.o 3x3/*.o 3x4/*.o
