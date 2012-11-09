#include "adext.h"

void BoardRestore(void)
{
    int i;

    // stop the current a/d operation
    outp(STATUS,0x0);
	// reset FIFO
	outp(CONTROLA, 0x0);
}

void BoardSetup(void)
{
int    i,j;


	// reset FIFO
	outp(CONTROLA, 0x0);
	// set board for counter gating and enable FIFO
	outp(CONTROLA, BIT0 | BIT2);
	outp(CONTROLB, 0x0);
	// set the board to issue interrupt 15 and use DMA channels 5 and 6
	outp(CONTROLB, BIT0 | BIT2 | BIT3 | BIT4 | BIT5 ); 
	// set board for bipolar, single ended, internal clock
	outp(CONTROLC, BIT6 | BIT4 | BIT3 | BIT0);
}

int SetBoardGains(void)
{
int    i;
	disable();
	// stop acquisition (just being safe)
	outp(STATUS, 0x0);
	outp(STATUS, 0x0);
	// set the data select register to point to the QRAM
	outp(DATASELECT, 0x0001);
	// initialize QRAM starting address to the number of channels in the scan 
	// minus 1
	outp(QRAMSTART, (unsigned char)(adinfo.nchannels - 1));
	// write out the gains for all of the channels
	for (i = 0; i < adinfo.nchannels; i++) {
		outpw(BASE, (WORD)(((adinfo.channel[i].adgain << 8) &0x0300 | (i & 0x00FF)))); 
	}	
	// reintialize the QRAM starting address
	outp(QRAMSTART, (unsigned char)(adinfo.nchannels - 1));
	enable();

    return(FALSE);
}

int    InitBoard()
{
int    i;
int    j;

	//fprintf(stderr,"InitBoard\n");
    if(sysinfo.debug){
        gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"Init A/D board\n");
    }

//    setcolor(WHITE);
//    gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"starting acquistion\n");
	
	SetBoardGains();
	// setup the Keithley board 
	BoardSetup();

    // make sure that the data source is the AD Converter
	outp(DATASELECT, 0x0000);
	// start acquisition 
	outp(STATUS, BIT7); 
	return(FALSE);
}	
	

int SetRate(float rate, float *rate_set)
/* this function sets the pacer clock to the rate nearest that
** specified by rate. It returns the rate that the clock was
** set to in rate_set
*/
{
/* base period is 200 ns (5 MHz) */
/* counter */
/* to determine desired rate, compute 1/(200 ns * rate 1 * rate 2) */
/* It seems that neither counter will work with a value below 2, so counter 1
	is arbitrarily set to 2 and the calculation of rate is done with counter 2. 
	That sets the minimum rate to approximately 40 Hz. */
	int	counter1;
	int	counter2;

	if ((rate > 333000.0) || (rate < 40))
		return(FALSE);
	counter1 = 2;
	counter2 = 5000000 / ((float) counter1 * rate);
	*rate_set = 1.0 / ((float) counter1 * (float) counter2 * 200e-9);
	outp(CNTRCTRL, 0xb4);                   // Ctr 2, mode 2, lsb-msb
	outp(COUNTER2, (unsigned char) (counter2 % 256));	// lsb 
	outp(COUNTER2, (unsigned char) (counter2 / 256));	// msb
	outp(CNTRCTRL, 0x74);                   // Ctr 1, mode 2, lsb-msb
	outp(COUNTER1, (unsigned char) counter1);        // lsb
	outp(COUNTER1, 0);                  	// msb
	sprintf(tmpstring,"RATE %g",*rate_set);
	EventString(tmpstring);
	return(TRUE);
}

