#define ADVERSION    "1.36b"
#define BORLAND
#define BORGRAPH
#define NOSMOUSE
#define _MSVC
#define ISR
//define Keithley Metrabyte DAS-1802 board or Data Translation DT2821 board
#define DAS-1800
//#define DT2821

//define NEWAMP or not
//#define NEWAMP

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>
#include <dos.h>
#include <math.h>
#include <alloc.h>
#include <malloc.h>
#ifndef BORLAND
#include <graph.h>
#endif
#ifdef BORLAND
#include <mem.h>
#include <bios.h>
#include <stdlib.h>
#include <stdarg.h>
#endif

#include <string.h>
#include <time.h>


#include "adstruct.h"

#ifdef BORGRAPH
#include <graphics.h>
#include "svgautil.h"
#include "svga16.h"
#include "twk16.h"
#include "twk256.h"
#include "svga256.h"
#include "svga32k.h"
#include "svga64k.h"
#include "svgatc.h"
#include "svgas3.h"
#else
#include "svgacc.h"
#endif
#define	TRUE	1
#define	FALSE	0
#define NO_FILE_STRING "NO FILE\0"
#ifdef MSVC
#define LIGHTGRAY	_LIGHTBLUE
#define DARKGRAY	_BLUE
#define BLACK		_BLACK
#define WHITE		_WHITE
#define getvect(x)		_dos_getvect(x)
#define setvect(x,y)		_dos_setvect(x,y)
#define enable			_enable
#define disable			_disable
#endif
#ifdef BORLAND
#define	_WHITE	WHITE
#define	_RED		RED
#define	_GREEN	GREEN
#define	_YELLOW	YELLOW
#define	_BLACK	BLACK
#define	_BLUE		BLUE
#define	MC_ON		YELLOW
#define	MC_OFF		BLUE
#define	MFORE		WHITE
#define	_kbhit()	_bios_keybrd(_KEYBRD_READY)
#define _getch()	getch()
#endif
#ifdef BORGRAPH
#define SET COPY_PUT
#endif
#define LO(x)	((x) & 0xFF)
#define	HI(x)	((x) >> 8)

#ifdef DT2821
#define ErrorSet	(((adcsr=inpw(ADCSR)) & BIT15) == BIT15)
#define DmaDone	(((supcsr=inpw(SUPCSR)) & BIT15) == BIT15)
#define DmaStatus	(supcsr=inpw(SUPCSR))
#endif

#ifdef DAS-1800
#define ErrorSet	(((boardstatus=inp(STATUS) & BIT4)) == BIT4)
#define DmaDone	0
#define DmaStatus	0
#endif

#define	BIT0	0x0001
#define	BIT1	0x0002
#define	BIT2	0x0004
#define	BIT3	0x0008
#define	BIT4	0x0010
#define	BIT5	0x0020
#define	BIT6	0x0040
#define	BIT7	0x0080
#define	BIT8	0x0100
#define	BIT9	0x0200
#define	BIT10	0x0400
#define	BIT11	0x0800
#define	BIT12	0x1000
#define	BIT13	0x2000
#define	BIT14	0x4000
#define	BIT15	0x8000

#define SEGSIZE	65536L
#define DMAPAGESIZE	131072L

/* Interrupt controller registers */
#define IRQMASTER  0x0020                /* master - lo lines */
#define IRQSLAVE   0x00A0                /* slave - high lines */
#define IRQEOI     0x20                  /* end of interrupt */
#define IRQCSR 0x00A1        		/* 8259 interrupt controller port */

/* IRQ 4 interrupt  */
#define IRQ4MASK  0x0010        /* IRQ 4 enable/disable mask */
#define IRQ4VEC	0x0C			/* interrupt vector for IRQ 4 */
/* IRQ 5 interrupt  */
#define IRQ5MASK  0x0020        /* IRQ 5 enable/disable mask */
#define IRQ5VEC	0x0D			/* interrupt vector for IRQ 5 */
/* IRQ 7 interrupt  */
#define IRQ7MASK  0x0080        /* IRQ 7 enable/disable mask ?*/
#define IRQ7VEC	0x0F			/* interrupt vector for IRQ 7 */
/* IRQ 10 interrupt  */
#define IRQ10MASK  0x0004        /* IRQ 10 enable/disable mask */
#define IRQ10VEC	0x72		/* interrupt vector for IRQ 10 */
/* IRQ 11 interrupt  */
#define IRQ11MASK  0x0008        /* IRQ 11 enable/disable mask */
#define IRQ11VEC	0x73		/* interrupt vector for IRQ 11 */
/* IRQ 15 interrupt  */
#define IRQ15MASK  0x0080        /* IRQ 15 enable/disable mask */
#define IRQ15VEC	0x77		/* interrupt vector for IRQ 15 */

