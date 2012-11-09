#include "adext.h" 
#include "iolib.h"
#include <bios.h>

int kbchar;
int	nlosteventstrings =0;

/*
************************************************************************
**
**   IO Routines
**
************************************************************************
*/
void EventString(char *string)
{
	if(!sysinfo.fileopen) return;
	if(sysinfo.neventstrings >= MAXEVENTSTRINGS){
		sysinfo.neventstrings = MAXEVENTSTRINGS;
		nlosteventstrings++;
		sprintf(tmpstring2,"LOST ES: %s at %s",string,FormatTS(ReadTS(),0));
		return;
	}
	sysinfo.eventstring[sysinfo.neventstrings].eventtime = ReadTS();
	strcpy(sysinfo.eventstring[sysinfo.neventstrings].eventstring,string); 
	sprintf(tmpstring2,"ES[%d]: %s at %s",sysinfo.neventstrings,string,
		FormatTS(sysinfo.eventstring[sysinfo.neventstrings].eventtime,0));
	ErrorMessage(tmpstring2);
	sysinfo.neventstrings++;
}

void EventStringAndTime(char *string,unsigned long timestamp)
{
	if(!sysinfo.fileopen) return;
	if(sysinfo.neventstrings >= MAXEVENTSTRINGS){
		sysinfo.neventstrings = MAXEVENTSTRINGS;
		nlosteventstrings++;
		sprintf(tmpstring2,"LOST ES: %s at %s",string,FormatTS(timestamp,0));
		return;
	}
	sysinfo.eventstring[sysinfo.neventstrings].eventtime = timestamp;
	strcpy(sysinfo.eventstring[sysinfo.neventstrings].eventstring,string); 
	/*
	sprintf(tmpstring2,"ES[%d]: %s at %s",sysinfo.neventstrings,string,
		FormatTS(sysinfo.eventstring[sysinfo.neventstrings].eventtime,0));
	ErrorMessage(tmpstring2);
	*/
	sysinfo.neventstrings++;
}
/* save to disk */
void SaveEventString(void)
{
    int size = sizeof(long) + 2*sizeof(char) + sysinfo.eventstring_size * sizeof(char);
	sysinfo.neventstrings--;
	if(sysinfo.neventstrings < 0){
		sysinfo.neventstrings = 0;
		return;
	}
    /* don't allow the disk writes to be interrupted */
    DisableADIRQ;
    /* write out the data type */
    if(fwrite(&sysinfo.eventtype, sizeof(char), 1, adinfo.fp)
         != 1) {
        ErrorMessage("Disk write ERROR: possible disk full");
    }
    /* write out the timestamp */
    if(fwrite(&sysinfo.eventstring[sysinfo.neventstrings].eventtime, sizeof(long), 1, adinfo.fp)
         != 1) {
        ErrorMessage("Disk write ERROR: possible disk full");
    }
    /* write out the string */
    if(fwrite(sysinfo.eventstring[sysinfo.neventstrings].eventstring, 
		sizeof(char),
         sysinfo.eventstring_size, adinfo.fp) != sysinfo.eventstring_size){
        ErrorMessage("Disk write ERROR: possible disk full");
    }
    /* write out the data type */
    if(fwrite(&sysinfo.eventtype, sizeof(char), 1, adinfo.fp)
         != 1) {
        ErrorMessage("Disk write ERROR: possible disk full");
    }
	/* reset the timestamp to indicate successful save */
    sysinfo.diskfree -= size;
    sysinfo.filesize += size;
    EnableADIRQ;
}

void SaveRawData()
{
    long size = sizeof(long) + 2*sizeof(char) + adinfo.dma_bufsize * sizeof(int);
    /* don't allow the disk writes to be interrupted */
    DisableADIRQ;
    /* write out the data type */
    if(fwrite(&sysinfo.conttype, sizeof(char), 1, adinfo.fp)
         != 1) {
        ErrorMessage("Disk write ERROR: possible disk full");
    }
    /* write out the timestamp */
    if(fwrite(adinfo.timestamp + adinfo.process_buf, sizeof(long), 1, adinfo.fp)
         != 1) {
        ErrorMessage("Disk write ERROR: possible disk full");
    }
    /* write out the data */
    if(fwrite(adinfo.dataptr[adinfo.process_buf], sizeof(int),
         adinfo.dma_bufsize, adinfo.fp) != adinfo.dma_bufsize){
        ErrorMessage("Disk write ERROR: possible disk full");
    }
    /* write out the data type */
    if(fwrite(&sysinfo.conttype, sizeof(char), 1, adinfo.fp)
         != 1) {
        ErrorMessage("Disk write ERROR: possible disk full");
    }
    sysinfo.diskfree -= size;
    sysinfo.filesize += size;
    EnableADIRQ;
}

void SaveSpikeData(short e_num)
/* save spikes to disk */
{
    ElectrodeInfo    *electrode;
    long size;


    electrode = adinfo.electrode + e_num;
    
    if (electrode->nspikes > 0) {
        /* write out electrode->nspikes spike objects */
        DisableADIRQ;
        if (fwrite(electrode->spikebuf, sizeof(Spike), electrode->nspikes,
            adinfo.fp) != electrode->nspikes){ 
            ErrorMessage("Disk write ERROR: possible disk full");
        } 
        adinfo.disk_lost -= electrode->nspikes;
        size = sizeof(Spike) * electrode->nspikes + 2*sizeof(char);
        sysinfo.diskfree -= size;
        sysinfo.filesize += size;
        EnableADIRQ;
    }
}

void ToggleDisk(void)
{
    sysinfo.disk = !sysinfo.disk;
	if(sysinfo.disk){
		EventString("DISKON");
	} else {
		EventString("DISKOFF");
	}
    //DrawMenu();
	UpdateDiskButton();
}

void ToggleGraphics(void)
{
    sysinfo.graphics = !sysinfo.graphics;
    DrawMenu();
}

	
/*
** for display routines
*/
void PrepareStartAcq(void)
{
	if(sysinfo.tracker_enabled) return;
    sysinfo.acqbon = 1;
}

void StartAcq(void)
{

// don't do it if the tracker is active
	if(sysinfo.tracker_enabled) return;
// on DAS boards, two consecutive STARTACQs from the master cause
// a serious misalignment of the buffer, so don't startacq unless it is off
	if(sysinfo.acq) return;	

    sysinfo.acq = 1;
	EventString("STARTACQ");
    sysinfo.stopped = 0;
    sysinfo.xoffset = 0;
    ADResetClock();
    sysinfo.currenttime = ReadTS();
// hack to make sure buffers are initialized correctly
// see InitAcq for relevant code
    adinfo.next_buf = 1; 
    InitDMAChannels();
	
	//	ResetDMAChannels();
    if(!SetRate(adinfo.rate_req,&adinfo.rate_set)) 
		SystemExit(2,"Set Clock Error");
    //DisplayADStatus();
    InitAcq();
}


void StopAcq(void)
{
	if(sysinfo.tracker_enabled) return;
    /* to stop to acquisition, set the acq flag to zero and the next
    ** pass through the ISR will turn off DMA and halt acquisition.
    ** Note that this means that one must wait for it to be completed.
    */
    sysinfo.acq = 0;
    sysinfo.acqbon = 0; /* set so that the subsequent updateacqbutton works
						correctly */
#ifdef DAS-1800	
	BoardRestore();
#endif
#ifdef DT2821	
     while(!(adinfo.error || adinfo.dmadone));
#endif	 
    /* clear the error and dmadone status */
    adinfo.dmadone = 0;
    adinfo.error = 0;
    sysinfo.currenttime = ReadTS();
	EventString("STOPACQ");

//    stopall();

}

void TestAndStopAcq(void)
{
    if(sysinfo.acq){
        StopAcq();
    } else {
        sysinfo.currenttime = ReadTS();
    }
}

void DisableMouse(void)
{
    if(sysinfo.mouse){
#ifdef SMOUSE
        mouseexit();
#endif
        sysinfo.mouseactive = 0;
    }
}

void EnableMouse(void)
{
    if(sysinfo.mouse){
#ifdef SMOUSE
        mouseenter();
        mouseshow();
#endif
        sysinfo.mouseactive = 1;
    }
}

void ToggleAcq(void)
{
	if(sysinfo.tracker_enabled) return;
    //DrawMenu();
    if(!sysinfo.acq){
        /*
        ** turn off the mouse
        */
        DisableMouse();
        /*
#ifdef DT2821		
        outpw(SUPCSR,BIT0);
#endif
        */
        /* get the current time */
        sysinfo.currenttime = ReadTS();
        /* begin a/d */
	  	PrepareStartAcq();
		UpdateAcqButton();
        StartAcq();
        //ProgramDMAController();
    } else {
        /* stop acquisition */
        StopAcq();
		UpdateAcqButton();
        EnableMouse();
    }
}

void DiskOn(void)
{
    if(sysinfo.fileopen){
		sysinfo.disk = 1;
		EventString("DISKON");
	}
}

void DiskOff(void)
{
    sysinfo.disk = 0;
	EventString("DISKOFF");
}

void ToggleAutoscale(void)
{
    sysinfo.autoscale = !sysinfo.autoscale;
    DrawMenu();
}

void CheckButtons(void)
{
int    i;

    for(i=0;i<MAXBUTTONS;i++){
        if(buttonlist[i].active){
            // check for in button
            if(sysinfo.mousex > buttonlist[i].x &&
            sysinfo.mousex < buttonlist[i].x2 &&
            sysinfo.mousey > buttonlist[i].y &&
            sysinfo.mousey < buttonlist[i].y2){
                // check for entry
                if(!buttonlist[i].inbutton){
                          buttonlist[i].handler(i,BUTTONENTER);
                    // set the button presses
                    if(sysinfo.mousebut == 1){
                        buttonlist[i].button1down = 1;
                    } else {
                        buttonlist[i].button1down = 0;
                    }
                          if(sysinfo.mousebut == 2){
                        buttonlist[i].button2down = 1;
                    } else {
                        buttonlist[i].button2down = 0;
                    }
                }
                // check for button 1 release
                if(sysinfo.mousebut == 0 && buttonlist[i].button1down){
                          buttonlist[i].handler(i,BUTTON1RELEASE);
                }
                // check for button 2 release
                if(sysinfo.mousebut == 0 && buttonlist[i].button2down){
                          buttonlist[i].handler(i,BUTTON2RELEASE);
                }
                // check for button 1 press
                if(sysinfo.mousebut == 1 && !buttonlist[i].button1down){
                          buttonlist[i].handler(i,BUTTON1PRESS);
                }
                // check for button 2 press
                if(sysinfo.mousebut == 2 && !buttonlist[i].button2down){
                          buttonlist[i].handler(i,BUTTON2PRESS);
                }
                buttonlist[i].inbutton = 1;
                if(sysinfo.mousebut == 1){
                    buttonlist[i].button1down = 1;
                } else {
                    buttonlist[i].button1down = 0;
                }
                if(sysinfo.mousebut == 2){
                    buttonlist[i].button2down = 1;
                } else {
                    buttonlist[i].button2down = 0;
                }
            } else {
                // check for button exit
                if(buttonlist[i].inbutton){
                          buttonlist[i].handler(i,BUTTONEXIT);
                }
                buttonlist[i].inbutton = 0;
            }
        }
    }
}

