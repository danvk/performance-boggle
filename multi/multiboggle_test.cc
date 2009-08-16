// Verify that multiboggle and normal boggler agree.
#include <iostream>
#include <vector>
#include <stdio.h>
#include "trie.h"
#include "boggler.h"
#include "multiboggle.h"
using namespace std;

int CompareBoards(Trie* dict, const char* base) {
  TrieUtils<Trie>::SetAllMarks(dict, 0);
  vector<int> ref;
  GenericBoggler<Trie> bd(dict);
  bd.ParseBoard(base);
  for (int a=0; a<26; a++) {
    bd.SetCell(0, 0, a);
    for (int b=0; b<26; b++) {
      bd.SetCell(3, 3, b);
      if (a == 'q' - 'a' || b == 'q' - 'a') continue;
      ref.push_back(bd.Score());
    }
  }
  TrieUtils<Trie>::SetAllMarks(dict, 0);

  MultiBoggle mb(dict);
  mb.ParseBoard(base);
  MultiBoggle::TwoHole two;
  mb.SolveTwoHole(&two);

  vector<MultiBoggle::OneHole> tl(26);
  vector<MultiBoggle::OneHole> br(26);
  for (int a=0; a<26; a++) {
    if (a == 'q' - 'a') continue;
    mb.FillHoleTL(two, a, &(tl[a]));
    mb.FillHoleBR(two, a, &(br[a]));
  }

  int mismatches = 0;
  int i = 0;
  for (int a=0; a<26; a++) {
    for (int b=0; b<26; b++) {
      if (a == 'q' - 'a' || b == 'q' - 'a') continue;
      int score = mb.MergeBoards(two, a, tl[a], b, br[b]);
      if (score != ref[i]) {
        fprintf(stderr, "Score mismatch! (%d, %d): %d != %d\n",
                a, b, score, ref[i]);
        mismatches += 1;
      }
      i += 1;
    }
  }

  if (mismatches) {
    cerr << "Found " << mismatches << " mismatches." << endl;
  } else {
    cout << "All test boards matched!" << endl;
  }
  return mismatches;
}

int main(int argc, char** argv) {
  Trie* t = Boggler::DictionaryFromFile("words");
  cout << "Loaded/compacted " << TrieUtils<Trie>::Size(t) << " words" << endl;

  CompareBoards(t, "abcdefghijklmnop");
  CompareBoards(t, "catdlinemaropets");
  CompareBoards(t, "abcdafghijkbmnoa");
}
