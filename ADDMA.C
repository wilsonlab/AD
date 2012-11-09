#include "adext.h"
/*
************************************************************************
**
**   DMA Routines
**
************************************************************************
*/
static dmachannel = 0;

void ProgramDMAController(void)
{
#ifdef DT2821
	DmaStatus;
	if((supcsr & BIT9) == BIT9){
#endif
#ifdef DAS-1800
	dmachannel = (dmachannel+1)%2;
	//fprintf(stderr,"dma=%d\n",dmachannel);
	if(dmachannel==1){
#endif	
#ifndef ISR
		adinfo.filled_buffer = 0;
#endif
		/* initialize the first DMA channel */
		switch(adinfo.dma_ch1){
		case 5:
//      sprintf(tmpstring,"dma5=%d %d:%d %d",dmachannel,
//	  adinfo.dmapage[adinfo.next_buf],
//	  adinfo.dmabase[adinfo.next_buf],
//	  adinfo.dma_bufsize);
//     ErrorMessage(tmpstring);
			/* initialize the 8237-A DMa controller channel 5 */
			InitDMA5(adinfo.dmapage[adinfo.next_buf], 
   	        	         adinfo.dmabase[adinfo.next_buf], 
				 adinfo.dma_bufsize);
			break;
		case 6:
			InitDMA6(adinfo.dmapage[adinfo.next_buf], 
				 adinfo.dmabase[adinfo.next_buf],
				 adinfo.dma_bufsize);
			break;
		}
	} else {
#ifndef ISR
		adinfo.filled_buffer = 1;
#endif
		switch(adinfo.dma_ch2){
		case 6:
//      sprintf(tmpstring,"dma6=%d %d:%d %d",dmachannel,
//	  adinfo.dmapage[adinfo.next_buf],
//	  adinfo.dmabase[adinfo.next_buf],
//	  adinfo.dma_bufsize);
//     ErrorMessage(tmpstring);
			/* initialize the 8237-A DMa controller channel 6 */
			InitDMA6(adinfo.dmapage[adinfo.next_buf], 
				 adinfo.dmabase[adinfo.next_buf],
      				 adinfo.dma_bufsize);
			break;
		case 7:
			InitDMA7(adinfo.dmapage[adinfo.next_buf], 
				 adinfo.dmabase[adinfo.next_buf],
                                 adinfo.dma_bufsize);
			break;
		}
	}
	adinfo.dmabuf_counter = (adinfo.dmabuf_counter + 1)%4;
}

void allocbuff(int far **buffer, WORD buffsize)
/* allocate a data buffer of buffsize and check its DMA length */
{
WORD	oldsize;
int	*dummyptr;

	//fprintf(stderr,"requested allocation of %d WORDS\n",buffsize);
	if((*buffer = (int far *)calloc(buffsize, sizeof(WORD))) == NULL){
		gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"ERROR allocating memory\n");
		SystemExit(3,"DMA buffer alloc error");
	}
	if((oldsize=getDMAlength(*buffer)) < buffsize){
	//	gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"DMA boundary crossed - attempting reallocation\n");
		free(*buffer);
		/* allocate dummy buffer to align the net block with a dma page.
		** note that the pointer to this buffer is not maintained
		** by this program thus any memory allocated is lost to the program
		*/
		dummyptr = malloc(oldsize*sizeof(WORD));
		if((*buffer = (int far *)malloc((buffsize)*sizeof(WORD))) == NULL){
			gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"ERROR allocating memory\n");
			SystemExit(3,"DMA buffer realloc error");
		}
                free(dummyptr);
	}
}

WORD getDMAlength(int far *buffer)
/* calculate the usable length of the buffer */
{
long	absadr;
WORD	dmastart;

	/* calculate absolute address of buffer */
	absadr = ((long)FP_SEG(buffer)<<4) + FP_OFF(buffer);

	/* calculate the dma offset address of buffer */
	dmastart = (WORD)(absadr>>1) & 0xFFFF;

	/* return the number of samples before the end of the buffer */
	return((WORD)(0x10000L-dmastart));
}

void GetDMAInfo(WORD bufsize, int far *buffer, WORD *DMAPage, WORD *DMAbase)
{
unsigned long 	AbsAddr, maxsize;
unsigned int	OffsAddr, SegAddr;

	/* calculate the segment and offset address of buffer  */
	SegAddr = FP_SEG(buffer);
	OffsAddr = FP_OFF(buffer);

	/* calculate the absolute address fo the buffer. First shift the segment left
	** four and add it to the offset. This is the real absolute address.
	** Now take the result and shift it right 1. This is done to simplify the
	** address calculations for the DMA controller
	*/
	AbsAddr = (((unsigned long) SegAddr << 4) + OffsAddr) >> 1;

	/* calculate the DMA page. The page is the upper WORD of AbsAddr
	** Now shift the page left one. Only even numbered dma pages are
	** usable
	*/

	*DMAPage = ((WORD)((AbsAddr & 0xFFFF0000) >> 16) << 1);

	/* the 8237A DMA controller needs a base value. the lower WORD of the
	** AbsAddr is the base
	*/
	*DMAbase = (WORD)(AbsAddr & 0xFFFF);

	/* Make sure that the buffer does not cross a 64K WORD DMA page */
	maxsize = 0x10000L - *DMAbase;
	if(bufsize > maxsize){
		//gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"ERROR: buffer crosses DMA page boundary\n");
		SystemExit(20,"buffer crosses DMA page boundary.");
	}
}

