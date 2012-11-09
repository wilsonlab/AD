#define	ISR	1

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <math.h>
#include <malloc.h>
#include <mem.h>
#include <bios.h>
#include <string.h>
#include <time.h>
#include "svgacc.h"


/* Interrupt controller registers */
#define IRQMASTER  0x0020                /* master - lo lines */
#define IRQSLAVE   0x00A0                /* slave - high lines */
#define IRQEOI     0x20                  /* end of interrupt */
#define	IRQ5VECT		0x0d
#define	IRQ5MASK		0x20
#define	IRQ7VECT		0x0f
#define	IRQ7MASK		0x80
#define IRQCSR 0x00A1        		/* 8259 interrupt controller port */
//#define EnableIRQ	outp(IRQMASTER+1,inp(IRQMASTER+1)&~IRQ5MASK);
//#define DisableIRQ	outp(IRQMASTER+1,inp(IRQMASTER+1) | IRQ5MASK);
#define EnableIRQ	outp(IRQMASTER+1,inp(IRQMASTER+1)&~irqmask);
#define DisableIRQ	outp(IRQMASTER+1,inp(IRQMASTER+1) | irqmask);

#define	PIO_BASE	792
#define	PORT_A	PIO_BASE				/* read/write */
#define	PORT_B	PIO_BASE+1        /* read/write */
#define	PORT_C	PIO_BASE+2        /* read/write */
#define	PIO_CONTROL	PIO_BASE+3     /* write only */

/* bit masks for the pio control word to configure ports; 0=output, 1=input*/
#define	PCLO_BIT	0x01
#define	PB_BIT	0x02
#define	PCHI_BIT	0x08
#define	PA_BIT	0x10

/*
** port C input bits
*/
/* bit 0 is the LSB of the horizontal coordinate */
#define 	PIO_HOR_LSB			0x01
/* bit 1 indicates which of 2 interlaced fields is being read */
#define	PIO_INTERLACE_FIELD	0x02
/* bit 2 is used for detecting memory overflow */
#define	PIO_MEM_OVERFLOW	0x04
/* bit 3 is used to receive signals from external equipment */
#define	PIO_EXT_IN			0x08
/*
** port C output bits
*/
/* bit 4 is used to signal external equipment through pin 8 of the aux
** camera jack
*/
#define	PIO_EXT_OUT			0x10
/* bit 5 is held low while reading tracker memory */
#define	PIO_READ_ENABLE	0x20
/* bit 6 is brought low then high to reset the interrupt latch and increment
** the memory counter
*/
#define	PIO_INC_MEM			0x40
#define	PIO_INT_RESET		0x40
/* bit 7 is held low to enable pio interrupts */
#define	PIO_INT_ENABLE		0x80

#define	PORT_C_OUTMASK		0xF0			/* mask for upper port C for output =240 */
#define	PORT_C_INMASK		0x0F		/* mask for lower port C for input */

#define 	MAX_TRACKER_ITEMS		253
#define 	TRACKER_XOFFSET		28		/* the camera offsets */
#define	TRACKER_YOFFSET		2
#define	TRUE	1
#define	FALSE	0
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
#ifdef OLD
#define DefaultView	setview((int)0,(int)0,(int)799,(int)599)
#define ScopeView	setview(sysinfo.scopeview.x,sysinfo.scopeview.y,\
	sysinfo.scopeview.x2,sysinfo.scopeview.y2)
#define MenuView	setview((int)0,(int)600,(int)799,(int)500)
#else
#define DefaultView
#define ScopeView
#define MenuView
#endif
#define 	TRACKER_XPIXELS 	328
#define	TRACKER_YPIXELS	244
#define	TRACKER_RECORD_ID	'T'

typedef struct view_type {
	int	x,y,x2,y2;
}View;

typedef struct coord_type {
	unsigned char	x,y;
} Coord;


typedef struct tracker_type {
	Coord	coord[MAX_TRACKER_ITEMS];
	Coord	pcoord[MAX_TRACKER_ITEMS];
	int	field;
	int	overflow;
	unsigned char	nitems;
	int	prevn;
	char	*errorstr;
	int	done;
	int	count;
	int	rate;
	unsigned long	time;
	unsigned long	prevtime;
	unsigned long	lost;
	FILE	*fp;
	unsigned long	filesize;
	short	state;
} Tracker;

