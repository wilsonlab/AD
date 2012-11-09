#include "adext.h"

/* bit masks for the pio control word to configure ports; 0=output, 1=input*/
#define	PCMASK	0x09
#define	PCHIMASK	0x08
#define	PCLOMASK	0x01
#define	PBMASK	0x02
#define	PAMASK	0x10

/* Amplifier filter definitions */
#define HDAMP_LOWCUT_TENTHHZ	0
#define HDAMP_LOWCUT_1HZ	BIT0
#define HDAMP_LOWCUT_10HZ	BIT1
#define HDAMP_LOWCUT_100HZ	BIT2
#define HDAMP_LOWCUT_300HZ	BIT3
#define HDAMP_LOWCUT_600HZ	BIT4
#define HDAMP_LOWCUT_900HZ	(BIT3 | BIT4)

#define HDAMP_HICUT_50HZ	0
#define HDAMP_HICUT_100HZ	BIT5   /* 120 HZ ACTUALLY */
#define HDAMP_HICUT_200HZ	BIT8
#define HDAMP_HICUT_250HZ	BIT9
#define HDAMP_HICUT_275HZ	(BIT5  | BIT8)
#define HDAMP_HICUT_325HZ	(BIT5  | BIT9)
#define HDAMP_HICUT_400HZ	(BIT8 | BIT9)
#define HDAMP_HICUT_475HZ	(BIT5 | BIT8 | BIT9)
#define HDAMP_HICUT_3KHZ	BIT6
#define HDAMP_HICUT_6KHZ	BIT7
#define HDAMP_HICUT_9KHZ	(BIT6|BIT7)

void WritePIO(int val)
{
#ifdef DT2821
    /*
    ** output to the DIO port on the 2821
    */
    outpw(DIODAT,val);
#else
    /*
    ** output to the parallel port A and B
    ** note that this writes a full word with the low byte going to
    ** port A and the high byte going to port B
    */
    outpw(PORT_A,val);
#endif
}

int ReadPIO(void)
{
#ifdef DT2821
    return(inpw(DIODAT));
#else
    return(inp(PORT_C));
#endif	
}

int PIOStatus(void)
{
#ifdef DT2821
    return(inpw(DIODAT));
#else
    return(inpw(PORT_A));
#endif	
}

void WriteAmpControl(int val)
{
   if (sysinfo.debug) {
      gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"pio write 0x%X\n",val);
   }
   adinfo.piostate = val;
   WritePIO(val);
    /*
    ** confirm the value 
    */
    if(adinfo.piostate != PIOStatus()){
	ErrorMessage("Amp Control Failure");
    }
}

void InitDTPIO(void)
{
int	val;

   if (sysinfo.debug) {
      gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"Initializing DT PIO\n");
   }
#ifdef DT2821
	// setup the pio for output
	WritePIO(0xFFFF);
	val = inpw(DACSR);
	// set the low bits
	outpw(DACSR,val | BIT0 | BIT1);
#else
	/*
	** configure the parallel port outputs
	** set ports A, and B as outputs, C as input
	*/
//	outp(PP_CONTROL,0x80 | (PCMASK & ~(PAMASK | PBMASK))); 
//	outp(PP_CONTROL,0x89);
// all ports as output
	outp(PP_CONTROL,0x80);
#endif	
	// raise the control bits
	WriteAmpControl(0xF000);
}

/*
** lower and raise the DC equalization register
** output lower 12 bits to the amp
*/
void SetEqualizationReg( int regval )
{
	 int val;

	// check for valid state
	 if (regval > 0x00ff){
	 gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"invalid equalization value %d\n",regval);
	 return;
	 }

	 /* raise the control bits, preserve the data bits */
	 WriteAmpControl( 0xF000 | adinfo.piostate );

	// output the state
	 val = 0xF000 | regval;
	 WriteAmpControl(val);

	 /* lower the register load bit */
	 val &= (~BIT15 );
	 WriteAmpControl(val);

	 /* raise the register load bit to deselect DC eq */
	 val |= BIT15;
	 WriteAmpControl(val);
}

