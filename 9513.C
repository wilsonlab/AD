/*
** library and test routines for accessing the 9513a on the
** Computer Boards CIO-CTR5/10
**
** Copyright 1995, Matthew Wilson
** Massachusetts Institute of Technology
** All rights reserved
** Initial version written February 1995
** Updated April 1995
*/

//#define STANDALONE

/*
** configure for 8 bit bus. F1 is 1MHz so use F3 for 100 usec
** timing.
** default io address is 0x300
** default irq is 5
*/
#include <stdio.h>
#include <math.h>
#include <conio.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "adext.h"

#ifdef OLD
#define IRQ5VEC    0x0D
#define IRQ5MASK    0x20
#define IRQ7VEC    0x0F
#define IRQ7MASK    0x80
/* Interrupt controller registers */
#define IRQMASTER  0x0020                /* master - lo lines */
#define IRQSLAVE   0x00A0                /* slave - high lines */
#define IRQEOI     0x20                  /* end of interrupt */

#define IRQCSR 0x00A1                /* 8259 interrupt controller port */
//#define EnableIRQ5    outp(IRQMASTER+1,inp(IRQMASTER+1)&0xdf);
//#define DisableIRQ5    outp(IRQMASTER+1,inp(IRQMASTER+1) | 0x20);
#endif
#define EnableClkIRQ    outp(IRQMASTER+1,inp(IRQMASTER+1)&~clkirqmask);
#define DisableClkIRQ    outp(IRQMASTER+1,inp(IRQMASTER+1) | clkirqmask);

#define CLKDATA    CLKBASE
#define CLKDATA2    CLKBASE2
#define CLKCTRL    CLKBASE+1
#define CLKCTRL2    CLKBASE2+1
#define CLKDIN    CLKBASE+2
#define CLKDIN2    CLKBASE2+2
#define CLKDOUT    CLKBASE+3
#define CLKDOUT2    CLKBASE2+3

#ifdef OLD
typedef enum  {INVALID,RESETCLK,STOPACQ,STARTACQ,DISKON,DISKOFF,FILECLOSE,
                    CLEARSCREEN,TESTSYNCH};

extern char far tmpstring[200];
#endif

extern int getnelectrodes(void); 

void _interrupt (*oldclkirq)();
int     got_command;
int    command;
unsigned long command_ts;
int    lost_commands;
unsigned long ReadTS(void);
char *FormatTS(unsigned long,int val);
int    clk_irq = 5;
int    clkirqmask;
int    clkirqvec;

#ifdef STANDALONE
// empty hooks
void StartAcq(void)
{
}

void TestAndStopAcq(void)
{
}
#endif

void IncIRQMask(int val)
{
    clkirqmask += val;
    sprintf(tmpstring,"irqmask %0X",clkirqmask);
    ErrorMessage(tmpstring);
    EnableClkIRQ;

}

void ClkWriteCommand(int com)
{
    //fprintf(stderr,"Wrote command # %d at ts %s\n",
        //com,FormatTS(ReadTS()));
    // write the command byte to the output data port and raise the interrupt bit
    outp(CLKDOUT,com | 0x0080);
    delay(1);
    // lower the interrupt line
    outp(CLKDOUT,com & 0x007F);
}

void interrupt far ClkGetCommand(void)
{
    disable();
    // indicate that a command was received
    if(got_command){
        // still have a previous command to process
        lost_commands++;
    }
    got_command = 1;
    command = inp(CLKDIN) & 0x007F;
    command_ts = ReadTS();

    // end of isr
    outp(IRQMASTER, IRQEOI);
    enable();
}

