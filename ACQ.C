#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <math.h>
#include <malloc.h>

/* True and false values */

#define TRUE	1
#define FALSE	0

#define LO(x) 	x & 0xFF
#define HI(x) 	x & 0xFF
#define ErrorSet	(((adcsr = inpw(ADCSR)) & BIT15) == BIT15)
#define DmaDone		(((supcsr = inpw(supcsr)) & BIT15) == BIT15)

/* Register BIT definitions */
#define BIT0 	0x0001
#define BIT1 	0x0002
#define BIT2 	0x0004
#define BIT3 	0x0008
#define BIT4 	0x0010
#define BIT5 	0x0020
#define BIT6 	0x0040
#define BIT7 	0x0080
#define BIT8 	0x0100
#define BIT9 	0x0200
#define BIT10 	0x0400
#define BIT11 	0x0800
#define BIT12 	0x1000
#define BIT13 	0x2000
#define BIT14 	0x4000
#define BIT15 	0x8000


/* Segment size and page sizes */
#define SEGSIZE		65536L
#define DMAPAGESIZE	131072L

/* DT2821 Registers */
#define BASE	0x240
#define ADCSR	BASE + 0x0
#define CHANCSR	BASE + 0x2
#define ADDAT	BASE + 0x4
#define DACSR	BASE + 0x6
#define DADAT	BASE + 0x8
#define DIODAT	BASE + 0xA
#define SUPCSR	BASE + 0xC
#define TMRCTR	BASE + 0xE

/* 8237-A DMA controller registers */
#define PAGE-REG5
#define OFFS-REG5
#define COUNT-REG5
#define MASK-REG5
#define MODE-REG5
#define FF-REG5
#define PAGE-REG6
#define OFFS-REG6
#define COUNT-REG6

/* Values used to convert A/D output to floating point */
#define	SCALE	0.004882812
#define OFFSET	10.0

/* Data type declarations */
typedef unsigned char BYTE;
typedef unsigned int WORD;

/* Function prototypes */
void GetDMAInfo(Word bufsize, int far *buffer, WORD *DMAPage, WORD *DMAbase);
void InitDMA5(WORD page, WORD offset, WORD count);
void InitDMA6(WORD page, WORD offset, WORD count);
void allocbuf(int far **buffer, WORD bufsize);
void stopall(void);
int InitBoard(void);
int GetData(void);
int set_clock(float rate, float *rate_set);
WORD getDMAlength(int far *buffer);

/* Global variables */
WORD 	supcsr;
WORD 	adcsr;
int 	numpts;
File 	*outfile;
int far *dataprt_a, *dataptr_b;

void main()
{
   int 		loops, n_loops;
   char		coding;
   WORD		DMAabase, DMAbbase;
   WORD		DMAaPage, DMAbPage;
   WORD		temp;
   float	req_rate, rate_set;

   do {
      printf("How many points in each buffer? (1-32756) ");
      scanf("%d", &numpts);
   } while ((numpts < 1) || (numpts > 32767));
   do {
      printf("How many buffers do you wish to acquire? (1-32756) ");
      scanf("%d", &n_loops);
   } while ((n_loops < 1) || (n_loops > 32767));

   printf("At what clock rate? (in Hz) ");
   scanf("%f", &req_rate);

   if (setclock(req_rate, &rate_set))
      exit(2);

   printf("Clock set for %f Hz\n", rate_set);

   /* Allocate the first DMA buffer */
   allocbuf(&dataptr_a, numpts);

   /* Get the DMA page and offset value for the first buffer. */
   GetDMAInfo(numpts, dataptr_a, &DMAaPage, &DMAabase);
   printf("buffer A info: maxsize %u, page %u, offset %u\n",
          getDMAlength(dataptr_a), DMAaPage, DMAabase);

   /* Initialize the 8237-A DMA controller channel 5 */
page 4-57
