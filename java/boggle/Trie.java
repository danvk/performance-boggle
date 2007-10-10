package boggle;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

/**
 * A Trie class that's particularly well-suited for solving Boggle boards.
 * @author danvk
 */
public final class Trie {
	public Trie() {
		children_ = new Trie[26];
		mark_ = 0;
		is_word_ = false;
		has_children_ = false;
	}
	
	// These six methods are all fast as could be
	public boolean IsWord() { return is_word_; }
	public boolean StartsWord(int i) { return children_[i] != null; }
	public Trie Descend(int i) { return children_[i]; }
	public void Mark(int mark) { mark_ = mark; }
	public int Mark() { return mark_; }
	public void MarkHigh() { mark_ |= 0x80000000; }

	// And now the slower methods used to construct tries
	public void AddWord(String word) {
		if (word.length() == 0) {
			is_word_ = true;
		} else {
			has_children_ = true;
			int c = word.charAt(0) - 'a';
			assert(0 <= c);
			assert(c <= 26);
			if (!StartsWord(c))
				children_[c] = new Trie();
			if (c != kQ)
				Descend(c).AddWord(word.substring(1));
			else
				Descend(c).AddWord(word.substring(2));
		}
	}
	
	/**
	 * Boggle words have between three and seventeen characters, and no 'q' not followed by a 'u'.
	 * @param word
	 * @return Whether word is a valid Boggle word.
	 */
	private static boolean IsBoggleWord(String word) {
		int size = word.length();
		if (size < 3 || size > 17) return false;
		for (int i=0; i<size; i++) {
			char c = word.charAt(i);
			if (c < 'a' || c > 'z') return false;
			if (c == 'q' && (i+1 >= size || word.charAt(i+1) != 'u')) return false;
		}
		return true;
	}
	
	/**
	 * Load an entire file's worth of words, one per line. Screens for valid Boggle words.
	 * @param file File containing one word/line.
	 */
	public void LoadFile(File file) throws IOException {
       FileReader file_reader = new FileReader(file);
       BufferedReader buf_reader = new BufferedReader(file_reader);
       do {
          String line = buf_reader.readLine();
          if (line == null) break;
          if (IsBoggleWord(line))
        	  AddWord(line);
       } while (true);
       buf_reader.close();
	}

	public boolean StartsAnyWord() { return has_children_; }
	public boolean IsWord(String wd) {
		if (wd.length() == 0) return IsWord();
		int c = wd.charAt(0) - 'a';
		if (c < 0 || c >= 26) return false;
		if (StartsWord(c)) {
			if (c == kQ) {
				if (wd.length() >= 2 && wd.charAt(1) == 'u')
					return Descend(c).IsWord(wd.substring(2));
				else
					return false;
			} else {
				return Descend(c).IsWord(wd.substring(1));
			}
		}
		return false;
	}
	
	/**
	 * Does a full traversal of the Trie and returns the number of words it contains.
	 * @return The number of complete words.
	 */
	public int Size() {
		int size = 0;
		if (IsWord()) size++;
		for (int i=0; i<26; i++) {
			if (StartsWord(i)) size += Descend(i).Size();	
		}
		return size;
	}
	
	
	private Trie[] children_;
	private boolean is_word_;
	private boolean has_children_;
	private int mark_;
	
	private static int kQ = 'q' - 'a';
}
