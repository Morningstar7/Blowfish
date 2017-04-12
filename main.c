/* Use syntax: 
 * 
 * 		blowfish <key> <input> <cipher/decipher> <output>
 * 
 * 		- <key>: the key used for encryption or decryption
 * 		- <input>: name of the input file
 * 		- <cipher/decipher>: -c for cipher / -d for decipher
 * 		- <output>: name of the output file
 * 
 * */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

#include "blowfish.h"

#define NTHREADS 4

void *threadRead(void *args);
void *threadWrite(void *args);
void *threadCipher(void *args);
void *threadDecipher(void *args);

/* global variables */

FILE *fIn, *fOut;

WORD pArray[PLENGTH];
WORD sBox[NSBOX][SLENGTH];
WORD inBlock[NTHREADS][2], outBlock[NTHREADS][2];

int stopCipher, stopWrite;

sem_t semIn[NTHREADS], semOut[NTHREADS];
sem_t semCiphIn[NTHREADS], semCiphOut[NTHREADS];

int main(int argc, char *argv[])
{
	int klen, i;
	int threadArray[NTHREADS];
	
	BYTE key[KLENGTH];
	
	pthread_t tId[NTHREADS];
	pthread_t tR, tW;
	
	struct timeval startT, endT;
	
	/* check of syntax use */
	if(argc != 5)
	{
		fprintf(stderr, "Syntax error: ./blowfish <key> <input> "
						"<cipher/decipher> <output>\n");
		exit(EXIT_FAILURE);
	}
	
	/* check key length */
	klen = strlen(argv[1]);	/* size in BYTE */
	
	/* checking BYTES values */
	if((klen) < 4 || (klen) > KLENGTH)
	{
		fprintf(stderr, "Error: key length not adequate. " 
						"From 4 to 56 characters.\n");
		exit(EXIT_FAILURE);
	}
	
	/* key copying */
	memcpy(key, argv[1], sizeof(BYTE) * klen);
	
	/* opening input file */
	fIn = fopen(argv[2], "r");
	
	if(fIn == NULL)
	{
		fprintf(stderr, "Error: input file not found.\n");
		exit(EXIT_FAILURE);
	}
	
	/* check -c and -d parameters */
	if(strcmp(argv[3], "-c") && strcmp(argv[3], "-d"))
	{
		fprintf(stderr, "Error: not valid parameters [-c] or [-d].\n");
		exit(EXIT_FAILURE);
	}
	
	/* opening output file */
	fOut = fopen(argv[4], "w");
	
	if(fOut == NULL)
	{
		fprintf(stderr, "Error: output file not created.\n");
		exit(EXIT_FAILURE);
	}
	
	/* array containing the identification numbers of the threads for
	 * memory access */
	 
	for(i=0; i<NTHREADS; i++)
	{
		threadArray[i] = i;
		
		/* semaphore initialization */
		sem_init(&semIn[i], 0, 0);
		sem_init(&semOut[i], 0, 0);
		sem_init(&semCiphIn[i], 0, 1);
		sem_init(&semCiphOut[i], 0, 1);
	}
		
	
	initializeBlowfish(key, pArray, sBox, klen);
	
	stopCipher = 0;
	stopWrite = 0;
		
	gettimeofday(&startT, NULL);
	
	pthread_create(&tR, 0, threadRead, NULL);

	pthread_create(&tW, 0, threadWrite, NULL);

	switch(argv[3][1])
	{
		case 'c':	
									
		/* threads creation */
		for(i=0; i<NTHREADS; i++)
		{
			pthread_create(&tId[i], 0, &threadCipher, 
						  (void *) &threadArray[i]);
		}					  

		break;
					
		case 'd':
					
		/* threads creation */
		for(i=0; i<NTHREADS; i++)
			pthread_create(&tId[i], 0, &threadDecipher, 
						  (void *) &threadArray[i]);
		break;
	}

	/* waiting threads */
	pthread_join(tR, NULL);

	for(i=0; i<NTHREADS; i++)
		pthread_join(tId[i], NULL);
		
	pthread_join(tW, NULL);
	
	gettimeofday(&endT, NULL);
	
	printf("Time ms: %.3f\n", 
						(double)(endT.tv_usec - startT.tv_usec) / 1000);
	
	fclose(fIn);
	fclose(fOut);
	
	return 0;
}

