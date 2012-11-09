/*************************************************************************
**
** AD - Program to acquire analog data from a DT2821 a/d board
** and process it for spike waveforms on multiple 4 channel electrodes.
** Handles up to 8 channels using interrupt driven, or polled dual-DMA,
** triggered scan a/d conversion up to 250 KHz. Continuous storage
** to disk of up to 8 channels at an aggregate rate of 100KHz can
** be accomplished in the interrupt driven mode. SVGA display of
** acquired data and processed spike waveforms. Has been tested on
** Pentium P90, PCI, ATI Mach64; I486DX266, Cirrus 5826 VLB.
** Runs in 640K (minimum memory ???). Uses system timer to achieve
** an absolute temporal resolution of 1 msec. Optional use of a CTM05/10 can
** improve resolution.
** Support will be included for Dragon Tracker video tracking hardware.
**
** Current support summary:
**        -    8 channels
**        -    250 KHz max sampling rate
**        -    continuous acquisition, display, and disk storage up to 100 KHz
**        -    4 channel spike detection
**
**    Work in progress:
**        -    channel gain, threshold specification
**        -    extracted spike display and storage
**        -    spike parameter scatter plots
**
** Known problems:
**     -    rapid keystroke entry (e.g. hold down a key) causes a/d errors
**            due to kb interrupts
**        -    graphics library has glitches which cause text display to occur
**            at incorrect coordinates particularly during rapid update
**
** Developed with Borland C++ 4.0 under MSDOS6.2
**
**    Created 10/17/94
**    Written by          Matthew A. Wilson, Ph.D.
**                            Assistant Professor
**                            Center for Learning and Memory
**                            Departments of Brain and Cognitive Sciences, and Biology
**                            Massachusetts Institute of Technology
**                            Cambridge, MA 02139
**
**                            wilson@ai.mit.edu
**     and        
**                  Loren Frank    
**                  Graduate Student
**                            Department of Brain and Cognitive Sciences
**                            Massachusetts Institute of Technology
**                            Cambridge, MA 02139
**
**                            loren@ai.mit.edu
**
** Copyright (c) 1994,
** Matthew A. Wilson and the Massachusetts Institute of Technology
** All Rights Reserved
** Duplication or use of this software without the expressed written
** consent of the author is strictly forbidden
**
*************************************************************************
*/
#include "adext.h"


/* Global variables */
#ifdef DAS-1800
WORD	boardstatus;
#endif

extern int got_command;        // indicates command on command bus
extern int got_command2;        // indicates command on command bus 2
int count1 = 0, count2 = 0, count3 = 0, istart, location;
int spikesaves = 0;
ADInfo    adinfo;
SystemInfo sysinfo;
WORD    supcsr,adcsr;        /* DT2821 register variables */
#ifdef BORLAND
void _interrupt (*oldadirq)();
void _interrupt (*oldnetworkvec)();
#endif
#ifdef MSVC
void  _interrupt far *oldadirq;
void _interrupt far *oldnetworkvec;
#endif
char    far tmpstring[200];
char    far tmpstring2[200];
char    far tmpstring3[200];
char    far tmpstring4[200];
char    far tmpstring5[200];
Button    buttonlist[MAXBUTTONS];
MessageArea    message_area[MAXBUTTONS];

#ifdef BORGRAPH
char *Fonts[NFONTS] = {
  "DefaultFont",   "TriplexFont",   "SmallFont",
  "SansSerifFont", "GothicFont", "ScriptFont", "SimplexFont", "TriplexScriptFont",
  "ComplexFont", "EuropeanFont", "BoldFont"
};

char *LineStyles[] = {
  "SolidLn",  "DottedLn",  "CenterLn",  "DashedLn",  "UserBitLn"
};

char *FillStyles[] = {
  "EmptyFill",  "SolidFill",      "LineFill",      "LtSlashFill",
  "SlashFill",  "BkSlashFill",    "LtBkSlashFill", "HatchFill",
  "XHatchFill", "InterleaveFill", "WideDotFill",   "CloseDotFill"
};

char *TextDirect[] = {
  "HorizDir",  "VertDir"
};

char *HorizJust[] = {
  "LeftText",   "CenterText",   "RightText"
};

char *VertJust[] = {
  "BottomText",  "CenterText",  "TopText"
};

int    GraphDriver;        /* The Graphics device driver        */
int    GraphMode;        /* The Graphics mode value        */
double AspectRatio;        /* Aspect ratio of a pixel on the screen*/
int    MaxX, MaxY;        /* The maximum resolution of the screen */
int    MaxColors;        /* The maximum # of colors available    */
int    ErrorCode;        /* Reports any graphics errors        */
struct palettetype palette;        /* Used to read palette info    */
#endif


/*
*****************************************************************
**
** Program structure:
**        Miscellaneous Routines
**        DT2821 Routines
**        Data Processing/ISR Routines
**        DMA Routines
**        Display Routines
**        IO Routines
**        Main Program
**
*****************************************************************
*/

/*
************************************************************************
**
**   Miscellaneous Routines
**
************************************************************************
*/
void SystemRestore(void)
{
    int i;

	BoardRestore();
	
#ifndef BORGRAPH
    // restore the text mode
    restext();
#endif
    // free the dma buffers
    for (i = 0; i < DMABUFFERS; i++) {
        if(adinfo.dataptr[i]){
           free(adinfo.dataptr[i]);
           adinfo.dataptr[i] = NULL;
        }
    }
    /* restore the interrupt service routine vector for the ad */
    setvect(sysinfo.adirqvec, oldadirq);
	/* and for the network packet driver */
    setvect(sysinfo.networkvec, oldnetworkvec);
    if(sysinfo.command_mode == SLAVE){
        ClkRestoreSystem();
    }
    if(sysinfo.command_mode2 == SLAVE){
        Clk2RestoreSystem();
    }
    if(sysinfo.tracker_enabled){
        RestoreTracker();
    }
}

void SystemExit(int errorlevel,char *string)
{
/*
    gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"Press any key to exit\n");
    _getch();
*/
      ErrorMessage("Exiting...");
    if (adinfo.fp){
        DisableADIRQ;
        fclose(adinfo.fp); 
        adinfo.fp = NULL;
        EnableADIRQ;
    }
      ErrorMessage("Restoring...");
    SystemRestore();
	fprintf(stderr,"Errorcode %d: %s\n",errorlevel,string);
    exit(errorlevel);
}

long pow2(WORD power)
{
    if(power == 0) return(1);
    return(1<<(power));
}

void AllocateBuffers(void)
{
int    i;

    /* allocate the DMA buffers */
    adinfo.dataptr = (int **) malloc(sizeof(int *) * DMABUFFERS);
    for (i = 0; i < DMABUFFERS; i++) {
        allocbuff(adinfo.dataptr+i, adinfo.dma_bufsize);

        /* get the DMA page and offset value for the first buffer */
		GetDMAInfo(adinfo.dma_bufsize,adinfo.dataptr[i],
               &adinfo.dmapage[i],&adinfo.dmabase[i]);
       /* fprintf(stderr,"buffer %d maxsize:%u page:%u offset:%u     ",
            i,getDMAlength(adinfo.dataptr[i]),adinfo.dmapage[i],
           adinfo.dmabase[i]); 
        StatusMessage(tmpstring); */
    }
    /* allocate space for the continuous display mode buffer */
    if((adinfo.prevptr = (int *)calloc(adinfo.dma_bufsize,sizeof(int))) == NULL)    {
            gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny, 
                           "MEMORY ERROR: unable to allocate prev buffer\n", i);
    }
    for(i=0;i<adinfo.nelectrodes;i++){
        /* allocate a display buffer for old points to allow erasure */
        if((adinfo.electrode[i].prevbufptr = (int *)calloc(
            adinfo.dma_bufsize / NELECTRODES, sizeof(int))) == NULL){
            gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny, 
                           "MEMORY ERROR: unable to allocate prev buffer E%d\n",
            i);
            SystemExit(7,"prev buffer MEMORY ERROR");
        }
        /* allocate the spike buffers */
        if((adinfo.electrode[i].spikebuf = (Spike *)calloc(
            MAXSPIKES, sizeof(Spike))) == NULL){
            gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny, 
                   "MEMORY ERROR: unable to allocate spike buffer E%d\n", i);
            SystemExit(7,"spike buffer MEMORY ERROR");
        }
		/* allocate space for the subthreshold buffer */
		if ((adinfo.electrode[i].subthresh = (char *) calloc(
					adinfo.nelect_chan, sizeof(char))) == NULL) {
            gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny, 
                   "MEMORY ERROR: unable to allocate subthreshold buffer E%d\n",
				   i);
            SystemExit(7,"spike buffer MEMORY ERROR");
		}	
    }
}