void InitDMA5(WORD page, WORD offset, WORD count)
/* initialize the DMA controller */
{

	/* set the 8237 mode register for demand transfer on DMA channel 5 */
	outp(MODE_REG,0x45);

	disable();
	/* reset DMA channel 5 bytepointer flipflop before 
	** writing new address/WORDcount info 
	*/
	outp(FF_REG,0);

	/* load the page offset value. note that the 8237A is an 8-bit device
	** therefore we have to write the offset and count values on BYTE at
	** a time
	*/
	outp(OFFS_REG5,LO(offset));
	outp(OFFS_REG5,HI(offset));

	/* reset DMA channel 5 bytepointer flipflop before 
	** writing new address/WORDcount info 
	*/
	outp(FF_REG,0);

	/* load the WORDcount information */
	outp(COUNT_REG5,LO(count-1));
	outp(COUNT_REG5,HI(count-1));
	enable();
	
	/* load the DMA page value */
	outp(PAGE_REG5,LO(page));

	/* enable DMA channel 5 */
	outp(MASK_REG,1);
}

void InitDMA6(WORD page, WORD offset, WORD count)
/* initialize the DMA controller */
{

	/* set the 8237 mode register for demand transfer on DMA channel 6 */
	outp(MODE_REG,0x46);

	disable();
	
	/* reset DMA channel 6 flipflop before writing new address/WORDcount info */
	outp(FF_REG,0);

	/* load the page offset value. note that the 8237A is an 8-bit device
	** therefore we have to write the offset and count values on BYTE at
	** a time
	*/
	outp(OFFS_REG6,LO(offset));
	outp(OFFS_REG6,HI(offset));

	/* reset DMA channel 6 flipflop before writing new address/WORDcount info */
	outp(FF_REG,0);
	/* load the WORDcount information */
	outp(COUNT_REG6,LO(count-1));
	outp(COUNT_REG6,HI(count-1));
	enable();

	/* load the DMA page value */
	outp(PAGE_REG6,LO(page));

	/* enable DMA channel 6 */
	outp(MASK_REG,2);
}

void InitDMA7(WORD page, WORD offset, WORD count)
/* initialize the DMA controller */
{

	/* set the 8237 mode register for demand transfer on DMA channel 7 */
	outp(MODE_REG,0x47);

	/* reset DMA channel 7 before writing new address/WORDcount info */
	outp(FF_REG,0);

	/* load the DMA page value */
	outp(PAGE_REG7,LO(page));

	/* load the page offset value. note that the 8237A is an 8-bit device
	** therefore we have to write the offset and count values on BYTE at
	** a time
	*/
	outp(OFFS_REG7,LO(offset));
	outp(OFFS_REG7,HI(offset));

	/* load the WORDcount information */
	outp(COUNT_REG7,LO(count-1));
	outp(COUNT_REG7,HI(count-1));

	/* enable DMA channel 7 */
	outp(MASK_REG,3);
}

void	InitDMAChannels(void)
{
	short previous_buf;                /* a somewhat misleading name. Stores the
					   buffer after adinfo.next_buf */
	/* reset the dma channel flag (DmaStatus equiv for DAS1802) */
	dmachannel = 0;

	/* initialize the first DMA channel */
	if(sysinfo.debug){
		gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"Init DMA\n");
	}
    previous_buf = prevbuf(adinfo.next_buf);
	switch(adinfo.dma_ch1){
	case 5:
		/* initialize the 8237-A DMa controller channel 5 */
		InitDMA5(adinfo.dmapage[previous_buf], 
			 adinfo.dmabase[previous_buf],adinfo.dma_bufsize);
		break;
	case 6:
		InitDMA6(adinfo.dmapage[previous_buf], 
			 adinfo.dmabase[previous_buf],adinfo.dma_bufsize);
		break;
	}

	/* initialize the SECOND DMA channel */
	switch(adinfo.dma_ch2){
	case 6:
		/* initialize the 8237-A DMa controller channel 6 */
		InitDMA6(adinfo.dmapage[adinfo.next_buf], adinfo.dmabase[adinfo.next_buf],
				 adinfo.dma_bufsize);
		break;
	case 7:
		InitDMA7(adinfo.dmapage[adinfo.next_buf], adinfo.dmabase[adinfo.next_buf],
				 adinfo.dma_bufsize);
		break;
	}
}

void ResetDMAChannels(void)
{
	dmachannel = 1;
	ProgramDMAController();
}
