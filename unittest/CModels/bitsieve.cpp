/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include <stdio.h>
#include <stdlib.h>

void primes(int *isPrime, int n) {
  int i, count = 0, m = 10000<<n, size = m+31>>5;

  for (i=0; i<size; i++) isPrime[i] = 0xffffffff;

  for (i=2; i<m; i++)
    if (isPrime[i>>5] & (1<<(i&31))) {
      for (int j=i+i; j<m; j+=i)
        isPrime[j>>5] &= ~(1<<(j&31));
      count++;
    }
   printf("%d %d\n",m,count);
}

void  sieve() {
    for (int i = 4; i <= 4; i++) {
      int *isPrime = (int*)malloc(sizeof(int)*((10000<<i)+31>>5));
      primes(isPrime, i);
    }
}

void main() {
    sieve();
}