void ADSetup(void)
{
int    i,j;

	BoardSetup();
	
    for(i=0;i<adinfo.nchannels;i++){
        adinfo.channel[i].adgain = 0;
        adinfo.channel[i].color = 15-i;
        adinfo.channel[i].triggerable = 1;
    adinfo.channel[i].thresh = 200;
adinfo.channel[i].subthresh = 1;
    }
    for(i=0;i<adinfo.nelectrodes;i++){
        for(j=0;j<adinfo.nelect_chan;j++){
            adinfo.electrode[i].channel[j] = j+i*adinfo.nelect_chan;
            adinfo.electrode[i].thresh[j] = 200;
			adinfo.electrode[i].subthresh[j] = 1;
        }
        adinfo.electrode[i].ntotal_spikes = 0;
        adinfo.electrode[i].displaythresh = 31;
    }
}

short nextbuf(short buf)
/* returns the index of the next buffer */
{
        return (buf + 1) % DMABUFFERS;
}

short prevbuf(short buf)
/* returns the index of the previous buffer */
{
        return buf > 0 ? buf - 1 : DMABUFFERS - 1;
}

unsigned long compute_timestamp(short bufindex, unsigned long offset)
{
   return *(adinfo.timestamp + bufindex) + (unsigned long) (((float) offset /
       (float) adinfo.dma_bufsize) * adinfo.conversion_duration);
}

int getnelectrodes(void)
{
    return adinfo.nelectrodes;
}

long DiskFree()
{
    struct diskfree_t free;
    long avail;

    if (_dos_getdiskfree(0, &free) != 0) {
     printf("Error in _dos_getdiskfree() call\n");
     exit(1);
    }
    return  (long) free.avail_clusters
        * (long) free.bytes_per_sector
        * (long) free.sectors_per_cluster;
}


/*
************************************************************************
**
**   DT2821 A/D Routines
**
************************************************************************
*/


void ADResetClock(void)
{
#ifdef OLD
    /* note that this will reset the PIO port as well */
    outpw(SUPCSR,BIT0);            /* initialize the board */
#endif
#ifdef DT2821
    outpw(TMRCTR,adinfo.clockreg);
#endif	
}

void InitAcq(void)
{
    int i;
    int boardstat;

    /* initialize the DT2821 board */
    if(boardstat = InitBoard()) {
        gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"ERROR initializing board.\n");
		SystemExit(boardstat,"Init board Error");
    }

    /* the DT2821 is now acquiring into buffer A */
    DmaStatus;    /* get supcsr variable */
    /*
    */
    adinfo.count = 0;
    adinfo.nbuffers = adinfo.computed_used = 0;
    adinfo.timestamp[0] = ReadTS();
    adinfo.computed_timestamp = (double) adinfo.timestamp[0];
    adinfo.dmadone = 0;
    adinfo.error = 0;
    adinfo.next_buf = 1;
    for (i = 0; i < adinfo.nelectrodes; i++) {
        adinfo.electrode[i].nspikes = 0;
        adinfo.electrode[i].prevstart = adinfo.buflen;
    }
    for (i = 0; i < DMABUFFERS; i++) 
        adinfo.buffer_valid[i] = 0;
}

/*
************************************************************************
**
**   Data Processing Routines
**
************************************************************************
*/
void interrupt GetCompletedBuffer(void)
{
   int    i;
   long   bufsize;
   unsigned long time;

/*
   fprintf(stderr, "in GetCompletedBuffer\n");
						fprintf(stderr, "%2x %2x %2x %2x %2x\n", 
								inp(DATASELECT), inp(CONTROLA), 
								inp(CONTROLB), inp(CONTROLC), inp(STATUS)); 
	*/
   /* disable interrupts while processing */
   disable();
#ifdef DT2821   
   if(sysinfo.debug){
        gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"ISR: Buffer f%d F%d : ",
        adinfo.next_buf,FilledBuffer);
   }
#endif   
   
   /* check for a/d error status */
   if(ErrorSet){
#ifdef DT2821  
      adinfo.error = adcsr;
#endif   
#ifdef DAS-1800  
      adinfo.error = boardstatus;
#endif   
      goto aderror;
   }
   /*
   ** get the time of occurence of the completion of the dma transfer.
   ** Use the system clock for now, but later go to the CTIO board.
   ** Note that the time of buffer completion is the time of buffer
   ** onset for the subsequent buffer.
   ** Empirical testing has shown that about 50-100 msec of jitter
   ** can be expected in the actual time of interrupt generation.
   ** Therefore the computed timestamp is a more accurate measure
   ** as long as its overall consistency with the system clock can
   ** be maintained.
   */
   time = ReadTS();
   adinfo.nbuffers++;
   adinfo.computed_timestamp += adinfo.conversion_duration;
   if (time <= (unsigned long) adinfo.computed_timestamp + 2) {
       /*
	   ** jitter in the interrupt time can only push the read timestamp forward,
	   ** so if the read timestamp is within 0.2 ms of the computed timestamp, 
	   ** use the read timestamp as the correct current time 
	   */
	   adinfo.timestamp[adinfo.next_buf] = time;
       adinfo.computed_timestamp = (double) time;
   }
   else {
       /* the computed timestamp is correct */
	   adinfo.timestamp[adinfo.next_buf] = (unsigned long) 
	   									   adinfo.computed_timestamp;
	   adinfo.computed_used++;							
   } 
   /*
   ** In the ISR
   ** FilledBuffer should return the buffer index of the buffer
   ** currently being filled because the ISR is called after
   ** the dmadone bit is set and the buffer bit has been automatically
   ** switched to the next buffer. The adinfo.filled_buffer must
   ** be set after dmadone has been detected (call to ISR)
   */

   adinfo.buffer_valid[adinfo.next_buf] = 1;
   /*
   ** increment the next buffer index.
   */
   /* adinfo.next_buf is the index of the buffer that is to be filled 
      next, so prevbuf(adinfo.next_buf) is the index of the buffer that was just      filled */
   adinfo.process_buf = prevbuf(adinfo.next_buf);
   adinfo.next_buf = nextbuf(adinfo.next_buf);

   adinfo.dmadone = 1;
   adinfo.error = 0;

   if(sysinfo.debug){
        gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,": buf %d valid :",
               adinfo.next_buf);
   }
   	/*
   	** check for the acq stop flag
   	*/
