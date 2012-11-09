#include "adext.h"
int box[8];

/*
************************************************************************
**
**   Display Routines
**
************************************************************************
*/
#ifdef BORGRAPH

/* Returns the color for white */
long WhitePixel()
{
  switch(getmaxcolor()) {
    case 32768: return 0x7fffL;
    case 65535: return 0xffffL;
   case 16777: return 0xffffffL;
   default   : return 15;
  };
}

int huge DetectVGA256()
{
  int Vid;

  printf("Which video mode would you like to use? \n");
  printf("  0) 320x200x256\n");
  printf("  1) 640x400x256\n");
  printf("  2) 640x480x256\n");
  printf("  3) 800x600x256\n");
  printf("  4) 1024x768x256\n");
  printf("  5) 640x480x256\n");
  printf("  6) 1280x1024x256\n");
  printf("\n> ");
  scanf("%d",&Vid);
  return Vid;
}

int huge DetectS3()
{
  int Vid;

  printf("Which video mode would you like to use?\n");
  printf("  0) 640x480x256\n");
  printf("  1) 800x600x256\n");
  printf("  2) 1024x768x256\n");
  printf("  3) 800x600x16\n");
  printf("  4) 1024x768x16\n");
  printf("  5) 1280x960x16\n");
  printf("  6) 1280x1024x16\n");
  printf("  7) 640x480x32768\n");
  printf("> ");
  scanf("%d",&Vid);
  return Vid;
}
int huge DetectVGA64k()
{
  int Vid;

  printf("Which video mode would you like to use? \n");
  printf("  0) 320x200x65536\n");
  printf("  1) 640x350x65536\n");
  printf("  2) 640x400x65536\n");
  printf("  3) 640x480x65536\n");
  printf("  4) 800x600x65536\n");
  printf("  5) 1024x768x65536\n");
  printf("  6) 1280x1024x65536\n");
  printf("\n> ");
  scanf("%d",&Vid);
  return Vid;
}

int huge DetectVGA24bit()
{
  int Vid;

  printf("Which video mode would you like to use? \n");
  printf("  0) 320x200x24-bit\n");
  printf("  1) 640x350x24-bit\n");
  printf("  2) 640x400x24-bit\n");
  printf("  3) 640x480x24-bit\n");
  printf("  4) 800x600x24-bit\n");
  printf("  5) 1024x768x24-bit\n");
  printf("  6) 1280x1024x24-bit\n");
  printf("\n> ");
  scanf("%d",&Vid);
  return Vid;
}

int huge DetectVGA32k()
{
  int Vid;

  printf("Which video mode would you like to use? \n");
  printf("  0) 320x200x32768\n");
  printf("  1) 640x350x32768\n");
  printf("  2) 640x400x32768\n");
  printf("  3) 640x480x32768\n");
  printf("  4) 800x600x32768\n");
  printf("  5) 1024x768x32768\n");
  printf("  6) 1280x1024x32768\n");
  printf("\n> ");
  scanf("%d",&Vid);
  return Vid;
}

#ifdef NOTVGA
int huge DetectVGA16()
{
  int Vid;

  printf("Which video mode would you like to use? \n");
  printf("  0) 320x200x16\n");
  printf("  1) 640x200x16\n");
  printf("  2) 640x350x16\n");
  printf("  3) 640x480x256\n");
  printf("  4) 800x600x16\n");
  printf("  5) 1024x768x16\n");
  printf("  6) 1280x1024x16\n");
  printf("\n> ");
  scanf("%d",&Vid);
  return Vid;
}
#else
int huge DetectVGA16()
{
  return 4;
}
#endif

int huge DetectTwk16()
{
  int Vid;

  printf("Which video mode would you like to use? \n");
  printf("  0) 704x528x16\n");
  printf("  1) 720x540x16\n");
  printf("  2) 736x552x16\n");
  printf("  3) 752x564x16\n");
  printf("  4) 768x576x16\n");
  printf("  5) 784x588x16\n");
  printf("  6) 800x600x16\n");
  printf("\n> ");
  scanf("%d",&Vid);
  return Vid;
};

int huge DetectTwk256()
{
  int Vid;

  printf("Which video mode would you like to use? \n");
  printf("  0) 320x400x256\n");
  printf("  1) 320x480x256\n");
  printf("  2) 360x480x256\n");
  printf("  3) 376x564x256\n");
  printf("  4) 400x564x256\n");
  printf("  5) 400x600x256\n");
  printf("  6) 320x240x256\n");
  printf("\n> ");
  scanf("%d",&Vid);
  return Vid;
};

#ifdef NOTSVGA
void SelectGraphics(void)
{
  int  Gd = DETECT, Gm;
  int  Drv;
  char GrErr;

  /* Find out which driver the user wants */
  printf("Which driver would you like to use?\n");
  printf("  0) Svga16\n");
  printf("  1) Svga256\n");
  printf("  2) Svga32k\n");
  printf("  3) Svga64k\n");
  printf("  4) SvgaTC\n");
  printf("  5) SvgaS3\n");
  printf("  6) Tweak256\n");
  printf("  7) Tweak16\n");
  printf("\n> ");
  scanf("%d",&Drv);
  switch(Drv)
  {
   case 0: installuserdriver("Svga16",DetectVGA16);
/*  If driver is linked with file, remove comments */
/*        registerfarbgidriver(Svga16_fdriver);  */
      break;

   case 1: installuserdriver("Svga256",DetectVGA256);
/*  If driver is linked with file, remove comments */
/*        registerfarbgidriver(Svga256_fdriver); */
      break;

   case 2: installuserdriver("Svga32k",DetectVGA32k);
/*  If driver is linked with file, remove comments */
/*        registerfarbgidriver(Svga32k_fdriver);  */
      break;

   case 3: installuserdriver("Svga64k",DetectVGA64k);
/*  If driver is linked with file, remove comments */
/*        registerfarbgidriver(Svga64k_fdriver);  */
      break;

   case 4: installuserdriver("SvgaTC",DetectVGA24bit);
/*  If driver is linked with file, remove comments */
/*        registerfarbgidriver(SvgaTC_fdriver);  */
      break;

/*   case 5: installuserdriver("SvgaS3",DetectVGAS3); */
/*  If driver is linked with file, remove comments */
/*        registerfarbgidriver(SvgaS3_fdriver);  */
//      break;

   case 6: installuserdriver("Twk16",DetectTwk16);
/*  If driver is linked with file, remove comments */
/*        registerfarbgidriver(Twk16_fdriver);  */
      break;

   case 7: installuserdriver("Twk256",DetectTwk256);
/*  If driver is linked with file, remove comments */
/*        registerfarbgidriver(Twk256_fdriver);  */
      break;
  }
}

#else
void SelectGraphics(void) 
{
   installuserdriver("Svga16", DetectVGA16);
}
#endif

void Pause(void)
{
  static char msg[] = "Esc aborts or press a key...";
  int c;

  StatusLine( msg );       /* Put msg at bottom of screen   */

  c = getch();         /* Read a character from kbd   */

  if( ESC == c ){       /* Does user wish to leave?   */
   closegraph();       /* Change to text mode     */
   exit( 1 );         /* Return to OS      */
  }

  if( 0 == c ){        /* Did use hit a non-ASCII key? */
   c = getch();       /* Read scan code for keyboard   */
  }

  cleardevice();       /* Clear the screen     */
}



/*                     */
/*   STATUSLINE: Display a status line at the bottom of the screen.   */
/*                     */

void StatusLine( char *msg )
{
  int height;

  setviewport( 0, 0, MaxX, MaxY, 0 );   /* Open port to full screen   */
  setcolor( MaxColors - 1 );     /* Set current color to white   */

  changetextstyle( DEFAULT_FONT, HORIZ_DIR, 1 );
  settextjustify( CENTER_TEXT, TOP_TEXT );
  setlinestyle( SOLID_LINE, 0, NORM_WIDTH );
  setfillstyle( EMPTY_FILL, 0 );

  height = textheight( "H" );         /* Detemine current height     */
  bar( 0, MaxY-(height+4), MaxX, MaxY );
  rectangle( 0, MaxY-(height+4), MaxX, MaxY );
  outtextxy( MaxX/2, MaxY-(height+2), msg );
  setviewport( 1, height+5, MaxX-1, MaxY-(height+5), 0 );

}

/*                     */
/*   DRAWBORDER: Draw a solid single line around the current   */
/*   viewport.                */
/*                     */

void DrawBorder(void)
{
  struct viewporttype vp;

  setcolor( MaxColors - 1 );     /* Set current color to white   */

  setlinestyle( SOLID_LINE, 0, NORM_WIDTH );

  getviewsettings( &vp );
  rectangle( 0, 0, vp.right-vp.left, vp.bottom-vp.top );

}


/*                     */
/*   CHANGETEXTSTYLE: similar to settextstyle, but checks for   */
/*   errors that might occur whil loading the font file.     */
/*                     */

void changetextstyle(int font, int direction, int charsize)
{
  int ErrorCode;

  graphresult();       /* clear error code     */
  settextstyle(font, direction, charsize);
  ErrorCode = graphresult();     /* check result      */
  if( ErrorCode != grOk ){     /* if error occured     */
   closegraph();
   printf(" Graphics System Error: %s\n", grapherrormsg( ErrorCode ) );
   exit( 1 );
  }
}

/*                     */
/*   GPRINTF: Used like PRINTF except the output is sent to the   */
/*   screen in graphics mode at the specified co-ordinate.     */
/*                     */

