tests = multiboggle_test multiboggle_perftest

test: multiboggle_test
	./multiboggle_test

mperf: multiboggle_perftest
	./multiboggle_perftest

BOGGLE=trie.o boggler.o
MBOGGLE=multiboggle.o $(BOGGLE)
multiboggle_test: multiboggle_test.cc $(MBOGGLE)
multiboggle_perftest: multiboggle_perftest.cc $(MBOGGLE)
multiboggle.o: multiboggle.h multiboggle.cc trie.h
