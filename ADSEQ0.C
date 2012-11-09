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
	if(ReadPIO2() != 0){
		EventString("BARPRESS");
	} else {
		EventString("NO PRESS");
	}
}

void StartLight(void)
{
	EventString("LIGHTON");
	/*
	** turn on the LEDs
	*/
	WritePIO2(0xFF);
	/*
	** activate sequence to turn off the light. This will control
	** the duration of the LED on period
	*/
	sysinfo.seq[LIGHTSEQOFF].active = 1;
	sysinfo.seq[LIGHTSEQOFF].time = ReadTS();
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
	
	sysinfo.nseq = 7;
}	
