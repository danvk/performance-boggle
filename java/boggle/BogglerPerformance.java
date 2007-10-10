package boggle;

import java.io.File;
import java.io.IOException;

public class BogglerPerformance {
	private static double secs() {
		return 0.001 * System.currentTimeMillis();
	}
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		System.out.println(System.getProperty("user.dir"));

		String dict_file = "words";
		if (args.length == 1) dict_file = args[0];

		Trie t = new Trie();
		try {
			File f = new File(dict_file);
			t.LoadFile(f);
		} catch (IOException e) {
			e.printStackTrace();
			return;
		}
		System.out.println("Loaded " + t.Size() + " words from dictionary");

		Boggler b = new Boggler(t);
		long hash = 1234;
		int prime = (1 << 20) - 3;
		int total_score = 0;

		String[] bases = { "abcdefghijklmnop",
				           "catdlinemaropets",
				         };
		double start = secs();
		for (int i=0; i<bases.length * 1; ++i) {
			b.ParseBoard(bases[i % bases.length]);
			for (int x1 = 0; x1 < 4; x1++) {
				for (int x2 = 0; x2 < 4; x2++) {
					for (int c1 = 0; c1 < 26; c1++) {
						b.SetCell(x1, 1, c1);
						for (int c2 = 0; c2 < 26; c2++) {
							b.SetCell(x2, 2, c2);
							int score = b.Score();
							hash *= (123 + score);
							hash = hash % prime;
							total_score += score;
						}
					}
				}
			}
		}
		double end = secs();
		
		double avg_score = 1.0 * total_score / b.NumBoards();
		double pace = 1.0 * b.NumBoards()/(end-start);
		System.out.println("Total socre: " + total_score + " = " + avg_score + " pts/bd");
		System.out.println("Score hash: " + Integer.toHexString((int) hash));
		System.out.println("Evaluated " + b.NumBoards() + " boards in " + (end-start) + " seconds = " + pace + " bds/sec");

		if (hash != 0x000C1D3D) {
			System.err.println("Hash mismatch, expected 0xC1D3D\n");
		}
	}

}