#define EnableIRQ15	disable();outp(IRQCSR,(inp(IRQCSR) &(~IRQ15MASK)));enable()
#define DisableIRQ15	disable();outp(IRQCSR,(inp(IRQCSR) | IRQ15MASK));enable()

#define EnableADIRQ		disable();outp(IRQCSR,(inp(IRQCSR) &(~sysinfo.adirqmask))); enable()
#define DisableADIRQ	disable();outp(IRQCSR,(inp(IRQCSR) | sysinfo.adirqmask)); enable()
//#define EnableClkIRQ	outp(IRQMASTER+1,inp(IRQMASTER+1)&~sysinfo.clkirqmask);
//#define DisableClkIRQ	outp(IRQMASTER+1,inp(IRQMASTER+1) | sysinfo.clkirqmask);
#define EnableTrackIRQ	outp(IRQMASTER+1,inp(IRQMASTER+1)&~sysinfo.trackirqmask);
#define DisableTrackIRQ	outp(IRQMASTER+1,inp(IRQMASTER+1) | sysinfo.trackirqmask);
#define EnableTrackerIRQ	outp(IRQMASTER+1,inp(IRQMASTER+1)&~trackerirqmask);
#define DisableTrackerIRQ	outp(IRQMASTER+1,inp(IRQMASTER+1) | trackerirqmask);

#ifdef	DAS-1800
#define	BASE		0x0240
#define DATASELECT	BASE+0x0002
#define CONTROLA	BASE+0x0004
#define CONTROLB	BASE+0x0005
#define CONTROLC	BASE+0x0006
#define STATUS		BASE+0x0007
#define QRAMSTART	BASE+0x000A
#define COUNTER1	BASE+0x000D
#define COUNTER2	BASE+0x000E
#define	CNTRCTRL	BASE+0x000F
#endif

#ifdef DT2821
#define	BASE		0x0240
/* registers used by the DT2821 */
#define	ADCSR		BASE+0x0000
#define	CHANCSR		BASE+0x2
#define	ADDAT		BASE+0x0004
#define	DACSR		BASE+0x0006
#define	DADAT		BASE+0x0008
#define	DIODAT		BASE+0x000A
#define	SUPCSR		BASE+0x000C
#define	TMRCTR		BASE+0x000E
#endif

/* 8237-A DMA controller registers */
#define	PAGE_REG5 	0x8B
#define	OFFS_REG5 	0xC4
#define COUNT_REG5 	0xC6
#define	PAGE_REG6	0x89
#define OFFS_REG6   0xC8
#define COUNT_REG6	0xCA
#define	PAGE_REG7	0x8A
#define OFFS_REG7   0xCC
#define COUNT_REG7	0xCE
#define	CMD_REG		0xD0
#define	MASK_REG		0xD4
#define	MODE_REG		0xD6
#define	FF_REG		0xD8

#define DT2821_ID	0x255
#define DT31EZ_ID	0x030

/* values used to convert A/D output to floating point */
//#define	SCALE	0.004882812
//#define	OFFSET	10.0

#define RoundUp(x) (((x) > (long) (x)) ? (long) (x) + 1: (long) (x))
#define absv(x) (((x) > 0.0) ? (x) : -(x))
/* amplifier control values */
#define MAX_AMP_GAIN	50000
#define GetAmpGain(gain) ((long) (((float) gain * 4095) / (float) MAX_AMP_GAIN)) 
#define GetActualGain(gain) (RoundUp(((float) gain * (float) MAX_AMP_GAIN) / 4095.0))

