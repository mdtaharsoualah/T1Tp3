#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef minway
#  define minway 1		/* associativité min */
#endif
#ifndef maxway
#  define maxway 4		/* Nombre max de bloc par ensemble*/
#endif
#ifndef minsize
#  define minsize (4*1024)	/* taille min du cache */
#endif
#ifndef maxsize
#  define maxsize (64*1024)	/* taille max */
#endif
#ifndef minline
#  define minline 16		/* taille min des lignes */
#endif
#ifndef maxline
#  define maxline 64		/* taille max des lignes */
#endif
#ifndef mnsets
#  define mnsets (maxsize/minline)/* Nombre max d'ensembles*/
#endif

#ifndef PREFETCH
#define PREFETCH 0
#endif

#ifdef MAIN
// pour éviter d'avoir à gérer un table d'adresse 64 bits...
// l'adresse ci dessous sera en début de section data et servira d'adresse de base pour l'historique des accès
int __cache_data_start__=1;
#else
extern int __cache_data_start__;
#endif

#ifdef MAIN
unsigned long long am=0;	 /* incrémenté à chaque accès mémoire */
long long  LRU[mnsets][maxway];  /* ancienneté des lignes */
unsigned long long ETIQ[mnsets][maxway];
                                /* étiquettes associées aux différentes lignes de cache */

int line=minline,			/* taille d'une ligne de cache */
  size=minsize,				/* taille du cache */
  way=minway,				/* associativité */
  nlines=minsize/minline,               /* nombre de lignes dans le cache */
  nsets=minsize/(minline*minway);	/* nombre d'ensembles */

int l2line, l2size, l2way, l2nlines, l2nsets ;
                               /* les log_2 des précédents */

// pour les statistiques
unsigned int occupation=0;// nombre de lignes de cache contenant une donnée
unsigned long long  dc=0, // nombre total de défauts de cache
  dcob=0,                 // défauts obligatoires
  dcco=0,                 // défauts dus à des conflits
  dcca=0;                 // défauts de capacité

unsigned long long  *hist_line=NULL;     // un tableau indiquant pour chaque tag s'il a déjà été lu par le cache
                                // traité comme un tableau de bits
int hist_line_size=(1<<20);     // la taille en octets du tableau d'historique des accès pour classifier les defauts.
                                // on utilise un bit/adresse de ligne et 1Mo permet donc de gérer une zone data de 8M lignes (>=128Mo)
unsigned long long hline_start; // l'adresse (en lignes) du début de la zone data
#endif

void initcache();
void ac(void* z);
void print_cache_stats(FILE *f, int n); 

// pour balayer toutes les configurations
#define LOOP_CACHE_CONFIG  for (size=minsize;  size <maxsize+1;printf("cache de %d o\n",size),size+=size) \
    for (line=minline;line <maxline+1;line+=line) \
      for (way=minway;way<maxway+1;way+=way)

#ifdef MAIN
int l2(int n) // renvoie le log en base 2 de n
{
  int l;
# if defined(__GNUC__) || defined(__clang__)
  l=__builtin_ctz(n);
# else
  l=0;
  unsigned int nn=n; while(nn&&!(nn&1)){l++;nn>>=1;} // portable mais moins rapide
# endif
  if(n != 1<<l){
      fprintf(stderr,"Le cache *doit* avoir des caractéristiques en puissance de deux\n");
      exit(1);
    }
  return l;
}

void initcache(){

  nsets= size/(way*line);
  assert(nsets <= mnsets);  

  nlines=nsets*way;
  
  /**RAZ des memoires etiquettes et LRU**/
  am=0;
  dc=0;
  for (int j=0; j<nsets;j++)
    for (int k=0;k<way;k++)
      {
	LRU[j][k]=-1; ETIQ[j][k]=0;
      }


  // calcul des log2
  l2line =   l2(line);
  l2size =  l2(size);
  l2way =  l2(way);
  l2nlines =  l2(nlines);
  l2nsets =  l2(nsets);

  // raz statistiques
  dcob=dcco=dcca=occupation=0;
  if(! hist_line)   hist_line = calloc(hist_line_size,1);
  else bzero(hist_line,hist_line_size);
  hline_start=((unsigned long long)&__cache_data_start__)>>l2line;


}

