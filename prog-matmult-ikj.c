#define MAIN
#include <stdio.h>
#include "cache.h"

#ifndef N
#  define N 64	/* Taille  des matrices et vecteurs*/
#endif
    
double x[N][N], y[N][N], z[N][N], s=0;

FILE *results;




int main()
{
  int i,j,k;
  results = fopen( "res-matmult-ikj", "w+" );

  /** Initialisation des matrices**/

  fprintf(results, "Multiplication de matrices IKJ  (%d) \n \n",N);
  fprintf(results, "&x[0][0]=%llx \t &y[0][0] =%llx &z[0][0]=%llx \n",(unsigned long long)&x[0][0],(unsigned long long) &y[0][0],(unsigned long long) &z[0][0]);
  fprintf(results, "SIZE\tWAY\tLINE\tN\tTaux echec\tOblig.\tConfl.\tCapac.\n");

  for (i=0;i<N;i++)
    {for (j=0;j<N;j++)
	{
	  x[i][j]=i+j;
	  y[i][j]=i-j;
	}
    }

   LOOP_CACHE_CONFIG
	{
	  initcache();
	  
	  for (i=0;i<N;i++)
	    {
	      for (k=0;k<N;k++)
		{  ac(&x[i][k]);
		  s= x[i][k];
		  for (j=0;j<N;j++)
		    {
		      ac(&z[i][j]);ac(&y[k][j]);
		      z[i][j]+=s*y[k][j];
		      ac(&z[i][j]);	
		    }
		}
	    }
		

          print_cache_stats(results,N);

	}

}
