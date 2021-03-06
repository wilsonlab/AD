
int nCS_HDAMP_InteractiveSetup( int nprocessing_state );
/*=========================================================================
=*/

/*====================================================================*/
/*====================================================================*/
/*====================================================================*/
/*       High Density Amplifier Control Software Functions            */
/*                                                                    */
/*====================================================================*/
/*====================================================================*/
/*====================================================================*/
#define HDAMP_SEQ_CTR_REG_LOAD   BIT12
#define HDAMP_SEQ_CTR_REG_INC    BIT13
#define HDAMP_AMP_REG_LOAD       BIT14
#define HDAMP_DC_EQU_SW_REG_LOAD BIT15

#define HDAMP_FILTER_REG_START	0
#define HDAMP_GAIN_REG_START	8

#define HDAMP_LOWCUT_TENTHHZ	0
#define HDAMP_LOWCUT_1HZ	BIT0
#define HDAMP_LOWCUT_10HZ	BIT1
#define HDAMP_LOWCUT_100HZ	BIT2
#define HDAMP_LOWCUT_300HZ	BIT3
#define HDAMP_LOWCUT_600HZ	BIT4
#define HDAMP_LOWCUT_900HZ	(BIT3 | BIT4)

#define HDAMP_HICUT_50HZ	0
#define HDAMP_HICUT_100HZ	BIT5   /* 120 HZ ACTUALLY */
#define HDAMP_HICUT_200HZ	BBIT8
#define HDAMP_HICUT_250HZ	BIT9
#define HDAMP_HICUT_275HZ	(BIT5  | BBIT8)
#define HDAMP_HICUT_325HZ	(BIT5  | BIT9)
#define HDAMP_HICUT_400HZ	(BBIT8 | BIT9)
#define HDAMP_HICUT_475HZ	(BIT5 | BBIT8 | BIT9)
#define HDAMP_HICUT_3KHZ	BIT6
#define HDAMP_HICUT_6KHZ	BIT7
#define HDAMP_HICUT_9KHZ	(BIT6|BIT7)

#define HDAMP_REF_FULL_GAIN	50000
#define HDAMP_ALL_CHANNELS	9       /* used for the display function */

int nHDAMP_last_pio_value_output;
int nHDAMP_filter_values[8];
int nHDAMP_gain_values[8];

int nHDAMP_InitAmp(void);
int nHDAMP_OutputValue( int nvalue );
int nHDAMP_SetEqualizeSwitches( int nswitchs_on );
int nHDAMP_LoadSeqCtrReg( int nreg_value );
int nHDAMP_IncSeqCtrReg( void );
int nHDAMP_LoadCurAmpReg( int nload_value );
int nHDAMP_SetFilterGainList( int* lpnfilter_settings, int *lpngain_settings);
int nHDAMP_LoadSingleFilterVal( int nchannel, int nfilter_setting);
int nHDAMP_LoadSingleGainVal( int nchannel, int ngain_setting );

int nHDAMP_WriteSetupFile( BYTE* szfile_name );
int nHDAMP_ReadSetupFile( BYTE* szfile_name);


/* display function for showing the settings */
int nHDAMP_DisplaySettings( int nhighlight, int nupdate, WORD wbox[4] );

/* single function which can be called by other functions to do all the 
 * amplifier initialization without having to worry about anything except
 * calling this function and the return code.
 */
int nHDAMP_ReadFileInitAmps(void);


/*====================================================================*/

/*====================================================================*/
int nHDAMP_InitAmp(void)
/* This will setup the DT2821 PIO ports as outputs and set the high 4 
 * control bits of the register (bits 12->15).  Setting these bits will
 * get the  control section to accept commands as these 4 bits are activited
 * by lowering the line.
 */
{
    int i;    

    /* set the DT2821 PIO port as output */
    vDT2821_pio_write( 0xFFFF );    /* to enable all reg's to 1's */

    vDT2821_pio_init( DT2821_PIO_OUTPUT, DT2821_PIO_OUTPUT );


    /* set the 4 control lines to hi */
    i = nHDAMP_OutputValue( HDAMP_SEQ_CTR_REG_LOAD | HDAMP_SEQ_CTR_REG_INC|
    			 HDAMP_AMP_REG_LOAD | HDAMP_DC_EQU_SW_REG_LOAD );
    return(0);
}