/*====================================================================*/
void LoadSeqCtrReg( int regval )
{
int val;

	 if ( (regval >15) || (regval <0) ) return;

	 /* raise the control lines */
	 WriteAmpControl( 0xF000 | adinfo.piostate );

	 /* or in the register value to set the data bus to */
	 val = 0xF000 | regval;
	 WriteAmpControl(val);

	 /* lower the seq ctr reg load bit */
	 val &= (~BIT12);
	 WriteAmpControl(val);

	 /* lower and raise the clock signal to pass the preset through on the 163*/
	 val &= (~BIT13);
	 WriteAmpControl(val);
	 val |= BIT13;
	 WriteAmpControl(val);

	 /* raise the seq ctr reg load bit */
	 val |= BIT12;
	 WriteAmpControl(val);
}
/*====================================================================*/
void IncSeqCtrReg( void )
/* this will lower and raise the Seq Ctr Reg Inc line.
 */
{
int val;

	 val = adinfo.piostate & (~BIT13);
	 WriteAmpControl(val);

	 val |= BIT13;
	 WriteAmpControl(val);

}
void LoadAmpReg( int regval )
/* lower and raise
** the control line to load the value into the register pointed to by
** the Sequence Register Counter (the 74163 chip).
*/
{
int val;
	 /* make sure that the load value is not outside of 0 -> 12 bits */
	 if ( (regval <0) || (regval > 0x0FFF) ) return;

	 /* raise all control lines with current 12 bit data */
	 val = adinfo.piostate | 0xf000;
	 WriteAmpControl(val);

	 /* output the state */
	 val = 0xF000 | regval;
	 WriteAmpControl(val);

	 /* lower the load reg control line */
	 val &= (~BIT14);
	 WriteAmpControl(val);

	 /* raise the load reg control line to lock the value in */
	 val |= BIT14;
	 WriteAmpControl(val);

	 return;
}

void SetAmpFilter( int channel, int filter)
{
#ifdef NEWAMP
	 if ( (channel <0) || (channel > NAMP_CHANNELS) ||
			(filter <0) || (filter> 0x03FF) ){
#else			
	 if ( (channel <0) || (channel > NAMP_CHANNELS) ||
			(filter <0) || (filter> 0x00FF) ){
#endif			
			gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"invalid filter %d\n",filter);
		return;
	 }

	 LoadSeqCtrReg(channel);
	 LoadAmpReg(filter);
	 adinfo.channel[channel].filter = filter;
}

void SetAmpChannelGain( int channel, int gain )
{
	 if ( (channel <0) || (channel > NAMP_CHANNELS) ||
			(gain <0) || (gain> 0x0FFF) ){
			gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"invalid gain %d\n",gain);
			return;
	 }

	 LoadSeqCtrReg(channel + 8);
	 LoadAmpReg(gain);
	 adinfo.channel[channel].ampgain = gain;
}

void Equalize(void)
{

	// close all of the equalization switches
	SetEqualizationReg(0x00ff);
	// wait for equalization
	sleep(1);
	// open them back up
	SetEqualizationReg(0);
}

void SetAmpControls(void)
{
int	i;
	// raise the control bits
	WriteAmpControl(0xF000 | adinfo.piostate);
	// set the equalization by lowering and raising bit 15
	WriteAmpControl(adinfo.piostate & ~BIT15);
	WriteAmpControl(adinfo.piostate | BIT15);
	SetEqualizationReg(0);

	for(i=0;i<NAMP_CHANNELS;i++){
	    SetAmpFilter(i,adinfo.channel[i].filter);
		SetAmpGain(i,GetActualGain(adinfo.channel[i].ampgain));
	}
}

int SetElectrodeAmpGains(int e_num,long gain)
{
int    i;
int	startch;
int	endch;

    startch = e_num*adinfo.nelect_chan;
    endch = startch + adinfo.nelect_chan;
    if(endch > NAMP_CHANNELS) return;
    for(i=startch;i<endch;i++){
        if(!SetAmpGain(i,gain)){
			return(0);
		}
    }
	return(1);
}

int SetAmpGains(long gain)
{
    int	i;

    for(i=0;i<NAMP_CHANNELS;i++){
        if (!SetAmpGain(i, gain)) 
			return 0;
    }
    return 1;
}

int SetAmpGain(int channel, long gain)
{
    int chan;
    if ((gain < 0) || (gain > 50000))
 		return 0;
	chan = channel % NAMP_CHANNELS;
    /* set the gain for an individual amp channel */
    adinfo.channel[chan].ampgain = GetAmpGain(gain);
    SetAmpChannelGain(chan, adinfo.channel[chan].ampgain);
    if (adinfo.nchannels >= NAMP_CHANNELS) {
        /* there are more than NAMP_CHANNELS channels, and the channels 0 and 8,
           1 and 9, and so on, will be linked */
        if (chan < channel)
			adinfo.channel[channel].ampgain = adinfo.channel[chan].ampgain;
		else if (chan + NAMP_CHANNELS < adinfo.nchannels)
		    /* if there is an channel NAMP_CHANNELS above the set channel */
			adinfo.channel[chan + NAMP_CHANNELS].ampgain = 
							adinfo.channel[chan].ampgain;
    }
  	return 1;
}