#ifdef DT2821
/* returns the index of the buffer which has been completed */
#define FilledBuffer (!((inpw(SUPCSR) & BIT9)==BIT9))
/* returns the index of the buffer currently being filled */
#define CurrentBuffer ((inpw(SUPCSR) & BIT9) == BIT9)
#endif

#ifdef OLD
#define DefaultView	setview((int)0,(int)0,(int)799,(int)599)
#define ScopeView	setview(sysinfo.scopeview.x,sysinfo.scopeview.y,\
	sysinfo.scopeview.x2,sysinfo.scopeview.y2)
#define MenuView	setview((int)0,(int)500,(int)799,(int)600)
#else
#define DefaultView
#define ScopeView
#define MenuView
#endif

#define MAXVALUE		2048
/* the DAS-1800 boards return values between -2047 and 2048, so a value of 0 
   corresponds to 0 V. The DT2821 boards return values between 0 and 4096, 
   however, so the values need to be offset by 2048. */
#ifdef DAS-1800
#define ADOFFSET		0
#endif
#ifdef DT2821
#define ADOFFSET		2048
#endif

#define MAX_AD_VOLTAGE	10

#define SPIKE_WINDOW_WIDTH	98
#define SPIKE_WINDOW_HEIGHT	160
#define PROJECTION_WINDOW_WIDTH	134
#define SCOPE_WIDTH		790
#define CHANNEL_BUTTON_WIDTH	98
#define ELECTRODE_BUTTON_WIDTH	146
#define BUTTON_HEIGHT		20
#define TRACKER_MODE 		2
#define SPIKE_MODE 		1
#define CONTINUOUS_MODE		0

/* 100s of usec in 1 second; used for conversions to Hz, etc. */
#define 	SECOND		1e4
/* definitions for the A/D pacer clock functions */
#define  CLOCK_SPEED      4000000
#define  MAX_COUNTER_VALUE    255
#define  MIN_FREQUENCY   0.476831
#define  MIN_PACER_CLOCK   0x0f00
#define	FLT_MAX				1e20


#define ESC	0x1b			/* Define the escape key	*/
#define TRUE	1			/* Define some handy constants	*/
#define FALSE	0			/* Define some handy constants	*/
#define PI	3.14159 		/* Define a value for PI	*/
#define ON	1			/* Define some handy constants	*/
#define OFF	0			/* Define some handy constants	*/

#define NFONTS 11

#define NORMAL_MESSAGE    "(c)lear (o)verlay (^E)q (^G)ains (PgUp/Dn)adgain (S)ave/(L)oad config (ALT Fn/Fn) ES"

#define NETWORKVEC	0x60
/*
** support amp control via the DIO port of the 2821 or via
** a standard parallel port
*/
#define	PP_BASE	0x278
#define	PORT_A	PP_BASE				/* read/write */
#define	PORT_B	PP_BASE+1        /* read/write */
#define	PORT_C	PP_BASE+2        /* read/write */
#define	PP_CONTROL	PP_BASE+3     /* write only */

/*
** support amp control via the 2nd DIO port of the 2821 or via
** a standard parallel port
*/
#define	PIO2_BASE	0x280
#define	PORT2_A	PIO2_BASE				/* read/write */
#define	PORT2_B	PIO2_BASE+1        /* read/write */
#define	PORT2_C	PIO2_BASE+2        /* read/write */
#define	PP2_CONTROL	PIO2_BASE+3     /* write only */

/*
** base address for the ports on the CIO-CTR10
*/
#define CLKBASE    0x300
#define CLKBASE2 0x304

/*
** base address for the ports on the DIO24 used by the tracker
*/
#define	PIO_BASE	792
#define	PIOPORT_A	PIO_BASE				/* read/write */
#define	PIOPORT_B	PIO_BASE+1        /* read/write */
#define	PIOPORT_C	PIO_BASE+2        /* read/write */
#define	PIO_CONTROL	PIO_BASE+3     /* write only */

/*
** flag variables indicating experimental mode of time paradigm
*/ 
#define PRETRAINING 1
#define SHAPING     2
#define TESTING     3 

/*
** stimulation protocols
*/
#define TIME       0
#define AREA       1
#define PROGRAMMED 2
#define RANDOM     3

/*
** stimulation modes
*/
#define NONE    0
#define TEST    1
#define CONTROL 2
#define BOTH    3