/*====================================================================*/
int nHDAMP_OutputValue( int nvalue )
/* this will output the value to the PIO port and save this value in the 
 * nHDAMP_last_pio_value_output variable.
 */
{ 
    /* set the 4 control lines to hi */
    vDT2821_pio_write( nvalue );

    /* save the value set */
    nHDAMP_last_pio_value_output = nvalue;

    return(0);  /* return success */

}

/*====================================================================*/
int nHDAMP_SetEqualizeSwitches( int nswitches_on )
/* This will output the switch bit values passed in to the amp data bus
 * (lower 12 bits) and will lower and raise the DC Equalize Switch register
 * bit to load the specified value into the switches.
 */
{
    int nvalue;

    /* make sure that a bogus value has not been passed */
    if (nswitches_on > 0x00ff)return( -1);

    /* raise the control lines to make sure we don't screw up data */
    nHDAMP_OutputValue( 0xF000 | nHDAMP_last_pio_value_output );

    nvalue = 0xF000 | nswitches_on;
    nHDAMP_OutputValue(nvalue);

    /* now output same value with the switch register load bit low */
    nvalue &= (~HDAMP_DC_EQU_SW_REG_LOAD );
    nHDAMP_OutputValue(nvalue);

    /* now raise the switch register load bit to lock in the values in sw reg*/
    nvalue |= HDAMP_DC_EQU_SW_REG_LOAD;    
    nHDAMP_OutputValue(nvalue);

    return(0);

}

/*====================================================================*/
int nHDAMP_LoadSeqCtrReg( int nreg_value )
/* This will load the sequence counter register with the value spec'd.
 * This will involve setting the register value number on the data bus and
 * lowering and raising the control line to load the counter register of the
 * 74'163 chip.
 */
{
    int nvalue;

    if ( (nreg_value >15) || (nreg_value <0) ) return (-1);

    /* raise the control lines to make sure we don't screw up data */
    nHDAMP_OutputValue( 0xF000 | nHDAMP_last_pio_value_output );

    /* or in the register value to set the data bus to */
    nvalue = 0xF000 | nreg_value;
    nHDAMP_OutputValue(nvalue);

    /* lower the seq ctr reg load bit */
    nvalue &= (~HDAMP_SEQ_CTR_REG_LOAD);
    nHDAMP_OutputValue(nvalue);

    /* lower and raise the clock signal to pass the preset through on the 163*/
    nvalue &= (~HDAMP_SEQ_CTR_REG_INC);
    nHDAMP_OutputValue(nvalue);
    nvalue |= HDAMP_SEQ_CTR_REG_INC;
    nHDAMP_OutputValue(nvalue);
    
    /* raise the seq ctr reg load bit */
    nvalue |= HDAMP_SEQ_CTR_REG_LOAD;
    nHDAMP_OutputValue(nvalue);

    return (0);   
}    
/*====================================================================*/
int nHDAMP_IncSeqCtrReg( void )
/* this will lower and raise the Seq Ctr Reg Inc line.
 */
{
    int nvalue;
    nvalue = nHDAMP_last_pio_value_output & (~HDAMP_SEQ_CTR_REG_INC);
    nHDAMP_OutputValue(nvalue);

    nvalue |= HDAMP_SEQ_CTR_REG_INC;
    nHDAMP_OutputValue(nvalue);

    return(0);

}