#ifdef DT2821   
   	if(sysinfo.acq){
      	/* interrupt on error, clear dmadone, dual dma, a/d clocked dma */
      	outpw(SUPCSR,BIT14 | BIT13 | BIT12 | BIT10);
   	} else {
      	/* this is the last buffer so turn off dual-dma. Acq will
      	** stop on the next dma done
      	*/
      	/* interrupt on error, clear dmadone */
      	outpw(SUPCSR,BIT14 | BIT10);
#endif	  
#ifdef DAS-1800
   	if(sysinfo.acq){
		// clear the status register
		outp(STATUS, 0x80);
	}
	else {
	  	// stop acquisition
		outp(STATUS, 0x0);
		outp(STATUS, 0x0);
		// disable fifo and counters
		outp(CONTROLA, 0x0);
#endif
      	if(sysinfo.debug){
            gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,
			        " *** terminate *** ");
      	}
        // need to make sure A/D has actually stopped. This requires
        // GetCompleteBuffer to be called twice. This condition is
        // checked in StartAcq
        sysinfo.stopped++;
   }

   /* fprintf(stderr, "GetCompletedBuffer section 2\n");
						fprintf(stderr, "%2x %2x %2x %2x %2x\n", 
								inp(DATASELECT), inp(CONTROLA), 
								inp(CONTROLB), inp(CONTROLC), inp(STATUS)); 
	*/
   /*
   ** reprogram the DMA controller. Note that this changes the filled
   ** buffer index adinfo.filled_buffer to be consistent with FilledBuffer
   */

   ProgramDMAController();

   	/* check for error */
   	if (ErrorSet)
#ifdef DT2821  
   		adinfo.error = adcsr;
#endif   
#ifdef DAS-1800  
   		adinfo.error = boardstatus;
#endif   
   aderror:
   if(adinfo.error != 0){
      sprintf(tmpstring,"ISR: Buffer %d %ld : Error %X                 ",
      prevbuf(adinfo.next_buf),
      adinfo.timestamp[adinfo.process_buf],
      adinfo.error);
      ErrorMessage(tmpstring);
   } else {
      if(sysinfo.showstatus){
         sprintf(tmpstring,"ISR: NBuffer %d PBuffer %d %7ld %7ld %7ld %7ld           ",
            adinfo.next_buf, adinfo.process_buf,
            adinfo.timestamp[adinfo.next_buf],
            adinfo.timestamp[adinfo.next_buf] -
            adinfo.timestamp[adinfo.process_buf],
            (unsigned long)(adinfo.computed_timestamp/1e3),
            (unsigned long)(adinfo.computed_timestamp/1e3) -
            adinfo.timestamp[adinfo.next_buf]);
         StatusMessage(tmpstring);
      }
   }
 #ifdef DT2821  
   if(sysinfo.debug){
        gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"ISR: Buffer f%d F%d\n", 
        adinfo.process_buf,FilledBuffer);
   }
#endif   
abort_isr:
   /* signal the end of the ISR to the controller */
   outp(IRQMASTER,IRQEOI);
   outp(IRQSLAVE,IRQEOI);
   /* reenable interrupts */
   enable();
//sprintf(tmpstring,"count=%ld proc=%d next=%d",
//    adinfo.count,adinfo.process_buf,adinfo.next_buf);
//ErrorMessage(tmpstring);
}