int gprintf( int *xloc, int *yloc, char *fmt, ... )
{
  va_list  argptr;       /* Argument list pointer   */
  char str[140];       /* Buffer to build sting into   */
  int cnt;         /* Result of SPRINTF for return */

  va_start( argptr, fmt );     /* Initialize va_ functions   */

  cnt = vsprintf( str, fmt, argptr );   /* prints string to buffer   */
  outtextxy( *xloc, *yloc, str );   /* Send string in graphics mode */
  *yloc += textheight( "H" ) + 2;      /* Advance to next line       */

  va_end( argptr );       /* Close va_ functions     */

  return( cnt );       /* Return the conversion count   */

}
/*                     */
/*   MAINWINDOW: Establish the main window for the demo and set   */
/*   a viewport for the demo code.            */
/*                     */

void MainWindow( char *header )
{
  int height;

  cleardevice();       /* Clear graphics screen   */
  setcolor( MaxColors - 1 );     /* Set current color to white   */
  setviewport( 0, 0, MaxX, MaxY, 0 );   /* Open port to full screen   */

  height = textheight( "H" );         /* Get basic text height      */

  changetextstyle( DEFAULT_FONT, HORIZ_DIR, 1 );
  settextjustify( CENTER_TEXT, TOP_TEXT );
  outtextxy( MaxX/2, 2, header );
  setviewport( 0, height+4, MaxX, MaxY-(height+4), 0 );
  DrawBorder();
  setviewport( 1, height+5, MaxX-1, MaxY-(height+5), 0 );
  settextjustify( LEFT_TEXT, TOP_TEXT );
}


/*   INITIALIZE: Initializes the graphics system and reports   */
/*   any errors which occured.            */
/*                     */

void Initialize(void)
{
  int xasp, yasp;       /* Used to read the aspect ratio*/

  GraphDriver = DETECT;      /* Request auto-detection   */
  initgraph( &GraphDriver, &GraphMode, "" );
  ErrorCode = graphresult();     /* Read result of initialization*/
  if( ErrorCode != grOk ){     /* Error occured during init   */
   printf(" Graphics System Error: %s\n", grapherrormsg( ErrorCode ) );
   exit( 1 );
  }

  getpalette( &palette );     /* Read the palette from board   */
  MaxColors = getmaxcolor() + 1;   /* Read maximum number of colors*/

  MaxX = getmaxx();
  MaxY = getmaxy();       /* Read size of screen     */

  getaspectratio( &xasp, &yasp );   /* read the hardware aspect   */
  AspectRatio = (double)xasp / (double)yasp; /* Get correction factor   */

}

/*                     */
/*   REPORTSTATUS: Report the current configuration of the system   */
/*   after the auto-detect initialization.         */
/*                     */

void ReportStatus(void)
{
  struct viewporttype    viewinfo;   /* Params for inquiry procedures*/
  struct linesettingstype lineinfo;
  struct fillsettingstype fillinfo;
  struct textsettingstype textinfo;
  struct palettetype    palette;

  char *driver, *mode;       /* Strings for driver and mode   */
  int x, y;
  char temp[200];

  settextjustify( LEFT_TEXT, TOP_TEXT );
  getviewsettings( &viewinfo );
  getlinesettings( &lineinfo );
  getfillsettings( &fillinfo );
  gettextsettings( &textinfo );
  getpalette( &palette );

  x = 4;
  y = 10;

  MainWindow( "Status report after InitGraph" );

  driver = getdrivername();
  mode = getmodename(GraphMode);   /* get current setting     */

  gprintf( &x, &y, "Graphics device   : %-20s (%d)", driver, GraphDriver );
  gprintf( &x, &y, "Graphics mode     : %-20s (%d)", mode, GraphMode );
  gprintf( &x, &y, "Screen resolution  : ( 0, 0, %d, %d )", getmaxx(), getmaxy() );

  gprintf( &x, &y, "Current view port  : ( %d, %d, %d, %d )",
  viewinfo.left, viewinfo.top, viewinfo.right, viewinfo.bottom );
  gprintf( &x, &y, "Clipping         : %s", viewinfo.clip ? "ON" : "OFF" );

  gprintf( &x, &y, "Current position   : ( %d, %d )", getx(), gety() );
  gprintf( &x, &y, "Colors available   : %d", MaxColors );
  gprintf( &x, &y, "Current color     : %d", getcolor() );

  gprintf( &x, &y, "Line style       : %s", LineStyles[ lineinfo.linestyle ] );
  gprintf( &x, &y, "Line thickness    : %d", lineinfo.thickness );

  gprintf( &x, &y, "Current fill style : %s", FillStyles[ fillinfo.pattern ] );
  gprintf( &x, &y, "Current fill color : %d", fillinfo.color );

  gprintf( &x, &y, "Current font      : %s", Fonts[ textinfo.font ] );
  gprintf( &x, &y, "Text direction    : %s", TextDirect[ textinfo.direction ] );
  gprintf( &x, &y, "Character size    : %d", textinfo.charsize );
  gprintf( &x, &y, "Horizontal justify : %s", HorizJust[ textinfo.horiz ] );
  gprintf( &x, &y, "Vertical justify   : %s", VertJust[ textinfo.vert ] );

  Pause();
}

void drwbox(int mode, int color, int x,int y, int x2, int y2)
{
   setwritemode(mode);
   setcolor(color);
   rectangle(x,y,x2,y2);
}

void drwstring(int mode, int color, char *string, int x, int y)
{
   setcolor(color);
   outtextxy(x,y,string);
}

#ifdef OLD
void drwpoint(int mode,int color,int x, int y)
{
   setwritemode(mode);
   putpixel(x,y,color);
}
#endif
#define drwpoint(M,C,X,Y)   putpixel(X,Y,C)

void drwfillbox(int mode, int color, int x,int y, int x2, int y2)
{
   setcolor(color);
   setwritemode(mode);
  setfillstyle( SOLID_FILL, color );
  bar( x, y, x2,y2);
}


void drwline(int mode, int color, int x, int y, int x2, int y2)
{
   setwritemode(mode);
   setcolor(color);
   line(x,y,x2,y2);
}
#endif
void SystemSetup(void)
{
#ifdef BORGRAPH
extern void Initialize();
extern void ReportStatus();
extern void SelectGraphics();

  SelectGraphics();
  Initialize();      /* Set system into Graphics mode   */
  //ReportStatus();     /* Report results of the initialization */
  settextstyle(DEFAULT_FONT,HORIZ_DIR,1);
  settextjustify(LEFT_TEXT, TOP_TEXT);
#else
   sysinfo.cpu = whichcpu();
   sysinfo.mouse = whichmouse();
   if((sysinfo.vga = whichvga()) == 0){
     fprintf(stderr,"ERROR: unknown video card\n");
     exit(-1);
   }
   sysinfo.vgamem = whichmem();
   setcard(sysinfo.vga,sysinfo.vgamem);
   res800();
   sysinfo.videomode = videomodeget();
   if (sysinfo.mouse > 0) {
     mouseenter();
     mousecursordefault();
     mouseshow();
     mouselocset(0,0);
     sysinfo.mouseactive = 1;
   } else {
     sysinfo.mouseactive = 0;
   }
#endif
   DisplaySetup();
}

void SetStringPositions(void)
{
    /* current time location */
    sysinfo.currenttimeview.x = 112;
    sysinfo.currenttimeview.y = 0;
    sysinfo.currenttimeview.x2 = 180;
    sysinfo.currenttimeview.y2 = sysinfo.theight;
    /* remaining time location */
    sysinfo.remaintimeview.x = 112;
    sysinfo.remaintimeview.y = sysinfo.theight + 4;
    sysinfo.remaintimeview.x2 = 180;
    sysinfo.remaintimeview.y2 = 2 * sysinfo.theight + 4;
    /* file size location */
    sysinfo.filesizeview.x = 305;
    sysinfo.filesizeview.y = 0;
    sysinfo.filesizeview.x2 = 397;
    sysinfo.filesizeview.y2 = sysinfo.theight;
    /* disk free space location */
    sysinfo.diskfreeview.x = 305;
    sysinfo.diskfreeview.y = sysinfo.theight + 4;
    sysinfo.diskfreeview.x2 = 397;
    sysinfo.diskfreeview.y2 = 2 * sysinfo.theight + 4;
    // rate display
    sysinfo.rateview.x = 403;
    sysinfo.rateview.y = sysinfo.theight + 4;
    sysinfo.rateview.x2 = 550;
    sysinfo.rateview.y2 = 2 * sysinfo.theight + 4;
    // filter display 
    sysinfo.filterview.x = 573;
    sysinfo.filterview.y = sysinfo.theight + 4;
    sysinfo.filterview.x2 = 799;
    sysinfo.filterview.y2 = 2 * sysinfo.theight + 4;
    // input string 
    sysinfo.inputview.x = 5;
    sysinfo.inputview.y = 549;
    sysinfo.inputview.x2 = 350;
    sysinfo.inputview.y2 = 557;
    // Version string
    sysinfo.versionview.x = 412;
    sysinfo.versionview.y = 0;
    sysinfo.versionview.x2 = 800;
    sysinfo.versionview.y2 = sysinfo.theight;
}
        
