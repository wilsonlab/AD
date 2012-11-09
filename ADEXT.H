#include "addefs.h"
/* Function prototypes */
unsigned long compute_timestamp(short bufindex, unsigned long offset);
int	InitBoard(void);
int	SetBoardGains(void);
int	GetData(void);
WORD getDMAlength(int far *buffer);
int getnelectrodes(void);
long DiskFree(void);
short nextbuf(short buf);
short prevbuf(short buf);
int LoadConfig(char *filename);
char *GetHeaderParameter(char **header, char *parm);
char **ReadHeader(FILE *infile, int *headersize);
float SpikeWindowScale(int channel);
float ContWindowScale(ChannelInfo *cptr);
int SetRate(float rate, float *rate_set);
int SetDTGains(void);
int SetElectrodeAmpGains(int e_num,long gain);
int SetAmpGains(long gain);
int SetAmpGain(int channel, long gain);
int	ReadPIO2(void);
int	ReadPIO(void);

void StartAcq(void);
void TestAndStopAcq(void);
void IncIRQMask(int val);
void ClkWriteCommand(int com);
void interrupt far ClkGetCommand(void);
void ClkSetupMasterSlave(void);
void ClkSetupSingle(void);
void ClkSetupStim(void);
void SetSingleStimParm(int pulselen);
void SetStimParm(int pulselen);
void ArmStimClocks(void);
void DisarmStimClocks(void);
void ResetStimOutput(void);
void Stimulate(void); 
void ClkResetClock(void);
void StartFout(void);
void StopFout(void);
void ArmClock(void);
void DisarmClock(void);
void ClkResetBoard(void);
void ClkSetupCommandBus(void);
void ClkRestoreSystem(void);
void ClkProcessCommand(void);
void ClkMasterProcessCommand(int com);
void ClkShowCommands(void);
void EventString(char *string);
void EventStringAndTime(char *string,unsigned long timestamp);
void SaveEventString(void);
void SaveRawData(void);
void SaveSpikeData(short e_num);
void ToggleDisk(void);
void ToggleGraphics(void);
void PrepareStartAcq(void);
void StartAcq(void);
void StopAcq(void);
void TestAndStopAcq(void);
void DisableMouse(void);
void EnableMouse(void);
void ToggleAcq(void);
void DiskOn(void);
void DiskOff(void);
void ToggleAutoscale(void);
void CheckButtons(void);
void CheckMouse(void);
void LoadAndUpdateConfig(char *filename);
void DisplayHeader(FILE *fp, char **header, long headersize);
void WriteHeader(void);
void SetThreshold(int channel,int thresh);
void SetElectrodeThresholds(int e_num,int thresh);
void SetAllThresholds(int thresh);
void ProcessString(void);
void InputMessage(char *string);
void ReadInputString(char *outputstr, char *inputstr);
void ADCloseFile(void);
void UpdateChannelSelected(void);
void GetStimArea(View *area);
void CheckKeyboard(void);
void SelectGraphics(void);
void SelectGraphics(void); 
void Pause(void);
void StatusLine( char *msg );
void DrawBorder(void);
void changetextstyle(int font, int direction, int charsize);
void MainWindow( char *header );
void Initialize(void);
void ReportStatus(void);
void drwbox(int mode, int color, int x,int y, int x2, int y2);
void drwstring(int mode, int color, char *string, int x, int y);
void drwpoint(int mode,int color,int x, int y);
void drwfillbox(int mode, int color, int x,int y, int x2, int y2);
void drwline(int mode, int color, int x, int y, int x2, int y2);
void DrawArrow(int color,int x, int y);
void SystemSetup(void);
void SetStringPositions(void);
void DrawSessionString(void);
void DisplaySetup(void);
void DisplayStartupStatus(void);
void DrawModeButton(char *string1, int color, int x, int y, int x2, int y2, 
                    short state);
void DrawButton(char *string1, char *string2, int x, int y, int x2, int y2, 
                short state);
void DrawButton2(char *string1, char *string2, char *string3,
				int x, int y, int x2, int y2);
void DefineButton(int bnum, int x, int y, int x2, int y2, char* data,
			void (*handler)());