typedef struct sysinfo_type {
	short		debug;
	int far	cpu;
	int far	mouse;
	int	far	vga;
	int	default_videomode;
	int far	videomode;
	int	far	vgamem;
	short	disk;
	short	graphics;
	short	acq;
	short	drawmode;
	short	overlay;
	short	quit;
	short	autoscale;
	short	refractory;
	View	scopeview;
	short	showstatus;
	short	displaymode;
	short	tracker;
} SystemInfo;

void CheckKeyboard(void);
void StatusMessage(char *string);
void StatusMessage2(char *string);
void StatusMessage3(char *string);
void StatusMessage4(char *string);
void TrackerOn(void);
void TrackerOff(void);

Tracker	tracker;
SystemInfo	sysinfo;
#if(ISR==1)
void interrupt	(*oldirq5vect)();
#endif

char	tmpstring[100];
int	irqmask = IRQ7MASK;


/*
************************************************************************
**
**   Display Routines
**
************************************************************************
*/
void DrawButton(char *string,int x, int y, int x2, int y2, short state)
{
	if(state == 1){
		drwbox(SET,(int)LIGHTGRAY,x,y,x2,y2);
		drwfillbox(SET,(int)LIGHTGRAY,x+1,y+1,x2-1,y2-1);
		drwstring(SET,BLACK,LIGHTGRAY,string,x+4,y+3);
		drwline(SET,WHITE,x+1,y+1,x2-1,y+1);
		drwline(SET,WHITE,x2-1,y+1,x2-1,y2-1);
		drwline(SET,BLACK,x+1,y+1,x+1,y2-1);
		drwline(SET,BLACK,x+1,y2-1,x2-1,y2-1);
	} else {
		drwbox(SET,(int)LIGHTGRAY,x,y,x2,y2);
		drwfillbox(SET,(int)DARKGRAY,x+1,y+1,x2-1,y2-1);
		drwstring(SET,LIGHTGRAY,DARKGRAY,string,x+4,y+3);
		drwline(SET,BLACK,x+1,y+1,x2-1,y+1);
		drwline(SET,BLACK,x2-1,y+1,x2-1,y2-1);
		drwline(SET,WHITE,x+1,y+1,x+1,y2-1);
		drwline(SET,WHITE,x+1,y2-1,x2-1,y2-1);
	}
}

void DrawMenu(void)
{
/*	fontsystem();*/
	DrawButton("(Q)uit",0,580,55,599,1);
	if(sysinfo.acq){
		DrawButton("(a)cq ON ",55,580,135,599,1);
	} else {
		DrawButton("(a)cq OFF",55,580,135,599,0);
	}
	if(sysinfo.disk){
		DrawButton("(d)isk ON ",135,580,220,599,1);
	} else {
		DrawButton("(d)isk OFF",135,580,220,599,0);
	}
	if(sysinfo.graphics){
		DrawButton("(g)raphics ON ",220,580,340,599,1);
	} else {
		DrawButton("(g)raphics OFF",220,580,340,599,0);
	}
	if(sysinfo.autoscale){
		DrawButton("a(u)toscale ON ",340,580,470,599,1);
	} else {
		DrawButton("a(u)toscale OFF",340,580,470,599,0);
	}
	if(sysinfo.showstatus){
		DrawButton("(S)tatus ON ",470,580,570,599,1);
	} else {
		DrawButton("(S)tatus OFF",470,580,570,599,0);
	}
	if(sysinfo.overlay){
		DrawButton("(o)verlay ON ",570,580,690,599,1);
	} else {
		DrawButton("(o)verlay OFF",570,580,690,599,0);
	}
	if(sysinfo.tracker){
		DrawButton("(t)racker ON ",690,580,799,599,1);
	} else {
		DrawButton("(t)racker OFF",690,580,799,599,0);
	}
}
void DrawMessages(void){
	/* status message area 4 */
	DrawButton("Message Area 4",500,519,799,539,1);
	/* status message area 3 */
	DrawButton("Message Area 3",0,519,500,539,1);
	/* status message area 2 */
	DrawButton("Message Area 2",0,539,799,559,1);
	/* status message area 1 */
	DrawButton("Message Area 1",0,559,799,579,1);
	/* error message area */
	DrawButton("Error Msg Area",500,539,799,559,1);
}

