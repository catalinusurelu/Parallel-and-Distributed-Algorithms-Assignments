// Usurelu Catalin Constanin
// 333CA

#include <fstream>
#include <cstring>
#include <sstream>
#include <vector>
#include <string>
#include <queue>
#include <cstdlib>
#include <limits>
#include <algorithm>
#include <mpi.h>

#define MESSAGE_SIZE 255

using namespace std;

int parinte;
vector<int> copii;

// Aloc memorie pentru topologie si o intializez cu 0
int** aloca_memorie_topologie(int height, int width)
{
	height++;
	width++;
    int** topologie = new int*[height];
	for(int i = 0; i < height; i++)
	{
		topologie[i] = new int[width];
		memset(topologie[i], 0, width * sizeof(int)); // initializez totul cu 0
	}

   return topologie;
}

// Trimit topologia intr-un singur mesaj (nu merge trimisa linie cu line ca se 
// intercaleaza mesajele)
void MPI_Send_Topology(int** topologie, int size, int dest, int tag)
{
	size++;
	int* linear = new int[size*size];
    for (int i = 0; i < size; i++)
    {
    	for(int j = 0; j < size; j++)
    	{
    		linear[i * size + j] = topologie[i][j];
    	}
    }

    MPI_Send(linear, size * size, MPI_INT, dest, tag, MPI_COMM_WORLD);
    
    delete linear;
}