/*
** Initialize the 9513 for counting using the external clock source
** from the master system. Use counters 1 and 2 for the 32 bit
** timestamp. Counter 1 contains the low order bytes and counts the external
** clock rising edges. Counter 2 counts the carry bits from counter 1
*/
void ClkSetupMasterSlave(void)
{
    disable();
    // point to master mode register
    outp(CLKCTRL,0x0017);
    // 10KHz F3 for Fout, comparators disabled, TOD disabled
    outp(CLKDATA,0x00D0);      // low byte
    // high byte next
    // BCD scale, disable auto increment, 8 bit bus, Fout off, Fout divide by 1
    outp(CLKDATA,0x00D1);    // high byte

    // setup counter 1 to count Fout edges from SRC1
    // point to mode register of counter 1
    outp(CLKCTRL,0x0001);
    // disable special gate, reload from load, count repetitively, count binary
    // count up, active high TC output
    outp(CLKDATA,0x0029);    // low byte
	/*
	** Change to allow use of internal clock for Master/Slave
	*/
    // no gating, count on rising edge, SRC1 for source
    outp(CLKDATA,0x0001); // high byte
    // setup counter 2 to count TC of counter 1
    // point to mode register of counter 2
    outp(CLKCTRL,0x0002);
    // disable special gate, reload from load, count repetitively, count binary
    // count up, active high TC output
    outp(CLKDATA,0x0029);    // low byte
    // no gating, count on rising edge, TC counter 1 for source
    outp(CLKDATA,0x0000); // high byte
    enable();
}


void ClkSetupSingle(void)
{
fprintf(stderr,"Single\n");
    disable();
    // point to master mode register
    outp(CLKCTRL,0x0017);
    // 10KHz F3 for Fout, comparators disabled, TOD disabled
    outp(CLKDATA,0x00D0);      // low byte
    // high byte next
    // BCD scale, disable auto increment, 8 bit bus, Fout off, Fout divide by 1
    outp(CLKDATA,0x00D1);    // high byte

    // setup counter 1 to count F3 edges 
    // point to mode register of counter 1
    outp(CLKCTRL,0x0001);
    // disable special gate, reload from load, count repetitively, count binary
    // count up, active high TC output
    outp(CLKDATA,0x0029);    // low byte
    // no gating, count on rising edge, F3 for source
    outp(CLKDATA,0x000D); // high byte

    // setup counter 2 to count TC of counter 1
    // point to mode register of counter 2
    outp(CLKCTRL,0x0002);
    // disable special gate, reload from load, count repetitively, count binary
    // count up, active high TC output
    outp(CLKDATA,0x0029);    // low byte
    // no gating, count on rising edge, TC counter 1 for source
    outp(CLKDATA,0x0000); // high byte
    enable();
}

void ClkSetupStim(void)
{
    disable();
    // point to master mode register
    outp(CLKCTRL2,0x0017);
    // 1KHz F4 for Fout, comparators disabled, TOD disabled
    outp(CLKDATA2,0x00E0);      // low byte
    // high byte next
    // BCD scale, disable auto increment, 8 bit bus, Fout off, Fout divide by 1
    outp(CLKDATA2,0x00D1);    // high byte

    // set up counter 5 to output pulses in counter mode K
    // point to mode register of counter 5
    outp(CLKCTRL2,0x0005);
    // disable special gate, reload from load or hold, count repetetively, 
    // count binary, count down, TC toggled output
    outp(CLKDATA2,0x0062);    // low byte
    // gate on counter 5, count on rising edge, F2 for source
    outp(CLKDATA2,0x008C); // high byte 

    // set up counter 4 to output pulses in counter mode K
    // point to mode register of counter 4
    outp(CLKCTRL2,0x0004);
    // disable special gate, reload from load or hold, count repetetively, 
    // count binary, count down, TC toggled output
    outp(CLKDATA2,0x0062);    // low byte
    // gate on counter 4, count on rising edge, F2 for source
    outp(CLKDATA2,0x008C); // high byte 

    // set up counter 2 to output pulses in counter mode K
    // point to mode register of counter 2
    outp(CLKCTRL2,0x0002);
    // disable special gate, reload from load or hold, count repetetively, 
    // count binary, count down, TC toggled output
    outp(CLKDATA2,0x0062);    // low byte
    // gate on counter 2, count on rising edge, F2 for source
    outp(CLKDATA2,0x008C); // high byte 

    // set up counter 3 to be in mode A
    // point to mode register of counter 3
    outp(CLKCTRL2,0x0003);
    // enable special gate, reload from load, count once, 
    // count binary, count down, TC toggled output
    outp(CLKDATA2,0x0042);    // stim once
    // no gating, count on F2
    outp(CLKDATA2,0x000C); // high byte

	 
	 enable();
}