void CheckMouse(void)
{

#ifdef SMOUSE
    mousestatus(&sysinfo.mousex,&sysinfo.mousey,&sysinfo.mousebut);
#endif
    sprintf(tmpstring,"M%d %d %d   ",sysinfo.mousex,sysinfo.mousey,
        sysinfo.mousebut);
    ErrorMessage(tmpstring);
    /*
    ** check the buttonlist
    */
    CheckButtons();
}

int LoadConfig(char *filename)
{
    FILE *infile;
    int i, e, echan;
    float gain;
    char tmp[80];
    char **header;
    char *p;

    if ((infile = fopen(filename, "r")) == NULL) {
        return 0;
    }

    header = ReadHeader(infile, &i);

    if ((p = GetHeaderParameter(header, "mode")) != NULL) 
        sysinfo.mode = atoi(p);
    if ((p = GetHeaderParameter(header, "rate")) != NULL) 
        adinfo.rate_req = (float) atof(p);
    if ((p = GetHeaderParameter(header, "nchannels:")) != NULL) 
        adinfo.nchannels = atoi(p);
    if ((p = GetHeaderParameter(header, "nelectrodes:")) != NULL) 
        adinfo.nelectrodes = atoi(p);
    if ((p = GetHeaderParameter(header, "nelect_chan:")) != NULL) 
        adinfo.nelect_chan = atoi(p);
    /* get the channel parameters */
    for (i = 0; i < adinfo.nchannels; i++) {
        e = i / adinfo.nelect_chan;
        echan = i % adinfo.nelect_chan;
        sprintf(tmp, "channel %d ampgain:", i);
        if ((p = GetHeaderParameter(header, tmp)) != NULL) 
            adinfo.channel[i].ampgain = GetAmpGain(atol(p));
        sprintf(tmp, "channel %d adgain:", i);
        if ((p = GetHeaderParameter(header, tmp)) != NULL) 
            adinfo.channel[i].adgain = atoi(p);
        sprintf(tmp, "channel %d filter:", i);
        if ((p = GetHeaderParameter(header, tmp)) != NULL) 
            adinfo.channel[i].filter = atoi(p);
        sprintf(tmp, "channel %d offset:", i);
        if ((p = GetHeaderParameter(header, tmp)) != NULL) 
            adinfo.channel[i].contoffset = atoi(p);
        sprintf(tmp, "channel %d contscale:", i);
        if ((p = GetHeaderParameter(header, tmp)) != NULL) 
            adinfo.channel[i].contwindowscale = atoi(p);
        sprintf(tmp, "channel %d color:", i);
        if ((p = GetHeaderParameter(header, tmp)) != NULL) 
            adinfo.channel[i].color = atoi(p);
        sprintf(tmp, "channel %d threshold:", i);
        if ((p = GetHeaderParameter(header, tmp)) != NULL) 
            adinfo.electrode[e].thresh[echan] = atoi(p);
    }
	/*
	** load event strings
	*/
	for(i=0;i<MAXEVENTKEYS;i++){
        sprintf(tmp, "EventString %d:", i);
        if ((p = GetHeaderParameter(header, tmp)) != NULL) {
			 strncpy(sysinfo.eventkey[i].eventstring,p,EVENTSTRINGSIZE-1);
		}
	}	
	/*
	** load config filenames
	*/
	for(i=0;i<MAXFASTCONFIG;i++){
        sprintf(tmp, "ConfigFile %d:", i);
        if ((p = GetHeaderParameter(header, tmp)) != NULL) {
			 strncpy(sysinfo.configfilename[i],p,MAXFILENAME-1);
		}
	}	
	return(1);
}

void LoadAndUpdateConfig(char *filename)	
{
	   if ((*filename != '\0') && (LoadConfig(filename))) {
		CreateMenu();
		DisplaySetup();
		SetRate(adinfo.rate_req, &adinfo.rate_set);
		/* reset the conversion duration */
		adinfo.conversion_duration = SECOND*adinfo.dma_bufsize
									 / adinfo.rate_set;
		InitDTPIO();
		SetAmpControls();
		ClearScreen();
		RefreshDisplay();
		DisplayStartupStatus();
		sprintf(tmpstring2, "Configuration file '%s' loaded", 
				 filename);
		ErrorMessage(tmpstring2);
	}
	else {
		sprintf(tmpstring2, "Error opening '%s' for reading", 
				 filename);
		ErrorMessage(tmpstring2);
	 }
}		

int SaveConfig(char *filename)
{
    FILE *outfile;
    int i, e, echan;
    float gain;
    char tmp[80];

    if ((outfile = fopen(filename, "w")) == NULL) {
        return 0;
    }

    fprintf(outfile, "%s\n", MAGIC_SOH_STR);
    fprintf(outfile, "%% mode: %d\n", sysinfo.mode);
    fprintf(outfile, "%% rate: \t\t%f\n", adinfo.rate_set);
    fprintf(outfile, "%% nelectrodes: %d\n", adinfo.nelectrodes);
    fprintf(outfile, "%% nchannels: \t\t%d\n", adinfo.nchannels);
    fprintf(outfile, "%% nelect_chan: \t\t%d\n", adinfo.nelect_chan);
    for (i = 0; i < adinfo.nchannels; i++) {
        e = i / adinfo.nelect_chan;
        echan = i % adinfo.nelect_chan;
        fprintf(outfile, "%% channel %d ampgain: \t%ld\n", i, 
                GetActualGain(adinfo.channel[i].ampgain));
        fprintf(outfile, "%% channel %d adgain: \t%d\n", i, 
                adinfo.channel[i].adgain);
        fprintf(outfile, "%% channel %d filter: \t%d\n", i,
                  adinfo.channel[i].filter);
        fprintf(outfile, "%% channel %d threshold: \t%d\n", i,
                        adinfo.electrode[e].thresh[echan]);
        fprintf(outfile, "%% channel %d color: \t\t%d\n", i,
                        adinfo.channel[i].color);
        fprintf(outfile, "%% channel %d offset: \t%d\n", i,
                        adinfo.channel[i].contoffset);
        fprintf(outfile, "%% channel %d contscale: \t%d\n", i,
                        adinfo.channel[i].contwindowscale);
    }
	/*
	** save event strings
	*/
	for(i=0;i<MAXEVENTKEYS;i++){
		fprintf(outfile,"%% EventString %d:\t%s\n",i,
			sysinfo.eventkey[i].eventstring);
	}	
	/*
	** save config filenames
	*/
	for(i=0;i<MAXFASTCONFIG;i++){
		fprintf(outfile,"%% ConfigFile %d:\t%s\n",i,
			sysinfo.configfilename[i]);
	}	
    fprintf(outfile, "%s\n", MAGIC_EOH_STR);
    fclose(outfile);
    return 1;
}

char **ReadHeader(FILE *fp,int *headersize)
{
int    hasheader;
char    line[MAXLINE];
long    start;
char    **header_contents;
char    **new_header_contents;
int    nheaderlines;
int    done;
int    status;
int    eol;

    if(fp == NULL) return(NULL);
    if(headersize == NULL) return(NULL);
    hasheader = 1;
    nheaderlines = 0;
    header_contents = NULL;
    done = 0;
    /*
    ** determine the starting file position
    */
    start = ftell(fp);
    /*
    ** look for the magic start-of-header string
    */
    if(fread(line,sizeof(char),MAGIC_SOH_STRSIZE,fp) != MAGIC_SOH_STRSIZE){
    /*
    ** unable to read the header
    */
    hasheader = 0;
    } else {
    /*
    ** null terminate the string
    */
    line[MAGIC_SOH_STRSIZE-1] = '\0';
    /*
    ** is it the magic start of header string?
    */
    if((status = strcmp(line,MAGIC_SOH_STR)) != 0){
        /*
        ** not the magic string
        */
        hasheader = 0;
    } 
    }
    if(!hasheader){
    /*
    ** no header was found so reset the file position to its starting
    ** location
    */
    fseek(fp,start,0L);
    } else
    /*
    ** read the header
    */
    while(!done && !feof(fp)){    
    /*
    ** read in a line from the header
    */
    if(fgets(line,MAXLINE,fp) == NULL){
        /*
        ** unable to read the header
        */
        fprintf(stderr,"ERROR in file header.");
		*headersize = 0;
		return(NULL);
    }
    /*
    ** zap the CR
    */
    if((eol = strlen(line)-1) >= 0){
        line[eol] = '\0';
    }
    /*
    ** look for the magic end-of-header string
    */
    if(strcmp(line,MAGIC_EOH_STR) == 0){
        /*
        ** done
        */
        done = 1;
    } else {
        /*
        ** add the string to the list of header contents
        ** by reallocating space for the header list
        ** (dont forget the NULL entry at the end of
        ** the list)
        */
        if(header_contents == NULL){
        if((header_contents = (char **)malloc(sizeof(char *)*2)) ==
        NULL){
            fprintf(stderr,"initial malloc failed. Out of memory\n");
            break;
        }
        } else {
        if((new_header_contents = (char **)calloc(
        nheaderlines+2,sizeof(char *))) == NULL){
            fprintf(stderr,"realloc failed. Out of memory\n");
            break;
        }
        /*
        ** copy the previous contents
        */
        memcpy(new_header_contents,header_contents,sizeof(char
        *)*(nheaderlines +1));
        /*
        ** and free the old stuff
        */
        free(header_contents);
        /*
        ** and reassign to the new stuff
        */
        header_contents = new_header_contents;
#ifdef OLD
        if((header_contents = (char **)realloc(header_contents,
        sizeof(char *)*(nheaderlines+2))) == NULL){
            fprintf(stderr,"realloc failed. Out of memory\n");
            break;
        }
#endif
        }
        if((header_contents[nheaderlines] = 
        (char *)malloc((strlen(line)+1)*sizeof(char))) == NULL){
            fprintf(stderr,"malloc failed. Out of memory\n");
            break;
        }
        strcpy(header_contents[nheaderlines],line);
        header_contents[nheaderlines+1] = NULL;
        nheaderlines++;
    }
    }
    /*
    ** report the headersize by comparing the current position with
    ** the starting position
    */
    *headersize = ftell(fp) - start;
    return(header_contents);
}

void DisplayHeader(fp,header,headersize)
FILE    *fp;
char    **header;
long    headersize;
{
int    i;

    if(fp == NULL) return;
    if(header != NULL){
    fprintf(fp,"Header size: \t%d bytes\n",headersize);
    fprintf(fp,"Header contents:\n");
    i = 0;
    while(header[i] != NULL){
        fprintf(fp,"\t%s\n",header[i++]);
    }
    } else {
    fprintf(fp,"No header\n");
    }
}

