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
#define EnableClk2IRQ    outp(IRQMASTER+1,inp(IRQMASTER+1)&~clk2irqmask);
#define DisableClk2IRQ    outp(IRQMASTER+1,inp(IRQMASTER+1) | clk2irqmask);

#define CLKDATA2    CLKBASE2
#define CLKCTRL2    CLKBASE2+1
#define CLKDIN2    CLKBASE2+2
#define CLKDOUT2    CLKBASE2+3

#ifdef OLD
typedef enum  {INVALID,RESETCLK,STOPACQ,STARTACQ,DISKON,DISKOFF,FILECLOSE,
                    CLEARSCREEN,TESTSYNCH};

extern char far tmpstring[200];
#endif

extern int getnelectrodes(void); 

void _interrupt (*oldclk2irq)();
int     got_command2;
static int    command2;
static unsigned long command2_ts;
int    lost_commands2;
unsigned long ReadTS(void);
char *FormatTS(unsigned long,int val);
int    clk2_irq = 7;
int    clk2irqmask;
int    clk2irqvec;

void Clk2WriteCommand(int com)
{
    //fprintf(stderr,"Wrote command # %d at ts %s\n",
        //com,FormatTS(ReadTS()));
    // write the command byte to the output data port and raise the interrupt bit
    outp(CLKDOUT2,com | 0x0080);
    delay(1);
    // lower the interrupt line
    outp(CLKDOUT2,com & 0x007F);
}

void interrupt far Clk2GetCommand(void)
{
    disable();
    // indicate that a command was received
    if(got_command2){
        // still have a previous command to process
        lost_commands2++;
    }
    got_command2 = 1;
    command2 = inp(CLKDIN2) & 0x007F;
    command2_ts = ReadTS();

    // end of isr
    outp(IRQMASTER, IRQEOI);
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

void ClkResetBoard2(void)
{
    outp(CLKCTRL2,0x00FF);
}


// SLAVE SYSTEM ROUTINE
void Clk2SetupCommandBus(void)
{
    fprintf(stderr,"Set up SLAVE command bus 2 interrupts\n");
    switch(clk2_irq){
    case 5:
        clk2irqmask = IRQ5MASK;
        clk2irqvec = IRQ5VEC;
        break;
    case 7:
        clk2irqmask = IRQ7MASK;
        clk2irqvec = IRQ7VEC;
        break;
    }
    oldclk2irq = getvect(clk2irqvec);
    /*set up the interrupt service routine to handle dma buffer processing*/
    setvect(clk2irqvec, Clk2GetCommand);

    // lower then raise bit 7 of the Dout line to enable external interrupts
    outp(CLKDOUT2,0x0000);
    //outp(CLKDOUT2,0x0080);

    /* enable the IRQ which will be raised on error or dmadone */
    EnableClk2IRQ;
}

// SLAVE SYSTEM ROUTINE
void Clk2RestoreSystem(void)
{
     /* restore the interrupt service routine vector */
    setvect(clk2irqvec, oldclk2irq);
}

// SLAVE SYSTEM ROUTINE
void Clk2ProcessCommand(void)
{
    int i;
	unsigned long timestamp;
	
    sprintf(tmpstring,"Received command # %d at ts %s",
        command2,FormatTS(command2_ts,1));
    ErrorMessage(tmpstring);
    switch(command2){
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
		while((inp(CLKDIN2) & 0x0080) == 0);
		while(inp(CLKDIN2) & 0x0080);
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
#ifdef OLD	
        //DisplayClk(1);
		timestamp = ReadTS();
#else
		disable();
		/*
		** wait for the final reset command by polling bit 7
		*/
		while((inp(CLKDIN2) & 0x0080) == 0);
		while(inp(CLKDIN2) & 0x0080);
		timestamp = ReadTS();
		enable();
#endif		
		EventStringAndTime("SYNCH",timestamp);
        sprintf(tmpstring,"Synch %s",FormatTS(timestamp,1));
        ErrorMessage(tmpstring);
        break;
	default:
		sprintf(tmpstring3,"%d",command2);
		EventString(tmpstring3);
        break;
    }
    got_command2 = 0;
}

/*
** MASTER SYSTEM ROUTINE
** Master system execution of commands issued to slave systems
*/
void Clk2MasterProcessCommand(int com)
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
        Clk2WriteCommand(com);
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
        Clk2WriteCommand(INVALID);
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
        Clk2WriteCommand(com);
		EventString("MASTERSTOPACQ");
        break;
    case STARTACQ:
        //fprintf(stderr,"Start ACQ\n");
        PrepareStartAcq();
        UpdateAcqButton();
        StartAcq();
        Clk2WriteCommand(com);
		EventString("MASTERSTARTACQ");
        break;
    case DISKON:
        //fprintf(stderr,"Disk ON\n");
        DiskOn();
        UpdateDiskButton();
        Clk2WriteCommand(com);
		EventString("MASTERDISKON");
        break;
    case DISKOFF:
        //fprintf(stderr,"Disk OFF\n");
        DiskOff();
        UpdateDiskButton();
        Clk2WriteCommand(com);
		EventString("MASTERDISKOFF");
        break;
    case FILECLOSE:
        //fprintf(stderr,"File CLOSE\n");
        DiskOff();
        UpdateDiskButton();
        ADCloseFile();
        UpdateFileButton();
        UpdateAcqButton();
        Clk2WriteCommand(com);
        break;
    case CLEARSCREEN:
        //fprintf(stderr,"Clear SCREEN\n");
        Clk2WriteCommand(com);
	    for (i = 0; i < getnelectrodes(); i++) {
		    ClearProjectionBoxes(i);
			DrawProjectionBorders(i);
		}
        break;
    case TESTSYNCH:
#ifndef SOFTSYNC
        StopFout();
        Clk2WriteCommand(com);
		timestamp = ReadTS();
		EventStringAndTime("SYNCH",timestamp);
        sprintf(tmpstring,"Synch %s",FormatTS(timestamp,1));
        ErrorMessage(tmpstring);
        delay(500);
        StartFout();
#else
        Clk2WriteCommand(com);
		// wait for the slave systems to enter the polling loop
        delay(1000);
		// issue the final sync command
		disable();
        Clk2WriteCommand(INVALID);
		timestamp = ReadTS();
		enable();
		EventStringAndTime("SYNCH",timestamp);
        sprintf(tmpstring,"Synch %s",FormatTS(timestamp,1));
        ErrorMessage(tmpstring);
#endif		
        break;
	default:
		sprintf(tmpstring3,"%d",com);
		EventString(tmpstring3);
        break;
    }
    got_command2 = 0;
}

