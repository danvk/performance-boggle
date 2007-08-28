CC = g++
#CPPFLAGS = -g -Wall -O3 -march=i686
CPPFLAGS = -g -Wall

progs = trie_test

all: $(progs)

trie_test: trie.o trie_test.o

clean:
	rm *.o $(progs)
