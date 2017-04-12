#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "blowfish.h"

void initialization(WORD pArray[PLENGTH], WORD sBox[NSBOX][SLENGTH]);
WORD F(WORD in, WORD sBox[NSBOX][SLENGTH]);

/* initialize pArray and sBoxes with PI hexadecimal values */
void initialization(WORD pArray[PLENGTH], WORD sBox[NSBOX][SLENGTH])
{
	memcpy(pArray, pArrayStatic, sizeof(WORD) * PLENGTH);
	memcpy(sBox, sBoxStatic, sizeof(WORD) * SLENGTH * NSBOX);
}

void initializeBlowfish(BYTE key[KLENGTH], WORD pArray[PLENGTH], 
						WORD sBox[NSBOX][SLENGTH], int klen)
{
	int i;
	int ncopy, rcopy;
	BYTE key2[PLENGTH*4];
	WORD emptyBlock[2], inBlock[2], outBlock[2];
	
	emptyBlock[0] = 0;
	emptyBlock[1] = 0;
	
	initialization(pArray, sBox);
	
	/* extended copy of the key */
	ncopy = PLENGTH*4/klen;
	for(i=0; i<ncopy; i++)
		memcpy(key2+i*klen, key, sizeof(BYTE) * klen);
	
	/* residual length */
	rcopy = PLENGTH*4 - ncopy*klen;
	memcpy(key2+ncopy*klen, key, sizeof(BYTE) * rcopy);
	
	/* XOR between the extended key and pArray */	
	for(i=0; i<PLENGTH; i++)
		pArray[i] ^= ((WORD *) key2)[i];
	
	/* cipher of pArray */	
	memcpy(inBlock, emptyBlock, sizeof(WORD) * 2);
	for(i=0; i<PLENGTH; i+=2)
	{
		blowfishCipher(inBlock, outBlock, pArray, sBox);
		
		pArray[i] = outBlock[0];
		pArray[i+1] = outBlock[1];
		
		inBlock[0] = pArray[i];
		inBlock[1] = pArray[i+1];
	}

	/* cipher sBox */
	for(i = 0; i < NSBOX * SLENGTH; i+=2)
	{
		blowfishCipher(inBlock, outBlock, pArray, sBox);
		
		sBox[i/SLENGTH][i%SLENGTH] = outBlock[0];
		sBox[(i/SLENGTH)][(i%SLENGTH)+1] = outBlock[1];
		
		inBlock[0] = sBox[i/SLENGTH][i%SLENGTH];
		inBlock[1] = sBox[(i/SLENGTH)][(i%SLENGTH)+1];
	}	
	
	return;
}

void blowfishCipher(WORD inBlock[2], WORD outBlock[2], 
					WORD pArray[PLENGTH], WORD sBox[NSBOX][SLENGTH])
{
	
	int i;
	WORD tmp;
	
	for(i=0; i<15; i++)
	{
		inBlock[0] ^= pArray[i];
		inBlock[1] ^= F(inBlock[0], sBox);
		SWAP(inBlock[0], inBlock[1], tmp);
	}
	
	inBlock[0] ^= pArray[15];
	inBlock[1] ^= F(inBlock[0], sBox);
	inBlock[1] ^= pArray[16];
	inBlock[0] ^= pArray[17];
	
	memcpy(outBlock, inBlock, sizeof(WORD) * 2);
}

void blowfishDecipher(WORD inBlock[2], WORD outBlock[2], 
					WORD pArray[PLENGTH], WORD sBox[NSBOX][SLENGTH])
{
	int i;
	WORD tmp;
	
	for(i=0; i<15; i++)
	{
		inBlock[0] ^= pArray[PLENGTH-1-i];
		inBlock[1] ^= F(inBlock[0], sBox);
		SWAP(inBlock[0], inBlock[1], tmp);
	}
	
	inBlock[0] ^= pArray[2];
	inBlock[1] ^= F(inBlock[0], sBox);
	inBlock[1] ^= pArray[1];
	inBlock[0] ^= pArray[0];
	
	memcpy(outBlock, inBlock, sizeof(WORD) * 2);
}

WORD F(WORD in, WORD sBox[NSBOX][SLENGTH])
{
	
	BYTE a, b, c, d, x;
	
	a = *((BYTE *)&in);
	b = *(((BYTE *)&in)+1);
	c = *(((BYTE *)&in)+2);
	d = *(((BYTE *)&in)+3);
	
	x = sBox[0][a] + sBox[1][b];
	x ^= sBox[2][c];
	x += sBox[3][d];
	
	return x;
}



