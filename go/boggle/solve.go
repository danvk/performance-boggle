// Copyright 2011 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)

package main

import (
  "trie"
  "fmt"
  "os"
  "bufio"
)

func main() {
  filename := "../words"
  f, err := os.Open(filename)
  if err != nil {
      fmt.Println(err)
      return
  }
  defer f.Close()

  r, err := bufio.NewReaderSize(f, 4*1024)
  if err != nil {
      fmt.Println(err)
      return
  }

  t := trie.CreateTrieFromFile(r)
  words := []string{ "hello", "goodbye", "what", "questioning" }
  for _, wd := range words {
    fmt.Println("IsWord('", wd, "') = ", trie.IsWord(t, wd))
  }
}
