/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include <stdio.h>
#include <stdlib.h>

int nsieve(int m, int *isPrime){
   int i, k, count;

   for (i=2; i<=m; i++) { isPrime[i] = 1; }
   count = 0;

   for (i=2; i<=m; i++){
      if (isPrime[i]) {
         for (k=i+i; k<=m; k+=i) isPrime[k] = 0;
         count++;
      }
   }
   printf("%d %d\n",m,count);
   return count;
}

void sieve() {
    for (int i = 1; i <= 4; i++ ) {
        int m = (1<<i)*10000;
        int* flags =(int*)malloc(sizeof(int)*(m+1));
        nsieve(m, flags);
    }
}

void main() {
    sieve();
}
