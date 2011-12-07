// Copyright 2011 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)

package trie

import (
  "fmt"
  "bufio"
  "os"
)

type Trie struct {
  is_word_ bool
  mark_ uint
  children_ []*Trie
}

func (t *Trie) IsWord() bool {
  return t.is_word_
}

func (t *Trie) SetIsWord() {
  t.is_word_ = true
}

func (t *Trie) StartsWord(c byte) bool {
  return t.children_[c] != nil
}

func (t *Trie) Descend(c byte) *Trie {
  return t.children_[c]
}
func (t *Trie) DescendMarkable(c byte) *Trie {
  return t.children_[c]
}

func (t *Trie) GetMark() uint {
  return t.mark_
}

func (t *Trie) Mark(mark uint) {
  t.mark_ = mark;
}

func (t *Trie) DescendOrAdd(c byte) *Trie {
  if t.children_[c] == nil {
    t.children_[c] = newTrie()
  }
  return t.children_[c]
}


func newTrie() *Trie {
  t := new(Trie)
  t.children_ = make([]*Trie, 26)
  return t
}

func NewTrie() *Trie {
  return newTrie()
}

func AddWord(t *Trie, word string) *Trie {
  if word == "" {
    t.SetIsWord()
    return t
  }

  if word[0] < 'a' || word[0] > 'z' {
    return nil
  }

  child := t.DescendOrAdd(word[0] - 'a')
  return AddWord(child, word[1:])
}

func IsWord(t *Trie, word string) bool {
  child := DescendPath(t, word)
  if child == nil { return false }
  return child.IsWord()
}

func DescendPath(t *Trie, word string) *Trie {
  if word == "" {
    return t
  }

  if word[0] < 'a' || word[0] > 'z' {
    return nil
  }

  if !t.StartsWord(word[0] - 'a') {
    return nil
  }

  return DescendPath(t.Descend(word[0] - 'a'), word[1:])
}


func CountWords(t *Trie) int {
  count := 0
  if t.IsWord() {
    count += 1
  }
  var i uint8
  for i = 0; i < 26; i++ {
    if t.StartsWord(i) {
      count += CountWords(t.Descend(i))
    }
  }
  return count
}

func PrintTrie(t *Trie, prefix string) {
  if t.IsWord() {
    fmt.Printf("%s\n", prefix)
  }

  var i uint8;
  for i = 0; i < 26; i++ {
    if t.StartsWord(i) {
      PrintTrie(t.Descend(i), prefix + string('a' + i))
    }
  }
}


func ReverseLookup(base, node *Trie) string {
  found, str := reverseLookup(base, node, "")
  if found {
    return str
  }
  return "?"
}

func reverseLookup(base, node *Trie, prefix string) (bool, string) {
  if base == node { return true, prefix }

  var i uint8
  for i = 0; i < 26; i++ {
    if base.StartsWord(i) {
      found, str := reverseLookup(base.Descend(i), node, prefix + string('a' + i))
      if found {
        return found, str
      }
    }
  }
  return false, ""
}


func CreateTrieFromFile(r *bufio.Reader) *Trie {
  t := NewTrie()
  for {
    line, isPrefix, err := r.ReadLine()
    if err == os.EOF {
      break
    }
    if isPrefix {
      fmt.Println("need more buffering!")
      return nil
    }
    if err != nil {
      fmt.Println("error! ", err)
      return nil
    }

    word := string(line)
    AddWord(t, word)
  }

  return t
}

func CreateTrieFromFilename(filename string) *Trie {
  f, err := os.Open(filename)
  if err != nil {
      fmt.Println(err)
      return nil
  }
  defer f.Close()

  r, err := bufio.NewReaderSize(f, 4*1024)
  if err != nil {
      fmt.Println(err)
      return nil
  }
  return CreateTrieFromFile(r)
}