void DrawBorders(void)
{
	/* scope border */
	drwbox(SET,_RED,sysinfo.scopeview.x-1,sysinfo.scopeview.y-1,
	sysinfo.scopeview.x2+1,sysinfo.scopeview.y2+1);
	drwbox(SET,_GREEN,sysinfo.scopeview.x-1,sysinfo.scopeview.y-1,
	sysinfo.scopeview.x+256,sysinfo.scopeview.y+256);
}

void RefreshDisplay(void)
{
	DrawMenu();
	DrawMessages();
	DrawBorders();
}

void ClearScreen(void){
	fillscreen((int)_BLACK);
	RefreshDisplay();
}


void StatusMessage(char *string)
{
	MenuView;
	if(string == NULL) return;
	if(sysinfo.debug > 1){
		fprintf(stderr,"%s\n",string);
	}else
	drwstring(SET,_BLUE,LIGHTGRAY,string,(int)4,(int)562);
}

void StatusMessage2(char *string)
{
	MenuView;
	if(string == NULL) return;
	if(sysinfo.debug > 1){
		fprintf(stderr,"%s\n",string);
	}else
	drwstring(SET,_BLUE,LIGHTGRAY,string,(int)4,(int)542);
}
void StatusMessage3(char *string)
{
	MenuView;
	if(string == NULL) return;
	if(sysinfo.debug > 1){
		fprintf(stderr,"%s\n",string);
	}else
	drwstring(SET,_BLUE,LIGHTGRAY,string,(int)4,(int)522);
}
void StatusMessage4(char *string)
{
	MenuView;
	if(string == NULL) return;
	if(sysinfo.debug > 1){
		fprintf(stderr,"%s\n",string);
	}else
	drwstring(SET,_BLUE,LIGHTGRAY,string,(int)504,(int)522);
}

void ErrorMessage(char *string)
{
	if(string == NULL) return;
	if(sysinfo.debug){
		fprintf(stderr,"%s\n",string);
	}else
	drwstring(SET,_RED,LIGHTGRAY,string,(int)504,(int)542);
}
void ToggleDisk(void)
{
	sysinfo.disk = !sysinfo.disk;
	DrawMenu();
}
void ToggleGraphics(void)
{
	sysinfo.graphics = !sysinfo.graphics;
	DrawMenu();
}
void ToggleAcq(void)
{
	sysinfo.acq = !sysinfo.acq;
	DrawMenu();
}
void ToggleAutoscale(void)
{
	sysinfo.autoscale = !sysinfo.autoscale;
	DrawMenu();
}

void ToggleTracker(void)
{
	sysinfo.tracker = !sysinfo.tracker;
	if(sysinfo.tracker){
		TrackerOn();
	} else {
		TrackerOff();
	}
	DrawMenu();
}


/* display contents of the tracker buffer in a tracker window */
void DisplayTracker(void)
{
int	i,j;
int		xoffset,yoffset;
int		x,y;
int		nx,ny;
int	color;
register Coord	*cptr,*pptr;

	if(tracker.errorstr == NULL) return;
	/*
	** go through the buffer and display everything on the tracker event list
	*/
	if(sysinfo.graphics){
	xoffset = 50;
	yoffset = 50;
	nx=0;
	ny=0;
	if(tracker.state == 0){
		color = _GREEN;
	} else {
		color = _RED;
	}
	cptr = tracker.coord;
	pptr = tracker.pcoord;
	for(i=0;i<tracker.nitems;i++){
		x = cptr->x + xoffset;
		y = cptr->y + yoffset;
		/*
		** now display it
		*/
		if(!sysinfo.overlay){
			/* erase the previous point */
			drwpoint(SET,(int)BLACK,(int)pptr->x+xoffset,(int)pptr->y+yoffset);
		}
		drwpoint(SET,color,x,y);
		pptr->x = cptr->x;
		pptr->y = cptr->y;
		cptr++;
		pptr++;
		/*
		** if the tracker isr gets called during the display then break out
		*/
		if(tracker.done) break;
	}
	if(!tracker.done && !sysinfo.overlay && (tracker.nitems < tracker.prevn)){
		for(j=tracker.nitems;j<tracker.prevn;j++,pptr++){
			/* erase the previous point */
			drwpoint(SET,(int)BLACK,(int)pptr->x+xoffset,
			(int)pptr->y+yoffset);
		if(tracker.done) break;
		}
	}
	/*
	** note that i will contain the number of points actually displayed.
	** This will normally be the same as nitems, unless an interrupt
	** is generated during the display
	*/
	tracker.prevn = i;
	}
}