/*====================================================================*/
int nHDAMP_LoadCurAmpReg( int nload_value )
/* This will put the value spec'd on the amp data bus and will lower and raise
 * the control line to load the value into the current register.  The register
 * which gets loaded will be that which is pointed to by the Sequence Register
 * Counter (the 74163 chip).
 */
{
    int nvalue;

    /* make sure that the load value is not outside of 0 -> 12 bits */
    if ( (nload_value <0) || (nload_value >4095) ) return(-1);

    /* raise all control lines with current 12 bit data */
    nvalue = nHDAMP_last_pio_value_output | 0xf000;
    nHDAMP_OutputValue(nvalue);

    /* output the data value on the amp's data bus */
    nvalue = 0xF000 | nload_value;
    nHDAMP_OutputValue(nvalue);
    
    /* lower the load reg control line */
    nvalue &= (~HDAMP_AMP_REG_LOAD);
    nHDAMP_OutputValue(nvalue);

    /* raise the load reg control line to lock the value in */
    nvalue |= HDAMP_AMP_REG_LOAD;
    nHDAMP_OutputValue(nvalue);

    return (0);
}
/*====================================================================*/
int nHDAMP_SetFilterGainList( int* lpnfilter_settings, int* lpngain_settings)
/* This will down load the filter setting and gain settings arrays which are
 * passed in.  Each list will be 8 entries long.  The amp will be loaded in 
 * sequence from filter0 to gain7.
 */
{
    int i;
    
    /* point the sequence counter register to the start of the filters */
    nHDAMP_LoadSeqCtrReg( HDAMP_FILTER_REG_START );
    
    /* load each of the filter settings and inc the seq ctr after each one */
    for (i=0; i<8; i++, lpnfilter_settings++){
      nHDAMP_LoadCurAmpReg( *lpnfilter_settings );
      nHDAMP_IncSeqCtrReg();
      nHDAMP_filter_values[i] = *lpnfilter_settings;    
    }
    
    /* load each of the gain settings and inc the seq ctr after each one */
    for (i=0; i<8; i++, lpngain_settings++){
      nHDAMP_LoadCurAmpReg( *lpngain_settings );
      nHDAMP_IncSeqCtrReg();
      nHDAMP_gain_values[i] = *lpngain_settings;    
    }
    
    return(0);

}
    
/*====================================================================*/
int nHDAMP_LoadSingleFilterVal( int nchannel, int nfilter_setting)
/* This will load the spec'd channel's filter setting register to that spec'd.
 */
{
    if ( (nchannel <0) || (nchannel > 8) ||
         (nfilter_setting <0) || (nfilter_setting> 0x03FF) ){
      return(-1);
    }

    nHDAMP_LoadSeqCtrReg( HDAMP_FILTER_REG_START + nchannel ); 
    nHDAMP_LoadCurAmpReg( nfilter_setting );
    nHDAMP_filter_values[nchannel] = nfilter_setting;    
    
    return(0);
}
/*====================================================================*/
int nHDAMP_LoadSingleGainVal( int nchannel, int ngain_setting )
/* This will load the spec'd channel's gain setting register to that spec'd.
 */
{
    if ( (nchannel <0) || (nchannel > 8) ||
         (ngain_setting <0) || (ngain_setting> 0x0FFF) ){
      return(-1);
    }

    nHDAMP_LoadSeqCtrReg( HDAMP_GAIN_REG_START + nchannel ); 
    nHDAMP_LoadCurAmpReg( ngain_setting );
    nHDAMP_gain_values[nchannel] = ngain_setting;    
    return(0);
}
/*====================================================================*/
int nHDAMP_ReadSetupFile( BYTE* szfile_name)
/* THis will read the setup file into the gain and filter arrays.
*/
{
    int i, hfile;
    BYTE sztemp[80];

    strcpy( sztemp, szdefault_bwcfg_path );
    strcat( sztemp, szfile_name );
    
    if ((hfile = open(sztemp, O_RDONLY|O_BINARY)) == -1){
      /* open failed - return -1 */
      return(-1);
    }else{
      /* turn off the DC equalization switches first */
      nHDAMP_SetEqualizeSwitches( 0 );

      /* read the filter settings */
      i=read(hfile, nHDAMP_filter_values, 16);
      if (i<0){
        (void)close(hfile);
        return(-1);
      }    
      /* read the gain settings */
      i=read(hfile, nHDAMP_gain_values, 16);
      if (i<0){
        (void)close(hfile);
        return(-1);
      }    
      (void)close(hfile);
    }
    return(0);
}


/*====================================================================*/
int nHDAMP_WriteSetupFile( BYTE* szfile_name )
/* This will save the current filter values into the setup file.
 */
{
    BYTE sztemp[80];
    int  i, hfile;
    
    /* build up the path name */
    strcpy( sztemp, szdefault_bwcfg_path );
    strcat( sztemp, szfile_name );
    if ((hfile = open(sztemp, 
                      O_CREAT  | O_WRONLY | O_BINARY | O_TRUNC,
		      S_IWRITE | S_IREAD)) == -1){
      /* open failed - return -1 */
      return(-1);
    }else{
      i=write(hfile, nHDAMP_filter_values, 16 );
      i=write(hfile, nHDAMP_gain_values, 16);
      (void)close(hfile);
      if (i<=0){
        return(-1);
      }
    }
    
    return(0);  
}