char temp[100];
void SpikeModeProcessBuffer(void)
{
/*
** Multiple buffer code 
*/
register int    i, j, k, l;
int             e_num;
register int    *dptr;
register int    *spikedataptr;
register Spike  *spikeptr;
register int    *tptr;          /* pointer to threshold buffer */
register char	*subptr;	/* pointer to subthreshold status array */
int            *previousdptr;
register int    *pdptr;
int             *dataptr;           /* pointer to the data in the buffer */
int             nspikes;
int             offset;
ElectrodeInfo   *electrode;
int             peak;
int             previousbuf;
int        start;
int        bufoffset;
register int        previouslength;
register int        currentlength;
int        spikefound;
long            sum;
int        totspikes=0;

    DisableADIRQ;
    dataptr = adinfo.dataptr[adinfo.process_buf];
    previousbuf = prevbuf(adinfo.process_buf);
    previousdptr = adinfo.dataptr[previousbuf]; 

    offset = ADOFFSET;

    if(sysinfo.debug){
        fprintf(stderr,"testing buffer validity: buf %d=%d : buf %d=%d :",
        adinfo.process_buf,
        adinfo.buffer_valid[adinfo.process_buf],
        (adinfo.process_buf),
        adinfo.buffer_valid[prevbuf(adinfo.process_buf)]);
    }

    if(sysinfo.detect_spikes){
        /*
        ** loop through once for each electrode in the buffer
        */
        for(e_num = 0; e_num < adinfo.nelectrodes; e_num++) {
            electrode  = adinfo.electrode + e_num;
            spikeptr = electrode->spikebuf;
            spikedataptr = spikeptr->dataptr;
            nspikes = 0;
            start = 0;
            /* 
	    ** the buffer start offset is 0 or 4 if there are two electrodes 
	    */
            bufoffset = e_num * adinfo.nelect_chan;
            dptr = dataptr + bufoffset;
            pdptr = previousdptr + electrode->prevstart + bufoffset;
            /*
            ** Check the last elements of the previous buffer  
            ** for an above threshold event
            */
            for(i = electrode->prevstart; i < adinfo.dma_bufsize; i+=
                adinfo.nchannels) {
                spikefound = 0;
                tptr = electrode->thresh;
		subptr = electrode->subthresh;
                for(j = 0; j < adinfo.nelect_chan; j++) {
                    /*
                    ** scan the buffer for threshold crossings
                    */
                    if (*(pdptr)-offset > *(tptr)) { 
			if (*subptr) {
			    /* if it started out subthreshold */
			    spikefound = 1;
			    /*
			    ** detected a threshold crossing. 
			    */
			    /* set the electrode number and the time */
			    spikeptr->type = 's';
			    spikeptr->type2 = 's';
			    spikeptr->electrode = e_num;
			    spikeptr->timestamp = compute_timestamp(previousbuf,
													    i);
			    /* move the data pointer back to the beginning of
			    the period to be saved */
			    pdptr -= (j + adinfo.prespike_points);
			    /* 
			    ** previouslength is the length of the section of 
			    ** the previous buffer to be saved
			    ** currentlength is the length of the section of the
			    ** current buffer to be saved
			    */
			    previouslength = adinfo.dma_bufsize - i + 
					     adinfo.prespike_points; 
			    currentlength = adinfo.spikelen - previouslength;
			    for(k=0; k<previouslength; k+=adinfo.nchannels) {
				/*
				** copy each word from the previous data buffer
				** to the appropriate electrode data buffer.
				** Note that the data pointer will be 
				** pointing to the data just past the spike
				** when the memory copy is completed
				*/
				memcpy(spikedataptr, pdptr, adinfo.nelect_size);
				pdptr += adinfo.nchannels;
				spikedataptr += adinfo.nelect_chan;
			    }
			    for(k=0; k < currentlength; k+= adinfo.nchannels) {
				/*
				** copy each word from the current data buffer 
				** to the appropriate electrode data buffer.
				*/
				memcpy(spikedataptr, dptr, adinfo.nelect_size);
				dptr += adinfo.nchannels;
				spikedataptr += adinfo.nelect_chan;
			    }
			    nspikes++;
			    /* there can only be one spike in this buffer */
			    i = adinfo.dma_bufsize;
			    /* 
			    ** the next threshold crossing event will start in 
			    ** the current data buffer. Increment start so 
			    ** that detection will begin after the current spike
			    */
			    start = currentlength;
			    /* 
			    ** move to the next spike 
			    */
			    spikeptr++;
			    spikedataptr = spikeptr->dataptr;
			    /*
			    ** hop to the next event. no need to look at the
			    ** other channels on the same electrode
			    */
			    /* set all of subthreshold varibles to indicate
			    ** suprathreshold status 
			    */
			    subptr = electrode->subthresh;	
			    for (l = 0; l < adinfo.nelect_chan; l++) {	
				*subptr = 0;
				subptr++;
			    }	
			    break;
			}
		    } 
		    else {
			// the input is currently subthreshold 
			*subptr = 1;
		    }	
                    pdptr++;
                    tptr++;
		    subptr++;
                }
                if (!spikefound){
                   pdptr += adinfo.elect_inc;
		}
            }
            /* 
            ** if the start point for the next spike is close to the
            ** beginning of the current buffer, scan the first points of
            ** the current buffer for a threshold crossing 
            */
            for(i = start; i < adinfo.prespike_points; i +=
                adinfo.nchannels) {
                spikefound = 0;
                tptr = electrode->thresh;
		subptr = electrode->subthresh;
                for(j = 0; j < adinfo.nelect_chan; j++) {
                    /*
                    ** scan the buffer for threshold crossings
                    */
                    if(*(dptr)-offset > *(tptr)) { 
			if (*subptr) {
			    /*
			    ** detected a threshold crossing. 
			    */
			    spikefound = 1;
			    /* set the electrode number and the time */
			    spikeptr->type = 's';
			    spikeptr->type2 = 's';
			    spikeptr->electrode = e_num;
			    spikeptr->timestamp = 
				compute_timestamp(adinfo.process_buf, i);
			    /* 
			    ** move the data pointer back to the beginning of
			    ** the buffer 
			    */
			    dptr = dataptr + bufoffset;
			    /* 
			    ** previouslength is the length of the section of 
			    ** the previous buffer to be saved
			    ** currentlength is the length of the section of the
			    ** current buffer to be saved
			    */
			    previouslength = adinfo.prespike_points - i;
			    currentlength = adinfo.spikelen - previouslength;
			    /* 
			    ** move the previous data pointer to the right 
			    ** place 
			    */
			    pdptr = previousdptr + adinfo.dma_bufsize - 
					    previouslength;
			    for(k=0; k<previouslength; k += adinfo.nchannels) {
				/*
				** copy each word from the previous data buffer
				** to the appropriate electrode data buffer.
				*/
				memcpy(spikedataptr, pdptr, adinfo.nelect_size);
				pdptr += adinfo.nchannels;
				spikedataptr += adinfo.nelect_chan;
			    }
			    for(k=0; k < currentlength; k += adinfo.nchannels) {
				/*
				** copy each word from the current data buffer 
				** to the appropriate electrode data buffer.
				*/
				memcpy(spikedataptr, dptr, adinfo.nelect_size);
				dptr += adinfo.nchannels;
				spikedataptr += adinfo.nelect_chan;
			    }
			    nspikes++;
			    i += currentlength;
			    /* spike detection should start immediately after 
			       this spike */
			    start = i;
			    /* move past the spike */
			    spikeptr++;
			    spikedataptr = spikeptr->dataptr;
			    /* 
			    ** set all of subthreshold varibles to indicate
			    ** suprathreshold status 
			    */
			    subptr = electrode->subthresh;	
			    for (l = 0; l < adinfo.nelect_chan; l++) {	
				*subptr = 0;
				subptr++;
			    }	
			    break;
			}
		    }
		    else {
			// the input is currently subthreshold
			*subptr = 1;
		    }	
                    dptr++;
                    tptr++;
		    subptr++;
                }
                if (!spikefound){
                   dptr += adinfo.elect_inc;
		}
            }        
            if (start == 0){
		/* 
		** no spike was found in the first few points of the buffer,
		** so start after adinfo.prespike_points 
		*/
		start = adinfo.prespike_points;
	    }
            /* find the spikes up to the point of potential overlap with the
            ** next buffer and copy them to spikeptr 
	    */
            currentlength = adinfo.spikelen;
            for(i = start; i < adinfo.buflen; i += adinfo.nchannels) {
                spikefound = 0;
                tptr = electrode->thresh;
		subptr = electrode->subthresh;
                for(j = 0; j < adinfo.nelect_chan; j++) {
                    /*
                    ** scan the buffer for threshold crossings
                    */
                    if(*(dptr)-offset > *(tptr)) { 
			if (*subptr) {
			    /*
			    ** detected a threshold crossing. 
			    */
			    spikefound = 1;
			    if(nspikes >= MAXSPIKES){
				ErrorMessage("WARNING: spike buffer overrun");
				break;
			    }
			    /* set the electrode number and the time */
			    spikeptr->type = 's';
			    spikeptr->type2 = 's';
			    spikeptr->electrode = e_num;
			    spikeptr->timestamp = 
				compute_timestamp(adinfo.process_buf, i);
			    /* move the data pointer back to the beginning of
			    ** the period to be saved 
			    */
			    dptr -= (j + adinfo.prespike_points);
			    for(k = 0; k < currentlength; k+=adinfo.nchannels) {
				/* copy each word from the data buffer to the 
				** appropriate electrode data buffer.
				** Note that the data pointer will be 
				** pointing to the data just past the spike
				** when the memory copy is completed
				*/
				memcpy(spikedataptr, dptr, adinfo.nelect_size);
				dptr += adinfo.nchannels;
				spikedataptr += adinfo.nelect_chan;
			    }
			    /* move to the next spike */
			    spikeptr++;
			    spikedataptr = spikeptr->dataptr;
			    nspikes++;
			    /* 
			    ** adinfo.nchannels will be added to i at the top 
			    ** of the for loop, so subtract one extra 
			    ** adinfo.nchannels
			    */
			    i += adinfo.spikesep - adinfo.nchannels;
			    /* 
			    ** set all of subthreshold varibles to indicate
			    ** suprathreshold status 
			    */
			    subptr = electrode->subthresh;	
			    for (l = 0; l < adinfo.nelect_chan; l++) {	
				*subptr = 0;
				subptr++;
			    }	
			    /*
			    ** hop to the next event. no need to look at the
			    ** other channels on the same electrode
			    */
			    break;
			}
		    }
		    else {
			// the input is currently subthreshold
			*subptr = 1;
		    }	
                    dptr++;
                    tptr++;
		    subptr++;
                }
                if (!spikefound) {
                    dptr += adinfo.elect_inc;
		}
            }
            if(sysinfo.debug){
                gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,
		    "Buf %d : E %d : Spk %ld      ", adinfo.filled_buffer,
		    j, electrode->nspikes);
            }
            /* 
	    ** processing might have found a spike that overlaps the section 
            ** of the previous buffer that will be processed next time around.
            ** Set the start point for the next processing run to be the point
            ** at which processing ended for this run 
	    */
            electrode->prevstart = i;
            electrode->nspikes = nspikes;
            totspikes += nspikes;
        }
	if(totspikes > adinfo.peak_rate){
	    adinfo.peak_rate = totspikes;
	}
    }
    EnableADIRQ;
}