/* display contents of the tracker buffer in a tracker window */
void WriteTrackerBuffer(void)
{
char	key;
	if(tracker.fp == NULL) return;
	if(tracker.nitems > MAX_TRACKER_ITEMS){
		ErrorMessage("Bad nitems");
	}
	DisableIRQ;
	/*
	** go through the buffer and store everything on the tracker event list
	*/
	if(sysinfo.disk && !tracker.done){
		key = TRACKER_RECORD_ID;
		fwrite(&key,sizeof(char),1,tracker.fp);
		fwrite(&tracker.nitems,sizeof(unsigned char),1,tracker.fp);
		fwrite(&tracker.state,sizeof(unsigned char),1,tracker.fp);
		fwrite(&tracker.time,sizeof(unsigned long),1,tracker.fp);
		fwrite(tracker.coord,sizeof(Coord),(int)tracker.nitems,tracker.fp);
		tracker.filesize += sizeof(Coord)*tracker.nitems + sizeof(unsigned long)
		+ 2*sizeof(unsigned char);
	}
	if(sysinfo.showstatus){
	sprintf(tmpstring,"%7luK   ",
		tracker.filesize/1000);
	StatusMessage3(tmpstring);
	}
	EnableIRQ;
}

#if(ISR==1)
void interrupt Tracker_ISR(void)
#else
void Tracker_ISR(void)
#endif
{
register unsigned int	xcoord,ycoord;
unsigned 	coord;
int	tracker_field;
int	tracker_overflow;
int	i,j;
unsigned int	portc;
int	tracker_in;
int	tracker_out;
int	tracker_done;
int	tracker_inc;
int	tracker_read;
char	*errorstr;
register Coord	*cptr;
int	nitems_processed;
short	field_mask;

	disable();
	outp(PORT_C,0xa0);
	// output int low, reset low, read high, ext low
	outp(PORT_C,0xd0);
	tracker.nitems = 0;
	/* This will be used to drive the dual diode
	** system. The front and back diodes will be on alternate frames
	*/
	if(tracker.state == 0){
		field_mask = 0x00;
	} else {
		field_mask = PIO_EXT_OUT;
	}
	nitems_processed = 0;
	/*
	** set the external output to the field being scanned. This point
	** should occur just before the end of vertical blanking and
	** therefore will be in effect at the beginning of the scan of the next
	** field
	*/

	/*
	** check for tracker memory overflow. This will be set if more than 253
	** objects are in the trackers field.
	*/
	tracker_overflow = (inp(PORT_C) & PIO_MEM_OVERFLOW);
	/*
	** In order to read the coordinates of the tracked objects, the
	** read line of the pio must be held low. Note that this clears
	** the memory overflow bit.
	*/
	//outp(PORT_C,PORT_C_OUTMASK & ~PIO_READ_ENABLE);
	//outp(PORT_C,inp(PORT_C) & ~PIO_READ_ENABLE);
	// output int low, reset high, read low, ext low
	outp(PORT_C,0x40 | field_mask | 0x80);
	if(tracker.count%tracker.rate == 0 /*|| (tracker.count+1)%tracker.rate == 0 */){
		tracker.prevtime = tracker.time;
		tracker.time = clock();
		//outp(PORT_C,208);
		/*
		** which interlace field.
		*/
		tracker.field = inp(PORT_C) & PIO_INTERLACE_FIELD;

		/*
		** read the coordinates
		*/
		/* get the LSB of the horizontal coord from port C */
		// xcoord = (inp(PORT_B)<< 1) + (portc & PIO_HOR_LSB);
		//xcoord = inp(PORT_B) << 1;
		//portc = inp(PORT_C);
	/*
		tracker_in = inp(PORT_C) & PIO_EXT_IN;
		if(tracker_in > 0){
			tracker_read = 208 &  ~PIO_INT_ENABLE; 		// 208
			tracker_done = 240;//PORT_C_OUTMASK;		// 240
			tracker_inc = 144 & ~PIO_INT_ENABLE;//PORT_C_OUTMASK & ~PIO_INC_MEM & ~PIO_READ_ENABLE;   	//144
			tracker_in = 1;
			tracker_out = 1;
		} else {
			tracker_read = 192 &~PIO_INT_ENABLE;//PORT_C_OUTMASK & ~PIO_EXT_OUT & ~PIO_READ_ENABLE; 	// 192
			tracker_done = 224;//PORT_C_OUTMASK & ~PIO_EXT_OUT;	// 224
			tracker_inc = 128 & ~PIO_INT_ENABLE;//PORT_C_OUTMASK & ~PIO_INC_MEM & ~PIO_EXT_OUT & ~PIO_READ_ENABLE;// 128
			tracker_in = 0;
			tracker_out = 0;
		}
	  */
		// outp(PORT_C,tracker_read);
		//outp(PORT_C,inp(PORT_C) & ~PIO_READ_ENABLE);
		/*
		** read in the coordinates
		*/
		ycoord = inp(PORT_A);
		xcoord = inp(PORT_B);
		if(tracker_overflow > 0){
			errorstr = "Over";
		} else
		if(ycoord == 0){
			errorstr = "None";
		} else {
			errorstr = "OK";
		}
		/* check for non-zero point count in the buffer. */
		if((ycoord != 0) || tracker_overflow > 0){
			cptr = tracker.coord;
			for(i=0;i<MAX_TRACKER_ITEMS;i++){
				coord = inpw(PORT_A);
				xcoord = (coord >> 7) + (inp(PORT_C) & PIO_HOR_LSB) - TRACKER_XOFFSET;
				ycoord = (coord & 0x00ff);
				/* check for 'no input' condition */
				if(ycoord == 0 && tracker_overflow == 0){
					 break;
				}
				/* increment the buffer to get to the next point */
				//outp(PORT_C,tracker_inc);
				//outp(PORT_C,inp(PORT_C) & ~PIO_INC_MEM & ~PIO_READ_ENABLE);
				//outp(PORT_C,tracker_read);
				//outp(PORT_C,inp(PORT_C) | PIO_INC_MEM & ~PIO_READ_ENABLE);
				// output int low, inc low, read low, ext low
				outp(PORT_C,0x00 | field_mask | 0x80);
				// output int low, inc high, read low, ext low
				outp(PORT_C,0x40 | field_mask | 0x80);
				// impose a one byte limit on x coords. y does not extend beyond 255
				if(xcoord > 255) continue;
				cptr->y = ycoord-TRACKER_YOFFSET;
				cptr->x = xcoord;
				cptr++;
				tracker.nitems++;
			}
			nitems_processed = i;
		} else { /* end of non-zero point count check */
			/*
			** bump the memory buffer
			*/
			//outp(PORT_C,tracker_inc);
			//outp(PORT_C,tracker_read);
			//outp(PORT_C,tracker_read);
		}
		//if(tracker.count%tracker.rate == 0){
			if(nitems_processed < MAX_TRACKER_ITEMS){
				tracker.done = 1;
				/* this will toggle the front/back diode output on the next
				** sync
				*/
				tracker.state = !tracker.state;
			} else {
				tracker.lost++;
			}
		//}
	} else { /* end of tracker rate test */
			errorstr = NULL;
	}
	tracker.overflow = tracker_overflow;
	tracker.errorstr = errorstr;
	/* raise the read enable allowing the the current buffer to
	** be filled at the next memory toggle */
	// output int low, reset high, read high, ext low
	outp(PORT_C,0x60 | field_mask);
#if(ISR ==1 )
	/*
	** reset the interrupt latch. this must be pulled low then high
	*/
	// output int low, reset low, read high, ext low
	outp(PORT_C,0x20 | field_mask);
	// output int low, reset low, read high, ext low
	outp(PORT_C,0x60 | field_mask);
	/* signal the end of the ISR to the controller */
	outp(IRQMASTER,IRQEOI);
#endif
	tracker.count++;
	enable();
}
void SystemSetup(void)
{

	sysinfo.cpu = whichcpu();
	sysinfo.mouse = whichmouse();
	sysinfo.vga = whichvga();
	sysinfo.vgamem = whichmem();
	setcard(sysinfo.vga,sysinfo.vgamem);
	res800();
	sysinfo.videomode = videomodeget();
	sysinfo.scopeview.x = 50;
	sysinfo.scopeview.y = 50;
	sysinfo.scopeview.x2 = TRACKER_XPIXELS + 50;
	sysinfo.scopeview.y2 = TRACKER_YPIXELS + 50;
/*	mouseenter();
	mousecursordefault(); */
}
void CheckKeyboard(void)
{
int	c;
short	extended;


	if(_kbhit()){
		c = _getch();
		if(!c){
			/*
			** extended character
			*/
			extended = 1;
			c = _getch();
			sprintf(tmpstring,"E%X %d %c   ",c,c,c);
			StatusMessage4(tmpstring);
		} else {
			extended = 0;
			sprintf(tmpstring,"%X %d %c   ",c,c,c);
			StatusMessage4(tmpstring);
		}
		if(extended){
			switch(c){
			case 'I':		/* extended: pgup */
				irqmask++;
				sprintf(tmpstring,"%X     ",irqmask);
				StatusMessage4(tmpstring);
				break;
				EnableIRQ;
			case 'Q':		/* extended: pgdown */
				irqmask--;
				sprintf(tmpstring,"%X     ",irqmask);
				StatusMessage4(tmpstring);
				EnableIRQ;
				break;
			case 'H':		/* extended: up arrow */
				tracker.rate++;
				break;
			case 'P':		/* extended: down arrow */
				tracker.rate--;
				if(tracker.rate < 1) tracker.rate =1;
				break;
			case 'M':		/* extended: right arrow */
				break;
			case 'K':		/* extended: left arrow */
				break;
			}
		} else
		switch(c){
		case 'a':
			ToggleAcq();
			break;
		case 'c':
			ClearScreen();
			break;
		case 'd':
			ToggleDisk();
			break;
		case 'g':
			ToggleGraphics();
			break;
		case 'm':
			sysinfo.displaymode = !sysinfo.displaymode;
			DrawMenu();
			break;
		case 'o':
			sysinfo.overlay = !sysinfo.overlay;
			DrawMenu();
			break;
		case 'r':
			sysinfo.drawmode = !sysinfo.drawmode;
			break;
		case 'Q':
			sysinfo.quit = 1;
			break;
		case 's':
			break;
		case 'S':
			sysinfo.showstatus = !sysinfo.showstatus;
			DrawMenu();
			break;
		case 't':
			ToggleTracker();
			break;
		case 'u':
			ToggleAutoscale();
			break;
		}
	}

}