void DisplaySetup()
{
   int i, j;
   int tmp;
   int buttonx, buttony;
   ElectrodeInfo *electrode;    
   ChannelInfo     *cptr;    
   View     *view;

   /* get the text height and width */
   sysinfo.theight = textheight("A");
   sysinfo.twidth = textwidth("A");

   /* set the coordinates for the string displays */
   /* note that these coordinates define the beginning of the alterable text
    (ie. in the time string, the x coordinate is the location of the hour */
   SetStringPositions();

    /* two "spike", two "projection" buttons */
    sysinfo.nspbuttons = 4;
    /* twelve buttons in all */
    sysinfo.nbuttons = 12;


   /* setup the spike and projection boxes for the first electrode */
   /* With 4 electrodes, there are six possible projections */
   /* put the spikes and projections in reasonable places */
   /* note that the buttons are all defined at the end of this routine so
      that the channel buttons are continguous and are followed by the 
      spike and projection buttons in the button list */
   /*
   ** ELECTRODE 1
   */

   /* setup the first electrode's spike display positions */
   electrode = adinfo.electrode;
   for (i = 0; i < adinfo.nelect_chan; i++) {
      electrode->spikewindow[i].x = 1 + i * SPIKE_WINDOW_WIDTH ;
      electrode->spikewindow[i].x2 = 1 + (i + 1) * SPIKE_WINDOW_WIDTH ;
      electrode->spikewindow[i].y = 63;
      electrode->spikewindow[i].y2 = electrode->spikewindow[i].y + 
                     SPIKE_WINDOW_HEIGHT;
      electrode->spikexoffset[i] = electrode->spikewindow[i].x + 4;
   }

   /* set the yoffset for the displayed traces */
   electrode->yoffset = electrode->spikewindow[0].y2 - 50;

   /* set the scale for the displayed traces */
   electrode->spikescale = (electrode->yoffset - electrode->spikewindow[0].y) /
                           (float) MAXVALUE; 

   /* setup the positions for the six projections. This code will need to be 
      changed if adinfo.nelect_chan != 4 */
   for (i = 0; i < NPROJECTIONS/2; i++) {
      tmp = i + NPROJECTIONS/2;
      electrode->projection[i].x = 395 + i * PROJECTION_WINDOW_WIDTH;
      electrode->projection[i].x2 = 395 + (i + 1) * PROJECTION_WINDOW_WIDTH;
      electrode->projection[i].y = 22;
      electrode->projection[i].y2 = 156;
      electrode->projection[i].infox = electrode->projection[i].x + 2;
      electrode->projection[i].infoy = electrode->projection[i].y + 2;
      electrode->projxoffset[i] = electrode->projection[i].x;
      electrode->projyoffset[i] = electrode->projection[i].y2;
      electrode->projection[tmp].x = 395 + i * PROJECTION_WINDOW_WIDTH;
      electrode->projection[tmp].x2 = 395 + (i + 1) * PROJECTION_WINDOW_WIDTH;
      electrode->projection[tmp].y = 156;
      electrode->projection[tmp].y2 = 290;
      electrode->projection[tmp].infox = electrode->projection[tmp].x + 2;
      electrode->projection[tmp].infoy = electrode->projection[tmp].y + 2;
      electrode->projxoffset[tmp] = electrode->projection[tmp].x;
      electrode->projyoffset[tmp] = electrode->projection[tmp].y2;
   }
   electrode->projwidth = PROJECTION_WINDOW_WIDTH;

   /* create the projection infomation strings */
   sprintf(electrode->projection[0].info, "0,1");
   sprintf(electrode->projection[1].info, "0,2");
   sprintf(electrode->projection[2].info, "0,3");
   sprintf(electrode->projection[3].info, "1,2");
   sprintf(electrode->projection[4].info, "1,3");
   sprintf(electrode->projection[5].info, "2,3");
      
   /*
   ** ELECTRODE 2
   */

   /* setup the second electrode's spike display positions */
   electrode = adinfo.electrode+1;
   for (i = 0; i < adinfo.nelect_chan; i++) {
      electrode->spikewindow[i].x = 1 + (i % adinfo.nelect_chan) * 
                                        SPIKE_WINDOW_WIDTH ;
      electrode->spikewindow[i].x2 = 1 + ((i % adinfo.nelect_chan) + 1) * 
                        SPIKE_WINDOW_WIDTH ;
      electrode->spikewindow[i].y = 330;
      electrode->spikewindow[i].y2 = electrode->spikewindow[i].y + 
                     SPIKE_WINDOW_HEIGHT;
      electrode->spikexoffset[i] = electrode->spikewindow[i].x + 4;
   }

   /* set the yoffset for the displayed traces */
   electrode->yoffset = electrode->spikewindow[0].y2 - 50;

   /* set the scale for the displayed traces */
   electrode->spikescale = (electrode->yoffset - electrode->spikewindow[0].y) /
                           (float) MAXVALUE; 

   /* setup the positions for the six projections. This code will need to be 
      changed if adinfo.nelect_chan != 4 */
   for (i = 0; i < NPROJECTIONS/2; i++) {
      tmp = i + NPROJECTIONS/2;
      electrode->projection[i].x = 395 + i * PROJECTION_WINDOW_WIDTH;
      electrode->projection[i].x2 = 395 + (i + 1) * PROJECTION_WINDOW_WIDTH;
      electrode->projection[i].y = 290 ;
      electrode->projection[i].y2 = 424;
      electrode->projection[i].infox = electrode->projection[i].x + 2;
      electrode->projection[i].infoy = electrode->projection[i].y + 2;
      electrode->projxoffset[i] = electrode->projection[i].x;
      electrode->projyoffset[i] = electrode->projection[i].y2;
      electrode->projection[tmp].x = 395 + i * PROJECTION_WINDOW_WIDTH;
      electrode->projection[tmp].x2 = 395 + (i + 1) * PROJECTION_WINDOW_WIDTH;
      electrode->projection[tmp].y = 424;
      electrode->projection[tmp].y2 = 558;
      electrode->projection[tmp].infox = electrode->projection[tmp].x + 2;
      electrode->projection[tmp].infoy = electrode->projection[tmp].y + 2;
      electrode->projxoffset[tmp] = electrode->projection[tmp].x;
      electrode->projyoffset[tmp] = electrode->projection[tmp].y2;
   }
   electrode->projwidth = PROJECTION_WINDOW_WIDTH;
      
   /* create the projection infomation strings */
   sprintf(electrode->projection[0].info, "0,1");
   sprintf(electrode->projection[1].info, "0,2");
   sprintf(electrode->projection[2].info, "0,3");
   sprintf(electrode->projection[3].info, "1,2");
   sprintf(electrode->projection[4].info, "1,3");
   sprintf(electrode->projection[5].info, "2,3");
      
   /* define the spike and projection buttons */
   sysinfo.spikebuttonbase = sysinfo.maxbutton + 1;
   for (j = 0; j < adinfo.nelectrodes; j++) {
       electrode = adinfo.electrode + j;
       DefineButton(++sysinfo.maxbutton, electrode->spikewindow[0].x, 
                    electrode->spikewindow[0].y - BUTTON_HEIGHT-1, 
                    electrode->spikewindow[0].x + ELECTRODE_BUTTON_WIDTH, 
                    electrode->spikewindow[0].y - 2, NULL, ButtonStatus);
       electrode->spbutton[0] = sysinfo.maxbutton;
       DefineButton(++sysinfo.maxbutton, electrode->spikewindow[3].x2 - 
                    ELECTRODE_BUTTON_WIDTH, electrode->spikewindow[0].y - 
                    BUTTON_HEIGHT-1, electrode->spikewindow[3].x2 - 1,
                    electrode->spikewindow[0].y - 2, NULL, ButtonStatus);
       electrode->spbutton[1] = sysinfo.maxbutton;

       /* define the channel buttons */
       /* setup the locations for the channel buttons in spike mode */
       for (i = 0; i < adinfo.nelect_chan; i++) {
           buttonx = electrode->spikewindow[0].x + (i % adinfo.nelect_chan) * 
                      CHANNEL_BUTTON_WIDTH;
           buttony = electrode->spikewindow[0].y2 + 1;
           DefineButton(++sysinfo.maxbutton, buttonx, buttony, buttonx +
                        CHANNEL_BUTTON_WIDTH, buttony + 1.6 * BUTTON_HEIGHT, 
                         NULL, ButtonStatus);
           electrode->channelbutton[i] = sysinfo.maxbutton;
       }
   }

   /* setup the locations for the channel buttons in continuous mode */
   sysinfo.contbuttonbase = sysinfo.maxbutton + 1;
   for (i = 0; i < adinfo.nchannels; i++) {
      if (adinfo.nchannels <= NAMP_CHANNELS) {
          DefineButton(++sysinfo.maxbutton, 12 + i * (CHANNEL_BUTTON_WIDTH + 2),
                       512, 10 + (i + 1) * (CHANNEL_BUTTON_WIDTH + 2), 
                       512 + 1.6 * BUTTON_HEIGHT, NULL, ButtonStatus);
          adinfo.channel[i].contbutton = sysinfo.maxbutton;
      }
      else {
          if (i < NAMP_CHANNELS) {
              DefineButton(++sysinfo.maxbutton, 12 + i * (CHANNEL_BUTTON_WIDTH + 2), 
                           479, 10 + (i + 1) * (CHANNEL_BUTTON_WIDTH + 2), 
                           479 + 1.6 * BUTTON_HEIGHT, NULL, ButtonStatus);
              adinfo.channel[i].contbutton = sysinfo.maxbutton;
          }
          else {
              DefineButton(++sysinfo.maxbutton, 12 + (i % NAMP_CHANNELS) * 
                           (CHANNEL_BUTTON_WIDTH + 2), 512, 10 + 
                           ((i % NAMP_CHANNELS) + 1) * (CHANNEL_BUTTON_WIDTH + 
                           2), 512 + 1.6 * BUTTON_HEIGHT, NULL, ButtonStatus);
              adinfo.channel[i].contbutton = sysinfo.maxbutton;
          }
      }
   }

   
   /* setup the coordinates of the continuous mode display window */
   sysinfo.scopeview.x = 15;
   sysinfo.scopeview.y = 25;
   sysinfo.scopeview.x2 = 795;
   sysinfo.scopeview.y2 = buttonlist[adinfo.channel[0].contbutton].y - 5; 


    /* set the coordinates for the tick erasure box */
    sysinfo.ctickview.x = sysinfo.scopeview.x + 1;
    sysinfo.ctickview.y = sysinfo.scopeview.y + 1;
    sysinfo.ctickview.x2 = sysinfo.scopeview.x + 5;
    sysinfo.ctickview.y2 = sysinfo.scopeview.y2 - 1;
    
   /* set the display offset and scales for the traces in continuous mode */
    cptr = adinfo.channel;
    for (i = 0; i < adinfo.nchannels; i++) {
        cptr->contoffset = sysinfo.scopeview.y + 40 + i * (sysinfo.scopeview.y2
                           - sysinfo.scopeview.y - 20) / adinfo.nchannels;    
		/* the inital scale for the traces is arbitrarily set to 1.0 */
		cptr->contscalemult = 1.0;
        SetContWindowScale(cptr);
        cptr++;
    }
}