void ContinuousModeProcessBuffer(void)
{
/*
** Multiple buffer code 
*/
register int    i, j, k, l;
int             e_num;
register int    *dptr;
register ChannelInfo  *cptr;
int             *dataptr;           /* pointer to the data in the buffer */
int             nspikes;
int             offset;
ElectrodeInfo   *electrode;
int             peak;
int             previousbuf;
int        start;
int        bufoffset;
int        spikefound;
long            sum;
int        totspikes=0;
int	cnum;

    DisableADIRQ;
    dataptr = adinfo.dataptr[adinfo.process_buf];
    sysinfo.trigger=0;
	 nspikes = 0;
	 spikefound = 0;

    offset = ADOFFSET;

    if(sysinfo.debug){
        fprintf(stderr,"testing buffer validity: buf %d=%d : buf %d=%d :",
        adinfo.process_buf,
        adinfo.buffer_valid[adinfo.process_buf],
        (adinfo.process_buf),
        adinfo.buffer_valid[prevbuf(adinfo.process_buf)]);
    }

    if(sysinfo.triggered_continuous){
        /*
        ** loop through once for each electrode in the buffer
        */
	/*
	** scan each channel
	*/
	for(cnum = 0; cnum < adinfo.nchannels; cnum++) {
	    cptr = adinfo.channel+cnum;
	    /*
	    ** is it a triggerable channel?
	    */
	    if(!cptr->triggerable) continue;
	    dptr = dataptr+cnum;
	    for(i = 0; i < adinfo.buflen; i += adinfo.nchannels) {
		if(*(dptr)-offset > cptr->thresh) {
		    if (cptr->subthresh) {
			/*
			** detected a threshold crossing. 
			*/
			spikefound = 1;
			if(nspikes >= MAXSPIKES){
			    ErrorMessage("WARNING: spike buffer overrun");
			    break;
			}
			sysinfo.trigger = i;
			nspikes++;
			/*
			** hop to the next event. no need to look at the
			** other channels on the same electrode
			*/
			break;
		    }
		} else {
		    /* the input is currently subthreshold */
		    cptr->subthresh = 1;
		}
		dptr += adinfo.nchannels;
	    }
	    if (spikefound) {
		/*
		** cant get more than one triggered event
		** per buffer so just break out
		*/
		break;
	    }
	}
#ifdef OLD
        for(e_num = 0; e_num < adinfo.nelectrodes; e_num++) {
            electrode  = adinfo.electrode + e_num;
            nspikes = 0;
            start = 0;
            /* 
	    ** the buffer start offset is 0 or 4 if there are two electrodes 
	    */
            bufoffset = e_num * adinfo.nelect_chan;
            dptr = dataptr + bufoffset;
            for(i = 0; i < adinfo.buflen; i += adinfo.nchannels) {
                spikefound = 0;
                tptr = electrode->thresh;
		subptr = electrode->subthresh;
                for(j = 0; j < adinfo.nelect_chan; j++) {
                    /*
                    ** scan the buffer for threshold crossings
                    */
		    /*
		    ** is the channel triggerable
		    */
                    if(*(dptr)-offset > *(tptr)) {
			if (*subptr) {
			    /*
			    ** detected a threshold crossing. 
			    */
			    spikefound = 1;
			    sysinfo.trigger = i;
			    nspikes++;
			    /*
			    ** hop to the next event. no need to look at the
			    ** other channels on the same electrode
			    */
			    break;
			}
		    }
		    else {
			// the input is currently subthreshold
			*subptr = 1;
		    }	
                    dptr++;
                    tptr++;
		    subptr++;
                }
                if (spikefound) {
		    /*
		    ** cant get more than one triggered event
		    ** per buffer so just break out
		    */
		    break;
		}
		dptr += adinfo.elect_inc;
            }
            if(sysinfo.debug){
                gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,
		    "Buf %d : E %d : Spk %ld      ", adinfo.filled_buffer,
		    j, electrode->nspikes);
            }
            /* 
	    ** processing might have found a spike that overlaps the section 
            ** of the previous buffer that will be processed next time around.
            ** Set the start point for the next processing run to be the point
            ** at which processing ended for this run 
	    */
            electrode->nspikes = nspikes;
            totspikes += nspikes;
        }
	if(totspikes > adinfo.peak_rate){
	    adinfo.peak_rate = totspikes;
	}
#endif
    }
    EnableADIRQ;
}

void CheckBoardID(void)
{
int	id;
#ifdef DT31EZ
	id = inpw(IDR);
	switch(id){
	case DT31EZ_ID:	
		fprintf(stderr,"DT31EZ identified\n");
		sysinfo.adboard_id = DT31EZ_ID;
		break;
	default:
		fprintf(stderr,"unknown board. Assuming DT2821. id = %d\n",id);
		sysinfo.adboard_id = DT2821_ID;
		break;
	}
#else
	sysinfo.adboard_id = DT2821_ID;
#endif	
}	

void WaitForKey(void)
{
	fprintf(stderr,"press any key to continue");
	getch();
}
/*
************************************************************************
**
**   Main program
**
************************************************************************
*/

