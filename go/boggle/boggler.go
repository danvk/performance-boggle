package boggler

import (
  // "fmt"
  "trie"
)

type Boggler struct {
  t *trie.Trie
  counter uint
  used uint
  bd [16]int
  score int
}

const kQ = 'q' - 'a'
var kWordScores []int = []int{ 0, 0, 0, 1, 1, 2, 3, 5, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };

func BogglerFromTrie(t *trie.Trie) *Boggler {
  b := new(Boggler)
  b.t = t
  return b
}

func BogglerFromDictionary(filename string) *Boggler {
  t := trie.CreateTrieFromFilename(filename)
  if t == nil {
    return nil
  }
  return BogglerFromTrie(t)
}

func (b *Boggler) Score(lets string) int {
  if len(lets) != 16 {
    return -1
  }

  for i := 0; i < 16; i++ {
    b.bd[i] = int(lets[i] - 'a')
  }

  b.counter += 1
  b.InternalScore()
  return b.score
}

func (this *Boggler) InternalScore() int {
  this.used = 0
  this.score = 0
  var i uint
  for i = 0; i < 16; i++ {
    c := uint8(this.bd[i])
    if this.t.StartsWord(c) {
      this.DoDFS(i, 0, this.t.Descend(c))
    }
  }
  return this.score
}

func (b *Boggler) Hit(x, y, length uint, t *trie.Trie) {
  idx := x*4 + y
  if b.used & (1 << idx) == 0 {
    cc := uint8(b.bd[idx])
    if t.StartsWord(cc) {
      b.DoDFS(idx, length, t.Descend(cc))
    }
  }
}
func (b *Boggler) Hit3x(x, y, length uint, t *trie.Trie) {
  b.Hit(x, y, length, t)
  b.Hit(x + 1, y, length, t)
  b.Hit(x + 2, y, length, t)
}
func (b *Boggler) Hit3y(x, y, length uint, t *trie.Trie) {
  b.Hit(x, y, length, t)
  b.Hit(x, y + 1, length, t)
  b.Hit(x, y + 2, length, t)
}
func (b *Boggler) Hit8(x, y, length uint, t *trie.Trie) {
  b.Hit3x(x-1, y-1, length, t)
  b.Hit(x-1, y, length, t)
  b.Hit(x+1, y, length, t)
  b.Hit3x(x-1, y+1, length, t)
}

func (b *Boggler) DoDFS(i, length uint, t *trie.Trie) {
  //fmt.Printf("i=%d, len=%d, word=%s\n", i, length, trie.ReverseLookup(b.t, t))
  c := b.bd[i]
  b.used ^= (1 << i)
  if c == kQ {
    length += 2
  } else {
    length += 1
  }

  if t.IsWord() {
    // fmt.Println("Found ", trie.ReverseLookup(b.t, t))
    if t.GetMark() != b.counter {
      t.Mark(b.counter)
      b.score += kWordScores[length]
    }
  }

  l := length
  switch i {
    case 0*4 + 0: b.Hit(0, 1, l,t); b.Hit(1, 0, l,t); b.Hit(1, 1, l,t)
    case 0*4 + 1: b.Hit(0, 0, l,t); b.Hit3y(1, 0, l,t); b.Hit(0, 2, l,t)
    case 0*4 + 2: b.Hit(0, 1, l,t); b.Hit3y(1, 1, l,t); b.Hit(0, 3, l,t)
    case 0*4 + 3: b.Hit(0, 2, l,t); b.Hit(1, 2, l,t); b.Hit(1, 3, l,t)

    case 1*4 + 0: b.Hit(0, 0, l,t); b.Hit(2, 0, l,t); b.Hit3x(0, 1, l,t)
    case 1*4 + 1: b.Hit8(1, 1, l,t)
    case 1*4 + 2: b.Hit8(1, 2, l,t)
    case 1*4 + 3: b.Hit3x(0, 2, l,t); b.Hit(0, 3, l,t); b.Hit(2, 3, l,t)

    case 2*4 + 0: b.Hit(1, 0, l,t); b.Hit(3, 0, l,t); b.Hit3x(1, 1, l,t)
    case 2*4 + 1: b.Hit8(2, 1, l,t)
    case 2*4 + 2: b.Hit8(2, 2, l,t)
    case 2*4 + 3: b.Hit3x(1, 2, l,t); b.Hit(1, 3, l,t); b.Hit(3, 3, l,t)

    case 3*4 + 0: b.Hit(2, 0, l,t); b.Hit(2, 1, l,t); b.Hit(3, 1, l,t)
    case 3*4 + 1: b.Hit3y(2, 0, l,t); b.Hit(3, 0, l,t); b.Hit(3, 2, l,t)
    case 3*4 + 2: b.Hit3y(2, 1, l,t); b.Hit(3, 1, l,t); b.Hit(3, 3, l,t)
    case 3*4 + 3: b.Hit(2, 2, l,t); b.Hit(3, 2, l,t); b.Hit(2, 3, l,t)
  }

  b.used ^= (1 << i)
}
