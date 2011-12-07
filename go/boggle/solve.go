// Copyright 2011 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)

package main

import (
//  "trie"
  "fmt"
  "boggler"
  "time"
  "os"
  "runtime/pprof"
  "log"
)

func main() {
  //t := trie.CreateTrieFromFilename("../words")
  //words := []string{ "hello", "goodbye", "what", "questioning" }
  //fmt.Println("Total words: ", trie.CountWords(t))
  //for _, wd := range words {
  //  fmt.Println("IsWord('", wd, "') = ", trie.IsWord(t, wd))
  //}

  b := boggler.BogglerFromDictionary("../words")
  boards := []string{
      "abcdefghijklmnop",
      "catdlinemaropets",
      "plsteaiertnrsges" }
  for _, bd := range boards {
    fmt.Println(bd, b.Score(bd))
  }

  // true, eqalize
  fmt.Println(boggler.BogglifyWord("equalize"))

  bds := [][]byte{[]uint8("abcdefghijklmnop"),
                  []uint8("catdlinemaropets")}
  var hash uint = 1234
  var total_score uint = 0
  var prime uint = (1 << 20) - 3;
  for _, base := range bds {
    for i := range base {
      base[i] -= 'a'
    }
  }

  f, err := os.Create("cpu_profile")
  if err != nil {
      log.Fatal(err)
  }

  pprof.StartCPUProfile(f)
  reps := 10
  start := time.LocalTime().Nanoseconds()
  for rep := 0; rep < reps; rep++ {
    hash = 1234
    for _, base := range bds {
      bd := make([]uint8, 16)
      for i, let := range base {
        bd[i] = let
      }

      for y1 := 0; y1 < 4; y1++ {
        for y2 := 0; y2 < 4; y2++ {
          var c1, c2 uint8
          for c1 = 0; c1 < 26; c1++ {
            bd[4*y1 + 1] = c1
            for c2 = 0; c2 < 26; c2++ {
              bd[4*y2 + 2] = c2
              score := b.ScoreBytes(bd)
              // fmt.Println(score, bd)
              hash *= (123 + uint(score))
              hash = hash % prime
              total_score += uint(score)
            }
          }
        }
      }
    }
    if (hash != 0x000C1D3D) {
      fmt.Println("Hash mismatch", hash, "!= 0xC1D3D on rep ", rep)
      // return
    }
  }
  end := time.LocalTime().Nanoseconds()
  pprof.StopCPUProfile()

  var secs float64 = 1.0e-9 * float64(end - start)
  var num_bds float64 = float64(b.NumBoards())

  fmt.Printf("Total score: %d = %f pts/bd\n",
              total_score, 1.0 * float64(total_score) / num_bds)
  fmt.Printf("Score hash: 0x%08X\n", hash);
  fmt.Printf("Evaluated %d boards in %f seconds = %f bds/sec\n",
      b.NumBoards(), secs, num_bds/secs);

  // Go:
  // Evaluated 216323 boards in 9.532613 seconds = 22692.938442 bds/sec

  // C++:
  // Evaluated 216320 boards in 2.358882 seconds = 91704.461923 bds/sec
}