void main(int argc, char **argv)
{
int    	nxtarg;
short   i, j;
int	q;
int	limit = 100;
int     noacq = 0;
int	tmpval;
ElectrodeInfo    *electrode;
int     errcount;
int	nsamples;
unsigned long clock;

#ifdef DT2821
     /* initialize the board  */
     outpw(SUPCSR,BIT0);
#endif
    sysinfo.oldtime = 0;
    sysinfo.pio2 = 0;
    sysinfo.olddisktime = 0;
    sysinfo.oldsynctime = 0;
    sysinfo.stim.pulselen = 0;
    sysinfo.stim.stimflag = 0;
    sysinfo.stim.tstflag = 0;
    sysinfo.stim.ctlflag = 0;
    sysinfo.stim.seqnumber = 0;
    sysinfo.stim.seqlength = 0;
	sysinfo.stim.test[0] = 0;
	sysinfo.stim.control[0] = 0;
	sysinfo.stim.grid = 0;
	sysinfo.stim.protocol = TIME;
	sysinfo.stim.mode = BOTH;
    sysinfo.command_mode = SINGLE;
    got_command = 0;
    sysinfo.command_mode2 = SINGLE;
    got_command2 = 0;
    adinfo.peak_rate = 0;
    adinfo.rate_req = 250000.0;        /* default sample rate */
    adinfo.irq = 15;                    /* default interrupt request */
    adinfo.dma_ch1 = 5;
    adinfo.dma_ch2 = 6;
    adinfo.nchannels = 8;            /* default number of a/d channels */
    adinfo.nelectrodes = 2;
    /* make sure that the buffer is a multiple of the number of channels */
    adinfo.error_count = 0;                 /* count of a/d errors during acq */
    adinfo.display_lost = 0;            /* number of buffers not displayed */
    adinfo.disk_lost = 0;              /* number of buffers not saved to disk */
    adinfo.nspikes = 0;                    /* number of detected spike events */
//    adinfo.overlap_window_size = (SPIKELEN - PRESPIKE) * adinfo.nelectrodes;
    adinfo.count = 0;
    adinfo.filled_buffer = 0;
    adinfo.nelect_chan = NELECTRODE_CHANNELS;
    for (i = 0; i < DMABUFFERS; i++){
       adinfo.buffer_valid[i] = 0; 
    }
    adinfo.nelect_size = adinfo.nelect_chan * sizeof(int);
    adinfo.prespike_points = adinfo.nchannels * NPRESPIKE;
    adinfo.next_buf = 1; 
    strcpy(adinfo.fname, NO_FILE_STRING);

    /* set the filters to 300 low and 6KHz high and set the amp gain to 12 */
    for (i = 0; i < NAMP_CHANNELS; i++) {
       adinfo.channel[i].filter = 136;
       adinfo.channel[i].ampgain = GetAmpGain(14);
    }
    for (i = 0; i < MAXFASTCONFIG; i++) {
	sysinfo.configfilename[i] = (char *)calloc(MAXFILENAME,sizeof(char));
    }

    sysinfo.helpon = 0;
    sysinfo.debug = 0; 
    sysinfo.debugwinx = 1; 
    sysinfo.debugwiny = 1; 
    sysinfo.disk = 0;                        /* flag to enable disk output */ 
    sysinfo.graphics = 1;                /* flag to enable graphics display */ 
    sysinfo.acq = 0;                        /* flag to enable acquisition */ 
    sysinfo.acqbon = 0;                        /* flag for acquisition button*/ 
    sysinfo.stopped = 2;                // indicates acq in full stopped state
    sysinfo.kbhandled = 1;
    sysinfo.drawmode = 0; 
    sysinfo.quit = 0;
    sysinfo.detect_spikes = 1;
    sysinfo.triggered_continuous = 0;
    sysinfo.continuous_pixelindex = 0;
    sysinfo.trigger = 0;
    sysinfo.autoscale = 0;
    sysinfo.tracker_enabled = 0;
    sysinfo.conttype = 'c';
    sysinfo.spiketype = 's';
    sysinfo.eventtype = 'e';
    sysinfo.eventstring_size = EVENTSTRINGSIZE;
    sysinfo.showstatus = 0;
    sysinfo.stringinput = 0;
    sysinfo.selected_button = 2;
    sysinfo.selected_channel = 0;
    sysinfo.selected_spbutton = 0;
    sysinfo.channel_selected = 1;
    sysinfo.maxbutton = 0;
    sysinfo.contoverlay = 0;
    sysinfo.diskfree = DiskFree();
    sysinfo.adirqmask = IRQ15MASK;
    sysinfo.adirqvec = IRQ15VEC;
    sysinfo.networkvec = NETWORKVEC;
    sysinfo.mode = SPIKE_MODE;
    sysinfo.neventstrings = 0;
    SetupSequences();
    nsamples = 3072;
    LoadConfig("default.cfg");

    nxtarg = 0;
    while(++nxtarg < argc){
        if(strcmp(argv[nxtarg],"-usage") == 0){
            fprintf(stderr,
            "usage: %s [-nchannels nchannels][-nelectrodes nelectrodes][-dma ch1 ch2][-rate rate][-bufsize nbytes]\n",
                argv[0]);
                fprintf(stderr,"\t[-master][-slave][-single][-adirq 10/15]\n");
                fprintf(stderr,"\t[-master2][-slave2][-pio2]\n");
            exit(0);
        } else
        if(strcmp(argv[nxtarg],"-version") == 0){
	    fprintf(stderr,"AD version %s\n",ADVERSION);
	    exit(0);
        } else
        if(strcmp(argv[nxtarg],"-nchannels") == 0){
            adinfo.nchannels = atoi(argv[++nxtarg]);
        } else 
        if(strcmp(argv[nxtarg],"-nelectrodes") == 0){
            adinfo.nelectrodes = atoi(argv[++nxtarg]);
        } else 
        if(strcmp(argv[nxtarg],"-limit") == 0){
            limit = atoi(argv[++nxtarg]);
        } else
        if(strcmp(argv[nxtarg],"-pio2") == 0){
            sysinfo.pio2 = 1;
        } else
        if(strcmp(argv[nxtarg],"-dma") == 0){
            adinfo.dma_ch1 = atoi(argv[++nxtarg]);
            adinfo.dma_ch2 = atoi(argv[++nxtarg]);
        } else
        if(strcmp(argv[nxtarg],"-debug") == 0){
            sysinfo.debug = 1;
        } else
        if(strcmp(argv[nxtarg],"-tracker") == 0){
            sysinfo.tracker_enabled = 1;
	    sysinfo.mode = TRACKER_MODE;
        } else
	if(strcmp(argv[nxtarg],"-rate") == 0){
            adinfo.rate_req = atof(argv[++nxtarg]);
        } else
	if(strcmp(argv[nxtarg],"-bufsize") == 0){
            nsamples = atoi(argv[++nxtarg]);
        } else
        if(strcmp(argv[nxtarg],"-config") == 0){
            LoadConfig(argv[++nxtarg]);
        } else
        if(strcmp(argv[nxtarg],"-adirq") == 0){
	    adinfo.irq = atoi(argv[++nxtarg]);
	    switch(adinfo.irq){
	    case 10:
		sysinfo.adirqmask = IRQ10MASK;
		sysinfo.adirqvec = IRQ10VEC;
		break;
	    case 15:
		sysinfo.adirqmask = IRQ15MASK;
		sysinfo.adirqvec = IRQ15VEC;
		break;
	    default:
		fprintf(stderr,"only irq 10 and 15 supported\n");
		break;
	    }
        } else
        if(strcmp(argv[nxtarg],"-master") == 0){
            sysinfo.command_mode = MASTER;
        } else
        if(strcmp(argv[nxtarg],"-slave") == 0){
            sysinfo.command_mode = SLAVE;
        } else
        if(strcmp(argv[nxtarg],"-master2") == 0){
            sysinfo.command_mode2 = MASTER;
        } else
        if(strcmp(argv[nxtarg],"-slave2") == 0){
            sysinfo.command_mode2 = SLAVE;
        } else
        if(strcmp(argv[nxtarg],"-single") == 0){
            sysinfo.command_mode = SINGLE;
        } else
        if(argv[nxtarg][0] == '-'){
            fprintf(stderr, "unknown option '%s'\n",argv[nxtarg]);
            exit(-1);
        }
    }

    if(sysinfo.tracker_enabled){
	nsamples = 1;
    }
    /* number of data points per buffer */
    adinfo.dma_bufsize = adinfo.nchannels*nsamples;  
	
    /* 
    ** 1 msec refractory trigger. This must be set to a reasonable value
    ** for actual acquisition
    */
    sysinfo.refractory = 1e-3*adinfo.rate_req/adinfo.nchannels;

    if(sysinfo.command_mode == SINGLE){
        fprintf(stderr,"SINGLE System Mode\n");
    } else 
    if(sysinfo.command_mode == SLAVE){
        fprintf(stderr,"SLAVE System Mode\n");
    } else 
    if(sysinfo.command_mode == MASTER){
        fprintf(stderr,"MASTER System Mode\n");
    } 
    if(sysinfo.command_mode2 == SINGLE){
        fprintf(stderr,"Command Bus2: Stimulation Mode\n");
    } else 
    if(sysinfo.command_mode2 == SLAVE){
        fprintf(stderr,"Command Bus2: SLAVE System Mode\n");
    } else 
    if(sysinfo.command_mode2 == MASTER){
        fprintf(stderr,"Command Bus2: MASTER System Mode\n");
    } 


    /*
    ** check the board id
    */
    CheckBoardID();

    ClkResetBoard();
    // use the SingleSetup for single systems. For synchronized systems
    // use MasterSlave setup
    if(sysinfo.command_mode == MASTER || sysinfo.command_mode ==SLAVE){
        ClkSetupMasterSlave();
    } else {
        ClkSetupSingle();
    }
    if(sysinfo.command_mode2 == SINGLE){
	// set up the stimulation clocks
	ClkSetupStim();
    }

    // prepare the command bus interrupt routine. Only for slave systems
    // dont do this on master systems do to potential conflicts with
    // IRQ5 and the tracker card
    if(sysinfo.command_mode == SLAVE){
        ClkSetupCommandBus();
    }
    if(sysinfo.command_mode2 == SLAVE){
        Clk2SetupCommandBus();
    }
    // set the clock to zero
    // note that this only resets the single system
    // to reset master/slave systems, Fout must be turned off and
    // each of the systems reset
    ClkResetClock();
    // arm the counters and begin counting immediately for single systems,
    // wait for Fout for master/slave systems
    ArmClock();
    if(sysinfo.command_mode == MASTER){
        // To synchronize counts for master/slave systems, 
        // start Fout after arming. On master/slave systems synch
        // must be established by issuing the resetclk command
        // from the master system. Starting Fout does not guarantee
        // synchronization, it simply allows the clock to start
        // This has no effect on single system setups
        StartFout();
    }

    /* set the mode to continuous if there are > NAMP_CHANNELS channels */
    if (adinfo.nchannels > NAMP_CHANNELS){
        sysinfo.mode = CONTINUOUS_MODE;
    }
     
    for (i = 0; i < adinfo.nelectrodes; i++){
        sysinfo.overlay[i] = 0;
    }


    fprintf(stderr,"System Settings:\n");
    if(!sysinfo.tracker_enabled){
#ifdef DAS-1800	
	fprintf(stderr,"\tAD board: Keithley Metrabyte DAS1802\n");
	fprintf(stderr,"\t\tIRQ=%d\n", adinfo.irq);
	fprintf(stderr,"\t\tDMA=%d,%d\n", adinfo.dma_ch1,adinfo.dma_ch2);
	fprintf(stderr,"\t\tBase addr=0x%0X\n", BASE);
	fprintf(stderr,"\tAmp control: DIO24\n");
	fprintf(stderr,"\t\tBase addr=0x%0X\n", PP_BASE);
#endif
#ifdef DT2821	
	fprintf(stderr,"\tAD board: Data Translations DT2821\n");
	fprintf(stderr,"\t\tIRQ=%d\n", adinfo.irq);
	fprintf(stderr,"\t\tDMA=%d,%d\n", adinfo.dma_ch1,adinfo.dma_ch2);
	fprintf(stderr,"\t\tBase addr=0x%0X\n", BASE);
#endif
    }
    fprintf(stderr,"\tClock board: CIO-CTR10\n");
    fprintf(stderr,"\t\tIRQ=%d\n", clk_irq);
    fprintf(stderr,"\t\tBase addr=0x%0X\n", CLKBASE);
    fprintf(stderr,"\t\tIRQ2=%d\n", clk2_irq);
    fprintf(stderr,"\t\tBase addr2=0x%0X\n", CLKBASE2);

    if(sysinfo.pio2){
	fprintf(stderr,"\tSecond PIO: DIO24\n");
	fprintf(stderr,"\t\tBase addr=0x%0X\n", PIO2_BASE);
    }
	
    if(sysinfo.tracker_enabled){
        SetupTracker();
    }

    WaitForKey();

    /* set the separation between spikes in terms of the dma buffer */
    adinfo.spikesep = adinfo.nchannels * (SINGLESPIKELEN - NPRESPIKE);

    /* set the length of the buffer that should be processed into a single spike
    ** to be the normal buffer length minus the length of a spike that could
    ** overlap between two data buffers */
    adinfo.buflen = adinfo.dma_bufsize - adinfo.spikesep;

    for (i = 0; i < adinfo.nelectrodes; i++) {
        adinfo.electrode[i].nspikes = 0;
        adinfo.electrode[i].prevstart = adinfo.buflen;
    }

    /* set the length of a single spike in terms of the dma buffer */
    adinfo.spikelen = adinfo.nchannels * SINGLESPIKELEN;

    /* set the increment to skip over intervening electrodes in the dma buffer*/
    adinfo.elect_inc = (adinfo.nelectrodes - 1) * adinfo.nelect_chan;

    if(!noacq){
       ADSetup();
    }

    CreateMenu();
    SystemSetup();


    if(sysinfo.debug){
        gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,
	"Base address 0x%x\n",BASE);
    }

    if(!SetRate(adinfo.rate_req,&adinfo.rate_set)){
	SystemExit(2,"Set Clock Error");
    }
    /*
    ** compute the duration in units of 100 microseconds of a
    ** single buffer a/d conversion 
    */
    adinfo.conversion_duration = SECOND * (double) adinfo.dma_bufsize / 
	                             (double) adinfo.rate_set;

	AllocateBuffers();
    InitDMAChannels();

    oldadirq = getvect(sysinfo.adirqvec);

    /*
    ** disconnect any network packet driver installed
    */
    oldnetworkvec = getvect(sysinfo.networkvec);
    setvect(sysinfo.networkvec, NULL);
	
    /*
    ** set up the interrupt service routine to handle dma buffer processing
    */
    setvect(sysinfo.adirqvec, GetCompletedBuffer);
    /* enable the IRQ which will be raised on error or dmadone */
    EnableADIRQ;

    InitDTPIO();
    if(sysinfo.pio2){
	SetupPIO2();
    }
    SetAmpControls();
    RefreshDisplay();
    DisplayStartupStatus();

    while(!sysinfo.quit) {
        /* check for keyboard and mouse input */
        if(sysinfo.mouse && sysinfo.mouseactive){
            CheckMouse();
        }
        /* check for command bus input. should only happen on slave systems */
        if(got_command){
            ClkProcessCommand();
        }
        if(got_command2){
            Clk2ProcessCommand();
        }
        if(_kbhit()){
            CheckKeyboard();
        }
	/*
	** dont allow acquisition and the tracker to run simultaneously
	*/
        if(sysinfo.acq && !sysinfo.tracker_enabled){
            /* process the completed buffer */
        if (sysinfo.mode == SPIKE_MODE){
		if(adinfo.count > 0){
		    SpikeModeProcessBuffer();
		}
	    }
            if (sysinfo.mode == CONTINUOUS_MODE){
		if(adinfo.count > 0){
		    ContinuousModeProcessBuffer();
		}
	    }
           if(adinfo.count > 1){
                for (i = 0; i < adinfo.nelectrodes; i++) {
                     electrode = adinfo.electrode + i;
                    electrode->ntotal_spikes += electrode->nspikes;
                    adinfo.nspikes += electrode->nspikes;
                }

                /* when the A/D for the current buffer is completed, an 
                **interrupt  will be generated sometime after this point. In 
                ** the A/D ISR the timestamp will be obtained and the dmadone 
                ** flag will be raised in the in the SUPCSR. When the ISR 
                ** returns, processing on the current buffer must end and 
                **processing on the newly filled buffer must begin.
                */
                /* if disk output is enabled then check to see whether
                ** the current a/d buffer has been filled. If so then cant
                ** afford to save data from the last filled buffer since
                ** time is now of the essence. In interrupt driven mode,
                ** check to see whether the ISR was recently called thus setting
                ** either the error or dmadone flags in the adinfo structure.
                ** In polled mode check the status of the dma done bit on the
                ** controller status register.
                */
                if(sysinfo.graphics){
                    /*
                    ** assume all were lost
                    */
                    for(i=0;i<adinfo.nelectrodes;i++){
                        electrode = adinfo.electrode+i;
                        adinfo.display_lost += electrode->nspikes;
                    }
                }
                if(sysinfo.disk){
                    /*
                    ** assume all were lost
                    */
                    for(i=0;i<adinfo.nelectrodes;i++){
                        electrode = adinfo.electrode+i;
                        adinfo.disk_lost += electrode->nspikes;
                    }
                }
		/* save event strings */
		if(sysinfo.fileopen && sysinfo.neventstrings > 0){
			SaveEventString();
		}
                if(sysinfo.disk){
                    if(sysinfo.mode == CONTINUOUS_MODE){
                        if(adinfo.error || adinfo.dmadone){
                             goto DMADONE;
                        }
                        /* save to disk */
                        SaveRawData();
                    } else {
                        for(i=0;i<adinfo.nelectrodes;i++){
                            if(adinfo.error || adinfo.dmadone){
                                goto DMADONE;
                            }
                            /* save to disk */
                            SaveSpikeData(i);
                        }
                    }
                }
			for(q=0;q<sysinfo.nseq;q++){
				if(!sysinfo.seq[q].active) continue;
				if(sysinfo.currenttime > 
				sysinfo.seq[q].time+sysinfo.seq[q].interval){
					sysinfo.seq[q].time = sysinfo.currenttime;
					sysinfo.seq[q].func();
				}
				if (sysinfo.currenttime < sysinfo.seq[q].time) {
					sysinfo.seq[q].time = sysinfo.currenttime;
				} 
			}
                /* display the time and amount of free space on the disk if
                   the current time is over half a second behind the timestamp*/
                if (sysinfo.currenttime + 10000 < 
                    adinfo.timestamp[adinfo.process_buf]) {
                    DisplayCurrentTime();
                    if(sysinfo.disk){
                        DisplayDiskInfo();
                        if ((sysinfo.diskfree < 2500000) && (sysinfo.disk)) {
                            /* we are getting close to the end of the disk.
                               Shut the file and turn disk storage off */
                            sysinfo.disk = 0;
                            DisableADIRQ;
                            fclose(adinfo.fp);
                            EnableADIRQ;
                            strcpy(adinfo.fname, NO_FILE_STRING);
                            sysinfo.fileopen = 0;
                            sysinfo.filesize = 0;
                            DrawMenu();
                            sprintf(tmpstring, "Error: disk full");
                            ErrorMessage(tmpstring);
                        }
                    }
                } else
                if (sysinfo.tracker && (sysinfo.currenttime + 1000 < 
                    adinfo.timestamp[adinfo.process_buf])) {
                    DisplayCurrentTime();
				}

                /* display the contents of the buffer */
                if(sysinfo.graphics){
                    if(sysinfo.tracker){
                        while(!tracker.done){
                        if(adinfo.error || adinfo.dmadone){
                            goto DMADONE;
                        }
                        }
                        DisplayTracker();
                    }
                    switch(sysinfo.mode){
                    case CONTINUOUS_MODE:
                        if(adinfo.error || adinfo.dmadone){
                            goto DMADONE;
                        }
                        if (!sysinfo.helpon) 
	                        DisplayContinuousData();
                        break;
                    case SPIKE_MODE:
                        for(i=0;i<adinfo.nelectrodes;i++){
                            if(adinfo.error || adinfo.dmadone){
                                goto DMADONE;
                            }
                            DisplayProjections(i);
                        }
                        for(i=0;i<adinfo.nelectrodes;i++){
                            if(adinfo.error || adinfo.dmadone){
                                goto DMADONE;
                            }
                            if (!sysinfo.helpon) 
                                DisplaySpikes(i);
                        }
                        break;
                    }
                }
            }
            if(sysinfo.showstatus){
                /* display status */
                if(adinfo.error || adinfo.dmadone){
                    goto DMADONE;
                }
                DisplayADStatus();
                //DrawElectrodeButtons();
            }
            /*
            ** made it all the way to the end without getting an a/d ISR
            ** interrupt so just wait it out
            */

	    if(sysinfo.tracker){
		while(!(adinfo.error || adinfo.dmadone)){
		    if(tracker.done){
			DisplayTracker();
		    }
		}
	    } else
	    while(sysinfo.acq && !(adinfo.error || adinfo.dmadone));
DMADONE:
            /* clear the error and dmadone status */
            adinfo.dmadone = 0;
            adinfo.error = 0;
#ifdef DAS-1800			
	    if((boardstatus & BIT4) == BIT4){
                sprintf(tmpstring,"ERROR detected status=%d",boardstatus);
#endif
#ifdef DT2821
            /* check for an a/d error */
            if((adcsr & BIT15) == BIT15){
                if(sysinfo.debug){
                    gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,
		    "ERROR detected ADCSR: %X SUPCSR:%X",
                    adcsr,supcsr);
                }
                sprintf(tmpstring,"ERROR detected ADCSR: %X SUPCSR:%X",
		    adcsr,supcsr);
#endif			
                ErrorMessage(tmpstring);
		EventString("ERROR");
                adinfo.error_count++;
                /* reinitialize the board and start acq again */
                /*
                outpw(SUPCSR,BIT0);
                */
		errcount=0;
		StartAcq();
		ErrorSet;
#ifdef DAS-1800					
		while(((boardstatus & BIT4) == BIT4) && errcount++ < 20){
		    StopAcq();
		    StartAcq();
		    ErrorSet;
		}	
					
#endif					
#ifdef DT2821			
            while(((adcsr & BIT15) == BIT15) && errcount < 20){
		// wait for the multiplexor to get back in sync 
		// following the error
                // skip a buffer
                StopAcq();
                // skip a buffer
                StartAcq();
                StopAcq();
                // skip a buffer
                StartAcq();
                StopAcq();
                // skip a buffer
                StartAcq();
            	ErrorSet;
                errcount++;
	    }
	    StopAcq();
	    StartAcq();
#endif
	    /* SystemExit(6); */
	    if(sysinfo.debug){
		gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"acquiring to Fbuf %d\n",adinfo.next_buf);
                }
#ifdef DT2821			
            }
#endif
#ifdef DAS-1800			
            }
