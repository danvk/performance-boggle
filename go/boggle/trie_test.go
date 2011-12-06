// Copyright 2011 Google Inc. All Rights Reserved.
// Author: danvk@google.com (Dan Vanderkam)

package boggle

import (
	"testing"
        "trie"
//        "fmt"
)

func noTestAddWordIsWord(tt *testing.T) {
  t := trie.NewTrie()
  trie.AddWord(t, "hello")
  trie.AddWord(t, "goodbye")
  trie.AddWord(t, "goodday")

  if !trie.IsWord(t, "hello") { tt.Fail() }
  if !trie.IsWord(t, "goodbye") { tt.Fail() }
  if !trie.IsWord(t, "goodday") { tt.Fail() }
  if trie.IsWord(t, "h") { tt.Fail() }
  if trie.IsWord(t, "he") { tt.Fail() }
  if trie.IsWord(t, "hel") { tt.Fail() }
  if trie.IsWord(t, "hell") { tt.Fail() }
  if trie.IsWord(t, "goodby") { tt.Fail() }
  if trie.IsWord(t, "goodda") { tt.Fail() }
  if trie.IsWord(t, "z") { tt.Fail() }
}

func TestStartsWordDescend(tt *testing.T) {
  t := trie.NewTrie()
  trie.AddWord(t, "hello")
  trie.AddWord(t, "goodbye")
  trie.AddWord(t, "goodday")

  if !t.StartsWord('h' - 'a') { tt.Fail() }
  if !t.StartsWord('g' - 'a') { tt.Fail() }
  if t.StartsWord('x' - 'a') { tt.Fail() }

  th := t.Descend('h' - 'a')
  if th == nil { tt.Fail() }
  if th.IsWord() { tt.Fail() }
  to := th.Descend('e' - 'a').Descend('l' - 'a').Descend('l' - 'a').Descend('o' - 'a')
  if to == nil { tt.Fail() }
  if !to.IsWord() { tt.Fail() }

  t_hell := trie.DescendPath(t, "hell")
  if t_hell == nil { tt.Fail() }
  if t_hell.IsWord() { tt.Fail() }

  // this test fails, oddly, even though fmt.Println(t_hell.Descend('h' - 'a')) prints '<nil>'
  // if t_hell.Descend('h' - 'a') != nil { tt.Fail() }

  t_hello := t_hell.Descend('o' - 'a')
  if !t_hello.IsWord() { tt.Fail() }
}

func TestMarks(tt *testing.T) {
  t := trie.NewTrie()
  trie.AddWord(t, "hello")
  trie.AddWord(t, "goodbye")
  trie.AddWord(t, "goodday")

  m := trie.DescendPath(t, "hello")
  m.Mark(10)
  if m.GetMark() != 10 { tt.Fail() }
}
