#include "adext.h"

int indisk = 0;
int diskirqerror = 0;

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
#define	PIO_READ_ENABLE		0x20
/* bit 6 is brought low then high to reset the interrupt latch and increment
** the memory counter
*/
#define	PIO_INC_MEM			0x40
#define	PIO_INT_RESET		0x40
/* bit 7 is held low to enable pio interrupts */
#define	PIO_INT_ENABLE		0x80

#define	PIOPORT_C_OUTMASK		0xF0			/* mask for upper port C for output =240 */
#define	PIOPORT_C_INMASK		0x0F		/* mask for lower port C for input */

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
#define 	TRACKER_XPIXELS 	328
#define	TRACKER_YPIXELS	244
#define	TRACKER_RECORD_ID	'T'

#define drwpoint(M,C,X,Y)   putpixel(X,Y,C)

void interrupt	(*oldtrackerirqvect)();

Tracker	tracker;
int	trackerirqmask;
int	trackerirqvect;
int	tracker_irq = 4;
static unsigned long tracker_lost[2];
static unsigned long tracker_frames;


void TrackerOn(void)
{
	EventString("TRACKERON");
	disable();
	/*
	** enable pio interrupts
	*/
	outp(PIOPORT_C,inp(PIOPORT_C) & ~PIO_INT_ENABLE);
	/* lower then raise the interrupt latch */
	outp(PIOPORT_C,inp(PIOPORT_C) & ~PIO_INT_RESET);
	outp(PIOPORT_C,inp(PIOPORT_C) | PIO_INT_RESET);
	/*
	** enable irq line
	*/
	EnableTrackerIRQ;
	enable();
	tracker.done = 0;
	tracker_lost[0] = 0;
	tracker_lost[1] = 0;
	tracker_frames = 0;
}
void TrackerOff(void){
	disable();
	/*
	** disable irq line
	*/
	DisableTrackerIRQ;
	/*
	** disable interrupts from the pio
	*/
	outp(PIOPORT_C,PIOPORT_C_OUTMASK | PIO_INT_ENABLE);
	enable();
	tracker.done = 0;
	EventString("TRACKEROFF");
}

/*
************************************************************************
**
**   Display Routines
**
************************************************************************
*/

void ToggleTracker(void)
{
	sysinfo.tracker = !sysinfo.tracker;
	if(sysinfo.tracker){
		TrackerOn();
		ErrorMessage("TRACKER ON");
	} else {
		TrackerOff();
		ErrorMessage("TRACKER OFF");
	}
	UpdateTrackerButton();
}


