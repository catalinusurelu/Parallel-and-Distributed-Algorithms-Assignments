#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"
#define INF 0xfffff

int ***conf;
int *c;
int *dmin;
int N;
int nc;
int s;
int searchedColors;
int nrColors;

void read_input_and_allocate(char* file)
{
	FILE *f = fopen(file, "r");
	fscanf(f, "%d", &N);
	fscanf(f, "%d", &nc);
	int i, j;

	conf = (int***) malloc(2 * sizeof(int**));
	c = (int*) calloc(nc, sizeof(int));
	dmin = (int*) calloc(nc, sizeof(int));

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

	int i, j, k, K, l, m, r;

	nrColors = nc;
	for (K = 1; K <= s; K++)
	{
		k = K % 2;
		memset(c, 0, nc * sizeof(int));

		for (i = 0; i < N; i++)
		{
			for (j = 0; j < N; j++)
			{
				searchedColors = 0;
				for (m = 0; m < nc; m++)
				{
					dmin[m] = INF;
				}

				for (r = 1; r < N; r++)
				{
					if (searchedColors == nrColors)
					{
						break;
					}

					if (j - r >= 0)
					{
						int lim1, lim2;
						lim1 = max(0, i - r);
						lim2 = min(N - 1, i + r);

						for (l = lim1; l <= lim2; l++)
						{
							if (dmin[conf[k][l][j - r]] == INF)
							{
								searchedColors++;
								dmin[conf[k][l][j - r]] = r;
							}
						}
					}

					if (i - r >= 0)
					{
						int lim1, lim2;
						lim1 = max(0, j - r);
						lim2 = min(N - 1, j + r);

						for (l = lim1; l <= lim2; l++)
						{
							if (dmin[conf[k][i - r ][l]] == INF)
							{
								searchedColors++;
								dmin[conf[k][i - r][l]] = r;
							}
						}
					}

					if (j + r < N)
					{
						int lim1, lim2;
						lim1 = max(0, i - r);
						lim2 = min(N - 1, i + r);

						for (l = lim1; l <= lim2; l++)
						{
							if (dmin[conf[k][l][j + r]] == INF)
							{
								searchedColors++;
								dmin[conf[k][l][j + r]] = r;
							}
						}
					}

					if (i + r  < N)
					{
						int lim1, lim2;
						lim1 = max(0, j - r);
						lim2 = min(N - 1, j + r);

						for (l = lim1; l <= lim2; l++)
						{
							if (dmin[conf[k][i + r][l]] == INF)
							{
								searchedColors++;
								dmin[conf[k][i + r][l]] = r;
							}
						}
					}

				}

				conf[(k + 1) % 2][i][j] = max_vector(dmin, nc);
				c[conf[(k + 1) % 2][i][j]]++;

			}
		}

		nrColors = 0;
		for (l = 0; l < nc; l++)
		{
			fprintf(f, "%d ", c[l]);
			if (c[l] != 0)
			{
				nrColors++;
			}
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
