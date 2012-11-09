/* data type declarations */

#define     MAX_TRACKER_ITEMS		253
#define MAXSEQ		10
#define MAXSPIKES    200            /* maximum spikes per buffer */
#define MAXCHANNELS    16
#define NELECTRODES    2
#define NELECTRODE_CHANNELS    4
#define NPROJECTIONS    6
#define DMABUFFERS    3
#define NPRESPIKE 6
#define PRESPIKE 24    /* (points * nchan) to save before spike trigger point */
#define ELECTRODESPIKELEN 128           
#define SINGLESPIKELEN  32              /* number of points in a spike event */

#define MAXBUTTONS 100
#define NAMP_CHANNELS 8
#define EVENTSTRINGSIZE    80
#define MAXEVENTKEYS 8
#define MAXEVENTSTRINGS 4
#define MAXFASTCONFIG 4
#define MAXFILENAME 15
#define MAXFILENAMELEN 80

#define STIMSEQOFF	 0			
#define STIMSEQON	 1			
#define BLOCKSEQ	 2
#define BLOCKSEQOFF	 3
#define LIGHTSEQON	 4
#define LIGHTSEQOFF	 5
#define CHECKBARSEQ	 6
#define FEEDERSEQON  7
#define FEEDERSEQOFF 8
#define DELTASEQ     9 
			
typedef struct seq_type {
	short	id;
	short	active;
	unsigned long	time;
	unsigned long	interval;
	void	(*func)();
} Sequence;			

enum  {BUTTONENTER=1, BUTTONEXIT, BUTTON1PRESS, BUTTON2PRESS, BUTTON1RELEASE,
BUTTON2RELEASE};

typedef enum    {SINGLE,MASTER,SLAVE};
typedef enum  {INVALID,RESETCLK,STOPACQ,STARTACQ,DISKON,DISKOFF,FILECLOSE,
					CLEARSCREEN,TESTSYNCH};

typedef unsigned char BYTE;
typedef unsigned short WORD;

typedef struct button_type {
    BYTE    active;
    BYTE    inbutton;
    BYTE    button1down;
    BYTE    button2down;
    short   x,y,x2,y2;
    void    (*handler)();
    char    *data;
} Button;

typedef struct message_area_type {
   short x, y, x2, y2;
} MessageArea;

typedef struct waveform_type {
    short    **data;
    short    count;
    short    length;
} Waveform;

typedef struct cluster_type {
    short    x,y,a,b;
    short    color;
} Cluster;

typedef struct view_type {
  	int    x,y,x2,y2;
    char   info[10];
    int    infox, infoy;
}View;

typedef struct stim_type {
	int percent;
	float voltage;
	unsigned long seq[255];
	int seqnumber;
	int seqlength;
	View tstarea;
	View ctlarea;
	int pulselen;
	int number;
	int interpulse;
	unsigned long delay;
	int phase;
	int stimflag;
	int tstflag;
	int ctlflag;
 	int protocol;
	int mode;
	int grid;
	int on;
	int test[MAXCHANNELS+1];
	int control[MAXCHANNELS+1];
} Stim;

typedef struct spike_type {
    char   type;
    int    electrode;         /* the electrode number */
    unsigned long   timestamp;
    int    dataptr[ELECTRODESPIKELEN];
    char   type2;
} Spike;

typedef struct electrode_info_type {
    int far     *prevbufptr;           /* used for display */
    Spike far   *spikebuf;           /* used for storing spikes */
    char far 	*subthresh;	/* determines whether a given channel has 
					    gone below threshold */
    int         nspikes;         /* # of spikes in each buffer */
    int         prevstart;        /* the index at which to start
			   processing in the previous buffer */
     
    long       ntotal_spikes;  /*cumulative spikes on this electrode */
    int        channel[NELECTRODE_CHANNELS];  /* channels on this electrode */
    int        thresh[NELECTRODE_CHANNELS];    /* threshold for triggering
					  spike */
    float	   spikewindowscale[NELECTRODE_CHANNELS];
				    /* scale for the spike window */
    float      spikescale;            /* the scale multiplier for     
				      spike display */
    int        yoffset;                  /* spike display yoffsets */
    int        spikexoffset[NELECTRODE_CHANNELS];
    View       spikewindow[NELECTRODE_CHANNELS];
    View       projection[NPROJECTIONS];    /* the boxes for the 
		       projections */ 
    int		displaythresh; /* waveforms with the displaythresh
			    points (of the four spike wave forms)
			    above threshold will not be displayed */
					    
    int        projxoffset[NPROJECTIONS];
    int        projyoffset[NPROJECTIONS];
    int        projwidth;
    short      channelbutton[NELECTRODE_CHANNELS];
    short      spbutton[2];
} ElectrodeInfo;


typedef struct channel_info_type {
    long        ampgain;
    int        adgain;
    int        filter;
    int        contoffset;
    int		   contscaletick;
    int		   contthreshtick;
    float      contwindowscale;
    float      contscalemult;
    int        color;
    short      contbutton;
    short      triggerable;
    int		thresh;
    char 	subthresh;
} ChannelInfo;

