// Usurelu Catalin Constanin
// 333CA

#include <fstream>
#include <math.h>
#include <mpi.h>

#include "Complex.hpp"

#define MANDELBROT  0
#define JULIA       1
#define NUM_COLORS  256

double x_min, x_max;
double y_min, y_max;
int width, height;
double resolution;
int max_steps;
int type;

int stripSize;
int y_start;
int y_finish;

Complex c;

unsigned char **image;

using namespace std;

// masterul citeste fisierelul de intrare
void read(char* filename)
{
    ifstream in(filename);
    in >> type >> x_min >> x_max >> y_min >> y_max >> resolution >> max_steps;
    if(type == JULIA)
    {
        in >> c.x >> c.y;
    }
    in.close();
}

void compute_mandelbrot(int y_start, int y_finish)
{
    int i, j;
    int step;
    Complex z;
    Complex c;

    // din cauza unor errori de precizie folosind double, am calculat i si j
    // in tandem cu cresterea rezolutiei. Putem sa le calculez separat
    // spre exemple i = (c.y - y_min) / resolution, dar nu putem compara corect
    // cu limita superioara ( 2.4 < 2.4 imi dadea true)
    for (c.y = y_min + y_start * resolution, i = y_start; 
         i < y_finish; c.y += resolution, i++)
    {
        for (c.x = x_min, j = 0; j < width; c.x += resolution, j++)
        {
            z = Complex(0, 0);
            step = 0;
            while (z.x * z.x + z.y * z.y < 2 * 2 && step < max_steps)
            {
                z = z * z + c;
                step ++;
            }           
            
            image[i][j] = step % NUM_COLORS;
        }
    }
}

void compute_julia(int y_start, int y_finish, Complex c)
{
    int i, j;
    Complex z;
    int step;

    // idem ca la mandelbrot
    for (z.y = y_min + y_start * resolution, i = y_start;
         i < y_finish; z.y += resolution, i++)
    {
        
        
        for (z.x = x_min, j = 0; j < width; z.x += resolution, j++)
        {
            Complex tmp = z;
        
            step = 0;
            while (z.x * z.x + z.y * z.y < 2 * 2 && step < max_steps)
            {
                z = z * z + c;
                step ++;
            }
            
            z = tmp;      
            
            image[i][j] = step % NUM_COLORS;
        }
    }
}

void write(char *filename)
{
    ofstream out(filename);
    out << "P2\n" 
        << width << " " << height << '\n' 
        << NUM_COLORS - 1 << '\n';

    for (int i = height - 1; i >= 0; i--)
    {
        for (int j = 0; j < width; j++)
        {
           out << (int)image[i][j] << " ";
        }
        out << '\n';
    }
    out.close();
}

// Aloca spatiu pentru imagine. Masterul aloca toata matricea
// corespunzatoare imagine ca sa poata primii de la celelalta
// procese, iar celelalte procese isi aloca doar liniile pe care
// vor lucra
void allocate_memory(int rank)
{
    image = new unsigned char* [height];
    if(rank == 0)
    {
        for(int i = 0; i < height; i++)
        {
            image[i] = new unsigned char[width];
        }
    }
    else
    {
        for(int i = y_start; i < y_finish; i++)
        {
            image[i] = new unsigned char[width];
        }
    }
}

// dealocam in acelasi mod cum am alocat (master - tot, celelalta
// procese, doar liniile lor)
void deallocate_memory(int rank)
{
    if(rank == 0)
    {
        for(int i = 0; i < height; i++)
        {
            delete image[i];
        }
    }
    else
    {
        for(int i = y_start; i < y_finish; i++)
        {
            delete image[i];
        }
    }
    delete image;
}

// masterul citeste datele de intrare si le trimite la celalte procese,
// celelalte procese primes datele de intrare
void read_and_distribute_startup_info(char* input, int rank, int numtasks)
{
     MPI_Status status;
     if (rank == 0) 
        {
            read(input);
                
            for(int i = 1; i < numtasks; i++)
            {
                MPI_Send(&type, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                MPI_Send(&x_min, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
                MPI_Send(&x_max, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
                MPI_Send(&y_min, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
                MPI_Send(&y_max, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
                MPI_Send(&resolution, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
                MPI_Send(&max_steps, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                
                if(type == JULIA)
                {
                    MPI_Send(&c.x, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
                    MPI_Send(&c.y, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
                }
            }
        }
        else
        {
            MPI_Recv(&type, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&x_min, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&x_max, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&y_min, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&y_max, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&resolution, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&max_steps, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
            
            if(type == JULIA)
            {
                MPI_Recv(&c.x, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
                MPI_Recv(&c.y, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
            }
        }    
}

int main(int argc, char **argv)
{
    int rank;
    int numtasks;
    MPI_Status status;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    read_and_distribute_startup_info(argv[1], rank, numtasks);

    // fiecare proces în parte isi calculeaza intervalul din matricea finala 
    // de pixeli pe care lucreaza
    width = (x_max - x_min) / resolution;
    height = (y_max - y_min) / resolution;
    stripSize = height / numtasks;
    y_start = stripSize * rank;
    y_finish = (rank == numtasks - 1 ? height : stripSize * (rank + 1));
    
    // alocam memorie
    allocate_memory(rank);
    
    // calculam mandelbrot sau julia
    if (type == MANDELBROT)
    {
        compute_mandelbrot(y_start, y_finish);
    }
    else
    {
        compute_julia(y_start, y_finish, c);
    }
    

    // masterul primeste date de la celalte procese. Pentru fiecare proces,
    // asteapta liniile procesate de acestea - cunoaste liniile de care
    // se ocupa fiecare proces
    if (rank == 0)
    {
        for (int i = 1; i < numtasks; i ++)
        {
            int upper_lim = (i == numtasks - 1 ? height : stripSize * (i + 1));
            for(int j = i * stripSize; j < upper_lim; j++)
            {
                MPI_Recv(image[j], width, MPI_UNSIGNED_CHAR, 
                         i, 1, MPI_COMM_WORLD, &status);
            }
        }
    }
    else 
    {
        // celelalte procese trimit liniile procesate din matrice
        for(int i = y_start; i < y_finish; i++)
        {
            MPI_Send(image[i], width, MPI_UNSIGNED_CHAR, 0, 1, MPI_COMM_WORLD);
        }
        
    }

    if (rank == 0)
    {
        write(argv[2]);
    }

    deallocate_memory(rank);
    MPI_Finalize();

    return 0;
}
