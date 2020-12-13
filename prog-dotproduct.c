#define MAIN
#include <stdio.h>
#include "cache.h"

#ifndef N
#  define N 1000	/* Taille  des matrices et vecteurs*/
#endif

FILE *results;

double x[N], y[N], s=0;


int main()
{
  int i,j,k;
  results = fopen( "res-dotproduct", "w+" );

  /** Initialisation des vecteurs**/

  for (i=0;i<N;i++)
    {
      for (j=0;j<N;j++)
	{
	  x[i]=i+j;
	  y[i]=i-j;
	}
    }	
  

  fprintf(results, "PRODUIT SCALAIRE (N=%d)\n \n", N);

  fprintf(results, "&x[0]=%llx \t &y[0] =%llx \n",(unsigned long long)&x[0],(unsigned long long) &y[0]);

  fprintf(results, "SIZE\tWAY\tLINE\tN\tTaux echec\tOblig.\tConfl.\tCapac.\n");

   LOOP_CACHE_CONFIG
	{
	  initcache();

	  s=0;
	  for (i=0;i<N;i++)
	    
	    {
	      ac(&x[i]);ac(&y[i]);
	      s = s + x[i]*y[i];
	    }
	  
	  //fprintf(results,"%d\t%d\t%d\t%d\t%f\n",size,way, line, N,(float)dc/(float)am);
          print_cache_stats(results, N);
	}
  
}