/*
** returns the string value of a parameter imbedded in the header
*/
char *GetHeaderParameter(header,parm)
char    **header;
char    *parm;
{
int    i;
char    *value;
char    *ptr;

    value = NULL;
    if(header != NULL){
    /*
    ** go through each line of the header
    */
    for(i=0;header[i] != NULL;i++){
        /*
        ** search for the parameter string which must start on the
        ** third character of the line
        */
        if(strlen(header[i]) < 3) continue;
        /*
        ** does it match
        */
        if(strncmp(header[i]+2,parm,strlen(parm)) == 0){
        /*
        ** now return the value which begins following
        ** the whitespace at the end of the parameter name
        */
        for(value=header[i]+2+strlen(parm)+1;value,*value!='\0';value++){
            /*
            ** skip white space
            */
            if(*value != ' ' && *value != '\t' && 
            *value != '\n'){
            /*
            ** found the value and return it
            */
            return(value);
            }
        }
        }
    }
    } 
    return(value);
}


void WriteHeader()
{
    int i, e, echan;

    fprintf(adinfo.fp, "%s\n", MAGIC_SOH_STR);
    if (sysinfo.mode == SPIKE_MODE) {
        fprintf(adinfo.fp, "%% mode: SPIKE\n");
    }
    else 
    if (sysinfo.mode == CONTINUOUS_MODE) {
            fprintf(adinfo.fp, "%% mode: CONTINUOUS\n");
    } else
    if (sysinfo.mode == TRACKER_MODE) {
            fprintf(adinfo.fp, "%% mode: TRACKER\n");
	}
    fprintf(adinfo.fp, "%% adversion: \t\t%s\n", ADVERSION);
    fprintf(adinfo.fp, "%% rate: \t\t%f\n", adinfo.rate_set);
    fprintf(adinfo.fp, "%% nelectrodes: %d\n", adinfo.nelectrodes);
    fprintf(adinfo.fp, "%% nchannels: \t\t%d\n", adinfo.nchannels);
    fprintf(adinfo.fp, "%% nelect_chan: \t\t%d\n", adinfo.nelect_chan);
    fprintf(adinfo.fp, "%% errors: \t\t%ld\n", adinfo.error_count);
    fprintf(adinfo.fp, "%% disk_errors: \t\t%ld\n", adinfo.disk_lost);
    fprintf(adinfo.fp, "%% dma_bufsize: \t\t%d\n", adinfo.dma_bufsize);
    fprintf(adinfo.fp, "%% spikelen: \t\t%d\n", SINGLESPIKELEN);
    fprintf(adinfo.fp, "%% spikesep: \t\t%d\n", adinfo.spikesep / adinfo.nchannels);
    for (i = 0; i < adinfo.nchannels; i++) {
        e = i / adinfo.nelect_chan;
        echan = i % adinfo.nelect_chan;
        fprintf(adinfo.fp, "%% channel %d ampgain: \t%ld\n", i, 
                GetActualGain(adinfo.channel[i].ampgain));
        fprintf(adinfo.fp, "%% channel %d adgain: \t%d\n", i, 
                adinfo.channel[i].adgain);
        fprintf(adinfo.fp, "%% channel %d filter: \t%d\n", i,
                  adinfo.channel[i].filter);
        fprintf(adinfo.fp, "%% channel %d threshold: \t%d\n", i,
                        adinfo.electrode[e].thresh[echan]);
        fprintf(adinfo.fp, "%% channel %d color: \t\t%d\n", i,
                        adinfo.channel[i].color);
        fprintf(adinfo.fp, "%% channel %d offset: \t%d\n", i,
                        adinfo.channel[i].contoffset);
        fprintf(adinfo.fp, "%% channel %d contscale: \t%d\n", i,
                        adinfo.channel[i].contwindowscale);
    }
    if (sysinfo.mode == SPIKE_MODE) {
        fprintf(adinfo.fp, "%% spike_size: \t\t%d\n", sizeof(Spike));
        fprintf(adinfo.fp, "%% fields: int electrode_num; long timestamp; int data[128]\n");
    }
    else {
        fprintf(adinfo.fp, "%% buffer_size: \t\t%d\n", sizeof(long) + 
                adinfo.dma_bufsize);
        fprintf(adinfo.fp, "%% fields: long timestamp; int data[%d]\n", 
                adinfo.dma_bufsize);
    }
	/* if this is a tracker machine, write out the tracker rate */
	if (sysinfo.tracker_enabled) {
		fprintf(adinfo.fp, "%% tracker_rate: \t\t%d\n", tracker.rate);
	}	
    fprintf(adinfo.fp, "%s\n", MAGIC_EOH_STR);
}

void SetThreshold(int channel,int thresh)
{
int e_num, e_channel;

    if(channel < 0) return;
    e_num = channel / adinfo.nelect_chan;
    e_channel = channel % adinfo.nelect_chan;
    if ((thresh > -2049) && (thresh < 2049)) {
        adinfo.electrode[e_num].thresh[e_channel] = thresh;
	adinfo.channel[channel].thresh = thresh;
	if (sysinfo.mode == SPIKE_MODE) {
	    ClearSpikeBox(e_num, e_channel);
	    DrawChannelButton(channel);
	    DrawSpikeBorder(e_num, e_channel);
	} else 
	if (sysinfo.mode == CONTINUOUS_MODE) {
	    SetContWindowScale(adinfo.channel + channel);
	    DrawChannelButton(channel);
	}
    }
}

void SetElectrodeThresholds(int e_num,int thresh)
{
int    i;
int	startch;
int	endch;

    startch = e_num*adinfo.nelect_chan;
    endch = startch + adinfo.nelect_chan;
    if(endch > NAMP_CHANNELS) return;
    sprintf(tmpstring,"MTHRESH %d-%d %d",startch,endch-1,thresh);
    EventString(tmpstring);
    for(i=startch;i<endch;i++){
        SetThreshold(i,thresh);
    }
}

void SetAllThresholds(int thresh)
{
int    i;

    sprintf(tmpstring,"MTHRESH %d-%d %d",0,adinfo.nchannels-1,thresh);
    EventString(tmpstring);
    for(i=0;i<adinfo.nchannels;i++){
        SetThreshold(i,thresh);
    }
}



void ProcessString(void)
/* process sysinfo.inputstring */
{
    int thresh;
    int e_num;

    if (sysinfo.inputstring[0] == 't') {
        /* set the threshold of the currently selected channel */ 
        thresh = atoi(sysinfo.inputstring+1);
        SetThreshold(sysinfo.selected_channel,thresh);
	sprintf(tmpstring,"THRESH %d %d",sysinfo.selected_channel,thresh);
	EventString(tmpstring);
	if (sysinfo.mode == CONTINUOUS_MODE) {
	    DrawContinuousTicks();
	}
    }
    if (sysinfo.inputstring[0] == '\024') {
        /* set the threshold of all channels */ 
        thresh = atoi(sysinfo.inputstring+1);
	e_num = sysinfo.selected_spbutton / 2;
	if (sysinfo.mode == SPIKE_MODE) {
	    SetElectrodeThresholds(e_num,thresh);
	} else
	if (sysinfo.mode == CONTINUOUS_MODE) {
	    SetAllThresholds(thresh);
	    DrawContinuousTicks();
	}
    }
    /* set the string position back to the first character */
    sysinfo.stringpos = 0;
    sysinfo.inputstring[sysinfo.stringpos] = 0;
}

void InputMessage(char *string)
{
    setcolor(_WHITE);
    outtextxy(sysinfo.inputview.x, sysinfo.inputview.y, string);
}
        
void ReadInputString(char *outputstr, char *inputstr)
{
    int length, xpos, theight, twidth;
    char c, *str;

    length = strlen(outputstr);
    twidth = textwidth(outputstr);
    sysinfo.stringpos = 0;

    str = inputstr;
    *str = '\0';
    setcolor(_WHITE);
    outtextxy(sysinfo.inputview.x, sysinfo.inputview.y, outputstr);

    while ((c = getch()) != 13) {
        /* while the enter key is not hit */
        if (c == '\010') {    // backspace key
            if (str != inputstr) {
                /* erase the input string and move the pointer back one */
                setcolor(_BLACK);
                outtextxy(sysinfo.inputview.x + twidth, sysinfo.inputview.y, inputstr);
                str--;
                *str = '\0';
            }
        }
        else { 
            *str = c;
            str++;
            *str = '\0';
        }
        setcolor(_WHITE);
        outtextxy(sysinfo.inputview.x + twidth, sysinfo.inputview.y, inputstr);
    }
    /* erase the strings */
    setcolor(_BLACK);
    outtextxy(sysinfo.inputview.x, sysinfo.inputview.y, outputstr);
    outtextxy(sysinfo.inputview.x + twidth, sysinfo.inputview.y, inputstr);

    return;
}

void ADCloseFile(void)
{
    /* close the input file */
    if(adinfo.fp){
        fclose(adinfo.fp);
    }
    sysinfo.fileopen = 0;
    strcpy(adinfo.fname, NO_FILE_STRING);
    adinfo.disk_lost = 0;
    sysinfo.filesize = 0;
    //sysinfo.display_lost = 0;
    //adinfo.nspikes = 0;
    DrawStrings();
}

void UpdateChannelSelected(void)
{
	sysinfo.channel_selected = 1;
	switch (sysinfo.selected_button) {
	case 0:
	case 1:
	    /* one of first two spike/projection buttons */
	    sysinfo.channel_selected = 0;
	    sysinfo.selected_spbutton = sysinfo.selected_button;
	    break;
	case 2:
	case 3:
	case 4:
	case 5:
	    /* one of the first four channel buttons */
	    sysinfo.selected_channel = sysinfo.selected_button -
				       2; 
	    break;
	case 6:
	case 7:
	    /* one of first two spike/projection buttons */
	    sysinfo.channel_selected = 0;
	    sysinfo.selected_spbutton = sysinfo.selected_button                                                         - 4;
	    break;
	case 8:
	case 9:
	case 10:
	case 11:
	    /* one of the first four channel buttons */
	    sysinfo.selected_channel = sysinfo.selected_button -
					4;
	    break;
	} 
}

