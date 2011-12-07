// Copyright 2011 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)

package main

import (
  "trie"
  "fmt"
  "boggler"
  "time"
)

func main() {
  t := trie.CreateTrieFromFilename("../words")
  words := []string{ "hello", "goodbye", "what", "questioning" }
  fmt.Println("Total words: ", trie.CountWords(t))
  for _, wd := range words {
    fmt.Println("IsWord('", wd, "') = ", trie.IsWord(t, wd))
  }

  b := boggler.BogglerFromTrie(t)
  boards := []string{
      "abcdefghijklmnop",
      "catdlinemaropets",
      "plsteaiertnrsges" }
  for _, bd := range boards {
    fmt.Println(bd, b.Score(bd))
  }

  start := time.LocalTime().Nanoseconds()
  for i := 0; i < 100; i++ {
    b.Score("abcdefghijklmnop")
  }
  end := time.LocalTime().Nanoseconds()

  fmt.Println(end - start, "ns")
}
