import java.util.HashMap;
import java.util.Map;


class ReducedPartialSolution {
	String filename; // ps1
	
	HashMap<String, Integer> ps1;
	HashMap<String, Integer> ps2;

	public ReducedPartialSolution(HashMap<String, Integer> ps1, 
			                      HashMap<String, Integer> ps2, 
			                      String filename) {
		this.ps1 = ps1;
		this.ps2 = ps2;
		this.filename = filename;
	}
}

class ReduceWorker extends Thread {
	ReduceWorkPool wp;

	public ReduceWorker(ReduceWorkPool workpool) {
		this.wp = workpool;
	}

	/**
	 * Procesarea unei solutii partiale. Aceasta poate implica generarea unor
	 * noi solutii partiale care se adauga in workpool folosind putWork(). Daca
	 * s-a ajuns la o solutie finala, aceasta va fi afisata.
	 */
	void processPartialSolution(ReducedPartialSolution rps) {
		int nWordsPs1 = 0;
		int nWordsPs2 = 0;
		float sum = 0;
		float frecv1;
		float frecv2;
		
		// in lexic pun perechi cuvant, true, adica imi memorez reuniunea cuvintelor
		// din cele 2 documente (cum e specificat in enunt, desi in final practic
		// era echivalent cu intersectia lor)
		HashMap<String, Boolean> lexic = new HashMap<String, Boolean>();
		
		for(Map.Entry<String, Integer> entry : rps.ps1.entrySet()) {
			lexic.put(entry.getKey(), true);
			nWordsPs1 += entry.getValue();
		}
		
		for(Map.Entry<String, Integer> entry : rps.ps2.entrySet()) {
			lexic.put(entry.getKey(), true);
			nWordsPs2 += entry.getValue();
		}
		
		for(Map.Entry<String, Boolean> entry : lexic.entrySet()) {
			frecv1 = 0;
			frecv2 = 0;
			
			Integer count = rps.ps1.get(entry.getKey());
			if(count != null) {
				frecv1 = ((float)count / (float)nWordsPs1);
			}
			
			count = rps.ps2.get(entry.getKey());
			if(count != null) {
				frecv2 = ((float)count / (float)nWordsPs2);
			}
			
			sum += frecv1 * frecv2 * 100;
		}
		
		synchronized(MapReduce.reduceResults) {
			MapReduce.reduceResults.add(new reduceResult(rps.filename, sum));
		}
		
	}

	public void run() {
		System.out.println("Thread-ul worker " + this.getName()
				+ " a pornit...");
		while (true) {
			ReducedPartialSolution ps = wp.getWork();
			if (ps == null)
				break;

			processPartialSolution(ps);
		}
		System.out.println("Thread-ul worker " + this.getName()
				+ " s-a terminat...");
	}

}