/*====================================================================*/
int nHDAMP_DisplaySettings( int nhighlight, int nupdate, WORD wbox[4] )
/* This will display the current amplifier settings in the box spec'd.
 * The highlight field spec's which field(s) are drawn in WHITE and will be:
 *    -1: none; 
 *    0->7 for the single channel # to highlight;
 *    HDAMP_ALL_CHANNELS for all highlighted;
 * The nupdate parameter spec's which field to update with the same parameter
 * values as for the nhighlight field.
 *
 * The values for the filter and gain settings are taken from the global 
 * arrays for the library.
 */
{
    BYTE  szline[50];
    int   ncolor, i, j;
#define NUM_SPACES_PER_ENTRY   10

    for( i=0; i<8; i++){
      /* see if we are to do the ith channel */
      if ( (nupdate != HDAMP_ALL_CHANNELS) && (i != nupdate) ){
        continue;
      }
      /* set the color */
      if ( (nhighlight == HDAMP_ALL_CHANNELS) || (i == nhighlight) ){
        ncolor = WHITE;
      }else{
        ncolor = scope_colors[i];
      }
      /* format the gain */
      sprintf(szline, "  %5u", 
             (WORD)(((float)HDAMP_REF_FULL_GAIN * (float)nHDAMP_gain_values[i])
	     		 / (float)4095) );
      vst_color(screen->handle, ncolor);
      v_gtext( screen->handle, wbox[0] + (i *NUM_SPACES_PER_ENTRY * UI_cellw),
      				wbox[1] + UI_cellh, szline );

      /* now format the filters setting line */
      /* first do the low cut filter setting - the highest freq will override*/
      j = nHDAMP_filter_values[i];

      /* set default */
      strcpy( szline, "????");

      if ( (j & 0x001F) == 0 ){
        strcpy( szline, " 0.1 ");
      }
      if (j & HDAMP_LOWCUT_1HZ){
        strcpy( szline,"   1 ");
      }
      if (j & HDAMP_LOWCUT_10HZ){
        strcpy( szline,"  10 ");
      }
      if (j & HDAMP_LOWCUT_100HZ){
        strcpy( szline," 100 ");
      }
      if ((j & HDAMP_LOWCUT_900HZ) ==HDAMP_LOWCUT_300HZ){
        strcpy( szline," 300 ");      
      }
      if ((j & HDAMP_LOWCUT_900HZ) ==HDAMP_LOWCUT_600HZ){
        strcpy( szline," 600 ");
      }
      if ((j & HDAMP_LOWCUT_900HZ)== HDAMP_LOWCUT_900HZ){
        strcpy( szline," 900 ");
      }

      /* set the default to ???? */
      strcpy( &szline[5],  " ????");
      /* now do the high freqs */
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ)) 
      					== HDAMP_HICUT_50HZ ){
	strcpy(&szline[5], "  50 ");
      }
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_100HZ  ){
        strcpy(&szline[5], " 125 ");
      }
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_200HZ  ){
        strcpy(&szline[5], " 200 ");
      }

      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_250HZ  ){
        strcpy(&szline[5], " 250 ");
      }
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_275HZ  ){
        strcpy(&szline[5], " 275 ");
      }
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_325HZ  ){
        strcpy(&szline[5], " 325 ");
      }
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_400HZ  ){
        strcpy(&szline[5], " 400 ");
      }
      if ((j & (HDAMP_HICUT_475HZ | HDAMP_HICUT_9KHZ))==
      					HDAMP_HICUT_475HZ  ){
        strcpy(&szline[5], " 475 ");
      }

      if ((j & HDAMP_HICUT_9KHZ) == HDAMP_HICUT_3KHZ ){
        strcpy(&szline[5], " 3KHz");
      }
      if ((j & HDAMP_HICUT_9KHZ) == HDAMP_HICUT_6KHZ ){
        strcpy(&szline[5], " 6KHz");
      }
      if ((j & HDAMP_HICUT_9KHZ) == HDAMP_HICUT_9KHZ ){
        strcpy(&szline[5], " 9KHz");
      }
      v_gtext( screen->handle, 
                     wbox[0] + (5 + (i*NUM_SPACES_PER_ENTRY * UI_cellw)),
      		     wbox[1], 
		     szline );

