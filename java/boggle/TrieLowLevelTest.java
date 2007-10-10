package boggle;

import static org.junit.Assert.*;
import org.junit.Before;
import org.junit.Test;

public class TrieLowLevelTest {
	private Trie trie;
	
	@Before public void Setup() {
		trie = new Trie();
		trie.AddWord("log");
		trie.AddWord("logjam");
		trie.AddWord("lam");
	}
	
	@Test public void testDescend() {
		assertNull(trie.Descend(0));
		assertNotNull(trie.Descend('l' - 'a'));
		Trie child = trie.Descend('l' - 'a');
		assertTrue(child.StartsAnyWord());
		assertFalse(child.IsWord());
		assertNotNull(child.Descend('a' - 'a'));
		Trie lam = child.Descend(0).Descend('m' - 'a');
		assertNotNull(lam);
		assertTrue(lam.IsWord());
		assertFalse(lam.StartsAnyWord());
	}
	
	@Test public void testSetup() {
		assertEquals(3, trie.Size());
		assertTrue(trie.IsWord("lam"));
		assertTrue(trie.IsWord("log"));
		assertTrue(trie.IsWord("logjam"));
	}
}