void DisplayStartupStatus(void)
{
int mjv, mnv, tp, i;
char   buf[100];
char   temp[100];

  setcolor( MaxColors - 2 );     /* Set current color to white   */
#ifdef FOO
   gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,
         "DMA channels: %d and %d",adinfo.dma_ch1,adinfo.dma_ch2);
   gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,
           "Buffer size: %d WORDS",adinfo.dma_bufsize);
    gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,
         "Electrode Buffer sizes:");
   for(i=0;i<adinfo.nelectrodes;i++){
     gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"\tE%d : %d WORDS",
           i,adinfo.electrode[i].bufsize);
   } 
   gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"A/D Clock: %f Hz",
         adinfo.rate_set);
   gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"%d A/D channels",
           adinfo.nchannels);
   gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,
         "spike limit at %d Hz (%d/buffer)",
         adinfo.spikelimit*adinfo.rate_set/adinfo.dma_bufsize,
         adinfo.spikelimit); */
   if (sysinfo.mouse > 0) {
#ifdef SMOUSE
     mouseinfo(&mjv, &mnv, &tp, &i);
#endif
      switch (tp) {
      case 1:
         sprintf(buf,"bus mouse");
         break;
      case 2:
         sprintf(buf,"serial mouse");
         break;
      case 3:
         sprintf(buf,"Inport mouse");
         break;
      case 4:
         sprintf(buf,"PS/2 mouse");
         break;
      case 5:
         sprintf(buf,"HP mouse");
         break;
      default:
         sprintf(buf,"unknown type");
      }
      gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,
         "Microsoft compatible %s detected with %d buttons on IRQ %d.",
         &buf, sysinfo.mouse, i);
      gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,
         "Software driver version is %d.%d (Microsoft equivalent version).",
         mjv, mnv);
    } else {
 //    gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"No Microsoft compatible mouse detected.\n\n");
    } 
#endif
}

void DrawModeButton(char *string1, int color, int x, int y, int x2, int y2, 
                    short state)
{
    if(state == 1){
        drwbox(SET,(int)LIGHTGRAY,x,y,x2,y2);
        drwfillbox(SET,(int)LIGHTGRAY,x+1,y+1,x2-1,y2-1);
        drwstring(SET,color,string1,x+4,y+8);
        drwline(SET,WHITE,x+1,y+1,x2-1,y+1);
        drwline(SET,WHITE,x2-1,y+1,x2-1,y2-1);
        drwline(SET,BLACK,x+1,y+1,x+1,y2-1);
        drwline(SET,BLACK,x+1,y2-1,x2-1,y2-1);
      } else {
        drwbox(SET,(int)LIGHTGRAY,x,y,x2,y2);
        drwfillbox(SET,(int)DARKGRAY,x+1,y+1,x2-1,y2-1);
        drwstring(SET,color,string1,x+4,y+8);
        drwline(SET,BLACK,x+1,y+1,x2-1,y+1);
        drwline(SET,BLACK,x2-1,y+1,x2-1,y2-1);
        drwline(SET,WHITE,x+1,y+1,x+1,y2-1);
        drwline(SET,WHITE,x+1,y2-1,x2-1,y2-1);
   }
}

void DrawButton(char *string1, char *string2, int x, int y, int x2, int y2, 
                short state)
{
    if(state == 1){
        drwbox(SET,(int)LIGHTGRAY,x,y,x2,y2);
        drwfillbox(SET,(int)LIGHTGRAY,x+1,y+1,x2-1,y2-1);
        drwstring(SET,BLUE,string1,x+4,y+8);
        if (string2 != NULL) 
            drwstring(SET,BLUE,string2,x+4,y+21);
        drwline(SET,WHITE,x+1,y+1,x2-1,y+1);
        drwline(SET,WHITE,x2-1,y+1,x2-1,y2-1);
        drwline(SET,BLACK,x+1,y+1,x+1,y2-1);
        drwline(SET,BLACK,x+1,y2-1,x2-1,y2-1);
   } else {
        drwbox(SET,(int)LIGHTGRAY,x,y,x2,y2);
        drwfillbox(SET,(int)DARKGRAY,x+1,y+1,x2-1,y2-1);
        drwstring(SET,BLUE,string1,x+4,y+8);
        if (string2 != NULL) 
            drwstring(SET,BLUE,string2,x+4,y+20);
        drwline(SET,BLACK,x+1,y+1,x2-1,y+1);
        drwline(SET,BLACK,x2-1,y+1,x2-1,y2-1);
        drwline(SET,WHITE,x+1,y+1,x+1,y2-1);
        drwline(SET,WHITE,x+1,y2-1,x2-1,y2-1);
   }
}

void DrawButton2(char *string1, char *string2, char *string3,
				int x, int y, 
                int x2, int y2)
{
        drwbox(SET,(int)LIGHTGRAY,x,y,x2,y2);
        drwfillbox(SET,(int)LIGHTGRAY,x+1,y+1,x2-1,y2-1);
        drwstring(SET,BLUE,string1,x+4,y+7);
        if (string2 != NULL) 
            drwstring(SET,YELLOW,string2,x+4,y+21);
        if (string3 != NULL) drwstring(SET,RED,string3,x+4,y+38);
        drwline(SET,WHITE,x+1,y+1,x2-1,y+1);
        drwline(SET,WHITE,x2-1,y+1,x2-1,y2-1);
        drwline(SET,BLACK,x+1,y+1,x+1,y2-1);
        drwline(SET,BLACK,x+1,y2-1,x2-1,y2-1);
}

void HighlightButton(int bnum,int color)
{
     drwbox(SET,color,buttonlist[bnum].x, buttonlist[bnum].y,
            buttonlist[bnum].x2, buttonlist[bnum].y2);
     drwbox(SET,color,buttonlist[bnum].x+1, buttonlist[bnum].y+1,
            buttonlist[bnum].x2-1, buttonlist[bnum].y2-1);
}
void UnHighlightButton(int bnum)
{
     drwbox(SET,(int)LIGHTGRAY,buttonlist[bnum].x, buttonlist[bnum].y, 
            buttonlist[bnum].x2, buttonlist[bnum].y2);
     drwline(SET, WHITE, buttonlist[bnum].x+1, buttonlist[bnum].y+1,
             buttonlist[bnum].x2-1, buttonlist[bnum].y+1);
     drwline(SET, WHITE, buttonlist[bnum].x2-1, buttonlist[bnum].y+1,
             buttonlist[bnum].x2-1, buttonlist[bnum].y2-1);
     drwline(SET, BLACK, buttonlist[bnum].x+1, buttonlist[bnum].y+1,
             buttonlist[bnum].x+1, buttonlist[bnum].y2-1);
     drwline(SET, BLACK, buttonlist[bnum].x+1, buttonlist[bnum].y2-1, 
             buttonlist[bnum].x2-1, buttonlist[bnum].y2-1);
}

void ButtonStatus(int bnum, int event)
{
            sprintf(tmpstring,"M%d %d %d %d:%d     ",
              sysinfo.mousex,sysinfo.mousey,
              sysinfo.mousebut,bnum,event);
       ErrorMessage(tmpstring);
       switch(event){

       case BUTTONENTER:
         HighlightButton(bnum,RED);
         break;
       case BUTTONEXIT:
         UnHighlightButton(bnum);
         break;
       case BUTTON1PRESS:
         HighlightButton(bnum,GREEN);
         break;
       case BUTTON2PRESS:
         HighlightButton(bnum,BLUE);
         break;
       case BUTTON1RELEASE:
         HighlightButton(bnum,RED);
         break;
       case BUTTON2RELEASE:
         HighlightButton(bnum,RED);
         break;
       }
}

void DefineButton(int bnum, int x, int y, int x2, int y2, char* data,
              void (*handler)())
{
   if(bnum >= MAXBUTTONS){
     gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,
           "Exceeded maximum number of buttons\n");
     return;
   }
   buttonlist[bnum].active = 1;
   buttonlist[bnum].x = x;
   buttonlist[bnum].y = y;
   buttonlist[bnum].x2 = x2;
   buttonlist[bnum].y2 = y2;
   buttonlist[bnum].handler = handler;
   buttonlist[bnum].data = data;
}