void TrackerOff(void){
#if(ISR==1)
	disable();
	/*
	** disable irq line
	*/
	outp(IRQMASTER+1,inp(IRQMASTER+1)|0x20);
	/*
	** disable interrupts from the pio
	*/
	outp(PORT_C,PORT_C_OUTMASK | PIO_INT_ENABLE);
	enable();
#endif
	tracker.done = 0;
}

void TrackerOn(void)
{
#if(ISR == 1)
	disable();
	/*
	** enable pio interrupts
	*/
	outp(PORT_C,inp(PORT_C) & ~PIO_INT_ENABLE);
	/* lower then raise the interrupt latch */
	outp(PORT_C,inp(PORT_C) & ~PIO_INT_RESET);
	outp(PORT_C,inp(PORT_C) | PIO_INT_RESET);
	/*
	** enable irq line
	*/
	outp(IRQMASTER+1,inp(IRQMASTER+1)&0xdf);
	enable();
#else
	/* lower then raise the interrupt latch */
	outp(PORT_C,inp(PORT_C) & ~PIO_INT_RESET);
	outp(PORT_C,inp(PORT_C) | PIO_INT_RESET);
#endif
	tracker.done = 0;
}

void main(int argc, char **argv)
{
char	ch;
int	i;
float	f;
unsigned long	time;
unsigned long	count;
int	ntimes;

	fprintf(stderr,"Beginning tracker test code\n");
	sysinfo.debug = 0;
	sysinfo.disk = 0;						/* flag to enable disk output */
	sysinfo.graphics = 1;				/* flag to enable graphics display */
	sysinfo.acq = 0;						/* flag to enable acquisition */
	sysinfo.drawmode = 0;
	sysinfo.overlay = 0;
	sysinfo.quit = 0;
	sysinfo.autoscale = 0;
	sysinfo.tracker = 1;
	tracker.count = 0;
	tracker.rate = 1;
	tracker.lost = 0;
	tracker.filesize = 0;
	if((tracker.fp = fopen("c:/tdata","w")) == NULL){
		fprintf(stderr,"unable to open data file c:/tdata\n");
		exit(-1);
	}
	time = clock();
	count = 0;
	 SystemSetup();
	 RefreshDisplay();
	/*
	** configure the pio board inputs and outputs
	** set ports A, B, and the lower part of C as inputs
	** set the upper part of C as output
	*/
	//outp(PIO_CONTROL,(PA_BIT | PB_BIT | PCLO_BIT) & ~PCHI_BIT); 	// 19
	outp(PIO_CONTROL,147);
#if(ISR==1)
	/*
	** set up the interrupt vector
	*/
	oldirq5vect = getvect(IRQ7VECT);
	setvect(IRQ7VECT,Tracker_ISR);
	/*
	** enable pio interrupts
	*/
	outp(PORT_C,PORT_C_OUTMASK & ~PIO_INT_ENABLE);
	/* lower then raise the interrupt latch */
	outp(PORT_C,inp(PORT_C) & 0xbf);
	outp(PORT_C,inp(PORT_C) | 0x40);
	/*
	** enable irq line
	*/
	outp(IRQMASTER+1,inp(IRQMASTER+1)&0xdf);
//	TrackerOn();
#endif
	while(!sysinfo.quit){
		if(!tracker.done) CheckKeyboard();
#if(ISR == 0)
		if(sysinfo.tracker){
			Tracker_ISR();
			for(i=0;i<25000;i++);
		}
#endif
		if(sysinfo.tracker && tracker.done){
			tracker.done = 0;
			DisplayTracker();
			if(sysinfo.showstatus && !tracker.done){
			sprintf(tmpstring,"n=%4d x=%4d y=%4d f=%1d s=%1d %6s %7lu %4lu %3d %7lu",
				tracker.nitems,tracker.coord[0].x,tracker.coord[0].y,tracker.field,tracker.state,
				tracker.errorstr,tracker.time,tracker.time - tracker.prevtime,tracker.rate,
				tracker.lost);
			StatusMessage2(tmpstring);
			}
			count=tracker.count;
			ntimes=0;

			if(sysinfo.disk){
				WriteTrackerBuffer();
			}
		} else
		if(sysinfo.tracker){
 /*			if(sysinfo.showstatus && tracker.errorstr != NULL && tracker.count != count){
				sprintf(tmpstring,"n=%4d x=%4d y=%4d f=%1d s=%1d %6s %7lu %4lu %3d %7d",
				tracker.nitems,tracker.coord[0].x,tracker.coord[0].y,tracker.field,tracker.state,
				tracker.errorstr,tracker.time,tracker.time - tracker.prevtime,tracker.rate,
				ntimes);
			StatusMessage3(tmpstring);

		}
		*/
		// how many times around the loop before sync interrupt
		if(tracker.count == count)
			ntimes++;
		}
	}
	/*
	** finished, so prepare to exit
	*/
	TrackerOff();
#if(ISR==1)
	/*
	** restore the IRQ vector
	*/
	setvect(IRQ7VECT,oldirq5vect);
#endif
}
