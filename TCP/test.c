#include <stdio.h>
#include <time.h>

int main (void)
{
   /* lire l'heure courante */
   time_t now = time (NULL);

   /* la convertir en heure locale */
   struct tm tm_now = *localtime (&now);

   /* Creer une chaine JJ/MM/AAAA HH:MM:SS */
   char s_now[sizeof "JJ/MM/AAAA HH:MM:SS"];

   strftime (s_now, sizeof s_now, "%d/%m/%Y %H:%M:%S", &tm_now);

   /* afficher le resultat : */
   printf ("'%s'\n", s_now);

   return 0;
}