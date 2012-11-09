/***************************************************************************
 DASDECL.H                                                  
                    
 This file includes all necessary function prototypes to interface to an 
 ASO driver.  Include this file in all your microsoft C (5.1 and up) 
 and Borland C++ (2.0 and up if NOT using C++ ) 

 ***************************************************************************/

typedef unsigned char BYTE ;
#define WORD unsigned short
typedef unsigned long DWORD ;

typedef WORD DASErr ;

enum { SyncMode, IntMode, DMAMode } ;
enum {InternalClk, ExternalClk };

/* definiton of ExtTrigOption Bits */
#define  LevelTrig       1
#define  NegTrig         2
#define  ScanTrig        4
#define  ACTrig          0x10
#define  EdgeTrig        0
#define  SingleTrig      0
#define  PosTrig         0


#define DDH     DWORD
#define FRAMEH  DWORD


#ifndef GainChanSize
#define  GainChanSize   16
#endif

typedef struct _GainChanTable {
   WORD      num_of_codes ;
   struct {
         BYTE        Chan;
         char        Gain;
   } GainChanAry[GainChanSize] ;

} GainChanTable ;


#ifndef ChanTableSize
#define ChanTableSize   256
#endif


typedef struct ChanTable {
   WORD      NumberOfChans ;
   struct{
         WORD        Mask;
         } ChanAry[ChanTableSize] ;
}ChanTable ;


typedef struct CfgStruct
   {
   WORD CurrentBoardType ;
   WORD BoardNum ;
   WORD Address ;
   WORD IntLevel ;
   WORD DMALevel ;
   }CfgStruct ;

