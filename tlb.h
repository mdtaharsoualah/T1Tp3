#define mway 4	/* Nombre max de bloc par ensemble*/
#define mnsets 1024	/* Nombre max d'ensembles*/

#define mtway 4		/* Associativit� max du TLB*/
#define mtnsets 32	/* nombre max d'ensembles du TLB*/
#define pagesize 1024	/* Taille d'une page */

int am=0,			/* incr�ment� � chaque acc�s m�moire */
  dc=0,				/* nombre de d�fauts de cache */
  dtlb=0,				/* nombre de d�fauts de cache */
  ETIQ[mnsets][mway],		/* �tiquette associ�es aux diff�rentes
				   lignes de cache */
  LRU[mnsets][mway],		/* anciennet� des lignes */
  tETIQ[mtnsets][mtway],		/* �tiquette associ�es aux diff�rentes
				   entr�es du TLB */
  tLRU[mtnsets][mtway];		/* anciennet� des entr�es du TLB */


int line,			/* taille d'une ligne de cache */
  size,				/* taille du cache */
  way,				/* associativit� */
  nsets;			/* nombre de blocs */

int tsize,			/* nombre d'entr�es du TLB */
  tway,				/* associativit� du TLB */
  tsets;			/* nombre d'ensemble du TLB */

// pour balayer toutes les configurations
#define LOOP_CACHE_CONFIG  for (size=4*1024; size <20000;size+=size) \
    for (line=16;line <100;line+=line) \
      for (way=1;way<8;way+=way)

#define LOOP_TLB_CONFIG  for (tsize=8; tsize <20;tsize+=tsize) \
      for (tway=1;tway<4;tway+=tway)


void initcache(){
  nsets= size /(way*line);

  /**RAZ des memoires etiquettes et LRU**/
  am =0;
  dc=0;
  for (int j=0; j<nsets;j++)
    for (int k=0;k<way;k++)
      {
	LRU[j][k]=0; ETIQ[j][k]=0;
      }
}

int iba (int index)
{
  /* renvoie l'indice du bloc le plus ancien*/
  int j,s, i=0;
  s=LRU[index][0];
  for (j=1;j<way;j++)
    if (s>LRU[index][j])
      {	s=LRU[index][j];i=j;}
  return (i);
}
	
int cache( unsigned long ad)
{	/* acces cache, gestion des etiquettes et anciennete, renvoie 1 si
	   defaut de cache*/
  int index,tag,set,j;
  ad=ad/line;
  tag = ad/nsets;
  index=ad%nsets;
  for (set=0;set<way;set++)
    if (ETIQ[index][set]==tag)
      {
	LRU[index][set]=am;
	return (0);
      }
  
  j= iba(index);
  ETIQ[index][j]=tag;
  LRU[index][j]=am;
  return (1);
	
}


void inittlb(){
  /* A FAIRE : mettre � 0 les m�moires tETIQ et tLRU */
  am=0;
  dtlb=0;
}

int tiba (int index)
{
  /* A FAIRE : renvoie l'indice de l'entr�e de TLB la plus ancienne dans le bloc de TLB 'index' */
  return 0;
}
	
int tlb( unsigned long ad)
{	/* acces TLB , gestion des etiquettes et anciennet�, renvoie 1 si
	   defaut de TLB*/
  /* AFAIRE : d�terminer l'adresse de la page dans ad et g�rer l'entr�e de TLB � cette adresse */

  return (1);
	
}


void ac(unsigned long z)
{	/* acces cache pour l'adresse z*/
	am++;
//	dc+=cache(z);
	dtlb+=tlb(z);
}

int min (int a, int b)
{
  if (a<b) return a;
  else return b;
}