void SetSingleStimParm(int pulselen)
{
int	pulsewidth;
int	interpulse;

    disable();
    // point to load register of counter 4
    /* this is the counter that determines the total length of the biphasic 
       stimulation */
    outp(CLKCTRL2,0x000C);
    
    outp(CLKDATA2,pulselen + 1); // low byte
    outp(CLKDATA2,0x0000); // high byte

    // point to hold register of counter 4
    outp(CLKCTRL2,0x0014);
    // set it to more than the requested duration 
    outp(CLKDATA2,pulselen + 1);    // low byte
    outp(CLKDATA2,0x0000); // high byte

    // point to load register of counter 5
    outp(CLKCTRL2,0x000D);

    // set interpulse interval
    outp(CLKDATA2, pulselen + 1);    // low byte
    outp(CLKDATA2, 0x0000); // high byte

    // point to the hold register for counter 5
    outp(CLKCTRL2,0x0015);
    // set it to the pulse length 
    outp(CLKDATA2, pulselen + 1);    // low byte
    outp(CLKDATA2,0x0000); // high byte
    // load them */

    outp(CLKCTRL2,0x0058);
    enable();
}

void SetStimParm(int pulselen)
{
int	pulsewidth;
int	interpulse;
int   low, high;

    disable();
	pulsewidth = pulselen/2;
	interpulse = sysinfo.stim.interpulse;
	  
    // point to load register of counter 3
    /* this is the counter that determines the interburst interval */ 
	 outp(CLKCTRL2,0x000B);
    high = (int)(sysinfo.stim.delay/256);
	 low  = sysinfo.stim.delay - 256*high;
	 outp(CLKDATA2, low);  // low byte
	 outp(CLKDATA2, high); // high byte
    // point to hold register of counter 3
    outp(CLKCTRL2,0x0013);
	 high = (int)(sysinfo.stim.number*(pulselen+interpulse)/256);
	 low  = sysinfo.stim.number*(pulselen+interpulse) - high*256;
    outp(CLKDATA2, low);   // low byte
    outp(CLKDATA2, high);  // high byte

    // point to load register of counter 4
    outp(CLKCTRL2,0x000C);
    // set interpulse interval
	 high = (int)(interpulse/256);
	 low  = interpulse - high*256;
    outp(CLKDATA2, low);   // low byte
    outp(CLKDATA2, high);  // high byte
    // point to the hold register for counter 4
    outp(CLKCTRL2,0x0014);
    // set it to the pulse length 
	 high = (int)(pulselen/256);
	 low  = pulselen - high*256;
    outp(CLKDATA2, low);   // low byte
    outp(CLKDATA2, high);  // high byte
    
    // point to load register of counter 5
    outp(CLKCTRL2,0x000D);
    // set interpulse interval
	 high = (int)(interpulse/256);
	 low  = interpulse - high*256;
    outp(CLKDATA2, low);   // low byte
    outp(CLKDATA2, high);  // high byte
    // point to the hold register for counter 5
    outp(CLKCTRL2,0x0015);
    // set it to the pulse length 
	 high = (int)(pulselen/256);
	 low  = pulselen - high*256;
    outp(CLKDATA2, low);   // low byte
    outp(CLKDATA2, high);  // high byte

    // point to load register of counter 2
	 // This is the pulse that ensures biphasic stimulation.
    outp(CLKCTRL2,0x000A);
    // set interpulse interval
	 high = (int)((interpulse+pulsewidth)/256);
	 low  = interpulse+pulsewidth - high*256;
    outp(CLKDATA2, low);   // low byte
    outp(CLKDATA2, high);  // high byte
    // point to the hold register for counter 2
    outp(CLKCTRL2,0x0012);
	// set for biphasic pulse (1/2 the pulse length)
	 high = (int)(pulsewidth/256);
	 low  = pulsewidth - high*256;
    outp(CLKDATA2, low);   // low byte
    outp(CLKDATA2, high);  // high byte

	 // load them all */
    outp(CLKCTRL2,0x005E);
    enable();
}

