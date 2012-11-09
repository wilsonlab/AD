#include "adext.h"

void BoardRestore(void)
{
    int i;

    // stop the current a/d operation
    outpw(SUPCSR,0x0);
    /* reinitialize the board  */
    outpw(SUPCSR,BIT0);
    // enable interrupts
	enable();
}

void BoardSetup(void)
{
int    i,j;

//DT2821 initialization
	disable();
	/* note that this will reset the PIO port as well */
	outpw(SUPCSR,BIT0);            /* initialize the board */
	enable();
}

int SetBoardGains(void)
{
int    i;
    /* enable writes to the channel/gain list and load number of channels */
    outpw(CHANCSR,BIT15|((adinfo.nchannels-1)&0x000F));
    if(ErrorSet) return(TRUE);


    /* load the channel gain list with channels 0-7 */
    for(i=0;i<adinfo.nchannels;i++){
        /* bits 0-3 are the channel #, bits 4-5 are the gain */
        outpw(ADCSR,(((adinfo.channel[i].adgain<<4)&0x0030) | (i&0x000F)));
        if(ErrorSet) return(TRUE);
    }

    /* disable writes to the channel gain list by clearing bit 15 */
    outpw(CHANCSR, inpw(CHANCSR) & 0x7FFF);
    if(ErrorSet) return(TRUE);
    return(FALSE);
}

int    InitBoard( void )
{
	int    i;
	int    j;

    if(sysinfo.debug){
        gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"Init A/D board\n");
    }
    /* initialize the A/D buffer information: A/D initialize, buffer B, clear
    ** DMA done
    */
    outpw(SUPCSR,BIT6|BIT9|BIT13);
    if(ErrorSet) return(30);

    /* enable writes to the channel/gain list and load number of channels */
    if (sysinfo.debug) {
           gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"%d channels\n",adinfo.nchannels);
        }
    outpw(CHANCSR,BIT15|((adinfo.nchannels-1)));
    if(ErrorSet) return(31);


    /* load the channel gain list with channels 0-7 */
    for(i=0;i<adinfo.nchannels;i++){
        /* bits 0-3 are the channel #, bits 4-5 are the gain */
        outpw(ADCSR,(((adinfo.channel[i].adgain<<4)&0x0030) | (i&0x000F)));
        if(ErrorSet) return(32);
    }

    /* disable writes to the channel gain list by clearing bit 15 */
    outpw(CHANCSR, inpw(CHANCSR) & 0x7FFF);
    if(ErrorSet) return(33);

    /* interrupt on a/d done, enable the A/D clock */
    outpw(ADCSR,BIT6 | BIT9);
    if(ErrorSet) return(34);

    /* preload the multiplexer, a/d clocked DMA, clear DMA done, interrupt on error */
    outpw(SUPCSR,BIT4|BIT10|BIT13|BIT14);
    if(ErrorSet) return(35);

    if(sysinfo.debug){
        gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"Waiting for MUX busy bit to clear\n");
    }
    /* wait for the multiplexer busy bit to clear indicating the data is ready
    ** to convert to an analog value
    */
        while((inpw(ADCSR) & BIT8) == BIT8);
    if(ErrorSet) return(36);

    /* issue software trigger, a/d clock */
    outpw(SUPCSR,BIT3|BIT10|BIT12);
    /* acquisition to buffer A has begun */
    if(ErrorSet) return(37);

    if(sysinfo.debug){
        gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"initialization complete\n");
    }

    return(0);
}

int SetRate(float rate, float *rate_set)
/* this function sets the pacer clock to the rate nearest that
** specified by rate. It returns the rate that the clock was
** set to in rate_set
*/
{
long    ticks;
WORD    prescale;
WORD    best_prescale;
long    best_ticks;
double    freq;
double    best_freq;
double    min_df;

    if ((rate<0.4) || (rate > 250000.0)) 
        return(FALSE);
    if (sysinfo.debug) {
           gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"using rate %g\n",rate);
        }

    best_freq = 0;
    // find the first valid prescale value
    for(prescale=0;prescale<16;prescale++){
        // prescale 0 is the same as 1
        if(prescale == 1) continue;
        // what is the counter value for this prescale
        ticks=(long) floor(4.0e6/(1L<<prescale)/rate+0.5);
        // the clock register is only 8 bits so cant go higher than 255
        if(ticks > 255 || ticks == 0) continue;
        // what is the frequency of this counter-prescale combination
        best_freq=4.0e6/((float)(1L<<prescale)*ticks);
        best_ticks = ticks;
        best_prescale = prescale;
        break;
    }
    if(best_freq == 0){
        gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"Invalid sampling rate: %g\n",rate);
		SystemExit(2,"Invalid sample rate");
    }
    min_df = fabs((double)(best_freq - rate));

    for(;prescale<16;prescale++){
        // what is the counter value for this prescale
        ticks=(long) floor(4.0e6/(1L<<prescale)/rate+0.5);
        // the clock register is only 8 bits so cant go higher than 255
        if(ticks > 255 || ticks == 0) continue;
        // what is the frequency of this counter-prescale combination
        freq=4.0e6/((float)(1L<<prescale)*ticks);
        // is this the closest
        if(fabs((double)(freq - rate)) < min_df){
            best_ticks = ticks;
            best_prescale = prescale;
            min_df = fabs((double)(freq - rate));
            best_freq = freq;
        }
    }
    //if(best_prescale == 1) best_prescale = 0;
    *rate_set=4.0e6/((float)(1L<<best_prescale)*best_ticks);
     setcolor(MaxColors - 1);
//     gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"found best freq at %g\n",best_freq);

    /* initialize the pacer clock for rate set */
    adinfo.clockreg=(WORD)((best_prescale<<8) | LO(~best_ticks)); /* create image of pacer reg */
    outpw(TMRCTR,adinfo.clockreg);
    sprintf(tmpstring,"RATE %g",*rate_set);
    EventString(tmpstring);
    if(ErrorSet) return(TRUE);

    return(TRUE);
}

