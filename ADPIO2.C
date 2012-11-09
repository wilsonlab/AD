#include "adext.h"

/* bit masks for the pio control word to configure ports; 0=output, 1=input*/
#define	PCMASK	0x09
#define	PCHIMASK	0x08
#define	PCLOMASK	0x01
#define	PBMASK	0x02
#define	PAMASK	0x10

/*
** This code gives access to a second DIO24 card
*/
void WritePIO2(int val)
{
    /*
    ** output a byte to the parallel port B
    */
    outp(PORT2_B,val);
}

int ReadPIO2(void)
{
    return(inp(PORT2_A));
}

void SetupPIO2()
{
	// set ports A for input B for output
	outp(PP2_CONTROL,0x80 | (PAMASK & ~(PBMASK))); 
}

void RaiseBitPIO2(int val)
{
    /*
    ** raise bits on parallel port B
    */
    outp(PORT2_B,val | inp(PORT2_B));
}

void LowerBitPIO2(int val)
{
    /*
    ** lowe bits on parallel port B
    */
    outp(PORT2_B,inp(PORT2_B) & ~val);
}