void ArmStimClocks(void)
{
    // arm counters 2,3,4&5, depending on stim mode
	 switch (sysinfo.stim.mode) {
	 case NONE:    
    outp(CLKCTRL2,0x003E);
	 break;
	 case TEST: //test only (2,3&4)
    outp(CLKCTRL2,0x002E);
	 break;
	 case CONTROL:  //control only (2,3&5)
    outp(CLKCTRL2,0x0036);
	 break;
	 case BOTH:  //both (2,3,4&5)
    outp(CLKCTRL2,0x003E);
	 break;
	 }
	 enable();
}

void DisarmStimClocks(void)
{
    // disarm counters 2,3,4&5
    outp(CLKCTRL2,0x00DE);
}

void ResetStimOutput(void)
{
    // reset outputs on 2,3,4&5 (sets TC toggle to low)
    outp(CLKCTRL2,0x00E2);
    outp(CLKCTRL2,0x00E3);
    outp(CLKCTRL2,0x00E4);
    outp(CLKCTRL2,0x00E5);
}

void Stimulate(void) 
{
char s[79];
char *stimmode[] = {"None", "Test only", "Control only", 
"Test & Control", "Programmed"};

	if (sysinfo.seq[STIMSEQON].active) {
//	if (sysinfo.stim.on) {
	  ResetStimOutput();
	  DisarmStimClocks();
	  if (sysinfo.stim.number == 1)
	  sprintf(s,
	  "STIM: 1 pulse, %.2f sec delay. Tetrodes:%s ",
	  (float)sysinfo.stim.interpulse/100, (float)sysinfo.stim.delay/100000, 
	  stimmode[sysinfo.stim.mode]);
	  else sprintf(s,
	  "STIM: %d pulses, %.1f msec intrvl, %.2f sec delay. Tetrodes:%s ",
	  sysinfo.stim.number, (float)sysinfo.stim.interpulse/100, 
	  	(float)sysinfo.stim.delay/100000, stimmode[sysinfo.stim.mode]);
	  ArmStimClocks();
	  EventString(s);
	  sysinfo.currenttime = ReadTS();
	  //printf(".");
	 }
	 else {
     ResetStimOutput();
	  DisarmStimClocks();
	  EventString("STIMULATION OFF");
	  }
}

void ClkResetClock(void)
{
    disable();
    // point to load register of counter 1
    outp(CLKCTRL,0x0009);
    // set it to zero
    outp(CLKDATA,0x0000);    // low byte
    outp(CLKDATA,0x0000); // high byte
    // point to load register of counter 2
    outp(CLKCTRL,0x000A);
    // set it to zero
    outp(CLKDATA,0x0000);    // low byte
    outp(CLKDATA,0x0000); // high byte
    // load them
    outp(CLKCTRL,0x0043);
    enable();
}

/*
** MASTER SYSTEM ROUTINE
** when the systems are configured in synchronous mode, activating
** Fout will start counting on all systems configured to count from
** SRC1. The master system will provide the Fout source and should
** have a loopback from Fout to SRC1 as well to insure coherence
** between systems
*/
void StartFout(void)
{
    // gate on Fout
    outp(CLKCTRL,0x00E6);
}

/*
** MASTER SYSTEM ROUTINE
** in synchronous mode this will stop counting on all systems
*/
void StopFout(void)
{
    // gate off Fout
    outp(CLKCTRL,0x00EE);
}

void ArmClock(void)
{
    // arm counter 1&2
    outp(CLKCTRL,0x0023);
}