static int iba (int index)
{
  /* renvoie l'indice du bloc le plus ancien*/
  int j,s, i=0;
  s=LRU[index][0];
  for (j=1;j<way;j++)
    if (s>LRU[index][j])
      {	s=LRU[index][j];i=j;}
  return (i);
}

// gère une table indiquant si une ligne déja été dans la cache dans le passé.
// pour de détecter les défauts obligatoires
// renvoie 0 si ligne inconnue, non 0 si ligne deja vue dans le cache
// De plus mémorise que la ligne est vue
// adl est une adresse de ligne
static long long deja_vu(unsigned long long adl)
{
  // on réduit la taille de l'adresse pour permettre l'adressage du tableau.
  // Sur linux, avec ASLR, le segment de donnée est un peu n'importe ou, et il
  // faut donc traiter une adresse sur 64 bits.
  // Pour limiter la taille des tableaux,
  // on passe à une adresse relative par rapport au début du segment de données.
  // Pour éviter d'utiliser des méthodes non portables pour trouver l'emplacement des segments,
  // on crée une donnée bidon en début de zone data pour créer une adresse relative.
  // Pour réduire l'encombrement, l'appartenance est donnée par un bit à un dans un tableau de long long.
  unsigned long long rel=adl-hline_start; // adresse de la ligne par rapport debut de zone data
  const long long msk = 0x3fll;       // pour garder les 6 bits de poids faible
  unsigned long long val= 1ll << (rel & msk) ;  // emplacement du bit codant l'état de la ligne
  unsigned long long relmsb=rel>>6;             // les n-6 bits de poids fort
  unsigned long long ret=hist_line[relmsb]&val;  // la valeur courante
  hist_line[relmsb] |= val;                      // on positionne le bit correspondant à la ligne
  return (ret != 0) ;
}

static int cache( unsigned long long adresse,int prefetch)
{	//acces cache, gestion des etiquettes et anciennete,
        //renvoie 1 si defaut de cache
  unsigned int index,set,j;
  unsigned long long adl, tag;

  adl=adresse>>l2line;
  tag = adl>>l2nsets;
  index=adl&(nsets-1);


  // on cherche si le tag est présent.
  // Si oui succès, et on met à jour la date d'accès pour le remplacement LRU
  // en cas de prefetch, LRU=0
    for (set=0;set<way;set++)
      if (ETIQ[index][set]==tag)
        {
          if(!prefetch)
            LRU[index][set]=am;
          return (0);
        }

  
  // un défaut de cache à traiter
  j= iba(index);         // on cherche la ligne la plus ancienne pour la remplacer
  ETIQ[index][j]=tag;    // on écrit le tag
  if (LRU[index][j]==-1) // une ligne jamais utilisée
    occupation++;
  LRU[index][j]=am;

  // de quel type de défaut s'agit-il ?
  // un défaut obligatoire si la ligne n'a pas été écrite dans la cache dans la passé
  if(!prefetch) {
    if(! deja_vu(adl)){
      dcob++;
    } else {
      // les autres défauts : conflit si cache non plein, capacité sinon
      int non_plein = (occupation < nlines);
      dcco += non_plein;
      dcca += !non_plein;
    }
  }
  return (1);
}


void print_cache_stats(FILE *f, int n){
  float aam=am;
  fprintf(f,"%d\t%d\t%d\t%d\t%f\t%f\t%f\t%f\n",size,way, line, n,dc/aam,dcob/aam,dcco/aam,dcca/aam);
}


//void ac(unsigned long long ad)
void ac(void * ad)
{	/* acces cache pour l'adresse ad*/
  int d;
  am++;
  d=cache((unsigned long long)ad,0);
  dc+=d;
#if PREFETCH
  cache((unsigned long long)ad+line,1);
#endif
}

#endif