/*      sprintf(szline, "%7d ", nHDAMP_filter_values[i]);
      v_gtext( screen->handle, 
                     wbox[0] + (5 + (i*NUM_SPACES_PER_ENTRY * UI_cellw)),
      		     wbox[1] + 2*UI_cellh, 
		     szline );
*/

    }
    return(0);
}
/*====================================================================*/
int nHDAMP_ReadFileInitAmps(void)
/* This is to be called from other functions to read the setup, init the 
 * amps and do the display to the screen.
 */
{
    int i;
    WORD wdefault_box[4] = {1,1,800, 50 };
        
    i = nHDAMP_ReadSetupFile( "hd_amp.amp");
    /* if the file existed then init the amps and set the values read */
    if (i >=0 ){
      nHDAMP_InitAmp();
      /* turn off all equalization switches as this can cause a low cut
       * filter setting of about 300 hz.
       */
      nHDAMP_SetEqualizeSwitches( 0 );
      /* load the filter  and gain settings read in from the file */
      nHDAMP_SetFilterGainList(nHDAMP_filter_values, nHDAMP_gain_values);
      nHDAMP_DisplaySettings( -1, HDAMP_ALL_CHANNELS, wdefault_box );

    }else{
      vst_color( screen->handle, WHITE );
      v_gtext( screen->handle, wdefault_box[0], wdefault_box[1], 
      				"Error Reading Amplifier Setup File");
      return(i);
    }
    /* all successful - return 0 */
    return( 0 );
 
}