void DisarmClock(void)
{
    // disarm counter 1&2
    outp(CLKCTRL,0x00C3);
	disable();
}

void ClkResetBoard(void)
{
    outp(CLKCTRL,0x00FF);
}

unsigned long pts = 0;

unsigned long ReadTS(void)
{
unsigned    long low1;
unsigned    long high1;
unsigned    long low2;
unsigned    long high2;
unsigned    long ts;

    disable();
    // save counter 1&2 to hold register
    outp(CLKCTRL,0x00A3);
    // point to hold register for counter 1
    outp(CLKCTRL,0x0011);
    // read the contents
    low1 = inp(CLKDATA);            // low byte
    high1 = inp(CLKDATA);   // high byte
    if(low1 == 0 && high1 == 0){
        // missed the carry bit so reload
        outp(CLKCTRL,0x00A3);
    }
    // point to hold register for counter 2
    outp(CLKCTRL,0x0012);
    // read the contents
    low2 = inp(CLKDATA);            // low byte
    high2 = inp(CLKDATA);   // high byte
    enable();
    ts =   low1 | (high1<<8) | (low2<<16) | (high2<<24);
        pts = ts;
    return(ts);
}

char timestring[80];
char *FormatTS(unsigned long ts,int val)
{
    if(val > 0){
        sprintf(timestring,"%1lu:%02lu:%02lu %04lu",
        ts/36000000L,(ts/600000L)%60,(ts/10000L)%60,ts%10000L);
    } else {
        sprintf(timestring,"%1lu:%02lu:%02lu",
        ts/36000000L,(ts/600000L)%60,(ts/10000L)%60);
    }
    return(timestring);
}

// SLAVE SYSTEM ROUTINE
void ClkSetupCommandBus(void)
{
    fprintf(stderr,"Set up SLAVE command interrupts\n");
    switch(clk_irq){
    case 5:
        clkirqmask = IRQ5MASK;
        clkirqvec = IRQ5VEC;
        break;
    case 7:
        clkirqmask = IRQ7MASK;
        clkirqvec = IRQ7VEC;
        break;
    }
    oldclkirq = getvect(clkirqvec);
    /*set up the interrupt service routine to handle dma buffer processing*/
    setvect(clkirqvec, ClkGetCommand);

    // lower then raise bit 7 of the Dout line to enable external interrupts
    outp(CLKDOUT,0x0000);
    //outp(CLKDOUT,0x0080);

    /* enable the IRQ which will be raised on error or dmadone */
    EnableClkIRQ;
}

// SLAVE SYSTEM ROUTINE
void ClkRestoreSystem(void)
{
     /* restore the interrupt service routine vector */
    setvect(clkirqvec, oldclkirq);
}

