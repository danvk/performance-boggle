package boggle;

import static org.junit.Assert.*;
import org.junit.Before;
import org.junit.Test;

public class BogglerTest {

	@Before
	public void setUp() throws Exception {
		t = new Trie();
		t.AddWord("tea");
		t.AddWord("eta");
		t.AddWord("ate");
		t.AddWord("eat");
		t.AddWord("teak");
		t.AddWord("fiver");
		t.AddWord("sixers");
		assertEquals(7, t.Size());
	}
	
	@Test public void testSolving() {
		Boggler b = new Boggler(t);
		assertEquals(0, b.NumBoards());
		assertEquals(4, b.Score("texxaxxxyyyyzzzz"));
		assertEquals(5, b.Score("texxakxxyyyyzzzz"));
		assertEquals(2, b.NumBoards());

		b.SetCell(0, 0, 'f' - 'a');
		b.SetCell(1, 0, 'i' - 'a');
		b.SetCell(2, 0, 'v' - 'a');
		b.SetCell(3, 0, 'e' - 'a');
		b.SetCell(3, 1, 'r' - 'a');
		assertEquals(2, b.Score());
		assertEquals(3, b.Score("sxxxixxxexxxrsxx"));
		assertEquals(4, b.NumBoards());
	}

	private Trie t;
}