// Primesc topologie de la orice tag 
void MPI_Recv_Topology(int** &topologie_primita, int size, MPI_Status& status)
{
	size++;
	int* linear = new int[size*size];
	MPI_Recv(linear, size * size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    topologie_primita = aloca_memorie_topologie(size, size);

    for (int i = 0; i < size; i++)
	{
		for(int j = 0; j < size; j++)
		{
			topologie_primita[i][j] = linear[i * size + j];
		}
	}
    
    delete linear;
}


void MPI_Recv_Topology(int** &topologie_primita, int size, int tag, MPI_Status& status)
{
	size++;
	int* linear = new int[size*size];
	MPI_Recv(linear, size * size, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
    
    topologie_primita = aloca_memorie_topologie(size, size);

    for (int i = 0; i < size; i++)
	{
		for(int j = 0; j < size; j++)
		{
			topologie_primita[i][j] = linear[i * size + j];
		}
	}
    
    delete linear;
}

// Face operatia de OR intre elementele matricilor si salveaza rezultatul in prima matrice
void matrix_or(int** &mat1, int** mat2, int width, int height)
{
	for (int i = 0; i < width; i++)
	{
		for(int j = 0; j < height; j++)
		{
			mat1[i][j] = mat1[i][j] | mat2[i][j];
		}
	}
}

// Verifica daca topologia este nula
bool isNull(int** topologie, int width, int height)
{
	bool ret = true;
	for (int i = 0; i < width; i++)
	{
		for(int j = 0; j < height; j++)
		{
			ret = ret && (topologie[i][j] != 1);
		}
	}
	
	return ret;
}

// Adaugam tag topologiei (pentru a diferentia 
// tag-urile mesajelor care contin topologia)
void addTag(int** topologie, int size, int tag)
{
	topologie[size][size] = tag;
}

int getTag(int** topologie, int size)
{
	return topologie[size][size];
}

// Initializez topologia cunoscuta de procesul curent cu lista sa de adiacenta citita din fisier
void initializeaza_topologie(int** &topologie, int& nr_vec, const int& size, const int& rank, const char* fisier_topologie)
{
	ifstream f(fisier_topologie);
	stringstream ss;
	string line;
	int nod;
	int vec;
	char char_linie;	
	
	topologie = aloca_memorie_topologie(size, size);
	
	nr_vec = 0;
	for (int i = 0; i < size; i ++)
	{
		getline(f, line);
		ss.clear();
		ss.str(line);
		ss >> nod;	// nodul caruia ii corespunde lista de adiacenta
		
		// linia corespunde nodului curent (este linia pe care o caut)
		if(nod == rank)
		{
			// sare peste liniutza
			ss >> char_linie;
			
			// citesc vecinii
			while(ss >> vec)
			{
				topologie[rank][vec] = 1;
				nr_vec++;
			}
			break; // doar linia asta ma interesa
		}
	}
	    
	f.close();
}

// Este practic implementarea pseudocodului de la laborator plus
// trimiterea la sfarsit de la radacina a topologiei finale
// celorlalte noduri
int** stabileste_topologie(const char* fisier_topologie)
{
    int size;
    int rank;
    MPI_Status status;
    
    // tag-uri
    const int sondaj_ecou = 1001;
    const int sondaj = 0;
    const int ecou = 1;
    const int ecou_final = 1002;
    const int topologie_finala = 1003;
    
    int tag;
    int source;
    int r;
    int temp;
    int nr_vec;
    
    int** topologie;
    int** topologie_primita;
    int** topologie_nula;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    initializeaza_topologie(topologie, nr_vec, size, rank, fisier_topologie);
    topologie_primita = aloca_memorie_topologie(size, size);
    topologie_nula = aloca_memorie_topologie(size, size);

    r = nr_vec;
    // Nodul radacina initializeaza procesul trimitand mesaje sondaj tuturor vecinilor (fiilor)
    if (rank == 0)
    {
        for (int i = 0; i < size; i++)
        {
        	// Trimitem ca mesaj de sondaj o topologie nula
            if (topologie[rank][i])
            {
            	addTag(topologie_nula, size, sondaj);
            	MPI_Send_Topology(topologie_nula, size, i, sondaj_ecou);
            }
        }
    }
    
    // Propag mesajele de sondaj pana la furnze
    else
    {
    	MPI_Recv_Topology(topologie_primita, size, status);
        parinte = status.MPI_SOURCE;
        
        r = r - 1; // numarul de ecouri pe care le asteapta (nu asteapta si de la parinte)

        // Propag mesaje sondaj la toti inafara de parinte
        for (int i = 0; i < size; i ++)
        {
            if (topologie[rank][i] && i != parinte)
            {
            	addTag(topologie_nula, size, sondaj);
            	MPI_Send_Topology(topologie_nula, size, i, sondaj_ecou);
            }
        } 
    }

    
    // Primesc ecouri (frunzele sar peste partea asta)
    while(r > 0)
    {
    	MPI_Recv_Topology(topologie_primita, size, status);
    	tag = getTag(topologie_primita, size);
    	
    	if(tag == sondaj)
    	{
    		source = status.MPI_SOURCE;
    		addTag(topologie_nula, size, ecou);
    		MPI_Send_Topology(topologie_nula, size, source, sondaj_ecou);
    	}
    	else if(tag == ecou)
    	{
    		matrix_or(topologie, topologie_primita, size, size);
    		r = r - 1;
    		
    		// Doar copii din arbore trimit topologie full => daca topologie nu e null => e copil
    		// Nu exista caz in care topologia sa fie nula
    		if(!isNull(topologie_primita, size, size))
    		{
    			copii.push_back(status.MPI_SOURCE);
    		}
    	}
    }       
    
    // Trimit ecouri spre radacina (intai primesc ecourile de jos ca sa trimit in sus)
    if(rank != 0)
    {
    	addTag(topologie, size, ecou);
    	MPI_Send_Topology(topologie, size, parinte, sondaj_ecou);
    }
    
    if(rank == 0)
    {
    	for (int i = 0; i < size; i++)
		{
			if (topologie[rank][i])
			{
				addTag(topologie, size, topologie_finala);
				MPI_Send_Topology(topologie, size, i, topologie_finala);
			}
		}
    }
    else
    {
    	
    	MPI_Recv_Topology(topologie, size, topologie_finala, status);
    	
    	for (int i = 0; i < copii.size(); i++) 
    	{
    		addTag(topologie, size, topologie_finala);
    		MPI_Send_Topology(topologie, size, copii[i], topologie_finala);
		}
    }
    
    return topologie;
}

// Folosim algoritmul de dirijare calea cea mai scurta
// Cum muchiile au cost 1 putem folosi BFS ca echivalent al lui Dijkstra
int* calculeaza_tabela_rutare(int** topolgie)
{
	int size;
	int rank;
	MPI_Status status;
	
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
	    
	int* tabela = new int[size];
	int* viz = new int[size];
	memset(viz, 0, size * sizeof(int));
	
	queue<int> q;
	
	tabela[rank] = rank;
	viz[rank] = 1;
	
	// Introduce vecinii si ii marcam ca potentiali next_hop
	for (int i = 0; i < size; i++)
	{
		if (topolgie[rank][i] == 1)
		{
		  q.push(i);
		  tabela[i] = i; // next hop
		  viz[i] = 1;
		}
	}
	
	while(!q.empty())
	{
		int nod = q.front();
		q.pop();
		for (int i = 0; i < size; i++)
		{
			if (topolgie[nod][i] == 1 && viz[i] == 0)
			{
			  q.push(i);
			  tabela[i] = tabela[nod];
			  viz[i] = 1;
			}
		}
	}
	
	delete viz;
	return tabela;
}


// Afisam tabelele de rutare si matricea de adiacenta
void print_end_phase_1(int** topologie, int* tabela)
{
	int size;
	int rank;
	MPI_Status status;
	
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    stringstream output;
    
	for (int i = 0; i < size; i++)
	{
		output.str("");
		output.clear();
		
	    if (rank == i)
	    {
	    	output << "ID process: " << i << '\n';
	    	for (int j = 0; j < size; j++)
	        {
	    		// destinatie, next_hop
	    		output << j << " " << tabela[j] << '\n';
	        }
	    }
	    
	    output << "\n\n";
	    
	    // Procesul 0 afiseaza si matricea de adiacenta
	    if(rank == i && rank == 0)
	    {
	    	output << "Matricea de adiacenta:" << '\n';
	    	for (int j = 0; j < size; j++)
	    	{
	    		for (int k = 0; k < size; k++)
	    		{
	    			output << topologie[j][k] << " ";
	    		}
	    		
	    		output << "\n";
	    	}
	    	
	    	output << "\n";
	    }
	    
	    if(rank == i)
	    {
	    	cout << output.str() << endl;
	    }
	    
	    MPI_Barrier(MPI_COMM_WORLD);
	}
}

// Verific daca toate elementele din vec sunt setate pe 1
bool full(int* vec, int size)
{
	bool ret = true;
	for(int i = 0; i < size; i++)
	{
		ret = ret && (vec[i] == 1);
	}
	
	return ret;
}

// Afisam informatii despre mesajul primit
void print_info(int rank, int src, int dest, int* routes, string body)
{
	stringstream output; // Il bag intr-un string ca poate se suprapun mesajele in cout
	output << "Nodul " << rank << " a primit mesajul cu sursa " << src << " si destinatia "
		   << dest << "\nNext hop: " << routes[dest] << '\n' << "Continut mesaj: " << body << endl << endl;
	cout << output.str();
}


// Faza 2
void begin_communications(int** topologie, int* routes, char* fisier_mesaje)
{
	ifstream f(fisier_mesaje);
	stringstream ss;
	int rank;
	int size;
	int nr_messages;
	char c_message[MESSAGE_SIZE];
	string message;
	string src;
	string dest;
	string body;
	
	const int broadcast_initial = 1005;
	const int broadcast_final = 1006;
	const int phase_2 = 1007;
	
	map<string, bool> recvd_broadcast;
		
	MPI_Status status;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	int* broadcast_src = new int[size];
	memset(broadcast_src, 0, size * sizeof(int));
	
	// Trimit broadcast initial
	ss << rank << " " << "B" << " Finished_phase_1";
	memset(c_message, 0, MESSAGE_SIZE * sizeof(char));
	memcpy(c_message, ss.str().c_str(), ss.str().size() * sizeof(char));
	for(int i = 0; i < size; i++)
	{
		if(i != rank)
		{
			MPI_Send(c_message, MESSAGE_SIZE, MPI_CHAR, i, broadcast_initial, MPI_COMM_WORLD);
		}
	}
	
	// Asteptam mesaj de broadcast de la toate sursele
	broadcast_src[rank] = 1;
	while(!full(broadcast_src, size))
	{
		// Primest mesaj de broadcast
		MPI_Recv(c_message, MESSAGE_SIZE, MPI_CHAR, MPI_ANY_SOURCE, broadcast_initial, MPI_COMM_WORLD, &status);
		ss.clear();
		ss.str(string(c_message));
		ss >> src >> dest;
		
		// Marchez faptul ca am primit de la aceasta sursa
		broadcast_src[atoi(src.c_str())] = 1;
			
		// Trimitem broadcast mai departe
		for(int i = 0; i < size; i++)
			{
				if(topologie[rank][i] && i != rank && i != status.MPI_SOURCE)
				{
					MPI_Send(c_message, MESSAGE_SIZE, MPI_CHAR, i, broadcast_initial, MPI_COMM_WORLD);
				}
			}
	}

	//Trimit mesajele din fisier
	
    f >> nr_messages;

    for (int i = 0; i < nr_messages; i++)
    {
        getline(f, message);
        ss.clear();
        ss.str(message);
        memset(c_message, 0, MESSAGE_SIZE * sizeof(char));
        memcpy(c_message, ss.str().c_str(), ss.str().size() * sizeof(char));
        
        ss >> src >> dest;

        // Eu sunt sursa mesajului, deci il trimit
        if (atoi(src.c_str()) == rank)
        {
            if (dest == "B")
            {
                for (int j = 0; j < size; j ++)
                {
                    if (topologie[rank][j] && j != rank)
                    {
                        MPI_Send(c_message, MESSAGE_SIZE, MPI_CHAR, j, phase_2, MPI_COMM_WORLD);
                    }
                }
            }
            else
            {
            	// Trimit mai departe pe drumul cel mai scurt (next hop)
                MPI_Send(c_message, MESSAGE_SIZE, MPI_CHAR, routes[atoi(dest.c_str())], phase_2, MPI_COMM_WORLD);
            }
        }
    }
    

	// Trimit broadcast final
    memset(broadcast_src, 0, size * sizeof(int));
    ss.str("");
    ss.clear();
    ss << rank << " " << "B" << " Finished_phase_2";
    memset(c_message, 0, MESSAGE_SIZE * sizeof(char));
    memcpy(c_message, ss.str().c_str(), ss.str().size() * sizeof(char));
	for(int i = 0; i < size; i++)
	{
		if(i != rank)
		{
			MPI_Send(c_message, MESSAGE_SIZE, MPI_CHAR, i, phase_2, MPI_COMM_WORLD);
		}
	}

	broadcast_src[rank] = 1;
	// Primim mesaje pana cand toate celelalte noduri trimit broadcast final
    while (!full(broadcast_src, size))
    {
        MPI_Recv(c_message, MESSAGE_SIZE, MPI_CHAR, MPI_ANY_SOURCE, phase_2, MPI_COMM_WORLD, &status);
        ss.str("");
        ss.clear();
        ss << c_message;
        ss >> src >> dest;
        getline(ss, body);    
        
        // Afisam detalii mesaj - pentru broadcastul final nu afisam ca sunt generate prea multe
        // si nu se mai vede outpul bine (sunt greu de gasit mesajele care conteaza :) )
        if (dest != "B" || body != " Finished_phase_2")
        {	
			print_info(rank, atoi(src.c_str()), atoi(dest.c_str()), routes, body);
        }

        // Daca este broadcast final
        if (dest == "B" && body == " Finished_phase_2")
        {
    		broadcast_src[atoi(src.c_str())] = 1;
    			
    		// Trimitem broadcast mai departe
    		for(int i = 0; i < size; i++)
			{
				if(topologie[rank][i] && i != rank && i != status.MPI_SOURCE)
				{
					MPI_Send(c_message, MESSAGE_SIZE, MPI_CHAR, i, phase_2, MPI_COMM_WORLD);
				}
			}
        }
        else
        {
             // Daca am mai primit acelasi broadcast odata nu il mai trimitem din nou
             // Astfel prevenim intrare in bucla infinita sau sa trimitem aiurea mesaje
        	 if (dest == "B")
        	 {
        		if(recvd_broadcast.find(body) == recvd_broadcast.end())
				{
					recvd_broadcast[body] = true;
				}
				else
				{
					continue; // ii dam drop
				}
        		 
				for (int j = 0; j < size; j ++)
				{
					if (topologie[rank][j] && j != rank)
					{
						MPI_Send(c_message, MESSAGE_SIZE, MPI_CHAR, j, phase_2, MPI_COMM_WORLD);
					}
				}
			}
        	// Mesaj normal
        	else if(atoi(dest.c_str()) != rank)
            {
        		MPI_Send(c_message, MESSAGE_SIZE, MPI_CHAR, routes[atoi(dest.c_str())], phase_2, MPI_COMM_WORLD);
            }
        	 // Am ajuns la destinatie
        	else if(atoi(dest.c_str()) == rank)
        	{
        		print_info(rank, atoi(src.c_str()), atoi(dest.c_str()), routes, body);
        	}
        }
    }

    f.close();
}


// Implementam algoritmul de alegere a liderului ca in curs (alegem procesul cu id-ul
// cel mai mic). Ca lider adjunge alegem procesul cu urmatorul cel mai mic id)
int alege_lider(int except)
{
	int rank;
	int size;
	int src;
	int id;
	
	const int wakeup = 1010;
	const int choose_leaders = 1011;
		
	MPI_Status status;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	vector<int> vecini;
	vecini.push_back(parinte);
	for(int i = 0; i < copii.size(); i++)
	{
		vecini.push_back(copii[i]);
	}
	
	bool* rec = new bool[size];
	for(int i = 0; i < size; i++)
	{
		rec[i] = false;
	}
	
	bool ws = false;
	int wr = 0;
	int V = rank; // lider
	int r = vecini.size();
	
	if(rank == except)
	{
		// Ii dam liderului un id de valoare maxima ca sa nu mai fie luat in considerare
		// la calculul procesului cu id minim (lider secund)
		V = std::numeric_limits<int>::max(); 
	}
	
	enum State {sleep, leader, adjunct, lost} state = sleep;
	
	//Trimitem mesaje de trezire
	
	// Daca e frunza inseamna ca e initiator
	if(copii.size() == 0)
	{
		ws = true;
		for(int i = 0; i < vecini.size(); i++)
		{
			MPI_Send(&rank, 1, MPI_INT, vecini[i], wakeup, MPI_COMM_WORLD);
		}
	}
	
	// Primim mesaje de trezire
	while(wr < vecini.size())
	{
		MPI_Recv(&src, 1, MPI_INT, MPI_ANY_SOURCE, wakeup, MPI_COMM_WORLD, &status);
		wr++;
		if(ws == false)
		{
			ws = true;
			for(int i = 0; i < vecini.size(); i++)
			{
				MPI_Send(&rank, 1, MPI_INT, vecini[i], wakeup, MPI_COMM_WORLD);
			}
		}
	}
	
	// Aici incepe algoritmul tree
	
	// Alegem liderul
	while(r > 1)
	{
		MPI_Recv(&id, 1, MPI_INT, MPI_ANY_SOURCE, choose_leaders, MPI_COMM_WORLD, &status);
		rec[status.MPI_SOURCE] = true;
		r--;
		V = min(V, id);
	}
	
	// De la un singur vecin q0 nu s-a primit mesaj (daca radacina are 2 fii, de la unul primeste in
	// whileul anterior si de la celalt aici)
	int q0;
	for(int i = 0; i < vecini.size(); i++)
	{
		if(rec[vecini[i]] == false)
		{
			q0 = vecini[i];
		}
	}
	
	MPI_Send(&V, 1, MPI_INT, q0, choose_leaders, MPI_COMM_WORLD);
	MPI_Recv(&id, 1, MPI_INT, q0, choose_leaders, MPI_COMM_WORLD, &status);
	V = min(V, id);
	
	if(V == rank)
	{
		state = leader;
	}
	else
	{
		state = lost;
	}
	
	for(int i = 0; i < vecini.size(); i++)
	{
		if(vecini[i] != q0)
		{
			MPI_Send(&V, 1, MPI_INT, vecini[i], choose_leaders, MPI_COMM_WORLD);
		}
	}
	
	return V;
}

int main(int argc, char* argv[])
{
    int rank;
    int size;
    int** topologie;
    int* routes;
    
    MPI_Init(&argc, &argv);
    
	MPI_Status status;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	
    ////////////////////////////////////////
    // 				ETAPA 1              ///
    ////////////////////////////////////////
	
    topologie = stabileste_topologie(argv[1]);
    routes = calculeaza_tabela_rutare(topologie);
    print_end_phase_1(topologie, routes);
    
    ////////////////////////////////////////
    // 				ETAPA 2              ///
    ////////////////////////////////////////
    
	
    begin_communications(topologie, routes, argv[2]);
    
    ////////////////////////////////////////
    // 				ETAPA 3              ///
    ////////////////////////////////////////
    
    int lider = alege_lider(-1);
    int adjunct = alege_lider(lider);
	
    stringstream out;
    out.str("");
    out.clear();
    
	// Afisam liderul si liderul adjunct
	for (int i = 0; i < size; i++)
	{
		// Lasam spatiu intre etapa 2 si 3
		if(rank == i)
		{
			out << rank << " " << lider << " " << adjunct << endl;
			cout << out.str();
		}	
		MPI_Barrier(MPI_COMM_WORLD);
	}
	
  
    MPI_Finalize();

    return 0;
}