// SLAVE SYSTEM ROUTINE
void ClkProcessCommand(void)
{
    int i;
	unsigned long timestamp;
	
    sprintf(tmpstring,"Received command # %d at ts %s",
        command,FormatTS(command_ts,1));
    ErrorMessage(tmpstring);
    switch(command){
    case RESETCLK:
    case 33:      // just a hack to temporarily deal with a bad cable
        // make sure acq is off
        TestAndStopAcq();
        DiskOff();
        ADCloseFile();
        DisplayCurrentTime();
        DisplayDiskInfo();
        UpdateAcqButton();
        UpdateDiskButton();
        UpdateFileButton();
#ifndef SOFTSYNC
        //fprintf(stderr,"Reset CLOCK\n");
        // set the counter to zero
        ClkResetClock();
        // arm it. Counting will not begin until Fout starts up
        //fprintf(stderr,"CLOCK ARMED\n");
        ArmClock();
#else
		disable();
		/*
		** wait for the final reset command by polling bit 7
		*/
		while((inp(CLKDIN) & 0x0080) == 0);
		while(inp(CLKDIN) & 0x0080);
        //fprintf(stderr,"Reset CLOCK\n");
        // set the counter to zero
        ClkResetClock();
        // arm it. Counting will not begin until Fout starts up
        //fprintf(stderr,"CLOCK ARMED\n");
        ArmClock();
		enable();
#endif		
        break;
    case STOPACQ:
        //fprintf(stderr,"Stop ACQ\n");
		EventString("MASTERSTOPACQ");
        TestAndStopAcq();
        UpdateAcqButton();
        break;
    case STARTACQ:
        //fprintf(stderr,"Start ACQ\n");
		EventString("MASTERSTARTACQ");
        PrepareStartAcq();
        UpdateAcqButton();
        StartAcq();
        break;
    case DISKON:
        //fprintf(stderr,"Disk ON\n");
		EventString("MASTERDISKON");
        DiskOn();
        UpdateDiskButton();
        break;
    case DISKOFF:
        //fprintf(stderr,"Disk OFF\n");
		EventString("MASTERDISKOFF");
        DiskOff();
        UpdateDiskButton();
        break;
    case FILECLOSE:
        //fprintf(stderr,"File CLOSE\n");
        DiskOff();
        UpdateDiskButton();
        ADCloseFile();
        UpdateFileButton();
        UpdateAcqButton();
        break;
    case CLEARSCREEN:
	    for (i = 0; i < getnelectrodes(); i++) {
		    ClearProjectionBoxes(i);
			DrawProjectionBorders(i);
		}
        //fprintf(stderr,"Clear SCREEN\n");
        break;
    case TESTSYNCH:
#ifndef SOFTSYNC	
        //DisplayClk(1);
		timestamp = ReadTS();
#else
		disable();
		/*
		** wait for the final reset command by polling bit 7
		*/
		while((inp(CLKDIN) & 0x0080) == 0);
		while(inp(CLKDIN) & 0x0080);
		timestamp = ReadTS();
		enable();
#endif		
		EventStringAndTime("SYNCH",timestamp);
        sprintf(tmpstring,"Synch %s",FormatTS(timestamp,1));
        ErrorMessage(tmpstring);
        break;
    }
    got_command = 0;
}

