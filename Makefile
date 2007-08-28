CC = g++
#CPPFLAGS = -g -Wall -O3 -march=i686
CPPFLAGS = -g -Wall

progs = trie_test boggler_test
all: $(progs)
test: trie_test boggler_test
	./trie_test && ./boggler_test

trie_test: trie.o trie_test.o

boggler_test: boggler.o trie.o boggler_test.o

clean:
	rm *.o $(progs)