void DefineMessageArea(int mnum, short x, short y, short x2, short y2)
/* Defines the mth message area's boundries */
{
   message_area[mnum].x = x;
   message_area[mnum].y = y;
   message_area[mnum].x2 = x2;
   message_area[mnum].y2 = y2;
}   

void do_quit(int bnum, int event)
{
   ButtonStatus(bnum,event);
   if(event == BUTTON1PRESS)
   sysinfo.quit = 1;
}
void do_toggleacq(int bnum, int event)
{
   ButtonStatus(bnum,event);
   if(event == BUTTON1PRESS)
   ToggleAcq();
}
void do_channelbutton(int bnum, int event)
{
   ButtonStatus(bnum,event);
   // get the button data to determine the channel
   if(event == BUTTONENTER){
     sysinfo.selected_channel = atoi(buttonlist[bnum].data);
   }
   if(event == BUTTON1PRESS){
     adinfo.electrode[sysinfo.selected_channel/
     adinfo.nelect_chan].thresh[sysinfo.selected_channel%adinfo.nelect_chan]
      -= 10;
       DrawChannelButtons();
   }
   if(event == BUTTON2PRESS){
     adinfo.electrode[sysinfo.selected_channel/
     adinfo.nelect_chan].thresh[sysinfo.selected_channel%adinfo.nelect_chan]
      += 10;
       DrawChannelButtons();
   }
}
void DefineChannelButtons(int buttonstart)
{
int   i,j;
int   channel;
int   offset;
char   *datastring;

   for(i=0;i<adinfo.nelectrodes;i++){
     offset = ELECTRODE_BUTTON_WIDTH*(i+1);
     for(j=0;j<adinfo.nelect_chan;j++){
       channel = adinfo.electrode[i].channel[j];
       datastring = (char *)malloc(4*sizeof(char));
       sprintf(datastring,"%d",channel);
       DefineButton(buttonstart+channel,channel*CHANNEL_BUTTON_WIDTH + offset,499,
       (channel+1)*CHANNEL_BUTTON_WIDTH-1+offset,519,datastring,do_channelbutton);
     }
   }
}


void CreateMenu(void)
{
   DefineButton(0,0,580,55,599,NULL,do_quit);      // quit
   DefineButton(1,55,580,135,599,NULL,do_toggleacq);     // acq
   DefineButton(2,135,580,220,599,NULL,ButtonStatus);     // disk
   DefineButton(3,220,580,340,599,NULL,ButtonStatus);     // file open
   DefineButton(4,340,580,440,599,NULL,ButtonStatus);     // mode
   sysinfo.maxbutton = 5;
   
   /* define the message areas */
   DefineMessageArea(0, 440, 580, 799, 599);
   DefineMessageArea(1, 75, 559, 799, 579);
   DefineMessageArea(2, 0, 559, 75, 579);
//  DefineMessageArea(3, 0, 519, 393, 539);
   //DefineMessageArea(4, 0, 5, 393, 25);

}

void UpdateDiskButton(void)
{
   if(sysinfo.disk){
       DrawButton("(D)isk ON", NULL, buttonlist[2].x, buttonlist[2].y, 
                  buttonlist[2].x2, buttonlist[2].y2, 1);
   } else {
       DrawButton("(D)isk OFF", NULL, buttonlist[2].x, buttonlist[2].y, 
                  buttonlist[2].x2, buttonlist[2].y2, 0);
   }
}

void UpdateAcqButton(void)
{
   if(sysinfo.acq){
       DrawButton("(A)cq ON",NULL, buttonlist[1].x, buttonlist[1].y, 
                  buttonlist[1].x2, buttonlist[1].y2, 1);
   } else {
       DrawButton("(A)cq OFF", NULL, buttonlist[1].x, buttonlist[1].y, 
                  buttonlist[1].x2, buttonlist[1].y2, 0);
   }
}

void UpdateFileButton(void)
{
   if(sysinfo.fileopen){
     DrawButton("(F)ile Opened", NULL, buttonlist[3].x, buttonlist[3].y, 
                buttonlist[3].x2, buttonlist[3].y2, 1);
   } else {
     DrawButton("(F)ile Closed", NULL, buttonlist[3].x, buttonlist[3].y, 
                buttonlist[3].x2, buttonlist[3].y2, 0);
   }
}

void DrawMenu(void)
{

   DrawButton("(Q)uit", NULL, buttonlist[0].x, buttonlist[0].y,buttonlist[0].x2,
              buttonlist[0].y2, 1);
   UpdateAcqButton();
   UpdateDiskButton();
   UpdateFileButton();
    switch(sysinfo.mode){
    case SPIKE_MODE:
         DrawButton("m(O)de Spike", NULL, buttonlist[4].x, buttonlist[4].y, 
                buttonlist[4].x2, buttonlist[4].y2, 1);
        break;
    case CONTINUOUS_MODE:
         DrawButton("m(O)de Cont", NULL, buttonlist[4].x, buttonlist[4].y, 
                buttonlist[4].x2, buttonlist[4].y2, 0);
        break;
   }
   /* status message area 4 */
//   DrawButton("Message Area 4", message_area[4].x, message_area[4].y,
//           message_area[4].x2, message_area[4].y2,1);
   /* status message area 3 */
//   DrawButton("Message Area 3", message_area[3].x, message_area[3].y,
//           message_area[3].x2, message_area[3].y2,1);
    /* status message area 1 */
    StatusMessage(NORMAL_MESSAGE);
    /* mode message area (index 2 in terms of defined message areas) */
       ModeMessage();
   /* error/message area */
   ErrorMessage("Message Area");
}

void DisplayRate()
{
    /* clear the old string */
    drwfillbox(SET, _BLACK, sysinfo.rateview.x, sysinfo.rateview.y, 
               sysinfo.rateview.x2, sysinfo.rateview.y2);
    sprintf(tmpstring, "(R)ate: %7.1f Hz", adinfo.rate_set);
    setcolor(_WHITE);
    outtextxy(sysinfo.rateview.x, sysinfo.rateview.y, tmpstring);
}

void DisplayAmpFilters()
{
    char filter[80];
    char high[5], low[5];

    /* clear the old string */
    drwfillbox(SET, _BLACK, sysinfo.filterview.x, sysinfo.filterview.y, 
               sysinfo.filterview.x2, sysinfo.filterview.y2);
    GetAmpFilters(low, high);
    sprintf(filter, "(^L)ow: %sHz  (^H)igh: %s", low, high);
    setcolor(_WHITE);
    outtextxy(sysinfo.filterview.x, sysinfo.filterview.y, filter);
}


void DrawStrings(void)
{
    /* erase the strings */
    drwfillbox(SET, _BLACK, sysinfo.currenttimeview.x - 112, 
               sysinfo.currenttimeview.y, sysinfo.currenttimeview.x, 
               sysinfo.currenttimeview.y2);
    drwfillbox(SET, _BLACK, sysinfo.filesizeview.x-textwidth("AAAAAAAA.AAA: "), 
               sysinfo.filesizeview.y, sysinfo.filesizeview.x, 
               sysinfo.filesizeview.y2);
    drwfillbox(SET, _BLACK, sysinfo.remaintimeview.x - 112, 
               sysinfo.remaintimeview.y, sysinfo.remaintimeview.x, 
               sysinfo.remaintimeview.y2);
    drwfillbox(SET, _BLACK, sysinfo.diskfreeview.x - 88, 
               sysinfo.diskfreeview.y, sysinfo.diskfreeview.x, 
               sysinfo.diskfreeview.y2);

    /* draw the current time string */
    setcolor(_WHITE);
    outtextxy(sysinfo.currenttimeview.x - 112, sysinfo.currenttimeview.y, 
              "current time: ");

    /* draw the remaining time string */
    outtextxy(sysinfo.remaintimeview.x - 112, sysinfo.remaintimeview.y, 
              "est. remain: ");

    /* draw the file size string */
    sprintf(tmpstring, "%s: ", adinfo.fname);
    outtextxy(sysinfo.filesizeview.x - textwidth(tmpstring), 
              sysinfo.filesizeview.y, tmpstring);

    /* draw the disk free string */
    outtextxy(sysinfo.diskfreeview.x - 88, sysinfo.diskfreeview.y, 
             "disk free: ");
}


void DisplayCurrentTime(void)
{
    sysinfo.currenttime = ReadTS();
    sysinfo.oldtime = sysinfo.currenttime;

    /* clear the clock time area */
    drwfillbox(SET, _BLACK, sysinfo.currenttimeview.x, 
               sysinfo.currenttimeview.y, sysinfo.currenttimeview.x2, 
               sysinfo.currenttimeview.y2);

    setcolor(_WHITE);
    /* draw the time string */
    outtextxy(sysinfo.currenttimeview.x, sysinfo.currenttimeview.y, 
              FormatTS(sysinfo.currenttime, 0));
}