/*
** MASTER SYSTEM ROUTINE
** Master system execution of commands issued to slave systems
*/
void ClkMasterProcessCommand(int com)
{
    int i;
	unsigned long timestamp;
	
    sprintf(tmpstring,"Executing command # %d at ts %s\n",
        com,FormatTS(ReadTS(),1));
    ErrorMessage(tmpstring);
    switch(com){
    case RESETCLK:
        // make sure acq is off
        TestAndStopAcq();
        DiskOff();
        ADCloseFile();
        DisplayCurrentTime();
        DisplayDiskInfo();
        UpdateAcqButton();
        UpdateDiskButton();
        UpdateFileButton();
        //fprintf(stderr,"Reset CLOCK\n");
        // turn off the clock signal for all systems
        StopFout();
        // send the reset command to the slave systems
        ClkWriteCommand(com);
#ifndef SOFTSYNC
        // reset the local clock
        ClkResetClock();
        //fprintf(stderr,"Waiting for slave systems...\n");
        // wait for the slave systems to process the command
        delay(1500);
        // turn the clock signal back on. The counters must be
        // armed otherwise the synchronization will fail  
        // note that even though the counter is armed, the Fout
        // signal used as the source is off therefore counting
        // will not begin
        sprintf(tmpstring,"CLOCK ARMED, FOUT ACTIVE\n");
        ErrorMessage(tmpstring);
        ArmClock();
        // start up the counter source used for all systems
        StartFout();
#else
        //fprintf(stderr,"Waiting for slave systems...\n");
        // wait for the slave systems to process the command
        delay(1500);
		/*
		** assume that all slave systems are in tight polling
		** loop waiting for final reset command
		** note that the actual command sent is not going to be
		** relevant, only the raising of the interrupt line (bit 7)
		*/
		disable();
        // send the command which will trigger actual reset on the slave systems
        ClkWriteCommand(INVALID);
		/* 
		** the key to synchronization is that the ArmClock command
		** be issued simultaneously on all systems
		*/
        // reset the local clock
        ClkResetClock();
        ArmClock();
		enable();
#endif		
        break;
    case STOPACQ:
        //fprintf(stderr,"Stop ACQ\n");
        TestAndStopAcq();
        UpdateAcqButton();
        ClkWriteCommand(com);
		EventString("MASTERSTOPACQ");
        break;
    case STARTACQ:
        //fprintf(stderr,"Start ACQ\n");
        PrepareStartAcq();
        UpdateAcqButton();
        StartAcq();
        ClkWriteCommand(com);
		EventString("MASTERSTARTACQ");
        break;
    case DISKON:
        //fprintf(stderr,"Disk ON\n");
        DiskOn();
        UpdateDiskButton();
        ClkWriteCommand(com);
		EventString("MASTERDISKON");
        break;
    case DISKOFF:
        //fprintf(stderr,"Disk OFF\n");
        DiskOff();
        UpdateDiskButton();
        ClkWriteCommand(com);
		EventString("MASTERDISKOFF");
        break;
    case FILECLOSE:
        //fprintf(stderr,"File CLOSE\n");
        DiskOff();
        UpdateDiskButton();
        ADCloseFile();
        UpdateFileButton();
        UpdateAcqButton();
        ClkWriteCommand(com);
        break;
    case CLEARSCREEN:
        //fprintf(stderr,"Clear SCREEN\n");
        ClkWriteCommand(com);
	    for (i = 0; i < getnelectrodes(); i++) {
		    ClearProjectionBoxes(i);
			DrawProjectionBorders(i);
		}
        break;
    case TESTSYNCH:
#ifndef SOFTSYNC
        StopFout();
        ClkWriteCommand(com);
		timestamp = ReadTS();
		EventStringAndTime("SYNCH",timestamp);
        sprintf(tmpstring,"Synch %s",FormatTS(timestamp,1));
        ErrorMessage(tmpstring);
        delay(500);
        StartFout();
#else
        ClkWriteCommand(com);
		// wait for the slave systems to enter the polling loop
        delay(1000);
		// issue the final sync command
		disable();
        ClkWriteCommand(INVALID);
		timestamp = ReadTS();
		enable();
		EventStringAndTime("SYNCH",timestamp);
        sprintf(tmpstring,"Synch %s",FormatTS(timestamp,1));
        ErrorMessage(tmpstring);
#endif		
        break;
    }
    got_command = 0;
}

#ifdef STANDALONE

void ClkShowCommands(void)
{
    fprintf(stderr,"\nCommands available:\n");
    fprintf(stderr,"    h    show this help screen\n");
    fprintf(stderr,"    a/A    arm/disarm counter\n");
    fprintf(stderr,"    f/F    FOUT on/off\n");
    fprintf(stderr,"    w        write to command bus\n");
    fprintf(stderr,"    g        get command from bus\n");
    fprintf(stderr,"    r        reset clock\n");
    fprintf(stderr,"    m    Master system mode\n");
    fprintf(stderr,"    s/S    Slave/Single system mode\n");
    fprintf(stderr,"    Q        quit\n\n");
}

