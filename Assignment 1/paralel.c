#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"
#include "omp.h"

#define INF 0xffff

int ***conf; // matrice de configuratii (facem modulo 2 la fiecare saptamana)
int* c;	// vectorul cu numarul de senatori pentru fiecare culoare
int N; // latura matrice
int nc; // numar culori
int s; // numar saptamani


void read_input_and_allocate(char* file)
{
	FILE *f = fopen(file, "r");
	fscanf(f, "%d", &N);
	fscanf(f, "%d", &nc);
	int i, j;

	conf = (int***) malloc(2 * sizeof(int**));
	c = (int*) calloc(nc, sizeof(int));

	for (i = 0; i < 2; i++)
		conf[i] = (int**) malloc(N * sizeof(int*));

	for (i = 0; i < 2; i++)
		for (j = 0; j < N; j++)
			conf[i][j] = (int*) malloc(N * sizeof(int));

	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++)
		{
			fscanf(f, "%d ", &conf[1][i][j]);
			c[conf[1][i][j]]++;
		}

	fclose(f);
}

inline int min(int a, int b)
{
	return a < b ? a : b;
}

inline int max(int a, int b)
{
	return a > b ? a : b;
}

inline int dist(int i, int j, int i1, int j1)
{
	return max(abs(i - i1), abs(j - j1));
}

int max_vector(int* v, int n)
{
	int maxc = 0;
	int i;
	for (i = 0; i < nc; i++)
	{
		if (v[i] == INF)
		{
			v[i] = 0;
		}
		if (v[maxc] < v[i])
		{
			maxc = i;
		}

	}

	return maxc;
}

void simulate(char* output)
{
	FILE *f = fopen(output, "w");

	int i, i1, j, j1, k, l, m, K;

	for (K = 1; K <= s; K++)
	{
        k = K % 2;
		memset(c, 0, nc * sizeof(int));

      // distanta minima actuala pana la un senator de culoare indice dmin
		#pragma omp parallel for \
			schedule(runtime) \
			private(i, j, i1, j1, m) \
			shared(conf, c)
		for (i = 0; i < N; i++)
		{
            int *dmin = (int*) calloc(nc, sizeof(int));
			for (j = 0; j < N; j++)
			{
				for(m = 0; m < nc; m++)
				{
					dmin[m] = INF;
				}

				for (i1 = 0; i1 < N; i1++)
				{
					for (j1 = 0; j1 < N; j1++)
					{
						if (!(i == i1 && j == j1))
						{
							dmin[conf[k][i1][j1]] = min(
									dmin[conf[k][i1][j1]],
									dist(i, j, i1, j1));
						}
					}
				}

				conf[(k + 1) % 2][i][j] = max_vector(dmin, nc);

#pragma omp critical
				{
					c[conf[(k + 1) % 2][i][j]]++; // numaram senatori
				}
			}
		}

		for (l = 0; l < nc; l++)
		{
			fprintf(f, "%d ", c[l]);
		}
		fprintf(f, "\n");
	}

	for (i = 0; i < N; i++)
	{
		for (j = 0; j < N; j++)
		{
			fprintf(f, "%d ", conf[(s + 1) % 2][i][j]);
		}
		fprintf(f, "\n");
	}
	fclose(f);
}

int main(int argc, char *argv[])
{
	s = atoi(argv[1]);
	read_input_and_allocate(argv[2]);
	simulate(argv[3]);

	return 0;
}