void DisplayDiskInfo(void)
{
    unsigned long time;
    float filesize, diskfree;

    filesize = (float) sysinfo.filesize / 1000000.0;
    diskfree = (float) sysinfo.diskfree / 1000000.0;

    /* draw the remaining time string */
    /* create the time remaining string (with filler zeros) if at least
       ten seconds have gone by since the last time it was displayed */
//    if ((sysinfo.disk) && (sysinfo.currenttime > (sysinfo.oldtime +100000L)) &&
 //       (sysinfo.diskfree != sysinfo.olddiskfree)) {
    if ((sysinfo.disk) && (sysinfo.currenttime > (sysinfo.oldtime +100000L))){
        time = (sysinfo.diskfree / (sysinfo.olddiskfree - sysinfo.diskfree))
                 * (sysinfo.currenttime - sysinfo.oldtime);
        /* update the oldtime and olddiskfree variables */
        sysinfo.oldtime = sysinfo.currenttime;
        sysinfo.olddiskfree = sysinfo.diskfree;
        sprintf(tmpstring, "%s", FormatTS(time,0));
    }
    else {
        sprintf(tmpstring,"?:??:??");
    }
    /* clear the remaining time area */
    drwfillbox(SET, _BLACK, sysinfo.remaintimeview.x, sysinfo.remaintimeview.y,
               sysinfo.remaintimeview.x2, sysinfo.remaintimeview.y2);

    setcolor(_WHITE);
    /* draw the time string */
    outtextxy(sysinfo.remaintimeview.x, sysinfo.remaintimeview.y, tmpstring);

    /* clear the file size and diskfree areas */
    drwfillbox(SET, _BLACK, sysinfo.filesizeview.x, sysinfo.filesizeview.y, 
               sysinfo.filesizeview.x2, sysinfo.filesizeview.y2);
    drwfillbox(SET, _BLACK, sysinfo.diskfreeview.x, sysinfo.diskfreeview.y, 
               sysinfo.diskfreeview.x2, sysinfo.diskfreeview.y2);

    /* if there are less than 50 Mb free, display the size information in red */
    if (diskfree < 50.0) 
        setcolor(_RED);
    else
        setcolor(_WHITE);

    /* draw the file size string */
    sprintf(tmpstring, "%5.1f Mb", filesize);
    outtextxy(sysinfo.filesizeview.x, sysinfo.filesizeview.y, tmpstring);

    /* draw the disk free string */
    sprintf(tmpstring, "%5.1f Mb", diskfree);
    outtextxy(sysinfo.diskfreeview.x, sysinfo.diskfreeview.y, tmpstring);

}

void DrawSPButtons(void)
{
    int   i;

    for(i=0;i<sysinfo.nspbuttons;i++) 
        DrawSPButton(i);
}

void DrawSPButton(int button)
{
    ElectrodeInfo *electrode;
    int b, e, blistnum, rate;

    b = button % 2;
    e = button / 2;
    electrode = adinfo.electrode + e;
    
    if (b == 0) {
        /* Spike window button */
        rate = (int) electrode->nspikes * ((float) SECOND / 
                (float) adinfo.conversion_duration);
        if (sysinfo.overlay[e])
            /* add an 'overlay' to the button */
            sprintf(tmpstring,"E%d %1d/s %5ld ov", e, rate,
                    electrode->ntotal_spikes);
        else
            sprintf(tmpstring,"E%d %1d/s %5ld", e, rate,
                    electrode->ntotal_spikes);
        blistnum = electrode->spbutton[0];
    }
    else {
        /* Projection button */
        sprintf(tmpstring, "E%d Projections", e);
        blistnum = electrode->spbutton[1];
    }
    DrawButton(tmpstring, NULL, buttonlist[blistnum].x, buttonlist[blistnum].y, 
               buttonlist[blistnum].x2, buttonlist[blistnum].y2, 1);
    if ((button == sysinfo.selected_spbutton) && (!sysinfo.channel_selected)) {
        /*
        ** highlight the selected button
        */
        HighlightButton(blistnum, _RED);
    }
}


void DrawChannelButtons(void)
{
    int i;
   
    for(i = 0; i < adinfo.nchannels; i++)
        DrawChannelButton(i); 
}
    
void DrawChannelButton(int channel)
{
    ChannelInfo *cptr;
    ElectrodeInfo *electrode;
    int e_num, echan, x, y; 
    int blistnum;
    long gain; 
    float winscale;
	float winscalemax;
    char tmpstring2[80];
	char tmpstring3[80];

    // make sure the button selected variables are set correctly if in spike
    //   mode
    if (sysinfo.mode == SPIKE_MODE)
        UpdateChannelSelected();
    e_num = ((channel % NAMP_CHANNELS) / adinfo.nelect_chan);
    echan = channel % adinfo.nelect_chan;
   
    gain = GetActualGain(adinfo.channel[channel].ampgain);
    cptr = adinfo.channel + channel;
    electrode = adinfo.electrode + e_num;
    if (sysinfo.mode == SPIKE_MODE) {
        blistnum = electrode->channelbutton[echan];
        /* display the gain and threshold and max */
        sprintf(tmpstring,"g %5ld  %d", gain, 1 << 
               adinfo.channel[channel].adgain);
        winscale = SpikeWindowScale(channel);
		winscalemax = SpikeWindowScale(channel) /
						adinfo.electrode[e_num].thresh[echan] * 
						2048.0;
        if (absv(winscale) < 1.0e-3)
            sprintf(tmpstring2,"t%4d %3.0fuV", 
                    adinfo.electrode[e_num].thresh[echan],
					winscale * 1.0e6);
        else if (absv(winscale) < 1.0)
            sprintf(tmpstring2,"t%4d %3.0fmV", 
                    adinfo.electrode[e_num].thresh[echan],
					winscale * 1.0e3);
        else
            sprintf(tmpstring2,"t%4d %3.0fV", 
                    adinfo.electrode[e_num].thresh[echan],
					winscale);
        if (absv(winscalemax) < 1.0e-3)
            sprintf(tmpstring3,"max %4.0fuV", winscalemax * 1.0e6);
        else if (absv(winscalemax) < 0.1)
            sprintf(tmpstring3,"max %2.2fmV", winscalemax * 1.0e3);
        else
            sprintf(tmpstring3,"max %1.3fV", winscalemax);
        DrawButton2(tmpstring, tmpstring2, tmpstring3,
				   buttonlist[blistnum].x, 
                   buttonlist[blistnum].y, 
				   buttonlist[blistnum].x2, 
                   buttonlist[blistnum].y2);
        /* Draw the partitions */
        x = (buttonlist[blistnum].x + buttonlist[blistnum].x2) / 2;
        y = (buttonlist[blistnum].y + buttonlist[blistnum].y2) / 2 - 8;
        drwline(SET, DARKGRAY, buttonlist[blistnum].x + 1, y, 
                buttonlist[blistnum].x2 - 1, y);
        drwline(SET, DARKGRAY, buttonlist[blistnum].x + 1, y+16, 
                buttonlist[blistnum].x2 - 1, y+16);
//        drwline(SET, DARKGRAY, x, y, x, buttonlist[blistnum].y2 - 1);
        if ((sysinfo.channel_selected) && (channel == sysinfo.selected_channel)) {
            /*
            ** highlight the selected channel
            */
            HighlightButton(blistnum, _RED);
         }
    }
    else {
        blistnum = adinfo.channel[channel].contbutton;

        /* calculate the scale for the distance between the two ticks */
        winscale = ContWindowScale(cptr);    
        /* write out the string with the channel number and the scale */
        if (absv(winscale) < 1.0e-3)
            sprintf(tmpstring,"  %2d  %3.0fuV", channel, winscale * 1.0e6);
        else if (absv(winscale) < 1.0)
            sprintf(tmpstring,"  %2d  %3.0fmV", channel, winscale * 1.0e3);
        else
            sprintf(tmpstring,"  %2d  %3.0fV", channel, winscale);

        /* write out the gain string */
        sprintf(tmpstring2,"g %5ld  %d", gain, 1 << 
                adinfo.channel[channel % NAMP_CHANNELS].adgain) ;
        DrawButton(tmpstring, tmpstring2, buttonlist[blistnum].x, 
                    buttonlist[blistnum].y, buttonlist[blistnum].x2, 
                    buttonlist[blistnum].y2, 1);
        /* Draw the partitions */
        x = (buttonlist[blistnum].x + buttonlist[blistnum].x2) / 2;
        y = (buttonlist[blistnum].y + buttonlist[blistnum].y2) / 2 + 2;
        drwline(SET, DARKGRAY, buttonlist[blistnum].x + 1, y, 
                buttonlist[blistnum].x2 - 1, y);
        drwline(SET, DARKGRAY, x, buttonlist[blistnum].y - 1, x, y);
        if(channel == sysinfo.selected_channel){
            /*
            ** highlight the selected channel
            */
            HighlightButton(blistnum, _RED);
         }
         drwbox(SET,_BLACK, buttonlist[blistnum].x + 5, 
                buttonlist[blistnum].y + 5, buttonlist[blistnum].x + 15, 
                buttonlist[blistnum].y + 15); 
         drwfillbox(SET, (int)cptr->color, buttonlist[blistnum].x + 6, 
               buttonlist[blistnum].y + 6, buttonlist[blistnum].x + 14, 
               buttonlist[blistnum].y + 14); 
     }
        
}


float SpikeWindowScale(int channel)
/* returns the distance from the threshold to the 0 point in Volts */
{
       ElectrodeInfo *electrode;
     int e, echan, i;
        float scaleval;

    e = channel / adinfo.nelect_chan;
    echan = channel % adinfo.nelect_chan;
    electrode = adinfo.electrode + e;
    if(GetActualGain(adinfo.channel[channel].ampgain) <= 0){
        scaleval = 1;
    } else {
        scaleval = ((float) electrode->thresh[echan] / (float) MAXVALUE * 
           (float) MAX_AD_VOLTAGE / (float) (GetActualGain(
           adinfo.channel[channel].ampgain) * 
           (1 << adinfo.channel[channel].adgain)));
    }
    return(scaleval);
}