#ifdef NEWAMP
void CycleAmpLowFilter(void)
{
int	filter;
int	newfilter;
int	i;

filter = adinfo.channel[0].filter & 0x001F;
	if(filter == HDAMP_LOWCUT_TENTHHZ){	// 0.1
	newfilter = HDAMP_LOWCUT_1HZ;
	}
	if(filter == HDAMP_LOWCUT_1HZ){	// 1
	newfilter = HDAMP_LOWCUT_10HZ;
	}
	if(filter == HDAMP_LOWCUT_10HZ){	// 10
	newfilter = HDAMP_LOWCUT_100HZ;
	}
	if(filter == HDAMP_LOWCUT_100HZ){	  // 100
	newfilter = HDAMP_LOWCUT_300HZ;
	}
	if(filter == HDAMP_LOWCUT_300HZ){		// 300
	newfilter = HDAMP_LOWCUT_600HZ;
	}
	if(filter == HDAMP_LOWCUT_600HZ){		// 600
	newfilter = HDAMP_LOWCUT_900HZ;
	}
	if(filter == HDAMP_LOWCUT_900HZ){		// 900
	newfilter = HDAMP_LOWCUT_TENTHHZ;
	}
	for(i=0;i<NAMP_CHANNELS;i++){
	    adinfo.channel[i].filter &= 0xFFE0;
   	    adinfo.channel[i].filter |= newfilter;
	    SetAmpFilter(i,adinfo.channel[i].filter);
	}
}

void CycleAmpHighFilter(void)
{
int	filter;
int	newfilter;
int	i;

filter = adinfo.channel[0].filter & 0x03E0;
	if(filter == HDAMP_HICUT_50HZ){	// 50
	newfilter = HDAMP_HICUT_100HZ;
	}
	if(filter == HDAMP_HICUT_100HZ){	// 100
	newfilter = HDAMP_HICUT_200HZ;
	}
	if(filter == HDAMP_HICUT_200HZ){	// 200
	newfilter = HDAMP_HICUT_250HZ;
	}
	if(filter == HDAMP_HICUT_250HZ){	// 250
	newfilter = HDAMP_HICUT_275HZ; 
	}
	if(filter == HDAMP_HICUT_275HZ){	// 275
	newfilter = HDAMP_HICUT_325HZ;
	}
	if(filter == HDAMP_HICUT_325HZ) {	// 325
	newfilter = HDAMP_HICUT_400HZ;
	}
	if(filter == HDAMP_HICUT_400HZ){	// 400
	newfilter = HDAMP_HICUT_475HZ;
	}
	if(filter == HDAMP_HICUT_475HZ) {	// 475
	newfilter = HDAMP_HICUT_3KHZ;
	}
	if(filter == HDAMP_HICUT_3KHZ){	// 3K
	newfilter = HDAMP_HICUT_6KHZ;
	}
	if(filter == HDAMP_HICUT_6KHZ){	// 6K
	newfilter = HDAMP_HICUT_9KHZ;
	}
	if(filter == HDAMP_HICUT_9KHZ){	  // 9K
	newfilter = HDAMP_HICUT_50HZ;
	}
	for(i=0;i<NAMP_CHANNELS;i++){
	adinfo.channel[i].filter &= 0xFC1F;
	adinfo.channel[i].filter |= newfilter;
	SetAmpFilter(i,adinfo.channel[i].filter);
	}
}