void GetStimArea(View *area)
{
	ReadInputString("left x-coord, in % of tracker area: ", 
		tmpstring);
	while (((area->x = atoi(tmpstring)) < 0) || (area->x > 100)) {
	/* the user typed in an invalid number, reset */
		sprintf(tmpstring2, "Error: %d out of acceptable range", area->x);
		ErrorMessage(tmpstring2);
		ReadInputString("left x-coord, in % of tracker area: ", 
			tmpstring);
		}
	ReadInputString("right x-coord, in % of tracker area: ", 
		tmpstring);
	while (((area->x2 = atoi(tmpstring)) < area->x) || (area->x2 > 100)) {
	/* the user typed in an invalid number, reset */
		sprintf(tmpstring2, "Error: %d out of acceptable range", area->x2);
		ErrorMessage(tmpstring2);
		ReadInputString("right x-coord, in % of tracker area: ", 
			tmpstring);
		}
		ReadInputString("left y-coord, in % of tracker area: ", 
		tmpstring);
	while (((area->y = atoi(tmpstring)) < 0) || (area->y > 100)) {
	/* the user typed in an invalid number, reset */
		sprintf(tmpstring2, "Error: %d out of acceptable range", area->y);
		ErrorMessage(tmpstring2);
		ReadInputString("left y-coord, in % of tracker area: ", 
			tmpstring);
		}
	ReadInputString("right y-coord, in % of tracker area: ", 
		tmpstring);
	while (((area->y2 = atoi(tmpstring)) < area->y) || (area->y2 > 100)) {
	/* the user typed in an invalid number, reset */
		sprintf(tmpstring2, "Error: %d out of acceptable range", area->y2);
		ErrorMessage(tmpstring2);
		ReadInputString("right y-coord, in % of tracker area: ", 
			tmpstring);
	}
 }
	