void HighlightButton(int bnum,int color);
void UnHighlightButton(int bnum);
void ButtonStatus(int bnum, int event);
void DefineMessageArea(int mnum, short x, short y, short x2, short y2);
void do_quit(int bnum, int event);
void do_toggleacq(int bnum, int event);
void do_channelbutton(int bnum, int event);
void DefineChannelButtons(int buttonstart);
void CreateMenu(void);
void UpdateDiskButton(void);
void UpdateAcqButton(void);
void UpdateTrackerButton(void);
void UpdateFileButton(void);
void DrawMenu(void);
void DisplayRate(void);
void DisplayAmpFilters(void);
void DrawStrings(void);
void DisplayCurrentTime(void);
void DisplayDiskInfo(void);
void DrawSPButtons(void);
void DrawSPButton(int button);
void DrawChannelButtons(void);
void DrawChannelButton(int channel);
void SetContWindowScale(ChannelInfo *cptr);
void DrawAmpButtons(void);
void DrawScopeBorder(void);
void DrawTrackerBorder(void);
void ClearScopeBox(void);
void ClearTrackerBox(void);
void ClearSpikeAreas(void);
void ClearHelpWindow(void);
void DrawHelpWindow(void);
void DrawStimWindow(void);
void DrawTrackerGrid(void);
void DrawStimProtocol(void);
void RedrawMain(void);
void DrawSPBorders(void);
void DrawSpikeBorders(int e_num);
void DrawSpikeBorder(int e_num, int channel);
void DrawProjectionBorder(int e_num, int channel);
void DrawProjectionBorders(int e_num);
void ClearSpikeBoxes(int e_num);
void ClearSpikeBox(int e_num, int channel);
void ClearProjectionBoxes(int e_num);
void ClearProjectionBox(int e_num, int channel);
void RefreshDisplay(void);
void ClearScreen(void);
void DisplayADStatus(void);
void StatusMessage(char *string);
void ModeMessage(void);
void StatusMessage3(char *string);
void StatusMessage4(char *string);
void ErrorMessage(char *string);
void ChannelMessage(char *string,short channel);
void DrawContinuousTicks(void);
void DisplayContinuousData(void);
void DisplayProjections(short e_num);
void DisplaySpikes(short e_num);
void ProgramDMAController(void);
void allocbuff(int far **buffer, WORD buffsize);
void SetupTracker(void);
void SetupPIO2(void);
void GetDMAInfo(WORD bufsize, int far *buffer, WORD *DMAPage, WORD *DMAbase);
void InitDMA5(WORD page, WORD offset, WORD count);
void InitDMA6(WORD page, WORD offset, WORD count);
void InitDMA7(WORD page, WORD offset, WORD count);
void InitDMAChannels(void);
void ResetDMAChannels(void);
void Clk2WriteCommand(int com);
void interrupt far Clk2GetCommand(void);
void ClkResetBoard2(void);
void Clk2SetupCommandBus(void);
void Clk2RestoreSystem(void);
void Clk2ProcessCommand(void);
void Clk2MasterProcessCommand(int com);
void BlockSeqOn(void);
void BlockSeqOff(void);
void StartStimAcq(void);
void EndStimAcq(void);
void CheckBar(void);
void StartLight(void);
void EndLight(void);
void StartDeltaWait(void);
void TurnFeederOn(void);
void TurnFeederOff(void);
void SetupSequences(void);
void WritePIO(int val);
void WriteAmpControl(int val);
void InitDTPIO(void);
void SetEqualizationReg( int regval );
void LoadSeqCtrReg( int regval );
void IncSeqCtrReg( void );
void LoadAmpReg( int regval );
void SetAmpFilter( int channel, int filter);
void SetAmpChannelGain( int channel, int gain );
void Equalize(void);
void SetAmpControls(void);
void CycleAmpLowFilter(void);
void CycleAmpHighFilter(void);
void GetAmpFilters(char *l, char *h);
void CycleAmpLowFilter(void);
void CycleAmpHighFilter(void);
void GetAmpFilters(char *l, char *h);
void WritePIO2(int val);
void RaiseBitPIO2(int val);
void LowerBitPIO2(int val);
void TrackerOn(void);
void TrackerOff(void);
void ToggleTracker(void);
void DisplayTracker(void);
void WriteTrackerBuffer(void);
void interrupt Tracker_ISR(void);
void RestoreTracker(void);
void main(int argc, char **argv);
void BoardRestore(void);
void BoardSetup(void);
void SystemRestore(void);
void SystemExit(int errorlevel,char *string);
void AllocateBuffers(void);
void ADSetup(void);
void ADResetClock(void);
void InitAcq(void);
void interrupt GetCompletedBuffer(void);
void SpikeModeProcessBuffer(void);
void ContinuousModeProcessBuffer(void);
void CheckBoardID(void);
void WaitForKey(void);

/*									*/
/*	Function prototypes						*/
/*									*/

int  gprintf(int *xloc, int *yloc, char *fmt, ... );

extern ADInfo	adinfo;
extern SystemInfo	sysinfo;
extern char	far tmpstring[200];
extern char	far tmpstring2[200];
extern char	far tmpstring3[200];
extern char	far tmpstring4[200];
extern char	far tmpstring5[200];
extern Button	buttonlist[MAXBUTTONS];
extern MessageArea	message_area[MAXBUTTONS];
extern WORD	supcsr,adcsr;		/* DT2821 register variables */

extern char *Fonts[NFONTS];
extern char *LineStyles[];

extern char *FillStyles[];
extern char *TextDirect[];

extern char *HorizJust[];

extern char *VertJust[];

struct PTS {
  int x, y;
};	/* Structure to hold vertex points	*/

extern int    GraphDriver;		/* The Graphics device driver		*/
extern int    GraphMode;		/* The Graphics mode value		*/
extern double AspectRatio;		/* Aspect ratio of a pixel on the screen*/
extern int    MaxX, MaxY;		/* The maximum resolution of the screen */
extern int    MaxColors;		/* The maximum # of colors available	*/
extern int    ErrorCode;		/* Reports any graphics errors		*/
extern struct palettetype palette;		/* Used to read palette info	*/




extern unsigned long ReadTS(void);
extern char *FormatTS(unsigned long,int val);
extern Tracker	tracker;
extern void interrupt	(*oldtrackerirqvect)();

extern int	trackerirqmask;
extern int	trackerirqvect;
extern int	tracker_irq;
extern int clk_irq;
extern int clk2_irq;