/*====================================================================*/
/*====================================================================*/
int nCS_HDAMP_InteractiveSetup( int nprocessing_state )
/* This will interact with the user to setup the amplifiers.
 * Services are provided to save settings to the file and to read settings
 * from a file.  There is a display which will show the current settings of the
 * amplifier.  
 *   Command keys are:
 *	- ^A All Channels Select
 *	  ^N Next Channel
 *	  ^P Previous Channel
 *	- ^G Gain Prompt
 *	  ^U Up Gain by 1 digital count
 *	  ^D Down Gain by 1 digital count
 *	- ^L Low Cut Filter Cycle
 *	  ^H High Cut Filter Cycle
 *	- ^E Equalize DC Component for Current Channel(s)
 *	- ^R Read Setup File "\bw\cfgs\hd_amp.amp" and set amp channels
 *	  ^W Write Setup File "\bw\cfgs\hd_amp.amp" with amp channel's settings
 *	- ^X eXit Sequence
 *
 * The Current Channel will be shown in bright white.  Unselected channels
 * are shown in the respective Discovery "cluster" number color.
 * When the sequence is eXitted with ^X the current settings will be displayed
 * in the respective channel's "cluster number" color to indicate that no 
 * channel is selected.
 */
{
#define HDAMP_IS_USER_WIN	4
#define INTERACT 		2
#define TURN_OFF_EQ_SWITCHES	3

    static BOOL bfirst_time_in_setup=TRUE;
    static int ncur_channel;
    int i, j, nnew_gain_value, nlow, nhi, nlcf_setting, nhcf_setting;
    DWORD dwnew_gain, dwts_start;
    
    switch (nprocessing_state){
    case INITIAL_STATE:
      /* if we have any valid values in the gain or the filter value arrays 
       * then don't change the values.
       */
      for (i=0; i<8; i++){
        if ( nHDAMP_gain_values[i] != 0   ||
   	     nHDAMP_filter_values[i] != 0   ){
	  bfirst_time_in_setup = FALSE;
	  break;
	}
      }

      if (bfirst_time_in_setup){
        /* init the amp */
        nHDAMP_InitAmp();
        /* give some bogus values to let us know it ran */
        for (i=0; i<8; i++){
	  nHDAMP_filter_values[i] = HDAMP_LOWCUT_TENTHHZ | HDAMP_HICUT_9KHZ;
          nHDAMP_gain_values[i] = i+1;
	}
        nHDAMP_SetFilterGainList(nHDAMP_filter_values, nHDAMP_gain_values);
      }
      vSU_DrawBox( user_windows[HDAMP_IS_USER_WIN].work, WHITE );
      nHDAMP_DisplaySettings( -1, HDAMP_ALL_CHANNELS,
      			 user_windows[HDAMP_IS_USER_WIN].work );
      nHDAMP_DisplaySettings( ncur_channel, ncur_channel,
      			 user_windows[HDAMP_IS_USER_WIN].work );

      /* if it is the first time in the setup program set all eq switches */
      if (bfirst_time_in_setup){
        bfirst_time_in_setup = FALSE;
        nHDAMP_SetEqualizeSwitches(0x00FF);
        vCS_SetNextState( TURN_OFF_EQ_SWITCHES, 3L );
      }else{
        vCS_SetNextState( INTERACT, 250L);
      }
      break;

    case TURN_OFF_EQ_SWITCHES:
      nHDAMP_SetEqualizeSwitches( 0);
      vCS_SetNextState( INTERACT , 250L );
      break;

    case INTERACT:
      /* assume the user will hit a key again - bump the poll rate of the CS */
      vCS_SetNextState( INTERACT, 120L);

      switch( nlast_keystroke ){

      case 1:	/* ^A for ALL channel select */
        ncur_channel = HDAMP_ALL_CHANNELS;
        nHDAMP_DisplaySettings( ncur_channel, ncur_channel,
      			 user_windows[HDAMP_IS_USER_WIN].work );
        break;

      case 4:	/* ^D for Down in Gain */
        if ( ncur_channel == HDAMP_ALL_CHANNELS){
	  nlow = 0;
	  nhi = 7;
	}else{
	  nlow = ncur_channel;
	  nhi = ncur_channel;
	}
	for (i=nlow; i<=nhi; i++){
	  if ( nHDAMP_gain_values[i] >0){
	    nHDAMP_gain_values[i]--;
            nHDAMP_LoadSingleGainVal( i, nHDAMP_gain_values[i] );
	  }
	}
        nHDAMP_DisplaySettings( ncur_channel, ncur_channel,
				 user_windows[HDAMP_IS_USER_WIN].work );
        break;

      case 5:	/* ^E for Equalize all channels */
        if ( ncur_channel == HDAMP_ALL_CHANNELS ){
	  i = 0x00FF;
	}else{
	  i = 1 << ncur_channel;
	}
        /* set the equalization switches for 3 milliseconds */
        nHDAMP_SetEqualizeSwitches( i );

        dwts_start = lCTM05TSRead();
	dwts_start += 30L;   /* set for 3 milliseconds */
        for (;;){
	  if ( dwts_start <= lCTM05TSRead() )break;
	}
        nHDAMP_SetEqualizeSwitches( 0 );

/*   	vCS_SetNextState( TURN_OFF_EQ_SWITCHES , 3L );
 */
        break;

      case 7:	/* ^G for gain setting prompt */
        if ( ncur_channel == HDAMP_ALL_CHANNELS){
          dwnew_gain = (DWORD)nHDAMP_gain_values[0] * 
			(DWORD)((float)HDAMP_REF_FULL_GAIN/(float)4095);
	  i = nCS_GetDWORDFromUser( "Enter New Gain for ALL Channels", 0L, 
 				(LONG)HDAMP_REF_FULL_GAIN, &dwnew_gain);
          if (i<0) break;    /* see if user hit esc */
          nnew_gain_value = (int)( (float)dwnew_gain *
	  			   ((float)4095 / (float)HDAMP_REF_FULL_GAIN));
	  for (i=0; i<8; i++){
	    nHDAMP_gain_values[i] = nnew_gain_value;
            nHDAMP_LoadSingleGainVal( i, nHDAMP_gain_values[i] );
	  }
	}else{
	  /* we have only a single channel selected */
          dwnew_gain = (DWORD)((float)nHDAMP_gain_values[ncur_channel] * 
	  			((float)HDAMP_REF_FULL_GAIN/(float)4095));
	  i = nCS_GetDWORDFromUser( "Enter New Gain for Current Channel", 0L, 
 				(LONG)HDAMP_REF_FULL_GAIN, &dwnew_gain);
          if (i<0) break;    /* see if user hit esc */
          /* add a half step up so that we round to the closest value */
	  dwnew_gain += (DWORD)
	                (((float)HDAMP_REF_FULL_GAIN/(float)4095)/(float)2);
          nnew_gain_value = (int)((float)dwnew_gain *
	  			  ((float)4095 / (float)HDAMP_REF_FULL_GAIN));
	  nHDAMP_gain_values[ncur_channel] = nnew_gain_value;
          nHDAMP_LoadSingleGainVal( ncur_channel, 
	  				nHDAMP_gain_values[ncur_channel] );
	}
	/* update the display */
        nHDAMP_DisplaySettings( ncur_channel, ncur_channel,
				 user_windows[HDAMP_IS_USER_WIN].work );
		    
        break;

      case 8:	/* ^H for high cut filter change */
        /* set nlow and nhi based on the current selected channel(s) */
        if (ncur_channel == HDAMP_ALL_CHANNELS){
	  nlow = 0;
	  nhi = 7;
	}else{
          /* only a single channel */
	  nlow = ncur_channel;
	  nhi = ncur_channel;
	}

        /* read the current filter value for the lowest selected channel */
        nhcf_setting = nHDAMP_filter_values[nlow] & 0x03E0;
        /* bump to the next higher filter frequency */
	switch (nhcf_setting){
        case HDAMP_HICUT_50HZ:
          nhcf_setting = HDAMP_HICUT_100HZ;
	  break;
        case HDAMP_HICUT_100HZ:
          nhcf_setting = HDAMP_HICUT_200HZ;
	  break;
        case HDAMP_HICUT_200HZ:
          nhcf_setting = HDAMP_HICUT_250HZ;
	  break;
        case HDAMP_HICUT_250HZ:
          nhcf_setting = HDAMP_HICUT_275HZ;
	  break;
        case HDAMP_HICUT_275HZ:
          nhcf_setting = HDAMP_HICUT_325HZ;
	  break;
        case HDAMP_HICUT_325HZ:
          nhcf_setting = HDAMP_HICUT_400HZ;
	  break;
        case HDAMP_HICUT_400HZ:
          nhcf_setting = HDAMP_HICUT_475HZ;
	  break;
        case HDAMP_HICUT_475HZ:
          nhcf_setting = HDAMP_HICUT_3KHZ;
	  break;
        case HDAMP_HICUT_3KHZ:
          nhcf_setting = HDAMP_HICUT_6KHZ;
	  break;
        case HDAMP_HICUT_6KHZ:
          nhcf_setting = HDAMP_HICUT_9KHZ;
	  break;
        case HDAMP_HICUT_9KHZ:
          nhcf_setting = HDAMP_HICUT_50HZ;
	  break;
        default:
          nhcf_setting = HDAMP_HICUT_9KHZ;
	  break;
        }

	/* now set all the selected channels to the new value */
	for (i=nlow; i<=nhi; i++){
	  /* OR in the low bit values for the high cut filter settings */
	  nHDAMP_filter_values[i] = 
	  		nhcf_setting | (nHDAMP_filter_values[i] & 0x001F);
          /* set the filter value in the amp for the ith channel */
          nHDAMP_LoadSingleFilterVal( i, nHDAMP_filter_values[i] );
	}
        /* show the current channel(s) new data */
        nHDAMP_DisplaySettings( ncur_channel, ncur_channel,
				 user_windows[HDAMP_IS_USER_WIN].work );
        break;

      case 12:	/* ^L for low cut filter change */
        /* set the high and low channel numbers which will be changed */
        if (ncur_channel == HDAMP_ALL_CHANNELS){
	  nlow = 0;
	  nhi = 7;
	}else{
          /* only a single channel */
	  nlow = ncur_channel;
	  nhi = ncur_channel;
	}

	/* read the setting of the first channel to change */
	nlcf_setting = nHDAMP_filter_values[nlow] & 0x001F;
        /* and bump the value to the next higher freq */
	switch (nlcf_setting){
        case HDAMP_LOWCUT_TENTHHZ:
          nlcf_setting = HDAMP_LOWCUT_1HZ;
	  break;
        case HDAMP_LOWCUT_1HZ:
          nlcf_setting = HDAMP_LOWCUT_10HZ;
	  break;
        case HDAMP_LOWCUT_10HZ:
          nlcf_setting = HDAMP_LOWCUT_100HZ;
	  break;
        case HDAMP_LOWCUT_100HZ:
          nlcf_setting = HDAMP_LOWCUT_300HZ;
	  break;
        case HDAMP_LOWCUT_300HZ:
          nlcf_setting = HDAMP_LOWCUT_600HZ;
	  break;
        case HDAMP_LOWCUT_600HZ:
          nlcf_setting = HDAMP_LOWCUT_900HZ;
	  break;
        case HDAMP_LOWCUT_900HZ:
          nlcf_setting = HDAMP_LOWCUT_TENTHHZ;
	  break;
        default:
          nlcf_setting = HDAMP_LOWCUT_1HZ;
          break;
        }

        /* set the new value for all the channels selected */
	for (i=nlow; i<=nhi; i++){
	  /* OR in the high bit values for the high cut filter settings */
	  nHDAMP_filter_values[i] = 
	  		nlcf_setting | (nHDAMP_filter_values[i] & 0x03E0);
          /* set the filter value in the amp for the ith channel */
          nHDAMP_LoadSingleFilterVal( i, nHDAMP_filter_values[i] );
	}
        /* show the current channel(s) new data */
        nHDAMP_DisplaySettings( ncur_channel, ncur_channel,
				 user_windows[HDAMP_IS_USER_WIN].work );
        break;

      case 14:	/* ^N for next higher channel select */
        /* unhighlight the current channel */
        nHDAMP_DisplaySettings( -1, ncur_channel,
				 user_windows[HDAMP_IS_USER_WIN].work );

        /* if current channel is ALL_CHANNELS or the top one set current to 0*/
        if ( (ncur_channel == HDAMP_ALL_CHANNELS) || (ncur_channel == 7)){
	  ncur_channel = 0;
	}else{
          /* just bump the channel # */
	  ncur_channel++;
	}

        /* show the current channel as highlighted */
        nHDAMP_DisplaySettings( ncur_channel, ncur_channel,
				 user_windows[HDAMP_IS_USER_WIN].work );
        break;
	
      case 16:	/* ^P for previous channel select */
        /* unhighlight the current channel */
        nHDAMP_DisplaySettings( -1, ncur_channel,
				 user_windows[HDAMP_IS_USER_WIN].work );

        /* if current channel is ALL_CHANNELS set current to 0*/
        if ( (ncur_channel == HDAMP_ALL_CHANNELS) ){
	  ncur_channel = 0;
	}else{
          /* just bump the channel # - roll over to 7 if it is now 0 */
          if (ncur_channel == 0){
	    ncur_channel = 7;
	  }else{
            ncur_channel--;
	  }
	}

        /* show the current channel as highlighted */
        nHDAMP_DisplaySettings( ncur_channel, ncur_channel,
				 user_windows[HDAMP_IS_USER_WIN].work );

        break;

      case 18:	/* ^R for read from file */
        j = nHDAMP_ReadSetupFile( "hd_amp.amp");
	if (j<0){
	  UI_DisplayError( UI_ERROR, "HD Amps CS Error",
	  				"Error Reading Amp Settings File");
          break;
	}
        nHDAMP_SetFilterGainList(nHDAMP_filter_values, nHDAMP_gain_values);
        nHDAMP_DisplaySettings( ncur_channel, HDAMP_ALL_CHANNELS,
				 user_windows[HDAMP_IS_USER_WIN].work );
        break;
	
      case 23:	/* ^W for write to file */
        j = nHDAMP_WriteSetupFile( "hd_amp.amp");
	if (j<0){
	  UI_DisplayError( UI_ERROR, "HD Amps CS Error",
  				"Error Writing Amp Settings File");
	}
        break;

      case 21:	/* ^U for up Gain */
        if ( ncur_channel == HDAMP_ALL_CHANNELS){
	  nlow = 0;
	  nhi = 7;
	}else{
	  nlow = ncur_channel;
	  nhi = ncur_channel;
	}
	for (i=nlow; i<=nhi; i++){
	  if ( nHDAMP_gain_values[i] < 4095){
	    nHDAMP_gain_values[i]++;
            nHDAMP_LoadSingleGainVal( i, nHDAMP_gain_values[i] );
	  }
	}
        nHDAMP_DisplaySettings( ncur_channel, ncur_channel,
				 user_windows[HDAMP_IS_USER_WIN].work );
        break;

      case 24:	/* ^X for eXit */
        vCS_SetNextState( TERMINATION_STATE, 100L);
        break;

      default:
        /* just set poll time to 1 sec. */
        vCS_SetNextState( INTERACT, 250L);
        break;
      }
      nlast_keystroke = 0;  /* reset it for next key hit from user */
      break;

    case TERMINATION_STATE:
      vSU_DrawBox( user_windows[HDAMP_IS_USER_WIN].work, BLUE );
      nHDAMP_DisplaySettings( -1, HDAMP_ALL_CHANNELS,
      			 user_windows[HDAMP_IS_USER_WIN].work );
      vCS_SetNextState( IDLE_STATE, 0L );
      break;

    default:
      UI_DisplayError(UI_ERROR, "Clocked Sequence Error",
		        "Interactive HD AMP Setup CS Hit Default State");
      return(-1);   /* this will abort the sequence */
      break;

    }  /* end of nprocessing_state switch block */

    return (0);
    
}
/*====================================================================*/
/*====================================================================*/