static long	avgx[2],avgy[2];
/* display contents of the tracker buffer in a tracker window */
void DisplayTracker(void)
{
int	i,j;
int	xoffset,yoffset;
int	x,y;
int	nx,ny;
int	color;
int   r;
register Coord	*cptr,*pptr;

	tracker.done = 0;
	if(tracker.errorstr == NULL) return;
	/*
	** go through the buffer and display everything on the tracker event list
	*/
	xoffset = 50;
	yoffset = 50;
	nx=0;
	ny=0;
	if(tracker.state == 0){
		color = _GREEN;
	} else {
		color = _RED;
	}
	avgx[tracker.state] = 0;
	avgy[tracker.state] = 0;
	cptr = tracker.coord;
	pptr = tracker.pcoord;
	for(i=0;i<tracker.nitems;i++){
		x = cptr->x + xoffset;
		y = cptr->y + yoffset;
		/*
		** keep track of the average location
		*/
		avgx[tracker.state] += cptr->x;
		avgy[tracker.state] += cptr->y;
		/*
		** now display it
		*/
		if(!sysinfo.trackoverlay){
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
	if(!tracker.done && !sysinfo.trackoverlay && (tracker.nitems < tracker.prevn)){
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
	if(!tracker.done){
		if(i > 0){
			avgx[tracker.state] /= i;
			avgy[tracker.state] /= i;
		}

// **** Stim stuff, by Jay, 1/97 ***** 
if (sysinfo.seq[STIMSEQON].active)
{
 sysinfo.stim.stimflag = 0;
 // If there is no stim area defined, assume everywhere is game for stimulation
 if ((sysinfo.stim.tstarea.x2 == 0) && (sysinfo.stim.ctlarea.x2 == 0)) 
    sysinfo.stim.stimflag = 1;
 if ((sysinfo.stim.tstarea.x2 > 0) && (sysinfo.stim.tstarea.y2 > 0)) 
  {
 	// For first pass into stimulating region set stimflag = 1, 
	// if still in region inactivate (-1), else 0 
	if (((int)100*avgx[0]/TRACKER_XPIXELS > sysinfo.stim.tstarea.x) 
 	 && ((int)100*avgx[0]/TRACKER_XPIXELS < sysinfo.stim.tstarea.x2)
 	 && ((int)100*avgy[0]/TRACKER_YPIXELS > sysinfo.stim.tstarea.y) 
 	 && ((int)100*avgy[0]/TRACKER_YPIXELS < sysinfo.stim.tstarea.y2))
	 {
	  sysinfo.stim.tstflag = 1 - 2*sysinfo.stim.tstflag*sysinfo.stim.tstflag;
	  sysinfo.stim.mode = TEST;
	 }
//	else if ((avgx[0] > 0) && (avgy[0] > 0)) sysinfo.stim.stimflag = 0;
	else sysinfo.stim.tstflag = 0;
  }
 if ((sysinfo.stim.ctlarea.x2 > 0) && (sysinfo.stim.ctlarea.y2 > 0)) 
  {
 	// For first pass into stimulating region set stimflag = 1, 
	// if still in region inactivate (-1), else 0 
	if (((int)100*avgx[0]/TRACKER_XPIXELS > sysinfo.stim.ctlarea.x) 
 	 && ((int)100*avgx[0]/TRACKER_XPIXELS < sysinfo.stim.ctlarea.x2)
 	 && ((int)100*avgy[0]/TRACKER_YPIXELS > sysinfo.stim.ctlarea.y) 
 	 && ((int)100*avgy[0]/TRACKER_YPIXELS < sysinfo.stim.ctlarea.y2))
	 {
	  sysinfo.stim.ctlflag = 1 - 2*sysinfo.stim.ctlflag*sysinfo.stim.ctlflag;      sysinfo.stim.mode = CONTROL;
	 }
//	else if ((avgx[0] > 0) && (avgy[0] > 0)) sysinfo.stim.stimflag = 0;
	else sysinfo.stim.ctlflag = 0;
  }
if ((sysinfo.stim.tstflag == 1) || (sysinfo.stim.ctlflag == 1)) 
        sysinfo.stim.stimflag = 1;
 if (sysinfo.stim.stimflag == 1) {
  if((r = (int)(rand()/327.68)) <= sysinfo.stim.percent) 
	{
	 if (sysinfo.stim.protocol == RANDOM) 
	 	sysinfo.stim.mode = (int)(4*rand()/32768);
	Stimulate();
	}
  }
}
	  
		if(sysinfo.disk){
			if(avgx[tracker.state] == 0 && avgy[tracker.state] == 0){
				tracker_lost[tracker.state]++;
			}
			tracker_frames++;
		}
		drwfillbox(SET, LIGHTGRAY, sysinfo.rateview.x, sysinfo.rateview.y, 
               sysinfo.rateview.x2, sysinfo.rateview.y2);
	    if(sysinfo.disk){
		sprintf(tmpstring,"\n %3d %2d %4s",
			tracker.nitems,
			tracker.rate,
			tracker.errorstr);
		} else {
		sprintf(tmpstring,"  %3d %2d %4s",
			tracker.nitems,
			tracker.rate,
			tracker.errorstr);
		}
		setcolor(WHITE);
		settextstyle(DEFAULT_FONT,HORIZ_DIR,2);
		outtextxy(sysinfo.rateview.x, sysinfo.rateview.y, tmpstring);
		if(tracker.state == 0){
		sprintf(tmpstring,"%3ld_%3ld %3ld %3ld",
			avgx[0],avgy[0],avgx[1],avgy[1]);
		} else  {
		sprintf(tmpstring,"%3ld %3ld %3ld_%3ld",
			avgx[0],avgy[0],avgx[1],avgy[1]);
		}
		outtextxy(sysinfo.rateview.x, sysinfo.rateview.y + 2*sysinfo.theight, 
		tmpstring);
		setcolor(WHITE);
		if(tracker_frames > 0){
		if(sysinfo.disk){
		sprintf(tmpstring," %5.2g%% %5.2g%%",
			100*(float)tracker_lost[0]/tracker_frames,
			100*(float)tracker_lost[1]/tracker_frames);
		} else {
		sprintf(tmpstring,"[%5.2g%% %5.2g%%]",
			100*(float)tracker_lost[0]/tracker_frames,
			100*(float)tracker_lost[1]/tracker_frames);
		}
		outtextxy(sysinfo.rateview.x, sysinfo.rateview.y + 4*sysinfo.theight, tmpstring);
		}
		settextstyle(DEFAULT_FONT,HORIZ_DIR,1);
		/*
		ErrorMessage(tmpstring);
		*/
	}
}

/* save the contents of the tracker buffer to disk */
void WriteTrackerBuffer(void)
{
char	key;
int		recordsize;
	if(adinfo.fp == NULL) return;
	if(tracker.nitems > MAX_TRACKER_ITEMS){
		ErrorMessage("Bad nitems");
	}
	DisableTrackerIRQ;
	indisk = 1;
	/*
	** go through the buffer and store everything on the tracker event list
	*/
	recordsize = 0;
	if(sysinfo.disk && !tracker.done){
		key = TRACKER_RECORD_ID;
		fwrite(&key,sizeof(char),1,adinfo.fp);
		fwrite(&tracker.nitems,sizeof(unsigned char),1,adinfo.fp);
		fwrite(&tracker.state,sizeof(unsigned char),1,adinfo.fp);
		fwrite(&tracker.time,sizeof(unsigned long),1,adinfo.fp);
		fwrite(tracker.coord,sizeof(Coord),(int)tracker.nitems,adinfo.fp);
		fwrite(&key,sizeof(char),1,adinfo.fp); 
		
		recordsize = sizeof(Coord)*tracker.nitems + sizeof(unsigned long)
		+ 3*sizeof(unsigned char);
	}
	EnableTrackerIRQ;

	indisk = 0;
	if(sysinfo.showstatus){
	sprintf(tmpstring,"%7luK   ",
		tracker.filesize/1000);
	ErrorMessage(tmpstring);
	}
	tracker.filesize += recordsize;
	sysinfo.filesize += recordsize;
	sysinfo.diskfree -= recordsize;
}

void interrupt Tracker_ISR(void)
{
register unsigned int	xcoord,ycoord;
unsigned 	coord;
int	tracker_field;
unsigned	tracker_overflow;
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

#ifndef OLD
	/*
	** check to make sure we are not in the middle of a disk write
	*/
	if(indisk){
		diskirqerror++;
	}
#endif
	disable();
	outp(PIOPORT_C,PIO_INT_ENABLE | PIO_READ_ENABLE);

	tracker_overflow = (inp(PIOPORT_C) & PIO_MEM_OVERFLOW);

	// output int low, reset low, read high, ext low
	outp(PIOPORT_C,0xd0);
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
	//tracker_overflow = (inp(PIOPORT_C) & PIO_MEM_OVERFLOW);
	/*
	** In order to read the coordinates of the tracked objects, the
	** read line of the pio must be held low. Note that this clears
	** the memory overflow bit.
	*/
	//outp(PIOPORT_C,PIOPORT_C_OUTMASK & ~PIO_READ_ENABLE);
	//outp(PIOPORT_C,inp(PIOPORT_C) & ~PIO_READ_ENABLE);
	// output int low, reset high, read low, ext low
	/* bit 7 is held high to disable pio interrupts */
	outp(PIOPORT_C,PIO_INT_RESET | field_mask | PIO_INT_ENABLE);

	if(tracker.count%tracker.rate == 0){
		tracker.prevtime = tracker.time;
		tracker.time = ReadTS();
		//outp(PIOPORT_C,208);
		/*
		** which interlace field.
		*/
		tracker.field = inp(PIOPORT_C) & PIO_INTERLACE_FIELD;

		/*
		** read the coordinates
		*/
		/* get the LSB of the horizontal coord from port C */
		// xcoord = (inp(PIOPORT_B)<< 1) + (portc & PIO_HOR_LSB);
		//xcoord = inp(PIOPORT_B) << 1;
		//portc = inp(PIOPORT_C);
		// outp(PIOPORT_C,tracker_read);
		//outp(PIOPORT_C,inp(PIOPORT_C) & ~PIO_READ_ENABLE);
		/*
		** read in the coordinates
		*/
		ycoord = inp(PIOPORT_A);
		xcoord = inp(PIOPORT_B);
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
			for(i=0;i<MAX_TRACKER_ITEMS-1;i++){
				coord = inpw(PIOPORT_A);
				xcoord = (coord >> 7) + (inp(PIOPORT_C) & PIO_HOR_LSB) - TRACKER_XOFFSET;
				ycoord = (coord & 0x00ff);
				/* check for 'no input' condition */
				if(ycoord == 0 && tracker_overflow == 0){
					 break;
				}
				/* increment the buffer to get to the next point */
				//outp(PIOPORT_C,tracker_inc);
				//outp(PIOPORT_C,inp(PIOPORT_C) & ~PIO_INC_MEM & ~PIO_READ_ENABLE);
				//outp(PIOPORT_C,tracker_read);
				//outp(PIOPORT_C,inp(PIOPORT_C) | PIO_INC_MEM & ~PIO_READ_ENABLE);
				// output int low, inc low, read low, ext low
				outp(PIOPORT_C,0x00 | field_mask | 0x80);
				// output int low, inc high, read low, ext low
				outp(PIOPORT_C,0x40 | field_mask | 0x80);
				// impose a one byte limit on x coords. y does not extend beyond 255
				// if(xcoord > 255) continue;
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
			//outp(PIOPORT_C,tracker_inc);
			//outp(PIOPORT_C,tracker_read);
			//outp(PIOPORT_C,tracker_read);
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
	outp(PIOPORT_C,0x60 | field_mask);
	/*
	** reset the interrupt latch. this must be pulled low then high
	*/
	// output int low, reset low, read high, ext low
	outp(PIOPORT_C,0x20 | field_mask);
	// output int low, reset low, read high, ext low
	outp(PIOPORT_C,0x60 | field_mask);
	/* signal the end of the ISR to the controller */
	outp(IRQMASTER,IRQEOI);
	tracker.count++;
	enable();
}


void SetupTracker(void)
{
	sysinfo.tracker = 0;
	tracker.count = 0;
	tracker.rate = 1;
	tracker.lost = 0;
	tracker.filesize = 0;
	/*
	** configure the pio board inputs and outputs
	** set ports A, B, and the lower part of C as inputs
	** set the upper part of C as output
	*/
	//outp(PIO_CONTROL,(PA_BIT | PB_BIT | PCLO_BIT) & ~PCHI_BIT); 	// 19
	outp(PIO_CONTROL,147);
	/*
	** set up the interrupt vector
	*/
	switch(tracker_irq){
	case 4:
		trackerirqvect = IRQ4VEC;
		trackerirqmask = IRQ4MASK;
		break;
	case 5:
		trackerirqvect = IRQ5VEC;
		trackerirqmask = IRQ5MASK;
		break;
	case 7:
		trackerirqvect = IRQ7VEC;
		trackerirqmask = IRQ7MASK;
		break;
	}
	fprintf(stderr,"\tTracker: DIO24\n");
	fprintf(stderr,"\t\tIRQ=%d\n", tracker_irq);
	fprintf(stderr,"\t\tBase addr=0x%0X\n", PIO_BASE);
	oldtrackerirqvect = getvect(trackerirqvect);
	setvect(trackerirqvect,Tracker_ISR);
#ifdef OLD
	/*
	** enable pio interrupts
	*/
	fprintf(stderr,"pio\n");
	outp(PIOPORT_C,PIOPORT_C_OUTMASK & ~PIO_INT_ENABLE);
	/* lower then raise the interrupt latch */
	outp(PIOPORT_C,inp(PIOPORT_C) & 0xbf);
	outp(PIOPORT_C,inp(PIOPORT_C) | 0x40);
	/*
	** enable irq line
	*/
	EnableTrackerIRQ;
#endif
}

void RestoreTracker(void)
{
	/*
	** restore the IRQ vector
	*/
	setvect(trackerirqvect,oldtrackerirqvect);
}

#ifdef OLD
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
	sysinfo.trackoverlay = 0;
	sysinfo.quit = 0;
	sysinfo.autoscale = 0;
	sysinfo.tracker = 1;
	
	SetupTracker();
//	TrackerOn();
	while(!sysinfo.quit){
		if(!tracker.done) CheckKeyboard();
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
RestoreTracker();
}
:?

:wq
:wq
#endif
