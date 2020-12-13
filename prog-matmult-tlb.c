#include <stdio.h>
#include "tlb.h"

#ifndef N
#  define N 100	/* Taille  des matrices et vecteurs*/
#endif


double x[N][N], y[N][N], z[N][N], s=0;

FILE *results;



int main()
{
  int i,j,k;
  results = fopen( "res-matmult-tlb", "w+" );

  /** Initialisation des matrices**/

  fprintf(results, "Multiplication de matrices (mode TLB)   (N=%d)\n \n",N);
  fprintf(results, "&x[0][0]=%x \t &y[0][0] =%x &z[0][0]=%x \n",&x[0][0], &y[0][0], &z[0][0]);

  fprintf(results, "ENTREES\tWAY\tN\tTaux échec\tEchec/N*N*N \n");

  for (i=0;i<N;i++)
    {for (j=0;j<N;j++)
	{
	  x[i][j]=i+j;
	  y[i][j]=i-j;
	}
    }

  
  
   LOOP_TLB_CONFIG
	{
	  /* D'abord IJK */
	  fprintf(results,"IJK\n");
	  inittlb();
	  for (i=0;i<N;i++)
	    {
	      for (j=0;j<N;j++)
		{
		  s=0;
		  for (k=0;k<N;k++)
		    {
		
		      ac((unsigned long) &x[i][k]);ac((unsigned long) &y[k][j]);
		      s = s + x[i][k]*y[k][j];
		    }
		  ac((unsigned long) &z[i][j]);
		  z[i][j]=s;
		
		}
	    }
		

	  fprintf(results,"%d\t%d\t%d\t%f\t%f\n",tsize,tway, N,(float)dtlb/(float)am, (float)dtlb/(N*N*N));
	
	  /* Puis IKJ */
	  fprintf(results,"IKJ\n");

   	  inittlb();
	  
	  for (i=0;i<N;i++)
	    {
	      for (k=0;k<N;k++)
		{  ac((unsigned long) &x[i][k]);
		  s= x[i][k];
		  for (j=0;j<N;j++)
		    {
		      ac((unsigned long) &z[i][j]);ac((unsigned long) &y[k][j]);
		      z[i][j]+=s*y[k][j];
		      ac((unsigned long) &z[i][j]);	
		    }
		}
	    }
		

	  fprintf(results,"%d\t%d\t%d\t%f\t%f\n",tsize,tway, N,(float)dtlb/(float)am, (float)dtlb/(N*N*N));

	}
}
