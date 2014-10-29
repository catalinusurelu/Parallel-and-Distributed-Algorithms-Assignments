import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import static java.lang.Math.*;

class MapPartialSolution {
	String filename;
	HashMap<String, Integer> partialSol;
	long start;
	long end;

	public MapPartialSolution(String filename, HashMap<String, Integer> partialSol, 
			                  long start, long end) {
		this.filename = filename;
		this.partialSol = partialSol;
		this.start = start;
		this.end = end;
	}
}

class MapWorker extends Thread {
	MapWorkPool wp;

	public MapWorker(MapWorkPool workpool) {
		this.wp = workpool;
	}

	/**
	 * Procesarea unei solutii partiale. Aceasta poate implica generarea unor
	 * noi solutii partiale care se adauga in workpool folosind putWork(). Daca
	 * s-a ajuns la o solutie finala, aceasta va fi afisata.
	 */
	void processPartialSolution(MapPartialSolution ps) {
		try {
			byte[] fragment = new byte[(int) (ps.end - ps.start) + 1];
			ArrayList<Byte> rest = new ArrayList<Byte>();
			String fragmentStr;
			String restStr;
			int fragmentStartOffset = 0;

			RandomAccessFile in = new RandomAccessFile(ps.filename, "r");
			in.seek(max(ps.start - 1, 0)); // an nevoie de un caracter din urma ca sa stiu
			in.readFully(fragment);        // daca e cuvant nou, sau interiorul unui cuvant

			String lastChar = new String(new byte[]{ (byte)fragment[fragment.length - 1] }, "US-ASCII");
			if(Character.isLetter(lastChar.charAt(0))) {
				int b;
				while ((b = in.read()) != -1) {
	
					String c = new String(new byte[]{ (byte)b }, "US-ASCII");
					// citesc pana cand gasesc un delimitator, adica tot cuvantul
					// de la sfarsit (conform cerintei)
					if(!Character.isLetter(c.charAt(0))) {
						break;
					}
					rest.add((byte) b);
				};
			}

			// convertes byte-ii cititi in string
			byte[] restBytes = new byte[rest.size()];
			for (int i = 0; i < restBytes.length; i++) {
				restBytes[i] = rest.get(i);
			}
			restStr = new String(restBytes, "US-ASCII");
			
			// sar peste bucata de inceput daca incep in mijlocul unui cuvant
			// adica citesc pana cand gasesc un delimitator (daca incep de la 0,
			// stim sigur ca nu incep in mijlocul unui cuvant)
			String temp = new String(fragment, "US-ASCII");
			if (ps.start != 0) {
				while (fragmentStartOffset < (int) (ps.end - ps.start) + 1) {
					if (!(Character.isLetter(temp.charAt(fragmentStartOffset)))) {
						break;
					}
					fragmentStartOffset++;
				};
			}

			fragmentStr = temp.substring(fragmentStartOffset) + restStr;

			// folosesc ca delimitatori orice nu e litera (practic cuvintele
			// sunt despartite de orice simbol inafara de litere, adica orice
			// nu e litera e delimitator - chiar si liniuta de despartie
			// (din punct de vedere lexical este adevarat)
			String[] words = fragmentStr.toLowerCase().split("[^a-z]+");
			HashMap<String, Integer> partialMap = new HashMap<String, Integer>();

			// solutia pentru fragmentul curent
			for (String word : words) {
				if(!word.equals("")) {
					if (!partialMap.containsKey(word)) {
						partialMap.put(word, 1);
					} else {
						partialMap.put(word, partialMap.get(word) + 1);
					}
				}
			}

			// adaug solutia pentru fragmentul curent la solutia partiala globala
			synchronized (ps.partialSol) {
				for (Map.Entry<String, Integer> entry : partialMap.entrySet()) {
					String word = entry.getKey();
					if (!ps.partialSol.containsKey(word)) {
						ps.partialSol.put(word, partialMap.get(word));
					} else {
						ps.partialSol.put(word, ps.partialSol.get(word) + partialMap.get(word));
					}
				}
			}

			in.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public void run() {
		System.out.println("Thread-ul worker " + this.getName()
				+ " a pornit...");
		while (true) {
			MapPartialSolution ps = wp.getWork();
			if (ps == null)
				break;

			processPartialSolution(ps);
		}
		System.out.println("Thread-ul worker " + this.getName()
				+ " s-a terminat...");
	}

}