import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;

import static java.lang.Math.*;


class reduceResult {
	String filename;
	float sim;
	
	reduceResult(String filename, float sim) {
		this.filename = filename;
		this.sim = sim;
	}
}

public class MapReduce {
	
	static int fragmentSize;
	static int nDocuments;
	static int nThreads;
	static String document;
	static float pragSimilaritate;
	static ArrayList<String> documente = new ArrayList<String>();
	static HashMap<String, HashMap<String, Integer>> mapResults;
	static ArrayList<reduceResult> reduceResults = new  ArrayList<reduceResult>();
	
	static MapWorkPool mapworkpool;
	static ReduceWorkPool reduceworkpool;
	
	static void readInput(String inputFilename)
	{
		try {
			BufferedReader in = new BufferedReader(new FileReader(inputFilename));
			
			document = in.readLine();
			fragmentSize = Integer.parseInt(in.readLine());
			pragSimilaritate= Float.parseFloat(in.readLine());
			nDocuments = Integer.parseInt(in.readLine());
			boolean duplicate = false;
	
			for(int i = 0; i < nDocuments; i++){
				String filename = in.readLine();
				if(!filename.equals(document)) {
					documente.add(filename);
				}
				else 
				{
					duplicate = true;
				}
			}
			
			if(duplicate) {
				nDocuments--;
			}
			
			in.close();
		} 
		catch (FileNotFoundException e) {
			e.printStackTrace();
		} 
		catch (IOException e) {
			e.printStackTrace();
		} 
	}
	
	static void afisare(String outputFilename) {

		try {
			BufferedWriter out = new BufferedWriter(new FileWriter(outputFilename));
			
			out.write("Rezultate pentru: (" + document + ")\n\n");
			for(int i = 0; i < reduceResults.size() && reduceResults.get(i).sim > pragSimilaritate; i++) {
				out.write(reduceResults.get(i).filename + " (" + 
						  new DecimalFormat("#.###").format( reduceResults.get(i).sim) + 
						  "%)\n");
			}
			
			out.close();
		} 
		catch (FileNotFoundException e) {
			e.printStackTrace();
		} 
		catch (IOException e) {
			e.printStackTrace();
		} 
	}
	
	static void solveMapTasks()
	{
		mapworkpool = new MapWorkPool(nThreads);
		// procesam fisierul pentru care verificam gradul de plagiere
		long len = 0;
		try {
			RandomAccessFile file = new RandomAccessFile(document, "r");
			len = file.length();
			file.close();
		}
		catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		catch (IOException e) {
			e.printStackTrace();
		} 
		
		for(long j = 0; j < len; j += fragmentSize)
		{
			mapworkpool.putWork(new MapPartialSolution(document, 
					                                   mapResults.get(document),
					                                   j, min(j + fragmentSize, len)));
		}
		
		
		// procesam celelalte fisiere
		for(int i = 0; i < nDocuments; i++) {
			len = 0;
			try {
				RandomAccessFile file = new RandomAccessFile(documente.get(i), "r");
				len = file.length();
				file.close();
			}
			catch (FileNotFoundException e) {
				e.printStackTrace();
			}
			catch (IOException e) {
				e.printStackTrace();
			} 
			
			for(long j = 0; j < len; j += fragmentSize)
			{
				mapworkpool.putWork(new MapPartialSolution(documente.get(i), 
						                                   mapResults.get(documente.get(i)),
						                                   j, min(j + fragmentSize, len)));
			} 
		
		}
		
		
		// pornim threaduri worker
		ArrayList<MapWorker> threads = new ArrayList<MapWorker>();
		for(int i = 0; i < nThreads; i++) {
			MapWorker mapworker = new MapWorker(mapworkpool);
			mapworker.start();
			threads.add(mapworker);
		}
		
		try {
			for(int i = 0; i < nThreads; i++) {
				threads.get(i).join();
			}
		}
		catch(Exception e) {};
		
	}
	
	static void solveReduceTasks() {
		reduceworkpool = new ReduceWorkPool(nThreads);
		
		for(int i = 0; i < nDocuments; i++) {
			reduceworkpool.putWork(new ReducedPartialSolution(mapResults.get(document),
					               mapResults.get(documente.get(i)), documente.get(i)));		
		}
		
		ArrayList<ReduceWorker> threads = new ArrayList<ReduceWorker>();
		for(int i = 0; i < nThreads; i++) {
			ReduceWorker reduceworker = new ReduceWorker(reduceworkpool);
			reduceworker.start();
			threads.add(reduceworker);
		}
		
		try {
			for(int i = 0; i < nThreads; i++) {
				threads.get(i).join();
			}
		}
		catch(Exception e) {};
		
		
		// sortam rezultatele finale descrescator dupa gradul de similaritate
		Collections.sort(reduceResults,new Comparator<reduceResult>() {
            public int compare(reduceResult a, reduceResult b) {
                if((double)a.sim > (double)b.sim) {
                	return -1;
                }
                else if ((double)a.sim < (double)b.sim) {
                	return 1;
                }
                
                return 0;
            }
		});
	}

	public static void main(String[] args) {
		
		nThreads = Integer.parseInt(args[0]);
		readInput(args[1]);
		
		mapResults = new  HashMap<String, HashMap<String, Integer>>();
		
		mapResults.put(document, new HashMap<String, Integer>());
		for(int i = 0; i < nDocuments; i++) {
			mapResults.put(documente.get(i), new HashMap<String, Integer>());
		}
		
		solveMapTasks();
		solveReduceTasks();
		afisare(args[2]);
		
	}
}