#endif
            else {
                /* the 2821 has now switched buffers. Data can be safely
                ** removed from the last buffer filled
                */
                adinfo.count++;
            }
         } else {
            /* save event strings */
            if(sysinfo.fileopen && sysinfo.neventstrings > 0){
				if(sysinfo.tracker)
					DisableTrackerIRQ;
                SaveEventString();
				if(sysinfo.tracker)
					EnableTrackerIRQ;
            }
            if(sysinfo.tracker && tracker.done){
                if(sysinfo.graphics){
                    DisplayTracker();
                }
                if(sysinfo.disk){
					DisableADIRQ;
                    WriteTrackerBuffer();
					EnableADIRQ;
                }
            }
            // update the system time
            sysinfo.currenttime = ReadTS();
            // update every second
            if(sysinfo.currenttime > sysinfo.oldtime+300L){
                sysinfo.oldtime = sysinfo.currenttime;
                DisplayCurrentTime();
			}
            if(sysinfo.currenttime > sysinfo.olddisktime+10000L){
                sysinfo.olddisktime = sysinfo.currenttime;
				if(sysinfo.tracker_enabled){
					DrawSessionString();
				}
				if(sysinfo.disk){
					DisplayDiskInfo();
					if ((sysinfo.diskfree < 2500000) && (sysinfo.disk)) {
						/* we are getting close to the end of the disk.
						   Shut the file and turn disk storage off */
						sysinfo.disk = 0;
						DisableADIRQ;
						fclose(adinfo.fp);
						EnableADIRQ;
						strcpy(adinfo.fname, NO_FILE_STRING);
						sysinfo.fileopen = 0;
						sysinfo.filesize = 0;
						DrawMenu();
						sprintf(tmpstring, "Error: disk full");
						ErrorMessage(tmpstring);
					}
				}
            }
			/*
			if((sysinfo.command_mode == MASTER) &&
            (sysinfo.currenttime > sysinfo.oldsynctime+600000L)){
                sysinfo.oldsynctime = sysinfo.currenttime;
				ClkMasterProcessCommand(TESTSYNCH);
			}
			*/
            if (sysinfo.currenttime < sysinfo.oldtime) {
                sysinfo.oldtime = sysinfo.currenttime;
            } 
            if (sysinfo.currenttime < sysinfo.olddisktime) {
                sysinfo.olddisktime = sysinfo.currenttime;
            } 
            if (sysinfo.currenttime < sysinfo.oldsynctime) {
                sysinfo.oldsynctime = sysinfo.currenttime;
            } 

			for(q=0;q<sysinfo.nseq;q++){
				if(!sysinfo.seq[q].active) continue;
				if(sysinfo.currenttime > 
				sysinfo.seq[q].time+sysinfo.seq[q].interval){
					sysinfo.seq[q].time = sysinfo.currenttime;
					sysinfo.seq[q].func();
				}
				if (sysinfo.currenttime < sysinfo.seq[q].time) {
					sysinfo.seq[q].time = sysinfo.currenttime;
				} 
			}
        }
        //CheckKeyboard();
    }
    if(sysinfo.acq){
        StopAcq();
    }
	SystemExit(0,"AD clean exit");
}
