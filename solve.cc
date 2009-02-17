// Simple Boggle Solver
//
// Given a dictionary and letters on the board, print the score.

#include <cstdio>
#include <string>
#include "boggler.h"
#include "trie.h"

const char kUsage[] =
"%s <dictionary file> [abcdefghijklmnop|a b c d e f g h i j k l m n o p]\n"
"A 'q' is treated as 'qu'.\n";

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "You must specify a dictionary file!\n");
    fprintf(stderr, kUsage, argv[0]);
    exit(1);
  }

  const char* dict_file = argv[1];
  FILE* f = fopen(dict_file, "r");
  if (f == NULL) {
    fprintf(stderr, "Couldn't open dictionary file %s\n", dict_file);
    exit(1);
  }
  fclose(f);

  if (argc != 3 && argc != 18) {
    fprintf(stderr, "Expected either a single string of letters or 16 "
                    "single-letter parameters, but got %d params.\n", argc - 2);
    fprintf(stderr, kUsage, argv[0]);
    exit(1);
  }

  // Parse the board.
  char* bd;
  if (argc == 3) {
    bd = argv[2];
    if (strlen(bd) != 16) {
      fprintf(stderr, "Board strings must contain sixteen characters, got %zu\n",
              strlen(bd));
      exit(1);
    }

    for (int i = 0; i < 16; i++) {
      if (bd[i] >= 'A' && bd[i] <= 'Z') {
        bd[i] = bd[i] + ('a' - 'A');
      } else if (bd[i] < 'a' || bd[i] > 'z') {
        fprintf(stderr, "Unexpected board character: '%c'\n", bd[i]);
        exit(1);
      }
    }
  } else {
    bd = new char[16];
    for (int i = 0; i < 16; i++) {
      const char* arg = argv[2 + i];
      int len = strlen(arg);
      if (len == 1) {
        if (arg[0] >= 'A' && arg[0] <= 'Z') {
          bd[i] = arg[0] - 'A' + 'a';
        } else if (arg[0] >= 'a' && arg[0] <= 'z') {
          bd[i] = arg[0];
        } else {
          fprintf(stderr, "Unexpected board letter '%s'\n", arg);
          exit(1);
        }
      } else if (len == 2 && (strcmp(arg, "qu") == 0 || strcmp(arg, "QU") == 0)) {
        bd[i] = 'q';
      } else {
        fprintf(stderr, "Invalid board letter '%s'\n", arg);
        exit(1);
      }
    }
  }

  // Finally!
  Trie* t = Boggler::DictionaryFromFile(dict_file);
  // fprintf(stdout, "Loaded %zu words from dictionary.\n",
  //          TrieUtils<Trie>::Size(t));

  Boggler b(t);
  if (!b.ParseBoard(bd)) {
    fprintf(stderr, "Couldn't parse board string '%s'\n", argv[2]);
    exit(1);
  }
  int score = b.Score();
  fprintf(stdout, "%s: %d\n", b.ToString().c_str(), score);
}