void SetContWindowScale(ChannelInfo *cptr)
/* sets the continous mode scale based on the multiplier in the ChannelInfo
   structure */
{
	/* the original setting, with a multiplier of 1, sets the tick mark to be
	   1/2th of the positive range of the waveform */
	cptr->contscaletick = cptr->contoffset - cptr->contscalemult * 
						  (sysinfo.scopeview.y2 - sysinfo.scopeview.y-20) /
						  (2.0 * adinfo.nchannels);
	cptr->contwindowscale = cptr->contscalemult * 
							(((float) sysinfo.scopeview.y2 - 
							(float) sysinfo.scopeview.y - 20) / 
							 (float) adinfo.nchannels)
							/ (float) MAXVALUE;    
}							
    
   
float ContWindowScale(ChannelInfo *cptr)
/* returns the distance from the threshold to the 0 point in Volts */
{
        float scaleval;
    if(GetActualGain(cptr->ampgain) <= 0){
        scaleval = 1;
    } else {
		scaleval =  (float) ((cptr->contoffset - cptr->contscaletick) / 
           (float) cptr->contwindowscale) / (float) MAXVALUE * 
           (float) MAX_AD_VOLTAGE / ((float)GetActualGain(
           cptr->ampgain) * (1 << cptr->adgain));
	}
    return(scaleval);
}

/*

int AmpGainValue(channel)
{
}

float AmpLowFilterValue(channel)
{
}

float AmpHighFilterValue(channel)
{
}


void DrawAmpButtons(void)
{
int   i,j;
int   channel;
int   offset;

    for(i=0;i<adinfo.nelectrodes;i++){
      offset = ELECTRODE_BUTTON_WIDTH*(i+1);
      for(j=0;j<NAMP_CHANNELS;j++){
         sprintf(tmpstring,"C%d %d %g",
            i,AmpGainValue(i),AmpLowFilterValue(i));
         DrawButton(tmpstring, NULL, channel*CHANNEL_BUTTON_WIDTH + offset,499,
         (channel+1)*CHANNEL_BUTTON_WIDTH-1+offset,519,1);
         if(channel == sysinfo.selected_channel){ */
            /*
            ** highlight the selected channel
            */
    /*        drwbox(SET,_RED,channel*CHANNEL_BUTTON_WIDTH + offset,499,
            (sysinfo.selected_channel+1)*CHANNEL_BUTTON_WIDTH-1+offset,519);
         }
      }
    }
} */

void DrawScopeBorder(void)
{
     drwbox(SET, _RED, sysinfo.scopeview.x, sysinfo.scopeview.y,
              sysinfo.scopeview.x2, sysinfo.scopeview.y2);
}

void ClearScopeBox(void)
{
     drwfillbox(SET, _BLACK, sysinfo.scopeview.x, sysinfo.scopeview.y,
              sysinfo.scopeview.x2, sysinfo.scopeview.y2);
}

void DrawSPBorders(void)
{
   int i;
   
    for (i = 0; i < adinfo.nelectrodes; i++) {
        DrawSpikeBorders(i);
        DrawProjectionBorders(i);
    }
}

void DrawSpikeBorders(int e_num)
   /* draw spike display borders */
{
    int i;
    for (i = 0; i < adinfo.nelect_chan; i++) 
        DrawSpikeBorder(e_num, i);
}

void DrawSpikeBorder(int e_num, int channel)
{
    View *spikewin;
    ElectrodeInfo *electrode;
    int y, echan;

    electrode = adinfo.electrode + e_num;
    spikewin = electrode->spikewindow + channel;

    /* draw the border */
    drwbox(SET,_RED,spikewin->x, spikewin->y, spikewin->x2, spikewin->y2);

    /* draw the threshold line */
    y = (int) (-electrode->thresh[channel] * electrode->spikescale 
              + electrode->yoffset);
    if (y > spikewin->y2) 
        y = spikewin->y2 - 1;
    else if (y < spikewin->y) 
        y = spikewin->y + 1;
    drwline(SET, _YELLOW, spikewin->x , y, spikewin->x + 5, y); 
    
    /* put red yellow points at 0 */
    drwline(SET, _RED, spikewin->x, electrode->yoffset, spikewin->x+3, 
            electrode->yoffset);
}

void DrawProjectionBorder(int e_num, int channel)
{
    ElectrodeInfo *eptr;
    View *proj;

    eptr = adinfo.electrode + e_num;
    proj = eptr->projection+channel;
    /* draw the box */
    drwbox(SET,_WHITE, proj->x, proj->y, proj->x2, proj->y2);

    /* draw the text */
    outtextxy(proj->infox, proj->infoy, proj->info);
}

void DrawProjectionBorders(int e_num)
   /* draw projection display borders */
{
    int i;
    for (i = 0; i < NPROJECTIONS; i++) 
        DrawProjectionBorder(e_num, i);
}

void ClearSpikeBoxes(int e_num)
{
    int i;

    for (i = 0; i < adinfo.nelect_chan; i++)
        ClearSpikeBox(e_num, i);
}

void ClearSpikeBox(int e_num, int channel)
{
    View *spikewin;

    spikewin = adinfo.electrode[e_num].spikewindow + channel;
    /* erase the box */
    drwfillbox(SET,_BLACK,spikewin->x, spikewin->y, spikewin->x2, spikewin->y2);
}

void ClearProjectionBoxes(int e_num)
{
    int i;

    for (i = 0; i < NPROJECTIONS; i++)
        ClearProjectionBox(e_num, i);
}

void ClearProjectionBox(int e_num, int channel)
{
    View     *projwin;
    int i;

    projwin = adinfo.electrode[e_num].projection + channel;
    /* erase the box */
    drwfillbox(SET,_BLACK,projwin->x, projwin->y, projwin->x2, projwin->y2);
}

void RefreshDisplay(void)
{
    setviewport( 0, 0, MaxX, MaxY, 0 ); /* Open port to full screen   */
    SetStringPositions();
    DrawStrings();
    DrawMenu();
    DisplayCurrentTime();
    DisplayDiskInfo();
    DisplayRate();
    DisplayAmpFilters();
    DrawChannelButtons();
    sprintf(tmpstring,"AD Version %s (c) 1995 M.Wilson, L.Frank, MIT",
        ADVERSION);
    setcolor(WHITE);
    outtextxy(sysinfo.versionview.x, sysinfo.versionview.y, tmpstring);

   
   if (sysinfo.mode == SPIKE_MODE) {
       DrawSPButtons();
       DrawSPBorders();
   }
   else {
       DrawScopeBorder();
       DrawContinuousTicks();
   }

}

void ClearScreen(void){
#ifdef MSGRAPHICS
        _clearscreen(_GVIEWPORT);
#endif
#ifdef BORGRAPH
   clearviewport();
#else
       fillscreen((int)_BLACK);
#endif
   RefreshDisplay();
}

void DisplayADStatus(void)
{
   /* get the SUPCSR status */
   DmaStatus; 
/*   if(sysinfo.debug){
     gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,
     "AD status: Acq %1d ADCSR:%X SUPCSR:%X :Tot %ld Err %ld Dsk %ld Dsp %ld : Spk %ld    ",
       adinfo.next_buf,adcsr,supcsr,
       adinfo.count,adinfo.error_count,
       adinfo.disk_lost,adinfo.display_lost,
       adinfo.nspikes);
   }
   if((supcsr & BIT9) == BIT9){
     sprintf(tmpstring,
     "Acq %1d ADCSR:%X SUPCSR:%X :Tot %ld Err %ld Dsk %ld Dsp %ld : Spk %ld    \0",
       adinfo.next_buf,adcsr,supcsr,
       adinfo.count,adinfo.error_count,
       adinfo.disk_lost,adinfo.display_lost,
       adinfo.nspikes);
   } else {
     sprintf(tmpstring,
     "Acq %1d ADCSR:%X SUPCSR:%X :Tot %ld Err %ld Dsk %ld Dsp %ld : Spk %ld   \0",
       adinfo.next_buf,adcsr,supcsr,
       adinfo.count,adinfo.error_count,
       adinfo.disk_lost,adinfo.display_lost,
       adinfo.nspikes);
   } */

   sprintf(tmpstring, "Er:%ld Pk:%d Dk:%ld Dp:%ld Sp:%ld Me:%ld",
           adinfo.error_count, adinfo.peak_rate, adinfo.disk_lost, adinfo.display_lost,
            adinfo.nspikes,farcoreleft());
   ErrorMessage(tmpstring);
    adinfo.peak_rate = 0;
}

void StatusMessage(char *string)
{
   //MenuView;
   if(string == NULL) return;
   if(sysinfo.debug > 1){
     gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"%s\n",string);
   }else
     DrawButton(string, NULL, message_area[1].x, message_area[1].y,
             message_area[1].x2, message_area[1].y2,1);
}

void ModeMessage(void)
{
      int color;
      short state;

    if (sysinfo.command_mode == MASTER) {
        sprintf(tmpstring, "(M)aster");
        color = RED;
        state = 1;
    }
    else if (sysinfo.command_mode == SLAVE) {
        sprintf(tmpstring, "  Slave");
        color = GREEN;
        state = 0;
    }
    else if (sysinfo.command_mode == SINGLE) {
        sprintf(tmpstring, " Single");
        color = BLUE;
        state = 1;
    }
    DrawModeButton(tmpstring, color, message_area[2].x, message_area[2].y,
             message_area[2].x2, message_area[2].y2,state);
}

void StatusMessage3(char *string)
{
   //MenuView;
   if(string == NULL) return;
   if(sysinfo.debug > 1){
     gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"%s\n",string);
   }else
     DrawButton(string, NULL, message_area[3].x, message_area[3].y,
             message_area[3].x2, message_area[3].y2,1);
}
void StatusMessage4(char *string)
{
   //MenuView;
   if(string == NULL) return;
   if(sysinfo.debug > 1){
     gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"%s\n",string);
   }else
     DrawButton(string, NULL, message_area[4].x, message_area[4].y,
             message_area[4].x2, message_area[4].y2,1);
}