void GetAmpFilters(char *l, char *h)
{
int   i, j;

//	 for( i=0; i<NAMP_CHANNELS; i++){
        i = 0;
		j = adinfo.channel[i].filter;
		if ( (j & (BIT0  | BIT1 | BIT2 | BIT3 | BIT4)) == 0 ){	  // 0.1 Hz
			strcpy(l, "0.1\0");
		}
		if (j & HDAMP_LOWCUT_1HZ){					// 1 HZ
			strcpy(l, "1\0");
		}
		if (j & HDAMP_LOWCUT_10HZ){					// 10 Hz
			strcpy(l, "10\0");
		}
		if (j & HDAMP_LOWCUT_100HZ){					// 100 Hz
			strcpy(l, "100\0");
		}
		if ((j & HDAMP_LOWCUT_900HZ) == HDAMP_LOWCUT_300HZ){		// 300Hz
			strcpy(l, "300\0");
		}
		if ((j & HDAMP_LOWCUT_900HZ) == HDAMP_LOWCUT_600HZ){		// 600Hz
			strcpy(l, "600\0");
		}
		if ((j & HDAMP_LOWCUT_900HZ) == HDAMP_LOWCUT_900HZ) {	// 900 Hz
			strcpy(l, "900\0");
		}

		/* now do the high freqs */
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ)) 
      					== HDAMP_HICUT_50HZ ){
			strcpy(h, "50\0");
      }
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_100HZ  ){
			strcpy(h, "100\0");
      }
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_200HZ  ){
			strcpy(h, "200\0");
      }
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_250HZ  ){
			strcpy(h, "250\0");
      }
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_275HZ  ){
			strcpy(h, "275\0");
      }
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_325HZ  ){
			strcpy(h, "325\0");
      }
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_400HZ  ){
			strcpy(h, "400\0");
      }
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_475HZ  ){
			strcpy(h, "475\0");
      }
      if ((j & HDAMP_HICUT_9KHZ) == HDAMP_HICUT_3KHZ ){
			strcpy(h, "3KHz\0");
      }
      if ((j & HDAMP_HICUT_9KHZ) == HDAMP_HICUT_6KHZ ){
			strcpy(h, "6KHz\0");
      }
      if ((j & HDAMP_HICUT_9KHZ) == HDAMP_HICUT_9KHZ ){
			strcpy(h, "9KHz\0");
      }
//	 }
}

#else
void CycleAmpLowFilter(void)
{
int	filter;
int	newfilter;
int	i;

	filter = adinfo.channel[0].filter & 0x001F;
	if(filter == 0x0000){	// 0.1
	newfilter = 0x0001;
	}
	if(filter == 0x0001){	// 1
	newfilter = 0x0002;
	}
	if(filter == 0x0002){	// 10
	newfilter = 0x0004;
	}
	if(filter == 0x0004){	  // 100
	newfilter = 0x0008;
	}
	if(filter == 0x0008){		// 300
	newfilter = 0x0010;
	}
	if(filter == 0x0010){		// 600
	newfilter = 0x0018;
	}
	if(filter == 0x0018){		// 900
	newfilter = 0x0000;
	}
	for(i=0;i<NAMP_CHANNELS;i++){
	    adinfo.channel[i].filter &= 0xFFE0;
   	    adinfo.channel[i].filter |= newfilter;
	    SetAmpFilter(i,adinfo.channel[i].filter);
	}
}

void CycleAmpHighFilter(void)
{
int	filter;
int	newfilter;
int	i;

filter = adinfo.channel[0].filter & 0x00E0;
	if(filter == 0x0000){	// 50
	newfilter = 0x0020;
	}
	if(filter == 0x0020){	// 100
	newfilter = 0x0040;
	}
	if(filter == 0x0040){	// 3K
	newfilter = 0x0080;
	}
	if(filter == 0x0080){	// 6K
	newfilter = 0x000C0;
	}
	if(filter == 0x00C0){	  // 9K
	newfilter = 0x0020;
	}
	for(i=0;i<NAMP_CHANNELS;i++){
	adinfo.channel[i].filter &= 0xFF1F;
	adinfo.channel[i].filter |= newfilter;
	SetAmpFilter(i,adinfo.channel[i].filter);
	}
}

void GetAmpFilters(char *l, char *h)
{
int   i, j;

//	 for( i=0; i<NAMP_CHANNELS; i++){
        i = 0;
		j = adinfo.channel[i].filter;
		if ( (j & (BIT0  | BIT1 | BIT2 | BIT3 | BIT4)) == 0 ){	  // 0.1 Hz
			strcpy(l, "0.1\0");
		}
		if (j & BIT0){					// 1 HZ
			strcpy(l, "1\0");
		}
		if (j & BIT1){					// 10 Hz
			strcpy(l, "10\0");
		}
		if (j & BIT2){					// 100 Hz
			strcpy(l, "100\0");
		}
		if (j &  BIT3){		// 300Hz
			strcpy(l, "300\0");
		}
		if (j & BIT4){		// 600 Hz
			strcpy(l, "600\0");
		}
		if ((j & (BIT3 | BIT4))== (BIT3 | BIT4)){	// 900 Hz
			strcpy(l, "900\0");
		}

		/* now do the high freqs */
		if ((j & (BIT5 | BIT6 | BIT7))== 0 ){
			strcpy(h, "50\0");
		}
		if (j & BIT5){
			strcpy(h, "100\0");
		}
		if (j & BIT6 ){
			strcpy(h, "3KHz\0");
		}
		if (j & BIT7 ){
			strcpy(h, "6KHz\0");
		}
		if ((j & (BIT6 | BIT7)) == (BIT6 | BIT7) ){
			strcpy(h, "9KHz\0");
		}
//	}		
}
#endif
