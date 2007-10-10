package boggle;

/**
 * Class for scoring Boggle boards... FAST!
 * @author danvk
 */
public class Boggler {
	public Boggler(Trie t) {
		dict_ = t;
		num_boards_ = 0;
		bd_ = new int[4][4];
	}
	
	/**
	 * Parses a sixteen-character board spec. Does NO checking.
	 * @param lets
	 * @return Was the board successfully parsed?
	 */
	public boolean ParseBoard(String lets) {
		for (int i=0; i<16; ++i)
			bd_[i/4][i%4] = lets.charAt(i) - 'a';
		return true;
	}
	
	public String Board() {
		StringBuilder b = new StringBuilder();
		for (int i=0; i<4; i++)
			for (int j=0; j<4; j++)
				b.append((char)('a' + bd_[i][j]));
		return b.toString();
	}
	
	/**
	 * Scores the current board and increments the board counter.
	 * @return The score
	 */
	public int Score() {
		num_boards_++;
		int score = 0;
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				int c = bd_[i][j];
				if (dict_.StartsWord(c))
					score += DoDFS(i, j, 0, dict_.Descend(c));
			}
		}
		return score;
	}
	
	public int Score(String lets) {
		if (!ParseBoard(lets))
			return -1;
		return Score();
	}
	
	public void SetCell(int x, int y, int c) {
		bd_[x][y] = c;
	}
	
	public int NumBoards() {
		return num_boards_;
	}
	
	private int DoDFS(int x, int y, int len, Trie t) {
		int c = bd_[x][y];
		bd_[x][y] = kCellUsed;

		len += (c==kQ ? 2 : 1);
		int score = 0;
		if (t.IsWord() && t.Mark() != num_boards_) {
			score += kWordScores[len];
			t.Mark(num_boards_);
		}

		int lx = x==0 ? 0 : x-1; int hx = x==3 ? 4 : x+2;
		int ly = y==0 ? 0 : y-1; int hy = y==3 ? 4 : y+2;
		for (int cx=lx; cx<hx; ++cx) {
			for (int cy=ly; cy<hy; ++cy) {
				int cc = bd_[cx][cy];
				if (cc == kCellUsed) continue;
				if (t.StartsWord(cc))
					score += DoDFS(cx, cy, len, t.Descend(cc));
			}
		}

		bd_[x][y] = c;
		return score;
	}
	
	private Trie dict_;
	private int[][] bd_;
	private int num_boards_;
	
	static int kWordScores[] = { 0, 0, 0, 1, 1, 2, 3, 5, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11 };
	static int kCellUsed = -1;
	static int kQ = 'q' - 'a';
}
