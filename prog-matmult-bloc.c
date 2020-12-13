#define MAIN
#include <stdio.h>
#include "cache.h"
#ifndef N
#  define N 150	/* Taille  des matrices et vecteurs*/
#endif


double x[N][N], y[N][N], z[N][N], s=0;

FILE *results;

static int min(int a, int b){
  return a<b ? a : b ;
}

int main()
{
  int i,j,k,jj,kk,B;
  results = fopen( "res-matmult-bloc", "w+" );

  /** Initialisation des matrices**/
  fprintf(results, "Multiplication de matrices par blocs\n");
  fprintf(results, "&x[0][0]=%llx \t &y[0][0] =%llx &z[0][0]=%llx \n",(unsigned long long)&x[0][0],(unsigned long long) &y[0][0],(unsigned long long) &z[0][0]);
  fprintf(results, "SIZE\tWAY\tLINE\tN\tB\tTaux echec\tOblig.\tConfl.\tCapac.\n");
  for (i=0;i<N;i++)
    {for (j=0;j<N;j++)
	{
	  x[i][j]=i+j;
	  y[i][j]=i-j;
	}
    }
  size = 16*1024;

  for (line=16;line <100;line+=line)
    for (way=1;way<8;way+=way)
      for (B=4;B<130;B+=B)
	{
	  initcache();
	  for (jj=0;jj<N;jj+=B)
	    for (kk=0;kk<N;kk+=B)
	      for (i=0;i<N;i++)
		{
		  for (j=jj;j<min(jj+B-1,N);j++)
		    {
		      s=0;
		      for (k=kk;k<min(kk+B-1,N);k++)
			{
		
			  ac((unsigned long) &x[i][k]);ac((unsigned long) &y[k][j]);
			  s = s + x[i][k]*y[k][j];
			}
		      ac((unsigned long) &z[i][j]);ac((unsigned long) &z[i][j]);
		      z[i][j]=s+z[i][j];
		
		    }
		}

		
          fprintf(results,"%d\t%d\t%d\t%d\t%d\t%f\t%f %f %f\n",size,way, line, N,B,dc/(float)am,dcob/(float)am,dcco/(float)am,dcca/(float)am);

	}

}
