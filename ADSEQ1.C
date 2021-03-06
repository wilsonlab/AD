#include "adext.h"

void BlockSeqOn(void)
{
	/*
	** start acquisition
	*/
	EventString("BLOCKON");
	/*
	** activate sequence to start acq
	*/
	sysinfo.seq[STIMSEQON].active = 1;
	sysinfo.seq[STIMSEQON].time = 0;
	sysinfo.seq[BLOCKSEQOFF].active = 1;
	sysinfo.seq[BLOCKSEQOFF].time = ReadTS();
}			

void BlockSeqOff(void)
{
	/*
	** stop acquisition
	*/
	EventString("BLOCKOFF");
	/*
	** activate sequence to start acq
	*/
	sysinfo.seq[BLOCKSEQOFF].active = 0;
	sysinfo.seq[STIMSEQON].active = 0;
}			

void StartStimAcq(void)
{
	/*
	** start acquisition
	*/
	StartAcq();
	EventString("STIMON");
	/*
	** stimulate
	*/
	Stimulate();
	/*
	** activate sequence to stop acq
	*/
	sysinfo.seq[STIMSEQOFF].active = 1;
	sysinfo.seq[STIMSEQOFF].time = ReadTS();
}			

void EndStimAcq(void)
{
	EventString("STIMOFF");
	/*
	** stop acquisition
	*/
	TestAndStopAcq();
	/*
	** shut off sequence 
	*/
	sysinfo.seq[STIMSEQOFF].active = 0;
}			

void CheckBar(void)
{
	/*
	** check the pio status for a bar press
	*/
	if(ReadPIO2() == 2){
		EventString("BARPRESS");
	} else {
		EventString("NO PRESS");
	}
}

void StartLight(void)
{
	EventString("LIGHTON");
	/*
	** turn on the LED1
	*/
	RaisePIO2(2);
	/*
	** activate sequence to turn off the light. This will control
	** the duration of the LED on period
	*/
	sysinfo.seq[LIGHTSEQOFF].active = 1;
	sysinfo.seq[LIGHTSEQOFF].time = ReadTS();
	/*
	** activate wait sequence to end trial and begin 
	** intertrial delta. This controls length of trial
	*/
	sysinfo.seq[DELTASEQ].active = 1;
	sysinfo.seq[DELTASEQ].time = ReadTS();
}			

void EndLight(void)
{
	EventString("LIGHTOFF");
	/*
	** turn off light
	*/
	WritePIO2(0x00);
	/*
	** inactivate the sequence to turn off the light.
	** note that the light-on sequence will continue to run
	*/
	sysinfo.seq[LIGHTSEQOFF].active = 0;
	/*
	** activate the sequence to check lever and operate feeder
	*/
	sysinfo.seq[FEEDERSEQON].active = 1;
	sysinfo.seq[FEEDERSEQON].time = ReadTS();
}			

void StartDeltaWait(void)
{
	EventString("END TIME TRIAL");
	/*
	** inactivate sequence that marks trial
	*/
	sysinfo.seq[DELTASEQ].active = 0;
	/* 
	** inactivate feeder control sequences 
	/*
	sysinfo.seq[FEEDERSEQON].active = 0;
	sysinfo.seq[FEEDERSEQOFF].active = 0;
}

void TurnFeederOn(void)
{
	if(ReadPIO2() == 2){
		EventString("FEEDER ON");
		/*
		** if bar press detected operate feeder, turn on seq to
		** stop feeder, and turn off feeder check/operate seq
		*/
		RaiseBitPIO2(1);
		sysinfo.seq[FEEDERSEQOFF].active = 1;
		sysinfo.seq[FFEDERSEQOFF].time = ReadTS();
		sysinfo.seq[FEEDERSEQON].active = 0;
	}
}

void TurnFeederOff(void)
{
	EventString("FEEDER OFF");
	/*
	** turns off the feeder and reactivates the seq to
	** check/operate the feeder
	*/
 	LowerBitPIO2(1);
	sysinfo.seq[FEEDERSEQON].active = 1;
	sysinfo.seq[FEEDERSEQON].time = ReadTS();
}

void SetupSequences(void)
{	
	sysinfo.seq[BLOCKSEQOFF].id = BLOCKSEQ;
	sysinfo.seq[BLOCKSEQOFF].interval = 0;
	sysinfo.seq[BLOCKSEQOFF].func = BlockSeqOff;
	sysinfo.seq[BLOCKSEQOFF].active = 0;
	
	sysinfo.seq[BLOCKSEQ].id = BLOCKSEQ;
	sysinfo.seq[BLOCKSEQ].interval = 0;
	sysinfo.seq[BLOCKSEQ].func = BlockSeqOn;
	sysinfo.seq[BLOCKSEQ].active = 0;
	
	sysinfo.seq[STIMSEQON].id = STIMSEQON;
	sysinfo.seq[STIMSEQON].interval = 0;
	sysinfo.seq[STIMSEQON].func = StartStimAcq;
	sysinfo.seq[STIMSEQON].active = 0;
	
	sysinfo.seq[STIMSEQOFF].id = STIMSEQOFF;
	sysinfo.seq[STIMSEQOFF].interval = 0;
	sysinfo.seq[STIMSEQOFF].func = EndStimAcq;
	sysinfo.seq[STIMSEQOFF].active = 0;

	sysinfo.seq[LIGHTSEQON].id = LIGHTSEQON;
	sysinfo.seq[LIGHTSEQON].interval = 0;
	sysinfo.seq[LIGHTSEQON].func = StartLight;
	sysinfo.seq[LIGHTSEQON].active = 0;
	
	sysinfo.seq[LIGHTSEQOFF].id = LIGHTSEQOFF;
	sysinfo.seq[LIGHTSEQOFF].interval = 20000;	/* default 2 sec */
	sysinfo.seq[LIGHTSEQOFF].func = EndLight;
	sysinfo.seq[LIGHTSEQOFF].active = 0;
	
	sysinfo.seq[CHECKBARSEQ].id = CHECKBARSEQ;
	sysinfo.seq[CHECKBARSEQ].interval = 1000;	/* check every 100 msec */
	sysinfo.seq[CHECKBARSEQ].func = CheckBar;
	sysinfo.seq[CHECKBARSEQ].active = 0;

	sysinfo.seq[FEEDERSEQON].id = FEEDERSEQON;
	sysinfo.seq[FEEDERSEQON].interval = 1000; 		/* check every 100 msec */
	sysinfo.seq[FEEDERSEQON].func = TurnFeederOn;
	sysinfo.seq[FEEDERSEQON].active = 0;

	sysinfo.seq[FEEDERSEQOFF].id = FEEDERSEQOFF;
	sysinfo.seq[FEEDERSEQOFF].interval = 200;		/* 2 msec feeder pulse */
	sysinfo.seq[FEEDERSEQOFF].func = TurnFeederOff; 
	sysinfo.seq[FEEDERSEQOFF].active = 0;

	sysinfo.seq[DELTASEQ].id = DELTASEQ;
	sysinfo.seq[DELTASEQ].interval = 100;   		/* check every 100 msec */
	sysinfo.seq[DELTASEQ].func = StartDeltaWait;
	sysinfo.seq[DELTASEQ].active = 0;
	
	sysinfo.nseq = 10;
}	
