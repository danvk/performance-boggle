CC = g++
CPPFLAGS = -g -Wall -O3 -I. -Iglog-src -Wno-sign-compare
#CPPFLAGS = -g -Wall -I. -Wno-sign-compare

tests = trie_test 3x3/boggler_test 3x3/ibuckets_test 4x4/boggler_test board-utils_test 4x4/perf_test 4x4/ibuckets_test score_subset_test
progs = $(tests) ibucket_breaker ibucket_boggle solve neighbors random_boards anneal normalize tree_tool
all: $(progs)

test: $(tests)
	./trie_test && \
        ./board-utils_test && \
        ./3x3/boggler_test && \
        ./3x3/ibuckets_test && \
        ./4x4/boggler_test && \
        ./4x4/ibuckets_test && \
        ./4x4/perf_test && \
        ./score_subset_test

perf: 4x4/perf_test
	./4x4/perf_test

GFLAGS=gflags/gflags.o gflags/gflags_reporting.o gflags/gflags_completions.o
GLOG=glog-src/logging.o glog-src/utilities.o glog-src/symbolize.o glog-src/demangle.o glog-src/raw_logging.o glog-src/vlog_is_on.o

BOGGLE_ALL=trie.o boggle_solver.o 3x3/boggler.o 4x4/boggler.o 3x4/boggler.o
IBUCKETS_ALL=trie.o bucket_solver.o 3x3/ibuckets.o 4x4/ibuckets.o 3x4/ibuckets.o
UTILS=board-utils.o
INIT=init.o
BREAK=ibucket_breaker.o $(IBUCKETS_ALL) $(UTILS)
RAND=mtrandom/mersenne.o

solve: solve.o $(BOGGLE_ALL) $(GFLAGS)
anneal: anneal.o optimizer.o $(BOGGLE_ALL) $(RAND) $(GFLAGS)

neighbors: neighbors.o $(GFLAGS)
random_boards: random_boards.o $(RAND) $(GFLAGS)
normalize: normalize.o $(GFLAGS) $(UTILS)

ibucket_boggle: ibucket_boggle.o $(IBUCKETS_ALL) $(BOGGLE_ALL) $(GFLAGS) breaking_tree.o
ibucket_breaker: ibucket_breaker_main.o $(BREAK) $(GFLAGS) $(BOGGLE_ALL) $(UTILS) $(RAND)
tree_tool: tree_tool.o $(IBUCKETS_ALL) $(BOGGLE_ALL) $(GFLAGS) breaking_tree.o

# Tests
board-utils_test: board-utils_test.o $(UTILS)
trie_test: trie.o trie_test.o
score_subset_test: score_subset_test.o $(RAND) $(BOGGLE_ALL) $(IBUCKETS_ALL) $(BREAK) $(GLOG) $(GFLAGS) $(INIT)
3x3/boggler_test: 3x3/boggler_test.o $(BOGGLE_ALL)
4x4/boggler_test: 4x4/boggler_test.o $(BOGGLE_ALL)
3x3/ibuckets_test: 3x3/ibuckets_test.o $(IBUCKETS_ALL) $(BOGGLE_ALL)

4x4/perf_test: 4x4/perf_test.o $(BOGGLE_ALL)
4x4/ibuckets_test: 4x4/ibuckets_test.o $(IBUCKETS_ALL) $(BOGGLE_ALL)

trie.o: trie.h trie.cc

# note: doesn't clean gflags/*.o or glog-src/*.o, since presumably those won't change.
clean:
	rm -f -r *.o $(progs) $(tests) *.dSYM mtrandom/*.o gflags/*.o 3x3/*.o 3x4/*.o 4x4/*.o