void ErrorMessage(char *string)
{
   if(string == NULL) return;
   if(sysinfo.debug){
     gprintf(&sysinfo.debugwinx, &sysinfo.debugwiny,"%s\n",string);
   }else
     DrawButton(string, NULL, message_area[0].x, message_area[0].y,
             message_area[0].x2, message_area[0].y2,1);
}

void ChannelMessage(char *string,short channel)
{
   if(string == NULL) return;
   drwstring(SET,BLUE,string,(int)CHANNEL_BUTTON_WIDTH*channel,
   (int)502);
}

void DrawContinuousTicks(void)
{
    int i;
    ChannelInfo *cptr;

    cptr = adinfo.channel;
    /* erase the old tick marks */
    drwfillbox(SET, _BLACK, sysinfo.ctickview.x, sysinfo.ctickview.y, 
                sysinfo.ctickview.x2, sysinfo.ctickview.y2);
    for (i = 0; i < adinfo.nchannels; i++) {
        /* Draw the 0 tick, clipping it to one pixel within the window */
        if (cptr->contoffset <= sysinfo.scopeview.y)
            cptr->contoffset = sysinfo.scopeview.y + 1;
        else if (cptr->contoffset >= sysinfo.scopeview.y2)
            cptr->contoffset = sysinfo.scopeview.y2 - 1;
        drwline(SET, cptr->color, sysinfo.scopeview.x+1, cptr->contoffset, 
                sysinfo.scopeview.x + 5, cptr->contoffset);
        /* Draw the scale tick if it would be within the window */
        if (cptr->contscaletick > sysinfo.scopeview.y + 1)
            drwline(SET,cptr->color, sysinfo.scopeview.x+1, cptr->contscaletick,
                    sysinfo.scopeview.x + 5, cptr->contscaletick);
        cptr++;
   }
}

/* display contents of the buffer in a scope window */
void DisplayContinuousData()
{
/* Multiple buffer code:
*/
int i, j;
int *dataptr;
ChannelInfo *cptr;
int *prevptr;
long   sum;
float   scale;
int peak;
int offset;
int val;
unsigned long   len;
int bufsize;
double  iscale;
double  xoffset;
int   lastxoffset;
int    overlay;

   bufsize = adinfo.dma_bufsize;
   dataptr = adinfo.dataptr[adinfo.process_buf];
   prevptr = adinfo.prevptr;
   overlay = sysinfo.contoverlay;

   iscale = (sysinfo.scopeview.x2 - sysinfo.scopeview.x - 5) /
            ((double)adinfo.dma_bufsize/adinfo.nchannels);
   
   offset = MAXVALUE;
   xoffset = (double) sysinfo.scopeview.x + 5;
   lastxoffset = (int) xoffset;
   for(i = 0; i < bufsize; i+= adinfo.nchannels){
      cptr = adinfo.channel;
      xoffset += iscale;
      /* only draw the point if it will not overlap with the last point */
      if ((int) xoffset > lastxoffset) {
         lastxoffset = (int) xoffset;
         for(j = 0; j < adinfo.nchannels; j++) { 
            val = (*dataptr - offset)*cptr->contwindowscale + cptr->contoffset; 
            /* clip the point to the window */
            if (val <= sysinfo.scopeview.y) 
               val = sysinfo.scopeview.y + 1;
            else if (val >= sysinfo.scopeview.y2) 
               val = sysinfo.scopeview.y2 - 1;
            if(!overlay){
               /* erase the previous point */
               drwpoint(SET, BLACK, lastxoffset, *prevptr);
               }
            drwpoint(SET, cptr->color, lastxoffset, val);
            *prevptr = val;
            dataptr++;
            prevptr++;
            cptr++;
         }
         /* incrememt lastoffset so the display skips a point each time */
         lastxoffset++;
      }
      else {
         dataptr += adinfo.nchannels;
      }
      /* check for a/d interrupt */
      if(adinfo.dmadone || adinfo.error) break;
   }
}

void DisplayProjections(short e_num)
{
/*
** Multiple buffer code
*/
register int     i, j, k;
register int     *spikedataptr;
register Spike     *spikeptr;
long    sum;
float   scale;
register float     max[MAXCHANNELS];
int     firstchan;
ElectrodeInfo   *electrode;
int overlay;

    electrode = adinfo.electrode + e_num;
    spikeptr = electrode->spikebuf;
    spikedataptr = spikeptr->dataptr;
    firstchan = e_num * adinfo.nelect_chan; 
     /*
    ** First find the max value on each channel and display the max values
    ** in the projections. This is done first as the projections are more 
    ** important than the spike waveforms as far as display is concerned.
    */
    for (i = 0; i < electrode->nspikes; i++) {
        for(j = 0; j < adinfo.nelect_chan; j++) 
            *(max+j) = -FLT_MAX;
          for(j = 0; j < SINGLESPIKELEN; j++){
            for(k = 0; k < adinfo.nelect_chan; k++) {
                /* find the maximum value for each electrode */
                if (*spikedataptr > *(max + k))
                   *(max+k) = *spikedataptr;

             spikedataptr++;
            }
        }
        spikeptr++;
        spikedataptr = spikeptr->dataptr;

        /* scale the values of the maximum amplitudes to fit into a projection
           box. */
        for(j = 0; j < adinfo.nelect_chan; j++) {
               if (*(max+j) < MAXVALUE) 
                /* if the maximum is below the 0 volt point, set the max to 0 */
                *(max+j) = 0;
            else 
                *(max+j) = (*(max+j)-MAXVALUE) / MAXVALUE * 
                           (electrode->projwidth - 1);
           }
        /* display the points on the relevant projections */
        /* projection 0 : channels 0 and 1 */

        drwpoint(SET, (int)WHITE, electrode->projxoffset[0] + 
                (int) *max, electrode->projyoffset[0] - (int) *(max+1));
        /* projection 1 : channels 0 and 2 */
        drwpoint(SET, (int)WHITE, electrode->projxoffset[1] + 
                (int) *max, electrode->projyoffset[1] - (int) *(max+2));
        /* projection 2 : channels 0 and 3 */
        drwpoint(SET, (int)WHITE, electrode->projxoffset[2] + 
                (int) *max, electrode->projyoffset[2] - (int) *(max+3));
        /* projection 3 : channels 1 and 2 */
        drwpoint(SET, (int)WHITE, electrode->projxoffset[3] + 
                (int) *(max+1), electrode->projyoffset[3] - (int) *(max+2));
        /* projection 4 : channels 1 and 3 */
        drwpoint(SET, (int)WHITE, electrode->projxoffset[4] + 
                (int) *(max+1), electrode->projyoffset[4] - (int) *(max+3));
        /* projection 5 : channels 2 and 3 */
        drwpoint(SET, (int)WHITE, electrode->projxoffset[5] + 
                (int) *(max+2), electrode->projyoffset[5] - (int) *(max+3));
        
        /* check for a/d interrupt */
        if(adinfo.dmadone || adinfo.error) break;
    }

}


void DisplaySpikes(short e_num)
{
/*
** Multiple buffer code
*/
int     i, j, k;
int    spikenum;
int     *spikedataptr;
Spike     *spikeptr;
int     *prevptr;
long    sum;
float   scale;
int     peak;
int     offset;
int     *xoffset;
int     xcoord[MAXCHANNELS];
int     yoffset;
int bufsize;
int     val;
int     overlay;
unsigned long   len;
ElectrodeInfo   *electrode;
View        *spikewin;

    electrode = adinfo.electrode + e_num;
    spikeptr = electrode->spikebuf;
    spikedataptr = spikeptr->dataptr;

    overlay = sysinfo.overlay[e_num];
    /*
    ** display all of the events on the specified electrode
    */
    scale = -electrode->spikescale;
    offset = MAXVALUE;
    yoffset = electrode->yoffset;
    for (i = 0; i < electrode->nspikes; i++) {
//    for (i = 0; i < 1; i++) {
//sprintf(tmpstring, "buffer offset: %ld", spikeptr->timestamp);
//ErrorMessage(tmpstring);
        /* set the initial x coordinates and initialize max */
        for(j = 0; j < adinfo.nelect_chan; j++) {
           *(xcoord+j) = *(electrode->spikexoffset + j);
        }
        prevptr = electrode->prevbufptr;
        for(j = 0; j < SINGLESPIKELEN; j++){
           spikewin = electrode->spikewindow;
           xoffset = xcoord;
           for(k = 0; k < adinfo.nelect_chan; k++) {
               val = (*spikedataptr - offset) * scale + yoffset; 
               /* clip the point to the display window */
               if (val <= spikewin->y) 
                  val = spikewin->y+1;
               else if (val >= spikewin->y2) 
                  val = spikewin->y2-1; 

               if(!overlay){
                   /* erase the previous point */
                   drwpoint(SET, (int)BLACK, *xoffset, *prevptr);
                }
                drwpoint(SET, (int)WHITE, *xoffset, val);
                *prevptr = val;
                spikedataptr++;
                spikewin++;
                prevptr++;
                *xoffset += 3;
                xoffset++;
            }
        }
        /* move on to the next spike */
        spikeptr++;
        spikedataptr = spikeptr->dataptr;
        adinfo.display_lost--;
        /* check for a/d interrupt */
        if(adinfo.dmadone || adinfo.error) break;
    }
}