main(int argc,char** argv)
{
unsigned long ts;
char    c;
int rate;
char    tmpstring[80];
int        mode;
int        nxtarg;

    mode = SINGLE;
    nxtarg = 0;
    while(++nxtarg < argc){
        if(strcmp(argv[nxtarg],"-usage") == 0){
            fprintf(stderr,"usage: %s [-master][-slave][-single]\n",
            argv[0]);
            exit(0);
        } else
        if(strcmp(argv[nxtarg],"-master") == 0){
            mode = MASTER;
        } else
        if(strcmp(argv[nxtarg],"-slave") == 0){
            mode = SLAVE;
        } else
        if(strcmp(argv[nxtarg],"-single") == 0){
            mode = SINGLE;
        } else
        if(argv[nxtarg][0] == '-'){
            fprintf(stderr,"invalid option: %s\n",argv[nxtarg]);
            exit(0);
        }
    }
    fprintf(stderr,"\n9513 Timer Control Program\n");
    fprintf(stderr,"--------------------------\n");
    if(mode == SINGLE){
        fprintf(stderr,"SINGLE System Mode\n");
    } else 
    if(mode == SLAVE){
        fprintf(stderr,"SLAVE System Mode\n");
    } else 
    if(mode == MASTER){
        fprintf(stderr,"MASTER System Mode\n");
    } 
    ClkShowCommands();


    ClkResetBoard();
    // use the SingleSetup for single systems. For synchronized systems
    // use MasterSlave setup
    if(mode == MASTER || mode ==SLAVE){
        ClkSetupMasterSlave();
    } else {
        ClkSetupSingle();
    }
    // prepare the command bus interrupt routine. Only for slave systems
    // dont do this on master systems do to potential conflicts with
    // IRQ5 and the tracker card
    if(mode == SLAVE){
        ClkSetupCommandBus();
    }
    // set the clock to zero
    // note that this only resets the single system
    // to reset master/slave systems, Fout must be turned off and
    // each of the systems reset
    ClkResetClock();
    // arm the counters and begin counting immediately for single systems,
    // wait for Fout for master/slave systems
    ArmClock();
    if(mode == MASTER){
        // To synchronize counts for master/slave systems, 
        // start Fout after arming. On master/slave systems synch
        // must be established by issuing the resetclk command
        // from the master system. Starting Fout does not guarantee
        // synchronization, it simply allows the clock to start
        // This has no effect on single system setups
        StartFout();
    }
    rate = 100;

    while(1){
        delay(rate);
        ts = ReadTS();
        fprintf(stderr,"%20s\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b",
            FormatTS(ts,1));
        // check for command bus input. should only happen on slave systems
        if(got_command){
            ClkProcessCommand();
        }
        if(kbhit()){
            c = getch();
            switch(c){
            case 'w':
                if(mode == MASTER){
                    fprintf(stderr,"\nAvailable Commands:\n");
                    fprintf(stderr,"\t1-ResetClk, 2-StopAcq, 3-StartAcq\n");
                    fprintf(stderr,"\t4-DiskOn, 5-DiskOff, 6-CloseFile\n");
                    fprintf(stderr,"\t7-ClearScreen\n");
                    fprintf(stderr,"Enter command #: ");
                    gets(tmpstring);
                    ClkMasterProcessCommand(atoi(tmpstring));
                } else {
                    fprintf(stderr,
                    "Not a MASTER system. Cannot write to the command bus.\n");
                }
                break;
            case 'h':
                ClkShowCommands();
                break;
            case 'g':
                // only useful on slave systems
                ClkGetCommand();
                break;
            case 'r':
                ClkResetClock();
                pts=0;
                break;
            case 'f':
                fprintf(stderr,"\nFOUT ON\n");
                StartFout();
                break;
            case 'F':
                fprintf(stderr,"\nFOUT OFF\n");
                StopFout();
                break;
            case 'm':
                fprintf(stderr,"\nEntering MASTER System mode\n");
                ClkSetupMasterSlave();
                mode = MASTER;
                break;
            case 'S':
                fprintf(stderr,"\nEntering SINGLE System mode\n");
                ClkSetupSingle();
                mode = SINGLE;
                break;
            case 's':
                fprintf(stderr,"\nEntering SLAVE System mode\n");
                ClkSetupMasterSlave();
                mode = SLAVE;
                break;
            case 'A':
                fprintf(stderr,"\nCOUNTER DISARMED\n");
                DisarmClock();
                break;
            case 'a':
                fprintf(stderr,"\nCOUNTER ARMED\n");
                ArmClock();
                break;
            case 'Q':
                if(mode == SLAVE){
                    ClkRestoreSystem();
                }
                exit(0);
                break;
            case '=':
                rate += 10;
                break;
            case '-':
                rate -= 10;
                if(rate < 0) rate = 0;
            }
        }
    }
}
#endif STANDALONE
