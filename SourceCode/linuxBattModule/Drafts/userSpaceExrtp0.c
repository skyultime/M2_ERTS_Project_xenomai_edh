#include <stdio.h>
#include <stdlib.h>

int main()
{
   int num;
   FILE *fptr;

   // use appropriate location
   fptr = fopen("/dev/rtp0","w");

   if(fptr == NULL)
   {
      printf("Error!\n");   
      exit(1);             
   }

   fclose(fptr);

   return 0;
}