DASErr far pascal K_ADRead( DWORD hDev, BYTE nChan, BYTE nGain, void far *pData);
DASErr far pascal K_BufListAdd (DWORD hFrame, void far *pBuf, DWORD dwSamples);
DASErr far pascal K_BufListReset (DWORD hFrame);
DASErr far pascal K_ClearFrame( DWORD hFrame ) ;
DASErr far pascal K_CloseAllDriverHandles( DWORD hDrv) ;
DASErr far pascal K_CloseDriver( DWORD hDrv );
DASErr far pascal K_ClrAboutTrig(DWORD hFrame);
DASErr far pascal K_ClrADFreeRun(DWORD hFrame );
DASErr far pascal K_ClrContRun( DWORD hFrame )  ;
DASErr far pascal K_CorrectData( DWORD hFrame);
DASErr far pascal K_DASDevInit(DWORD hDev);
DASErr far pascal K_DAWrite( DWORD hDev, BYTE nChan, DWORD dwData);
DASErr far pascal K_DAWriteGain( DWORD hDev, WORD nChan, WORD nGain, DWORD dwData);
DASErr far pascal K_DecBufBehind( DWORD hFrame );
DASErr far pascal K_DIRead( DWORD hDev, BYTE nChan, void far *pData );
DASErr far pascal K_DMAAlloc( DWORD hFrame, DWORD dwSamples, void far * far *pBuf, WORD  far * phMem);
DASErr far pascal K_DMAFree( WORD hMem);
DASErr far pascal K_DMAStart( DWORD hFrame) ;
DASErr far pascal K_DMAStatus( DWORD hFrame, short far *pStatus, DWORD far *pCount) ;
DASErr far pascal K_DMAStop( DWORD hFrame, short far *pStatus, DWORD far *pCount) ;
DASErr far pascal K_DOWrite( DWORD hDev, BYTE nChan, DWORD dwData );
DASErr far pascal K_FormatChnGAry( short far *pArray ) ;
DASErr far pascal K_FreeDevHandle( DWORD  hDev);
DASErr far pascal K_FreeFrame( DWORD hFrame);
DASErr far pascal K_GetAboutTrig(DWORD hFrame, DWORD far *pSamples);
DASErr far pascal K_GetADCommonMode(DWORD hDev, WORD far *pMode);
DASErr far pascal K_GetADConfig(DWORD hDev, WORD far *pMode);
DASErr far pascal K_GetADFrame(DWORD hDev, DWORD far  * phFrame );
DASErr far pascal K_GetADFreeRun(DWORD hFrame, short far *pStatus);
DASErr far pascal K_GetADMode(DWORD hDev, WORD far *pMode);
DASErr far pascal K_GetADTrig( DWORD hFrame, short far *pOpt, short far *pChan, DWORD far *pLevel);
DASErr far pascal K_GetBSPTAdd(DWORD hDev,short  BrdNum, void far * far * pBSPT);
DASErr far pascal K_GetBuf( DWORD hFrame,void  far *  far *pBuf, DWORD far *pSamples);
DASErr far pascal K_GetBufB( DWORD hFrame,void  far *  far *pBuf, DWORD far *pSamples);
DASErr far pascal K_GetBurstRateHz (DWORD hFrame, float far *pfBurstRateHz);
DASErr far pascal K_GetBurstTicks( DWORD hFrame, short far *pTicks );
DASErr far pascal K_GetCalData( DWORD hFrame, short nChan, short nGain, short far *pType, void far * pCalData );
DASErr far pascal K_GetCfgFileName(DWORD hDev, char far * far * pszName);
DASErr far pascal K_GetChn( DWORD hFrame, short far *pChan);
DASErr far pascal K_GetChnGAry( DWORD hFrame, void far * far *pArray);
DASErr far pascal K_GetClk( DWORD hFrame, short far *pMode);
DASErr far pascal K_GetClkRate( DWORD hFrame, DWORD far *pRate);
DASErr far pascal K_GetContRun( DWORD hFrame, short far *pMode) ;
DASErr far pascal K_GetDAFrame(DWORD hDev, DWORD far  * phFrame  );
DASErr far pascal K_GetDevHandle(DWORD hDrv, WORD  nBoardNum, DWORD far * phDev);
DASErr far pascal K_GetDIFrame(DWORD hDev, DWORD far  * phFrame  );
DASErr far pascal K_GetDITrig( DWORD hFrame, short far * pOpt, short far *pChan, DWORD far *pPattern);
DASErr far pascal K_GetDOCurVal( DWORD hFrame, void far *pValue) ;
DASErr far pascal K_GetDOFrame(DWORD hDev, DWORD far  * phFrame  );
DASErr far pascal K_GetErrMsg(DWORD hDev, short nDASErr, char  far * far *pszErrMsg);
DASErr far pascal K_GetExtClkEdge( DWORD hFrame, short far *pEdge ) ;
DASErr far pascal K_GetG( DWORD hFrame, short far *pGain);
DASErr far pascal K_GetGate( DWORD hFrame, short far *pMode);
DASErr far pascal K_GetOutputClk ( DWORD hFrame, short far *pMode, DWORD far *pTics);
DASErr far pascal K_GetPreTrig( DWORD hFrame, WORD far *pCount);
DASErr far pascal K_GetShellVer(WORD far *pVersion ) ;
DASErr far pascal K_GetSSH( DWORD hFrame, WORD far *pMode ) ;
DASErr far pascal K_GetStartStopChn( DWORD hFrame, short far *pStart, short  far *pStop);
DASErr far pascal K_GetStartStopG( DWORD hFrame, short far *pStart, short far *pStop, short far *pGain);
DASErr far pascal K_GetSync ( DWORD hFrame, short far *pMode );
DASErr far pascal K_GetTrig( DWORD hFrame,short  far *pMode);
DASErr far pascal K_GetTrigHyst( DWORD hFrame, short far *pHyst);
DASErr far pascal K_GetVer(DWORD hDev, short far * pSpecVer, short far * pDrvVer);
DASErr far pascal K_InitFrame( DWORD hFrame) ;
DASErr far pascal K_INTAlloc( DWORD hFrame, DWORD dwSamples, void far * far *pBuf, WORD  far *phMem) ;
DASErr far pascal K_IntAlloc( DWORD hFrame, DWORD dwSamples, void far * far *pBuf, WORD  far *phMem) ;
DASErr far pascal K_IntFree( WORD hMem);
DASErr far pascal K_IntStart( DWORD hFrame) ;
DASErr far pascal K_IntStatus( DWORD hFrame, short far *pStatus, DWORD far *pCount) ;
DASErr far pascal K_IntStop( DWORD hFrame, short far *pStatus, DWORD far *pCount) ;
DASErr far pascal K_MoveDataBuf(short far *pDest, short far *pSource, WORD nCount);
DASErr far pascal K_OBMDataMove( DWORD hFrame )  ;
DASErr far pascal K_OBMGetAddressStep( DWORD hFrame, WORD far *pStep);
DASErr far pascal K_OBMGetChanAry( DWORD hFrame, ChanTable far * far *pArray) ;
DASErr far pascal K_OBMGetMode( DWORD hFrame, WORD far *pMode ) ;
DASErr far pascal K_OBMGetPostTrigger( DWORD hFrame, DWORD far *pPostTrig );
DASErr far pascal K_OBMGetPreTrigger( DWORD hFrame, DWORD far *pPreTrig);
DASErr far pascal K_OBMGetRepeatCount( DWORD hFrame, DWORD far *pCount );
DASErr far pascal K_OBMGetStartAddress(DWORD hFrame, DWORD far *pAddr);
DASErr far pascal K_OBMGetTriggerAddress( DWORD hFrame, DWORD far *pAddr);
DASErr far pascal K_OBMSetAddressStep( DWORD hFrame, WORD nStep);
DASErr far pascal K_OBMSetChanAry( DWORD hFrame, ChanTable far *pArray) ;
DASErr far pascal K_OBMSetMode( DWORD hFrame, WORD nMode ) ;
DASErr far pascal K_OBMSetPostTrigger( DWORD hFrame, DWORD dwPostTrig);
DASErr far pascal K_OBMSetRepeatCount( DWORD hFrame, DWORD dwCount );
DASErr far pascal K_OBMSetStartAddress(DWORD hFrame, DWORD dwAddr) ;
DASErr far pascal K_OBMSetTriggerAddress( DWORD hFrame, DWORD dwAddr);
DASErr far pascal K_OBMStart( DWORD hFrame);
DASErr far pascal K_OBMStatus( DWORD hFrame, WORD far *pStatus, DWORD far *pCount);
DASErr far pascal K_OBMStop( DWORD hFrame, WORD far *pStatus, DWORD far *pCount);
DASErr far pascal K_OpenDriver( char far * szDevName, char far * szCfgName, DWORD far * phDrv);
DASErr far pascal K_RestoreChnGAry( short far *pArray ) ;
DASErr far pascal K_SetAboutTrig(DWORD hFrame, DWORD dwSamples);
DASErr far pascal K_SetADCommonMode(DWORD hDev, WORD nMode);
DASErr far pascal K_SetADConfig(DWORD hDev, WORD nMode);
DASErr far pascal K_SetADFreeRun(DWORD hFrame) ;
DASErr far pascal K_SetADMode(DWORD hDev, WORD nMode);
DASErr far pascal K_SetADTrig( DWORD hFrame, short nOpt, short nChan, DWORD dwLevel);
DASErr far pascal K_SetBuf( DWORD hFrame,void  far *pBuf, DWORD dwSamples);
DASErr far pascal K_SetBufB( DWORD hFrame,void  far * pBuf, DWORD Samples);
DASErr far pascal K_SetBufOverrunCtrl( DWORD hFrame, short bBufCtrl );
DASErr far pascal K_SetBurstRateHz (DWORD hFrame, float fBurstRateHz);
DASErr far pascal K_SetBurstTicks( DWORD hFrame, short nTicks );
DASErr far pascal K_SetCalMode( DWORD hFrame, short nCalMode);
DASErr far pascal K_SetChn( DWORD hFrame, short nChan);
DASErr far pascal K_SetChnGAry( DWORD hFrame, void far *pArray);
DASErr far pascal K_SetClk( DWORD hFrame, short nMode);
DASErr far pascal K_SetClkRate( DWORD hFrame, DWORD dwDivisor);
DASErr far pascal K_SetContRun( DWORD hFrame ) ;
DASErr far pascal K_SetCritVec(DWORD hDev, short BrdNum, void (far * pfRoutine)());
DASErr far pascal K_SetDITrig( DWORD hFrame, short nOpt, short nChan, DWORD nPattern);
DASErr far pascal K_SetDMABuf( DWORD hFrame,void far *pBuf, DWORD dwSamples);
DASErr far pascal K_SetDMABufB( DWORD hFrame, void far *pBuf, DWORD Samples);
DASErr far pascal K_SetExtClkEdge( DWORD hFrame, short nEdge ) ;
DASErr far pascal K_SetG( DWORD hFrame, short nGain);
DASErr far pascal K_SetGate( DWORD hFrame, short nMode) ;
DASErr far pascal K_SetIRQMap(DWORD hDev, short IRQ0, short IRQ8);
DASErr far pascal K_SetOutputClk ( DWORD hFrame, short nMode, DWORD nTics);
DASErr far pascal K_SetPreTrig( DWORD hFrame, short nCount);
DASErr far pascal K_SetSSH( DWORD hFrame, WORD nMode ) ;
DASErr far pascal K_SetStartStopChn( DWORD hFrame, short nStart, short nStop);
DASErr far pascal K_SetStartStopG( DWORD hFrame, short nStart, short nStop, short nGain);
DASErr far pascal K_SetSync ( DWORD hFrame, short nMode );
DASErr far pascal K_SetTrig( DWORD hFrame,short nMode);
DASErr far pascal K_SetTrigHyst( DWORD hFrame, short nHyst);
DASErr far pascal K_SyncStart( DWORD hFrame) ;