typedef struct dma_into_type {
    int far    *dataptr;
    WORD       dmapage;
    WORD       dmabase;
    short      buffer_valid;
    unsigned long    timestamp;
} DMAInfo;

typedef struct eventstring_type {
    char		eventstring[EVENTSTRINGSIZE];
    unsigned long	eventtime;
} EventStringType;

typedef struct adinfo_type {
    float    rate_req;
    float    rate_set;
    short    irq;
    short    dma_ch1;
    short    dma_ch2;
    int      dma_bufsize;
    WORD     adcsr;
    WORD     supcsr;
    WORD     clockreg;
    int far  **dataptr;
    int far  *prevptr;    /* the previous buffer for continuous 
                             display mode */
    WORD     dmapage[DMABUFFERS];
    WORD     dmabase[DMABUFFERS];
    short    buffer_valid[DMABUFFERS];
    unsigned long    timestamp[DMABUFFERS];
    unsigned long        count;
    short    next_buf;        /* the index of the buff er being filled */
    short    process_buf; /* the index of the next buffer to be processed */
    int      buflen; /* the length of the buffer minus the overlap*/
    int      prespike_points;
    int      spikelen;  
    int      spikesep;
    int      elect_inc;  
    int      nelect_size; /* the size, in bytes, of NELECTRODE_CHANNELS 
                             integers */
    short    nchannels;
    short    nelectrodes;
    short    nelect_chan;
    short    firstbuffer;
    short    filled_buffer;
    short    current_buffer;
    long     error_count;
    WORD     error;
    long     display_lost;
    long     disk_lost;
    long	 computed_used;
    long	 nbuffers;
    long     nspikes;
    short    dmadone;
    double 	 conversion_duration;
    double 	 computed_timestamp;
    short    overlap_window_size;
    ElectrodeInfo    electrode[NELECTRODES];
    ChannelInfo    channel[MAXCHANNELS];
    int      spikelimit;
    int      dmabuf_counter;
    int      piostate;
    char     fname[MAXFILENAMELEN];
    FILE     *fp;
    int		peak_rate;
} ADInfo;

typedef struct sysinfo_type {
    short        debug;
    int        debugwinx;
    int        debugwiny;
    int far    cpu;
    int far    mouse;
    int far    vga;
    int        default_videomode;
    int far    videomode;
    int far    vgamem;
    short      disk;
    short       fileopen;
    short      graphics;
    short      acq;
    short      acqbon; /* state of the acquisition button */
    short      stopped;
    short      drawmode;
    short      overlay[NELECTRODES];
    short      contoverlay;
    short      trackoverlay;
    short      quit;
    short      autoscale;
    short      refractory;
    short      showstatus;
    char 	   conttype;
    short      stringinput;
    short      stringpos;
    char       inputstring[200];
    int    	   inputchar;
    short      nbuttons;
    short      selected_button;
    short      selected_channel;
    short      channel_selected;
    short      spikebuttonbase;
    short      contbuttonbase;
    short      selected_spbutton;
    short      nspbuttons;
    short      maxbutton;
    short      mode;
    short      detect_spikes;
    View       scopeview;
    View       spikeview;
    View       helpview;
    View       trackview;
    short	   helpon;			
    int        mousex;
    int        mousey;
    int        mousebut;
    BYTE       mouseactive;
    long       diskfree;
    long       filesize;
    long       olddiskfree;
    unsigned long	   oldtime;
    unsigned long	   olddisktime;
    unsigned long	   oldsynctime;
    unsigned long       currenttime;
    View       inputview;
    View       ctickview;
    View       currenttimeview;
    View       remaintimeview;
    View       diskfreeview;
    View       filesizeview;
    View       rateview;
    View       filterview;
    View       versionview;
    int		   theight;
    int		   twidth;
    short      kbhandled;
    short		command_mode;
    short		command_mode2;
    short		tracker;
    short		tracker_enabled;
    char		spiketype;
    char		eventtype;
    short		eventstring_size;
    struct eventstring_type	eventstring[MAXEVENTSTRINGS];	
    int			neventstrings;
    struct eventstring_type	eventkey[MAXEVENTKEYS];	
    char *configfilename[MAXFASTCONFIG];	
    int		adirqmask;
    int		adirqvec;
    int		adboard_id;
    int		networkvec;
    char	sessionstring[EVENTSTRINGSIZE];
    char	epochstring[EVENTSTRINGSIZE];
    unsigned long epochtime;
    Sequence	seq[MAXSEQ];
    Stim stim;
    int  	timemode;
    int		nseq;
    int		pio2;
    float		desired_timebase;
    unsigned long	trigger;
    short		triggered_continuous;
    int			continuous_pixelindex;
 double			xoffset;
} SystemInfo;

typedef struct coord_type {
    short x;
    unsigned char	y;
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


