// Copyright 2011 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)

package trie

import (
//  "fmt"
)

type Trie interface {
  IsWord() bool
  StartsWord(byte) bool
  Descend(byte) Trie
}

type MarkableTrie interface {
  Trie
  Mark(uint)
  GetMark() uint
  DescendMarkable(byte) MarkableTrie
}

type MutableTrie interface {
  Trie
  SetIsWord()
  DescendOrAdd(byte) MutableTrie
}

// The simple definition ("MutableTrie; MarkableTrie") results in duplicate
// name errors.
type MutableMarkableTrie interface {
  MarkableTrie
  SetIsWord()
  DescendOrAddMarkable(byte) MutableMarkableTrie
}

type simpleTrie struct {
  is_word_ bool
  mark_ uint
  children_ []*simpleTrie
}

func (t *simpleTrie) IsWord() bool {
  return t.is_word_
}

func (t *simpleTrie) SetIsWord() {
  t.is_word_ = true
}

func (t *simpleTrie) StartsWord(c byte) bool {
  return t.children_[c] != nil
}

func (t *simpleTrie) Descend(c byte) Trie {
  return t.children_[c]
}
func (t *simpleTrie) DescendMarkable(c byte) MarkableTrie {
  return t.children_[c]
}

func (t *simpleTrie) GetMark() uint {
  return t.mark_
}

func (t *simpleTrie) Mark(mark uint) {
  t.mark_ = mark;
}

func (t *simpleTrie) DescendOrAdd(c byte) MutableTrie {
  if t.children_[c] == nil {
    t.children_[c] = newSimpleTrie()
  }
  return t.children_[c]
}

func (t *simpleTrie) DescendOrAddMarkable(c byte) MutableMarkableTrie {
  if t.children_[c] == nil {
    t.children_[c] = newSimpleTrie()
  }
  return t.children_[c]
}


func newSimpleTrie() *simpleTrie {
  t := new(simpleTrie)
  t.children_ = make([]*simpleTrie, 26)
  return t
}

func NewTrie() MutableMarkableTrie {
  return newSimpleTrie()
}

func AddWord(t MutableTrie, word string) MutableTrie {
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

func IsWord(t Trie, word string) bool {
  child := DescendPath(t, word)
  if child == nil { return false }
  return child.IsWord()
}

func DescendPath(t Trie, word string) Trie {
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