void *threadRead(void *args)
{
	int i, y;
	
	while(1)
	{		
		/* read data from input file */
		for(i=0; i<NTHREADS; i++)
		{			
			sem_wait(&semCiphIn[i]);
			
			inBlock[i][0] = 0;
			inBlock[i][1] = 0;
			
			y = fscanf(fIn,  "%c%c%c%c%c%c%c%c", &(((BYTE *)(inBlock[i]))[0]),
												 &(((BYTE *)(inBlock[i]))[1]),
												 &(((BYTE *)(inBlock[i]))[2]),
												 &(((BYTE *)(inBlock[i]))[3]),
												 &(((BYTE *)(inBlock[i]))[4]),
												 &(((BYTE *)(inBlock[i]))[5]),
												 &(((BYTE *)(inBlock[i]))[6]),
												 &(((BYTE *)(inBlock[i]))[7]));
												 
			
			if(y == EOF)
			{
				if(stopCipher == 0)
					stopCipher = 1;
				
				/* unlock waiting threads */
				for(i=0; i<NTHREADS; i++)
					sem_post(&semIn[i]);
				
				
				return NULL;
			}
				
			/* signal to corresponding semaphore */	
			sem_post(&semIn[i]);
		}
	}
	
	return NULL;
}

void *threadWrite(void *args)
{
	int i;
	
	while(1)
	{			
		for(i=0; i<NTHREADS; i++)
		{
			
			sem_wait(&semOut[i]);

			if(stopWrite == 0)
			{
				fprintf(fOut, "%c%c%c%c%c%c%c%c",((BYTE *)outBlock[i])[0],
										 		 ((BYTE *)outBlock[i])[1],
									 			 ((BYTE *)outBlock[i])[2],
												 ((BYTE *)outBlock[i])[3],
												 ((BYTE *)outBlock[i])[4],
												 ((BYTE *)outBlock[i])[5],
												 ((BYTE *)outBlock[i])[6],
												 ((BYTE *)outBlock[i])[7]);

				sem_post(&semCiphOut[i]);
			}

			if(stopWrite > 0)
			{
				if(stopWrite == NTHREADS)
				{	
					return NULL;
				}
				else 
					stopWrite++;
			}
		}
	}
	
	return NULL;
}

/* args contain the number of the thread (int) */
void *threadCipher(void *args)
{
	int i;
	int n = *((int*)args);
	
	while(1)
	{
		sem_wait(&semIn[n]);
		if(stopCipher == 0)
		{
			sem_wait(&semCiphOut[n]);	
			
			blowfishCipher(inBlock[n], outBlock[n], pArray, sBox);
		}

		if(stopCipher == 0)
		{
			sem_post(&semOut[n]);
			sem_post(&semCiphIn[n]);
		}
		else
		{
			if(stopCipher == NTHREADS)
			{
				stopWrite = 1;
				for(i=0; i<NTHREADS; i++)
					sem_post(&semOut[i]);
			}
			else 
				stopCipher++;

			return NULL;
		}
	}
	
	return NULL;
}

/* args contain the number of the thread (int) */
void *threadDecipher(void *args)
{
	int i;
	int n = *((int*)args);
	
	while(1)
	{
		sem_wait(&semIn[n]);
		if(stopCipher == 0)
		{
			sem_wait(&semCiphOut[n]);
			
			blowfishDecipher(inBlock[n], outBlock[n], pArray, sBox);
		}

		if(stopCipher == 0)
		{
			sem_post(&semOut[n]);
			sem_post(&semCiphIn[n]);
		}
		else
		{
			if(stopCipher == NTHREADS)
			{
				stopWrite = 1;
				for(i=0; i<NTHREADS; i++)
					sem_post(&semOut[i]);
			}
			else 
				stopCipher++;

			return NULL;
		}
	}
	
	return NULL;	
}
