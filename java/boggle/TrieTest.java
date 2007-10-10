package boggle;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import org.junit.Test;
import junit.framework.TestCase;

public class TrieTest extends TestCase {
	@Test public void testSimpleLookup() {
		Trie t = new Trie();
		t.AddWord("blog");
		t.AddWord("blogger");
		t.AddWord("cat");
		assertEquals(3, t.Size());
	}
	
	@Test public void testFileLoad() {
		try {
			File tmp = File.createTempFile("trietesttmp", "txt");
			BufferedWriter out = new BufferedWriter(new FileWriter(tmp));
			out.write("agriculture\n");
			out.write("culture\n");
			out.write("boggle\n");
			out.write("tea\n");
			out.write("teapot\n");
			out.close();
			Trie t = new Trie();
			t.LoadFile(tmp);
			assertTrue(t.IsWord("agriculture"));
			assertTrue(t.IsWord("culture"));
			assertTrue(t.IsWord("boggle"));
			assertTrue(t.IsWord("tea"));
			assertTrue(t.IsWord("teapot"));
			assertTrue(!t.IsWord("teap"));
			assertTrue(!t.IsWord("random"));
			assertTrue(!t.IsWord("cultur"));
			assertEquals(5, t.Size());
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	@Test public void testQs() {
		Trie t = new Trie();
		t.AddWord("quiche");
		assertTrue(t.IsWord("quiche"));
		assertFalse(t.IsWord("qiche"));
	}
}