void CheckKeyboard(void)
{
unsigned int gain;
ChannelInfo *cptr;
short    extended;
short    tmpval, tmpval2;
short    e_num, channel, ce_num, spe_num, echan, espbutton;
short     restoreacq;
int 		i, n, nflag;
float 		d;
FILE		*fp;
int		done;
int     *x, *y;


    if(_kbhit()){
    	kbchar = _getch();
	    if (sysinfo.helpon) {
	    /* turn help off */
		sysinfo.helpon = 0;
		ClearHelpWindow();
		if (sysinfo.mode == SPIKE_MODE) {
		    /* redraw the spike and projection boxes */
		    for (tmpval = 0; tmpval < adinfo.nelectrodes;tmpval++) 
			DrawSpikeBorders(tmpval);
		    DrawSPButtons();
		}
		if (sysinfo.mode == CONTINUOUS_MODE) {
		    /* continuous mode */
		    DrawScopeBorder();
		    DrawContinuousTicks();
		} else
		if (sysinfo.mode == TRACKER_MODE) {
		    DrawTrackerBorder();
		}
		DrawChannelButtons();
	    } else {
		sysinfo.kbhandled = 1;
		if(!kbchar){
		    /*
		    ** extended character
		    */
		    extended = 1;
		    kbchar = _getch();
		    //sprintf(tmpstring,"E%X %d %c   ",kbchar,kbchar,kbchar);
		    //ErrorMessage(tmpstring);
		} else {
		    extended = 0;
		    /*
		    ** check for the numlock
		    */
		    if( !sysinfo.stringinput &&
		    (_bios_keybrd(_KEYBRD_SHIFTSTATUS) & 0x20)){
			/*
			** remap the numlocked keys to their extended
			** equivalents
			*/
			switch(kbchar){
			case '7':
			    extended = 1;
			    kbchar = 'G';
			    break;
			case '1':
			    extended = 1;
			    kbchar = 'O';
			    break;
			case '6':
			    extended = 1;
			    kbchar = 'M';
			    break;
			case '4':
			    extended = 1;
			    kbchar = 'K';
			    break;
			case '8':
			    extended = 1;
			    kbchar = 'H';
			    break;
			case '2':
			    extended = 1;
			    kbchar = 'P';
			    break;
			}
		    }
		    //sprintf(tmpstring,"%X %d %c   ",kbchar,kbchar,kbchar);
		    //ErrorMessage(tmpstring);
		}
		if (sysinfo.stringinput) {
		    switch(kbchar){
			case 13: // enter key 
			    ProcessString();
			    sysinfo.stringinput = 0;            
			    break;
			case '\010':    // backspace key
			    sysinfo.stringpos--;
			    sysinfo.inputstring[sysinfo.stringpos] = '\000';
			    break;
			default:
			    sysinfo.inputstring[sysinfo.stringpos] = kbchar;
			    sysinfo.stringpos++;
			    sysinfo.inputstring[sysinfo.stringpos] = '\000';
			    break;
		    }
		    /* erase the old string */
		    drwfillbox(SET, _BLACK, sysinfo.inputview.x, 
			sysinfo.inputview.y,sysinfo.inputview.x2, 
			sysinfo.inputview.y2);
		    /* draw the new string */
		    setcolor(_WHITE);
		    outtextxy(sysinfo.inputview.x, sysinfo.inputview.y, 
				  sysinfo.inputstring);
		} else {
		    if(extended){
			switch(kbchar){
			case 120:        /* Alt-1 */
			    ReadInputString("Light Seq Interval (sec): ", 
			    tmpstring);
			    sysinfo.seq[LIGHTSEQON].interval = 1e4*atof(tmpstring);
			    sysinfo.seq[LIGHTSEQON].active = 1;
			    sysinfo.seq[LIGHTSEQON].time = ReadTS();
			    break;
			case 121:        /* Alt-2 */
			    sysinfo.seq[LIGHTSEQON].active = 0;
			    break;
			case 122:        /* Alt-3 */
			    sysinfo.seq[CHECKBARSEQ].active = !sysinfo.seq[CHECKBARSEQ].active;
			    sysinfo.seq[CHECKBARSEQ].time = ReadTS();
			    break;
			case ';':        /* extended: F1 */
			    EventString(sysinfo.eventkey[0].eventstring);
			    break;
			case '<':        /* extended: F2 */
			    EventString(sysinfo.eventkey[1].eventstring);
			    break;
			case '=':        /* extended: F3 */
			    EventString(sysinfo.eventkey[2].eventstring);
			    break;
			case '>':        /* extended: F4 */
			    EventString(sysinfo.eventkey[3].eventstring);
			    break;
			case '?':        /* extended: F5 */
			    EventString(sysinfo.eventkey[4].eventstring);
			    break;
			case '@':        /* extended: F6 */
			    EventString(sysinfo.eventkey[5].eventstring);
			    break;
			case 'A':        /* extended: F7 */
			    EventString(sysinfo.eventkey[6].eventstring);
			    break;
			case 'B':        /* extended: F8 */
			    EventString(sysinfo.eventkey[7].eventstring);
			    break;
			case 'C':        /* extended: F9 */
			    if (!sysinfo.disk) {
				ReadInputString("Epoch ID: ", 
				    sysinfo.epochstring);
				DrawSessionString();
				sprintf(tmpstring,
					"EPOCH %s", sysinfo.epochstring);
				sysinfo.epochtime = ReadTS();
				EventString(tmpstring);
				/*
				LoadAndUpdateConfig(sysinfo.configfilename[0]);	
				*/
			    }
			    break;
			case 'D':        /* extended: F10 */
			    if (!sysinfo.disk) {
				ReadInputString("Session ID: ", 
				     sysinfo.sessionstring);
				DrawSessionString();
				sprintf(tmpstring,
				    "SESSION %s", sysinfo.sessionstring);
				EventString(tmpstring);
				/* LoadAndUpdateConfig(sysinfo.configfilename[1]); */
			    }
			    break;
			case 'E':        /* extended: F11 */
			    LoadAndUpdateConfig(sysinfo.configfilename[2]);	
			    break;
			case 'F':        /* extended: F12 */
			    break;
			case 'h':        /* extended: Alt F1 */
			    if (!sysinfo.disk) {
				ReadInputString("Enter F1 event string: ", 
				    sysinfo.eventkey[0].eventstring);
			     }
			    break;
			case 'i':        /* extended: Alt F2 */
			    if (!sysinfo.disk) {
				ReadInputString("Enter F2 event string: ", 
				    sysinfo.eventkey[1].eventstring);
			    }
			    break;
			case 'j':        /* extended: Alt F3 */
				if (!sysinfo.disk) {
				ReadInputString("Enter F3 event string: ", 
				 sysinfo.eventkey[2].eventstring);
				 }
				break;
			case 'k':        /* extended: Alt F4 */
				if (!sysinfo.disk) {
				ReadInputString("Enter F4 event string: ", 
				 sysinfo.eventkey[3].eventstring);
				 }
				break;
			case 'l':        /* extended: Alt F5 */
			    if (!sysinfo.disk) {
				ReadInputString("Enter F5 event string: ", 
				 sysinfo.eventkey[4].eventstring);
			    }
			    break;
			case 'm':        /* extended: Alt F6 */
			    if (!sysinfo.disk) {
				ReadInputString("Enter F6 event string: ", 
				 sysinfo.eventkey[5].eventstring);
			    }
			    break;
			case 'n':        /* extended: Alt F7 */
			    if (!sysinfo.disk) {
				ReadInputString("Enter F7 event string: ", 
				 sysinfo.eventkey[6].eventstring);
			    }
			    break;
			case 'o':        /* extended: Alt F8 */
			    if (!sysinfo.disk) {
				ReadInputString("Enter F8 event string: ", 
				 sysinfo.eventkey[7].eventstring);
			    }
			    break;
			case 'p':        /* extended: Alt F9 */
			/*
			    ReadInputString("Enter config filename to assign to F9: ", 
			     tmpstring);
			     strncpy(sysinfo.configfilename[0],tmpstring,MAXFILENAME);
			 */
			    break;
			case 'q':        /* extended: Alt F10 */
			/*
			    ReadInputString("Enter config filename to assign to F10: ", 
			     tmpstring);
			     strncpy(sysinfo.configfilename[1],tmpstring,MAXFILENAME);
			 */
			     break;
			case 'r':        /* extended: Alt F11 */
			     ReadInputString("Enter config filename to assign to F11: ", 
				tmpstring);
			     strncpy(sysinfo.configfilename[2],tmpstring,MAXFILENAME);
			     break;
			case 's':        /* extended: Alt F12 */
				ReadInputString("Enter config filename to assign to F12: ", 
				 tmpstring);
				 strncpy(sysinfo.configfilename[3],tmpstring,MAXFILENAME);
				break;
			case 'G':        /* extended: home */
			    if(sysinfo.mode == SPIKE_MODE){
				/* select projection 0 */
				UnHighlightButton(sysinfo.selected_button + 
					sysinfo.spikebuttonbase);
				sysinfo.selected_button = 1;
				/* one of first two spike/projection buttons */
				sysinfo.channel_selected = 0;
				sysinfo.selected_spbutton = sysinfo.selected_button;
				DrawSPButton(sysinfo.selected_spbutton);
			    } else 
			    if(sysinfo.mode == CONTINUOUS_MODE){
				/* change the color of the current trace */
				cptr = adinfo.channel+ sysinfo.selected_channel;
				cptr->color = (cptr->color + 1) % MaxColors; 
				DrawChannelButton(sysinfo.selected_channel);
			    }
			    break;
			case 'O':        /* extended: end */
			    if(sysinfo.mode == SPIKE_MODE){
				/* select projection 1 */
				UnHighlightButton(sysinfo.selected_button + 
					sysinfo.spikebuttonbase);
				sysinfo.selected_button = 7;
				sysinfo.channel_selected = 0;
				sysinfo.selected_spbutton = sysinfo.selected_button                                                         - 4;
				DrawSPButton(sysinfo.selected_spbutton);
			    } else 
			    if(sysinfo.mode == CONTINUOUS_MODE){
				/* change the color of the current trace */
				cptr = adinfo.channel + sysinfo.selected_channel;
				cptr->color = cptr->color > 0 ? cptr->color - 1 :
							  MaxColors - 1; 
				DrawChannelButton(sysinfo.selected_channel);
			    }
			    break;
			case 'I':        /* extended: pgup */
				SetAmpGain(sysinfo.selected_channel,
				GetActualGain(adinfo.channel[sysinfo.selected_channel].ampgain)*2);
				DrawChannelButtons();
				break;
			case 'Q':        /* extended: pgdown */
				SetAmpGain(sysinfo.selected_channel,
				GetActualGain(adinfo.channel[sysinfo.selected_channel].ampgain)/2);
				DrawChannelButtons();
				break;
			case 0x84:        /* extended: CTRL pgup */
				if(sysinfo.mode == SPIKE_MODE){
					SetElectrodeAmpGains(sysinfo.selected_spbutton / 2,
					GetActualGain(adinfo.channel[sysinfo.selected_channel].ampgain)*2);
				} else
				if(sysinfo.mode == CONTINUOUS_MODE){
					SetAmpGains(
					GetActualGain(adinfo.channel[sysinfo.selected_channel].ampgain)*2);
				}
				DrawChannelButtons();
				break;
			case 0x76:        /* extended: CTRL pgdown */
				if(sysinfo.mode == SPIKE_MODE){
					SetElectrodeAmpGains(sysinfo.selected_spbutton / 2,
					GetActualGain(adinfo.channel[sysinfo.selected_channel].ampgain)/2);
				} else
				if(sysinfo.mode == CONTINUOUS_MODE){
					SetAmpGains(
					GetActualGain(adinfo.channel[sysinfo.selected_channel].ampgain)/2);
				}
				DrawChannelButtons();
				break;
			case 'H':        /* extended: up arrow */
				if (sysinfo.mode == CONTINUOUS_MODE) {
					cptr = adinfo.channel + sysinfo.selected_channel;
					cptr->contoffset -= 10;
					cptr->contscaletick -= 10;
					if (cptr->contoffset < sysinfo.scopeview.y - 1) {
						/* the trace would be off screen, so reset the 
							offset and scale tick */
						cptr->contoffset += 10;
						cptr->contscaletick += 10;
					}
					DrawChannelButton(sysinfo.selected_channel);
					 DrawContinuousTicks();
				}
				break;
			case 'P':        /* extended: down arrow */
				if (sysinfo.mode == CONTINUOUS_MODE) {
					cptr = adinfo.channel + sysinfo.selected_channel;
					cptr->contoffset += 10;
					cptr->contscaletick += 10;
					if (cptr->contoffset > sysinfo.scopeview.y2 + 1) {
						/* the trace would be off screen, so reset the 
							offset and scale tick */
						cptr->contoffset -= 10;
						cptr->contscaletick -= 10;
					}
					DrawChannelButton(sysinfo.selected_channel);
					 DrawContinuousTicks();
				}
				break;
			case 'M':        /* extended: right arrow */
			    tmpval = sysinfo.selected_button;
			    sysinfo.selected_button++;
			    if (sysinfo.mode == SPIKE_MODE) {
				if (sysinfo.selected_button >= sysinfo.nbuttons)
					sysinfo.selected_button = 0;
				UnHighlightButton(tmpval + sysinfo.spikebuttonbase);
				/* figure out which button is now selected */
				/* for the sake of speed, the button numbers are 
					hard coded */
				/* set the selected spbutton to 0 if a button on
				   the first electrode is selected, and 1 if a button 
				   on the second electrode is selected */
				sysinfo.selected_spbutton = 2 * (sysinfo.selected_button 
											/ (sysinfo.nbuttons / 
											adinfo.nelectrodes));
				sysinfo.channel_selected = 1;
				switch (sysinfo.selected_button) {
				case 0:
				case 1:
				    /* one of first two spike/projection buttons */
				    sysinfo.channel_selected = 0;
				    sysinfo.selected_spbutton = sysinfo.selected_button;
				    DrawSPButton(sysinfo.selected_spbutton);
				    break;
				case 2:
				case 3:
				case 4:
				case 5:
				    /* one of the first four channel buttons */
				    sysinfo.selected_channel = sysinfo.selected_button -
										       2; 
				    DrawChannelButton(sysinfo.selected_channel);
				    break;
				case 6:
				case 7:
				    /* one of first two spike/projection buttons */
				    sysinfo.channel_selected = 0;
				    sysinfo.selected_spbutton = sysinfo.selected_button                                                         - 4;
				    DrawSPButton(sysinfo.selected_spbutton);
				    break;
				case 8:
				case 9:
				case 10:
				case 11:
				    /* one of the first four channel buttons */
				    sysinfo.selected_channel = sysinfo.selected_button -
											    4;
				    DrawChannelButton(sysinfo.selected_channel);
				    break;
				} 
			    }
			    if (sysinfo.mode == CONTINUOUS_MODE) {
				/* only channel buttons in continuous mode */
				if (sysinfo.selected_button >= adinfo.nchannels)
					sysinfo.selected_button = 0;
				UnHighlightButton(tmpval + sysinfo.contbuttonbase);
				sysinfo.selected_channel = sysinfo.selected_button;
				DrawChannelButton(sysinfo.selected_channel);
			    }
			    break;
			case 'K':        /* extended: left arrow */
			    tmpval = sysinfo.selected_button;
			    sysinfo.selected_button--;
			    if (sysinfo.mode == SPIKE_MODE) {
				if (sysinfo.selected_button < 0)
				    sysinfo.selected_button = sysinfo.nbuttons - 1;
				UnHighlightButton(tmpval + sysinfo.spikebuttonbase);
				/* figure out which button is now selected */
				/* for the sake of speed, the button numbers are 
					hard coded */
				/* set the selected spbutton to 0 if a button on
				   the first electrode is selected, and 1 if a button 
				   on the second electrode is selected */
				sysinfo.selected_spbutton = 2 * (sysinfo.selected_button 
											/ (sysinfo.nbuttons / 
											adinfo.nelectrodes));
				sysinfo.channel_selected = 1;
				switch (sysinfo.selected_button) {
				case 0:
				case 1:
					/* one of first two spike/projection buttons */
					sysinfo.channel_selected = 0;
					sysinfo.selected_spbutton = sysinfo.selected_button;
					DrawSPButton(sysinfo.selected_spbutton);
					break;
				case 2:
				case 3:
				case 4:
				case 5:
				    /* one of the first four channel buttons */
				    sysinfo.selected_channel = sysinfo.selected_button -
										       2; 
				    DrawChannelButton(sysinfo.selected_channel);
				    break;
				case 6:
				case 7:
				    /* one of first two spike/projection buttons */
				    sysinfo.channel_selected = 0;
				    sysinfo.selected_spbutton = sysinfo.selected_button                                                         - 4;
				    DrawSPButton(sysinfo.selected_spbutton);
				    break;
				case 8:
				case 9:
				case 10:
				case 11:
				    /* one of the first four channel buttons */
				    sysinfo.selected_channel = sysinfo.selected_button -
											    4;
				    DrawChannelButton(sysinfo.selected_channel);
				    break;
				} 
			    }
			    if (sysinfo.mode == CONTINUOUS_MODE) {
				/* only channel buttons in continuous mode */
				if (sysinfo.selected_button < 0) 
					sysinfo.selected_button = adinfo.nchannels - 1;
				UnHighlightButton(tmpval + sysinfo.contbuttonbase);
				sysinfo.selected_channel = sysinfo.selected_button; 
				DrawChannelButton(sysinfo.selected_channel);
			    }
			    break;
			default:
			    //sysinfo.kbhandled = 0;
			    break;
			}
		    } else {
			switch(kbchar){
			case '\001':    // ^A set amp controls
			    if (!sysinfo.disk) {
				SetAmpControls();
				DrawChannelButtons();
				DisplayAmpFilters();
			    }
			    break;
			case '\002':    // ^B set timebase for continuous display
			    if (!sysinfo.disk) {
				tmpval = sysinfo.acq;
				if (tmpval){
				   StopAcq();
				}
				ReadInputString("Enter timebase (sec): ", 
									 tmpstring);
				if(sysinfo.mode == CONTINUOUS_MODE){
				    if (*tmpstring != '\0'){
					sysinfo.desired_timebase = atof(tmpstring);
				    }
				    sprintf(tmpstring, "Timebase %g sec",
					sysinfo.desired_timebase);
				    ErrorMessage(tmpstring);
				}
				if(tmpval){
				    StartAcq();
				}
			    }
			    break;
			case '\003':    // ^C toggle triggered continuous mode
				sysinfo.triggered_continuous =
					!sysinfo.triggered_continuous;
				sprintf(tmpstring, "Trig %d",sysinfo.triggered_continuous);
				ErrorMessage(tmpstring);
				break;
			case '\005':    // ^E equalize
				if (!sysinfo.disk) {
					tmpval = sysinfo.acq;
					if (tmpval) 
					   StopAcq();
					Equalize();
					DisplayAmpFilters();
					// go back to the original acq state
					if(tmpval)
						StartAcq();
				}
				break;
			case '\007':    // ^G enter amp gain
				if (!sysinfo.disk) {
					tmpval = sysinfo.acq;
					if (tmpval) 
					   StopAcq();
					ReadInputString("Enter amp gain (14-50000): ", 
										 tmpstring);
				if(sysinfo.mode == SPIKE_MODE){
					if ((*tmpstring != '\0') && 
						(SetElectrodeAmpGains(sysinfo.selected_spbutton / 2,
						atol(tmpstring)))) {
						DrawChannelButtons();
					}
					else {
						sprintf(tmpstring, "Invalid Gain");
						ErrorMessage(tmpstring);
					}
				}
				if(sysinfo.mode == CONTINUOUS_MODE){
					if ((*tmpstring != '\0') && 
						(SetAmpGains(atol(tmpstring)))) {
						DrawChannelButtons();
					}
					else {
						sprintf(tmpstring, "Invalid Gain");
						ErrorMessage(tmpstring);
					}
				}
					if(tmpval)
						StartAcq();
				}
				break;
			case '\014':    // ^L cycle low filter
				if (!sysinfo.disk) {
					CycleAmpLowFilter();
					DisplayAmpFilters();
				}
				break;
			case '\010':    // ^H cycle high filter
				if (!sysinfo.disk) {
					CycleAmpHighFilter();
					DisplayAmpFilters();
				}
				break;
			case '\024':    // ^T set all thresholds
				if ((sysinfo.mode == SPIKE_MODE) 
				||(sysinfo.mode == CONTINUOUS_MODE)){
					sysinfo.stringinput = 1;
					sysinfo.inputstring[0] = '\024';
					sysinfo.stringpos++;
					sysinfo.inputstring[sysinfo.stringpos] = '\000';
				}
				break;
			case '<':
				/* adjust the projection box display threshold down
					by one */
				e_num = sysinfo.selected_spbutton / 2;
				if (adinfo.electrode[e_num].displaythresh > 0)
					adinfo.electrode[e_num].displaythresh--;
				ClearSpikeBoxes(e_num);
				DrawSpikeBorders(e_num);
				break;
			case '>':
				/* adjust the projection box display threshold up
					by one */
				e_num = sysinfo.selected_spbutton / 2;
				if (adinfo.electrode[e_num].displaythresh < 
					SINGLESPIKELEN - 1)
					adinfo.electrode[e_num].displaythresh++;
				ClearSpikeBoxes(e_num);
				DrawSpikeBorders(e_num);
				break;
			case '*':
				ReadInputString("Output to PIO2: ",
						tmpstring);
				WritePIO2(atoi(tmpstring));
				break;
			case '=':
				if ((sysinfo.mode == SPIKE_MODE) && 
					 (sysinfo.channel_selected)) {
					e_num = sysinfo.selected_channel / adinfo.nelect_chan;
					channel = sysinfo.selected_channel % adinfo.nelect_chan;
					adinfo.electrode[e_num].thresh[channel] += 10;
					ClearSpikeBox(e_num, channel);
					DrawChannelButton(sysinfo.selected_channel);
					DrawSpikeBorder(e_num, channel);
				   }
				break;
			case '-':
				if ((sysinfo.mode == SPIKE_MODE) && 
					 (sysinfo.channel_selected)) {
					e_num = sysinfo.selected_channel / adinfo.nelect_chan;
					channel = sysinfo.selected_channel % adinfo.nelect_chan;
					adinfo.electrode[e_num].thresh[channel] -= 10;
					ClearSpikeBox(e_num, channel);
					DrawChannelButton(sysinfo.selected_channel);
					DrawSpikeBorder(e_num, channel);
				}
				break;
			case '+':
				if ((sysinfo.mode == SPIKE_MODE) && 
					 (sysinfo.channel_selected)) {
					/* add 50 to the threshold */ 
					e_num = sysinfo.selected_channel / adinfo.nelect_chan;
					channel = sysinfo.selected_channel % adinfo.nelect_chan;
					adinfo.electrode[e_num].thresh[channel] += 50;
					ClearSpikeBox(e_num, channel);
					DrawChannelButton(sysinfo.selected_channel);
					DrawSpikeBorder(e_num, channel);
				}
				else {
				    /* scale the continuous mode display upward by 
				       incrementing the mulitplier by .1   */
				    cptr = adinfo.channel + sysinfo.selected_channel;
				    cptr->contscalemult += .1;
				    SetContWindowScale(cptr);
				    DrawContinuousTicks();
				}	
				break;
			case '_':
				if ((sysinfo.mode == SPIKE_MODE) && 
					 (sysinfo.channel_selected)) {
					e_num = sysinfo.selected_channel / adinfo.nelect_chan;
					channel = sysinfo.selected_channel % adinfo.nelect_chan;
					adinfo.electrode[e_num].thresh[channel] -= 50;
					ClearSpikeBox(e_num, channel);
					DrawSpikeBorder(e_num, channel);
				}
				else {
					/* scale the continuous mode display downward by 
					   incrementing the mulitplier by .1 unless it is 
					   already at the minimum of .1  */
					cptr = adinfo.channel + sysinfo.selected_channel;
					if (cptr->contscalemult  > .1) 
						cptr->contscalemult -= .1;
					SetContWindowScale(cptr);
					DrawChannelButton(sysinfo.selected_channel);
					DrawContinuousTicks();
				}	
				break;
			case '}':        /* increase adgain */
				if (!sysinfo.disk) {
					tmpval = sysinfo.acq;
					if (tmpval) 
					   StopAcq();
					cptr = adinfo.channel + sysinfo.selected_channel;
					cptr->adgain++;
					if(cptr->adgain > 3) 
						cptr->adgain = 3;
					DrawChannelButton(sysinfo.selected_channel);
					if (adinfo.nchannels > NAMP_CHANNELS) {
						gain = cptr->adgain;
						/* also draw the "linked" channel's button */    
						/* assume a maximum of 2 x NAMP_CHANNELS 
						   channels */
						tmpval2 = (sysinfo.selected_channel > 
							NAMP_CHANNELS) ? sysinfo.selected_channel % 
							NAMP_CHANNELS : sysinfo.selected_channel + 
							NAMP_CHANNELS;
						  cptr = adinfo.channel + tmpval2;
						cptr->adgain = gain;
						DrawChannelButton(tmpval2);   
					}
					// go back to the original acq state
					
					sprintf(tmpstring,
						"ADGAIN %d-%d %d",sysinfo.selected_channel,
						gain);
					EventString(tmpstring);
					if(tmpval)
						StartAcq();
				}
				break;
			case '{':        /* decrease adgain */
				if (!sysinfo.disk) {
					tmpval = sysinfo.acq;
					if (tmpval) 
					   StopAcq();
					cptr = adinfo.channel + sysinfo.selected_channel;
					cptr->adgain--;
					if(cptr->adgain < 0)
						cptr->adgain = 0;
					DrawChannelButton(sysinfo.selected_channel);
					if (adinfo.nchannels > NAMP_CHANNELS) {
						gain = cptr->adgain;
						/* also draw the "linked" channel's button */    
						/* assume a maximum of 2 x NAMP_CHANNELS 
						   channels */
						tmpval2 = (sysinfo.selected_channel > 
							NAMP_CHANNELS) ? sysinfo.selected_channel % 
							NAMP_CHANNELS : sysinfo.selected_channel + 
							NAMP_CHANNELS;
						  cptr = adinfo.channel + tmpval2;
						cptr->adgain = gain;
						DrawChannelButton(tmpval2);   
					}
					sprintf(tmpstring,
						"ADGAIN %d-%d %d",sysinfo.selected_channel,
						gain);
					EventString(tmpstring);
					// go back to the original acq state
					if(tmpval)
						StartAcq();
				}
				break;
			case '?':
				/* display the help screen */
				if (!sysinfo.disk) {
					/* draw the help screen */
					if (sysinfo.mode == SPIKE_MODE) 
						ClearSpikeAreas();
					else
						ClearScopeBox();
					DrawHelpWindow();
				}
				sysinfo.helpon = 1;
			    break;
			case 'A':
				ToggleAcq();
				break;
			case 'c':
				if (sysinfo.mode == SPIKE_MODE) {
					e_num = sysinfo.selected_spbutton / 2;
					if ((sysinfo.selected_spbutton  % (sysinfo.nspbuttons 
						/ adinfo.nelectrodes)) == 0) {
							ClearSpikeBoxes(e_num);
							DrawSpikeBorders(e_num);
					}
					else {
						ClearProjectionBoxes(e_num);
						DrawProjectionBorders(e_num);
					}
				} else 
				if (sysinfo.mode == CONTINUOUS_MODE) {
					ClearScopeBox();
					 DrawScopeBorder();
					 DrawContinuousTicks();
				 } else
				if (sysinfo.mode == TRACKER_MODE) {
					ClearTrackerBox();
					 DrawTrackerBorder();
				}
				break;
			case 'D':
				if (sysinfo.fileopen)
					ToggleDisk();
					sysinfo.olddiskfree = sysinfo.diskfree;
					sysinfo.oldtime = sysinfo.currenttime;
					sysinfo.olddisktime = sysinfo.currenttime;
				break;
			case 'F':
				if (!sysinfo.disk) {
					tmpval = sysinfo.acq;
					if (tmpval)
						StopAcq();
					if (!sysinfo.fileopen) {
						/* open the input file */
						ReadInputString("Enter name for the data file: ",
								tmpstring);
						// check for the forced override character
						tmpval2 = (tmpstring[strlen(tmpstring)-1] == '!') ?
									1 : 0;
						if (tmpval2)
							/* erase the last character of the string */
							tmpstring[strlen(tmpstring)-1] = '\0';
						if(strlen(tmpstring)+1>=MAXFILENAMELEN){
							sprintf(tmpstring2, 
							"Error: filename %s too long", 
							tmpstring);
							ErrorMessage(tmpstring2);
						} else 
						if((!tmpval2) && (access(tmpstring, 0) == 0)) {
							/* the file already exists */
							sprintf(tmpstring2, "Error: %s already exists", 
								   tmpstring);
							ErrorMessage(tmpstring2);
						}
						else if ((adinfo.fp = fopen(tmpstring, "wb")) ==
								NULL) {
							sprintf(tmpstring2, "Error opening %s for writing", 
								   tmpstring);
							ErrorMessage(tmpstring2);
						}
						else {
							WriteHeader(); 
							strcpy(adinfo.fname, tmpstring);
							sysinfo.diskfree = DiskFree();
							sysinfo.filesize = 0;
							sysinfo.fileopen = !sysinfo.fileopen;
							UpdateFileButton();
						}
						/*
						** save the session and epoch id
						*/
						sprintf(tmpstring,
							"SESSION %s", sysinfo.sessionstring);
						EventString(tmpstring);
						sprintf(tmpstring,
							"EPOCH %s", sysinfo.epochstring);
						EventString(tmpstring);
					}
					else {
						ADCloseFile();
						UpdateFileButton();
					}
					if (tmpval) {
						PrepareStartAcq();
						UpdateAcqButton();    
						StartAcq();
						UpdateAcqButton();    
					}
					DrawStrings();
					DisplayCurrentTime();
					DisplayDiskInfo();
				}
				break;
			case 'g':
				if ((!sysinfo.disk) && (sysinfo.channel_selected)) {
					tmpval = sysinfo.acq;
					if (tmpval) 
						StopAcq();
					ReadInputString("Enter amp gain for channel (12-50000): ", tmpstring);
					if ((*tmpstring != '\0') && 
						(SetAmpGain(sysinfo.selected_channel, 
						atol(tmpstring)))){
						DrawChannelButton(sysinfo.selected_channel);   
						if (adinfo.nchannels > NAMP_CHANNELS) {
							/* also draw the "linked" channel's button */    
							/* assume a maximum of 2 x NAMP_CHANNELS 
							   channels */
							tmpval2 = (sysinfo.selected_channel > 
								NAMP_CHANNELS) ? sysinfo.selected_channel % 
								NAMP_CHANNELS : sysinfo.selected_channel + 
								NAMP_CHANNELS;
							DrawChannelButton(tmpval2);   
						}
					}
					else {
						sprintf(tmpstring, "Invalid Gain");
						ErrorMessage(tmpstring);
					}
					  // go back to the original acq state
					if(tmpval){
						 StartAcq();
					}
				}
				break;
			case 'O':
				/*
				** dont allow mode changes in tracker mode
				*/
				if(sysinfo.mode == TRACKER_MODE) break;
				if ((!sysinfo.disk) && (adinfo.nchannels<=NAMP_CHANNELS)) {
					sysinfo.mode = !sysinfo.mode;
					sysinfo.detect_spikes = sysinfo.mode;
		sysinfo.channel_selected = 1;
		sysinfo.selected_channel = 0;
		sysinfo.selected_spbutton = 0;
		// make sure that the first channel button is selected
					sysinfo.selected_button = (sysinfo.mode == SPIKE_MODE) ?
					  2 : 0;
					ClearScreen();
				}
				break;
			case 'o':
				if (sysinfo.mode == SPIKE_MODE) {
					/* don't overlay if on a projection button */
					if ((sysinfo.selected_spbutton  % (sysinfo.nspbuttons 
						/ adinfo.nelectrodes)) !=1) {
						e_num = sysinfo.selected_spbutton / 
								(sysinfo.nspbuttons / adinfo.nelectrodes);
						sysinfo.overlay[e_num] = !sysinfo.overlay[e_num];
						DrawSPButton(sysinfo.selected_spbutton);
					}
				 } else
				if (sysinfo.mode == CONTINUOUS_MODE) {
					sysinfo.contoverlay = !sysinfo.contoverlay;
				 } 
				if (sysinfo.tracker_enabled) {
					sysinfo.trackoverlay = !sysinfo.trackoverlay;
				}
				break;
			case 'r':
				sysinfo.drawmode = !sysinfo.drawmode;
				break;
			case 't': /* the user will set the threshold */
				if (((sysinfo.mode == SPIKE_MODE) 
				||(sysinfo.mode == CONTINUOUS_MODE)) &&
					(sysinfo.channel_selected)) {
					sysinfo.stringinput = 1;
					sysinfo.inputstring[0] = 't';
					sysinfo.stringpos++;
					sysinfo.inputstring[sysinfo.stringpos] = '\000';
				}
				break;
			//case 'e':
				//sysinfo.detect_spikes = !sysinfo.detect_spikes;
			//	break;
			case 'Q':
				if ((!sysinfo.disk) && (!sysinfo.acq)) {
					sysinfo.quit = 1;
				}
				break;
			case 'S':
				if (!sysinfo.disk) {
					tmpval = sysinfo.acq;
					if (tmpval)
						StopAcq();
					ReadInputString("Enter name of config file to save: ", 
									tmpstring);
					   if ((*tmpstring != '\0') && (SaveConfig(tmpstring))) {
						ErrorMessage("Configuration file written");
					}
					else {
						sprintf(tmpstring2, "Error opening %s for writing", tmpstring);
						ErrorMessage(tmpstring2);
					}
					if (tmpval)
						StartAcq();
				}
				break;
			case 'L':
				if (!sysinfo.disk) {
					tmpval = sysinfo.acq;
					if (tmpval)
						StopAcq();
					ReadInputString("Enter name of config file to load: ", 
									tmpstring);
					LoadAndUpdateConfig(tmpstring);
					if (tmpval) {
						PrepareStartAcq();
						UpdateAcqButton();
						StartAcq();
					}
				}
				break;
			case 'p':
				/* stimulate the little guy */
				if (sysinfo.stim.pulselen > 0) {
				 if (!sysinfo.seq[STIMSEQON].active){
					SetStimParm(sysinfo.stim.pulselen);
					StartStimAcq();
					sprintf(tmpstring2, "Stimulation is on");
					EventString("Startstimseq"); }
				 else {
					EndStimAcq();
					sprintf(tmpstring2, "Stimulation is off");
					EventString("Endstimseq");  }
				 ErrorMessage(tmpstring2);
				 sysinfo.seq[STIMSEQON].time = 0;
				} 
				else {
			sprintf(tmpstring2, "Error: pulse length out of acceptable range");
					ErrorMessage(tmpstring2);
				}
				break;
			case 'P':
				/* display the stim screen */
				if (!sysinfo.disk) {
					/* draw the stim screen */
					DrawStimWindow();
				
					/* temporary removal of stim params: by Jay 12/96 */
					/*		ReadInputString("",tmpstring);
					restoreacq =  1;         */ 
					done = 0;
					while(!done)
					{
						kbchar = _getch();
						if (!kbchar)   /* Extended keyboard chars. */
						 {
						 kbchar = _getch();
						 switch (kbchar) {
						 case 75:      /* Left arrow */
						 sysinfo.stim.phase -= 5;
						 if (sysinfo.stim.phase < 0) sysinfo.stim.phase = 355;
						 DrawStimWindow();
						 break;
						 case 77:      /* Right arrow */
						 sysinfo.stim.phase += 5;
						 if (sysinfo.stim.phase >= 360) sysinfo.stim.phase = 0;
						 DrawStimWindow();
						 break;
						 default: 
						 break;
						  }
						 }
						else {
						switch (kbchar) {
						case 'z':
						while ((kbchar = _getch()) != 13)
						 printf("Char #%d\n",kbchar);
						break;
						case '%':
					ReadInputString("Enter percentage of stim trials: ", 
					tmpstring);
					if (((sysinfo.stim.percent = atoi(tmpstring)) < 0) ||
						 (sysinfo.stim.percent > 100)) {
						 /* the user typed in an invalid number. 
							reset sysinfo.stim.percent */
			   sprintf(tmpstring2, "Error: %d out of acceptable range", 
								 sysinfo.stim.percent);
						 ErrorMessage(tmpstring2);
						 sysinfo.stim.percent = 0;
					}
						DrawStimWindow();
						break;
						case 'g':
						sysinfo.stim.grid = !sysinfo.stim.grid;
						DrawStimWindow();
						DrawTrackerGrid();
						break;
						case 'o':
				 sysinfo.seq[STIMSEQON].active = !sysinfo.seq[STIMSEQON].active;
						DrawStimWindow();
						break;
						case 'l':
					ReadInputString(
					 "Enter total length of pulse in ms (0.1-25): ", 
					tmpstring);
					if (((sysinfo.stim.pulselen = atof(tmpstring)*100) < 1) ||
						 (sysinfo.stim.pulselen > 255)) {
						 /* the user typed in an invalid number. 
							reset sysinfo.stim.pulselen */
						sprintf(tmpstring2, "Error: %d out of acceptable range",
					sysinfo.stim.pulselen);
						 ErrorMessage(tmpstring2);
						 sysinfo.stim.pulselen = 0;
					}
					else {
						/* set up the stimulation parameters */
						SetStimParm(sysinfo.stim.pulselen);
					}
						DrawStimWindow();
						break;
						case 'n':
					ReadInputString(
					 "Enter number of pulses in burst: ", tmpstring);
					if (((sysinfo.stim.number = atoi(tmpstring)) < 1) ||
						 (sysinfo.stim.number > 1000)) {
						 /* the user typed in an invalid number. 
							reset sysinfo.stim.number */
					sprintf(tmpstring2, "Error: %d out of acceptable range", 
					sysinfo.stim.number);
						 ErrorMessage(tmpstring2);
						 sysinfo.stim.number = 0;
					}
					else {
						/* set up the stimulation parameters */
						SetStimParm(sysinfo.stim.pulselen);
					}
						DrawStimWindow();
						break;
						case 'i':
					ReadInputString(
		    "Enter interpulse interval within burst, in ms: ", tmpstring);
					if ((sysinfo.stim.interpulse = atof(tmpstring)*100) < 1) {
						 /* the user typed in an invalid number. 
							reset sysinfo.stim.interpulse */
					sprintf(tmpstring2, "Error: %d out of acceptable range", 
					sysinfo.stim.interpulse);
						 ErrorMessage(tmpstring2);
						 sysinfo.stim.interpulse = 0;
					}
					else {
						/* set up the stimulation parameters */
						SetStimParm(sysinfo.stim.pulselen);
					}
						DrawStimWindow();
						break;
						case 'd':
					ReadInputString(
					 "Enter minimum delay between bursts, in sec: ", tmpstring);
					if ((sysinfo.stim.delay = atof(tmpstring)*10000) < 0)
					{
						 /* the user typed in an invalid number. 
							reset sysinfo.stim.delay */
					sprintf(tmpstring2, "Error: %d out of acceptable range", 
					sysinfo.stim.delay);
						 ErrorMessage(tmpstring2);
						 sysinfo.stim.delay = 0;
					}
					else {
						/* set up the stimulation parameters */
						ClkSetupStim();
						SetStimParm(sysinfo.stim.pulselen);
					}
						DrawStimWindow();
						break;
						case 'a':
						 InputMessage("(T)est or (C)ontrol area?");
					 	 kbchar = _getch();
						 setcolor(BLACK);
						 outtextxy(sysinfo.inputview.x, sysinfo.inputview.y, 
					 	  "(T)est or (C)ontrol area?");
						 setcolor(WHITE);
						 switch (kbchar) {
						 case 't':
						 GetStimArea(&sysinfo.stim.tstarea);
						 if ((sysinfo.stim.mode == CONTROL) || 
						  (sysinfo.stim.mode == BOTH))
						 sysinfo.stim.mode = BOTH;
						 else sysinfo.stim.mode = TEST;
						 sysinfo.stim.protocol = AREA;
						 break;
						 case 'c':
						 GetStimArea(&sysinfo.stim.ctlarea);
						 if ((sysinfo.stim.mode == TEST) || 
						  (sysinfo.stim.mode == BOTH))
						 sysinfo.stim.mode = BOTH;
						 else sysinfo.stim.mode = CONTROL;
						 sysinfo.stim.protocol = AREA;
						 break;
						 default:
						 outtextxy(sysinfo.inputview.x, sysinfo.inputview.y, 
						 "Not a valid option.");
					 	 kbchar = _getch();
						 setcolor(BLACK);
						 outtextxy(sysinfo.inputview.x, sysinfo.inputview.y, 
						 "Not a valid option.");
						 setcolor(WHITE);
						 break;
					 	 }
						DrawStimWindow();
						break;
						case 'v':
					ReadInputString("Enter stimulating voltage: ", 
					tmpstring);
					if (((sysinfo.stim.voltage = atof(tmpstring)) < 0.) ||
						 (sysinfo.stim.voltage > 100.)) {
						 /* the user typed in an invalid number. 
							reset sysinfo.stim.voltage */
					 sprintf(tmpstring2, "Error: %g out of acceptable range", 
								 sysinfo.stim.voltage);
						 ErrorMessage(tmpstring2);
						 sysinfo.stim.voltage = 0;
					}
						DrawStimWindow();
						break;
						case 't':
				setcolor(_WHITE);
					outtextxy(sysinfo.inputview.x, sysinfo.inputview.y - 10, 
						"Enter test electrode number or 'c' to ");
					ReadInputString( "clear all. Hit <CR> when done:"
				   , tmpstring);
				setcolor(_BLACK);
					outtextxy(sysinfo.inputview.x, sysinfo.inputview.y - 10, 
						"Enter test electrode number or 'c' to ");
				setcolor(_WHITE);
					 n = atoi(tmpstring);
					 if (tmpstring[0] == 'c') 
					   {
						for (i = -1; ++i <= MAXCHANNELS;) 
						 sysinfo.stim.test[i] = 0;
						DrawStimWindow();
						break;
						}
					while ((n > 0) && (n < MAXCHANNELS))
					{
					 nflag = 0;
					 for (i = 0;  ++i <= sysinfo.stim.test[0];)
						 if (sysinfo.stim.test[i] == n) nflag = 1;
					 for (i = 0;  ++i <= sysinfo.stim.control[0];)
						 if (sysinfo.stim.control[i] == n) nflag = 1;
					 if (nflag == 1) 
					 {
					 ReadInputString(
				"Electrode is assigned.  Enter another or <CR>:"
					  ,tmpstring);
					 n = atoi(tmpstring);
					 }
					else
					 {
					sysinfo.stim.test[0]++;
					sysinfo.stim.test[sysinfo.stim.test[0]] = n;
					ReadInputString(
				"Enter test electrode number or <CR>:"
				   , tmpstring);
					 n = atoi(tmpstring);
					  }
					 }
					DrawStimWindow();
					break;
						case 'c':
				setcolor(_WHITE);
					outtextxy(sysinfo.inputview.x, sysinfo.inputview.y - 10, 
						"Enter control electrode number or 'c' to ");
					ReadInputString(
				"clear all. Hit <CR> when done:"
				   , tmpstring);
				setcolor(_BLACK);
					outtextxy(sysinfo.inputview.x, sysinfo.inputview.y - 10, 
						"Enter control electrode number or 'c' to ");
					setcolor(_WHITE);
					 n = atoi(tmpstring);
					 if (tmpstring[0] == 'c') 
					   {
						for (i = -1; ++i <= MAXCHANNELS;) 
						 sysinfo.stim.control[i] = 0;
						DrawStimWindow();
						break;
						}
					while ((n > 0) && (n < MAXCHANNELS))
					 {
					 nflag= 0;
					 for (i = 0;  ++i <= sysinfo.stim.test[0];)
						 if (sysinfo.stim.test[i] == n) nflag = 1;
					 for (i = 0;  ++i <= sysinfo.stim.control[0];)
						 if (sysinfo.stim.control[i] == n) nflag = 1;
					 if (nflag == 1) 
					  {
					  ReadInputString(
			"Electrode is assigned.  Enter another or <CR>:"
					   ,tmpstring);
					  n = atoi(tmpstring);
					  }
					 else
					 {
					 sysinfo.stim.control[0]++;
					 sysinfo.stim.control[sysinfo.stim.control[0]] = n;
					ReadInputString( 
			"Enter control electrode number or <CR>:"
					, tmpstring);
					 n = atoi(tmpstring);
					  }
					 }
					DrawStimWindow();
					break;
					case 'p':
					DrawStimProtocol();
						 kbchar = _getch();
						 switch (kbchar) {
						 case 'n':
						 sysinfo.stim.mode = NONE;
						 break;
						 case 't':     
						 sysinfo.stim.mode = TEST;
						 break;
						 case 'c':
						 sysinfo.stim.mode = CONTROL;
						 break;
						 case 'b':
						 sysinfo.stim.mode = BOTH;
						 break;
						 case 'i':
						 sysinfo.stim.protocol = TIME;
						 break;
						 case 'a':
						 sysinfo.stim.protocol = AREA;
						 break;
						 case 'r':
						 sysinfo.stim.protocol = RANDOM;
						 break;
						 case 'p':
						 sysinfo.stim.protocol = PROGRAMMED;
				/* Sequencer programming loop */
					setcolor(_WHITE);
					outtextxy(sysinfo.inputview.x, sysinfo.inputview.y - 10, 
						"Enter each delay in seconds or 'c' to ");
					ReadInputString( "clear all. Hit <CR> when done:"
				   , tmpstring);
				setcolor(_BLACK);
					outtextxy(sysinfo.inputview.x, sysinfo.inputview.y - 10, 
						"Enter each delay in seconds or 'c' to ");
				setcolor(_WHITE);
					d = atof(tmpstring);
					while ((d > 0) || (tmpstring[0] == 'c'))
					 {
					 if (tmpstring[0] == 'c') 
					   {
						for (i = 0; ++i <= sysinfo.stim.seqlength;) 
						 sysinfo.stim.seq[i] = 0;
						sysinfo.stim.seqlength = 0;
						}
					 else
						{
						sysinfo.stim.seqlength++;
						if (sysinfo.stim.seqlength >= 255 ) 
						 { fprintf(stderr, "Sequencer full.\n"); exit(1); }
				        sysinfo.stim.seq[sysinfo.stim.seqlength] = d*10000;
					    }
					 DrawStimProtocol();
					 ReadInputString( "Enter delay or <CR>:" , tmpstring);
					 d = atof(tmpstring);
					 }
						 break;
					 default: 
					 break;
					 }
					DrawStimWindow();
					break;
				case 's':
				sprintf(tmpstring,"adstim.cfg");
				if ((fp = fopen(tmpstring, "wb")) == NULL) {
				 sprintf(tmpstring2, "Error opening %s for writing", 
				 tmpstring);
				 ErrorMessage(tmpstring2);
				 break;
				 }
				fwrite(&sysinfo.stim, sizeof(sysinfo.stim), 1, fp); 
//				for (i = - 1;  ++i <= 4;)
//			fwrite(&sysinfo.seq[i].interval, sizeof(unsigned long), 1, fp); 
	//	fwrite(&sysinfo.seq[BLOCKSEQ].active, sizeof(short), 1, fp); 
//		fwrite(&sysinfo.seq[BLOCKSEQ].time, sizeof(unsigned long), 1, fp); 
				sprintf(tmpstring2, 
				   "Stim parameters saved to file %s.", tmpstring);
			ErrorMessage(tmpstring2);
				fclose(fp);
				break;
				case 'r':
				sprintf(tmpstring,"adstim.cfg");
				if ((fp = fopen(tmpstring, "rb")) == NULL) {
				 sprintf(tmpstring2, "Error opening %s for reading", 
				 tmpstring);
				 ErrorMessage(tmpstring2);
				 break; }
				fread(&sysinfo.stim, sizeof(sysinfo.stim), 1, fp); 
//				for (i= - 1;  ++i <= 4;) 
//			fread(&sysinfo.seq[i].interval, sizeof(unsigned long), 1, fp); 
//		fread(&sysinfo.seq[BLOCKSEQ].active, sizeof(short), 1, fp); 
//		fread(&sysinfo.seq[BLOCKSEQ].time, sizeof(unsigned long), 1, fp); 
				sprintf(tmpstring2, 
				   "Stim parameters read from file %s.", tmpstring);
			ErrorMessage(tmpstring2);
				DrawStimWindow();
				fclose(fp);
				break;
				default:
			//	  RedrawMain();
					done = 1;
					break;
				}           /* switch loop */
			  }				/* extended keyboard check */
			 }            /* keypress loop */
				 ClkSetupStim();
				 SetStimParm(sysinfo.stim.pulselen);
				 RedrawMain();
			}           /* disk off loop */
				break;
			case 'R':
				if (!sysinfo.disk) {
					tmpval = sysinfo.acq;
					if (tmpval)
						StopAcq();
					ReadInputString("Enter rate (0-250000): ", tmpstring);
					adinfo.rate_req = atof(tmpstring);
					if(!SetRate(adinfo.rate_req, &adinfo.rate_set))  {
						sprintf(tmpstring, "Cannot set rate to %6.1f", adinfo.rate_req);
						ErrorMessage(tmpstring);
					}
					else {
						/* reset the conversion duration */
						adinfo.conversion_duration = SECOND*adinfo.dma_bufsize
													 / adinfo.rate_set;
						ErrorMessage("Rate set");
					}
					DisplayRate();
					if (tmpval)
						StartAcq();
				}
				break;
			case 's':
				DisplayADStatus();
				break;
			case 'u':
				if (!sysinfo.disk) {
					ToggleAutoscale();
				}
				break;
			case 'M':
				if(sysinfo.command_mode == MASTER){
					tmpval = sysinfo.acq;
					// if acq is on, turn it off before getting input
					if (tmpval){
						StopAcq();
					}
					StatusMessage("c(L)ear (R)eset clock; s(T)op acq; (S)tart Acq; Disk O(N)/O(F)F; (C)lose file; test s(Y)nch");
					ReadInputString("Enter Master system command: ", tmpstring);
					restoreacq =  1;        
					if (*tmpstring != '\0') {
						tmpval2 = 0;
						switch (*tmpstring) {
						case 'L':
						    tmpval2 = CLEARSCREEN;
							restoreacq =  1;        
							break;
						case 'R':
							tmpval2 = RESETCLK;
							restoreacq =  0;        
							break;
						case 'T':
							tmpval2 = STOPACQ;
							restoreacq =  0;        
							break;
						case 'S':
							tmpval2 = STARTACQ;
							restoreacq =  0;        
							break;
						case 'N':
							tmpval2 = DISKON;
							break;
						case 'F':
							tmpval2 = DISKOFF;
							break;
						case 'C':
							tmpval2 = FILECLOSE;
							break;
						case 'Y':
							tmpval2 = TESTSYNCH;
							break;
						}
						if (tmpval2 != INVALID)
							ClkMasterProcessCommand(tmpval2);
						StatusMessage(NORMAL_MESSAGE);
					}
					// if acq was on and acq state was not explicitly
					// set by a command turn it back on
					if (tmpval && restoreacq) {
						PrepareStartAcq();
					}
					//UpdateAcqButton();
					if (tmpval && restoreacq) {
						StartAcq();
					}
					UpdateAcqButton();
				}
				else {
					ErrorMessage("Not a Master system."); 
				}
				break;
			case '`':
				if(sysinfo.command_mode == MASTER)
				StartFout();
				break;
			case '~':
				if(sysinfo.command_mode == MASTER){
					StopFout();
				}
				DisplayCurrentTime();
				break;
			case 'E':            // generate an a/d error for debugging
				disable();
				delay((int)(200*(250000.0/adinfo.rate_set)));
				enable();
				break;
			case 'T':
				if(sysinfo.tracker_enabled){
					ToggleTracker();
				}
				break;
#ifdef OLD					
			case '}':            
				IncIRQMask(1);
				break;
			case '{':            
				IncIRQMask(-1);
				break;
#endif					
			case ')':
				tracker.rate++;
				break;
			case '(':
				tracker.rate--;
				if(tracker.rate < 1) tracker.rate =1;
				break;
			default:
				//sysinfo.kbhandled = 0;
				break;
			}
		    }
		}
	    }
	}
}

