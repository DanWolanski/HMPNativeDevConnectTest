//THis is for loging

#pragma warning( disable : 4390 )
#define MAX_LOG_STR_SIZE 1024
#define LOGNAME "App.log"

// Log levels
#define ALWAYS	-1 
#define ERROR	0
#define EVENT	2
#define API		4
#define INFO	6
#define ENTRY	8
#define EXIT	10
#define DEBUG	12
#define LOGALL	20

#define NOINDEX -1

static int index=NOINDEX;  // this is a global so that if there is no index in scope
						   // logging macros will not fail
static int LogLevel = LOGALL;

static void Log(int level, int index, const char* Format, ...);
static char *LogLevel2Str(int level);
class CAppLog  
{
public:
	CAppLog();
	virtual ~CAppLog();
	void Write(char *LogStr);
	void Open(char *logname,int level,FILE *fh);
	void Close();

private:
	int mLogLevel;
	char mLogName[MAX_LOG_STR_SIZE];
	FILE *mLogfh;
};


CAppLog::CAppLog():
mLogfh(NULL)
{
	
}
void CAppLog::Close(){
	Log(DEBUG,NOINDEX,"Closing log file %s",mLogName);
	fclose(mLogfh);
	mLogfh=NULL;
}
void CAppLog::Open(char *logname=LOGNAME,int level=LOGALL,FILE *fh=NULL )
{
	mLogLevel=level;
	strcpy(mLogName,logname);

	if (mLogfh==NULL){

		mLogfh = fopen(mLogName, "wt");
		if (!mLogfh)
		{
			printf("Error opening log file %s\n",mLogName);
			exit(1);
		}

	}

	Log(DEBUG,NOINDEX,"Log file %s opened, Level=%s",
		mLogName,
		LogLevel2Str(mLogLevel));
	

}

void CAppLog::Write(char *LogStr){
		fprintf(mLogfh, "%s\n", LogStr);
		fflush(mLogfh);
}

CAppLog::~CAppLog()
{
	
	
	if(mLogfh){
		Close();
	}

}

static CAppLog *gAppLog=NULL;
static CAppLog AppLog;
static int log_crc;
#define CRC(rc,logstr)\
	log_crc=rc;\
	if(log_crc == -1){ Log(ERROR,index,"Error in %s",logstr);} \
	if(log_crc == -1)

#define LOG_FILE_INIT(szLogName)\
	gAppLog=&AppLog;\
	gAppLog->Open(szLogName,LogLevel);\
	
#define LOG_SETLOGLEVEL(level)\
	LogLevel=level;\
	Log(ALWAYS,NOINDEX,"Setting LogLevel to %s",LogLevel2Str(LogLevel));

#define LOG_ENTER(szFunc)\
	 char FuncName[MAX_LOG_STR_SIZE] = szFunc;\
	 Log(ENTRY,index,"Entering %s",FuncName);

#define LOG_EXIT()\
 Log(EXIT,index,"Exiting %s",FuncName);


#define LOG_ENTRYEXIT(szFunc)\
	 CFuncLogger flogger(szFunc,index);

void Log(int level, int index, const char* Format, ...)
{
	char LogStr[MAX_LOG_STR_SIZE] = {0};
	va_list ArgList;
	SYSTEMTIME SystemTime;


	if(level > LogLevel){
		return;
	}

	::GetLocalTime(&SystemTime);
	sprintf(LogStr+strlen(LogStr), 
					"%02d:%02d:%02d.%03d ",
					SystemTime.wHour,
					SystemTime.wMinute,
					SystemTime.wSecond,
					SystemTime.wMilliseconds);


	sprintf(LogStr+strlen(LogStr),"%s   ",LogLevel2Str(level));
	
	if(index == NOINDEX){
		sprintf(LogStr+strlen(LogStr),"XXX| ");
	}
	else{
		sprintf(LogStr+strlen(LogStr),"%03d| ",index);
	}
	va_start(ArgList, Format);
	vsprintf(LogStr+strlen(LogStr), Format, ArgList);

	
	printf("%s\n", LogStr);
	fflush(stdout);

	
	if (gAppLog!=NULL){
		gAppLog->Write(LogStr);

	}
	
}

char *DXTerm2Str(long lTermMask)
	{
	if(lTermMask & TM_DIGIT) 		 return "TM_DIGIT";
	else if(lTermMask & TM_MAXDTMF)	 return "TM_MAXDTMF";
	else if(lTermMask & TM_MAXSIL)	 return "TM_MAXSIL";
	else if(lTermMask & TM_MAXNOSIL) return "TM_MAXNOSIL";
	else if(lTermMask & TM_LCOFF)	 return "TM_LCOFF";
	else if(lTermMask & TM_IDDTIME)	 return "TM_IDDTIME";
	else if(lTermMask & TM_MAXTIME)	 return "TM_MAXTIME";
	else if(lTermMask & TM_USRSTOP)	 return "TM_USRSTOP";
	else if(lTermMask & TM_TONE)	 return "TM_TONE";
	else if(lTermMask & TM_EOD)		 return "TM_EOD";
	else if(lTermMask & TM_ERROR)	 return "TM_ERROR";
	else if(lTermMask & TM_PATTERN)	 return "TM_PATTERN";
	if (lTermMask == 0) return "Normal Termination";
	return "";
	}

char *SRLMode2Str(int mode){
	switch(mode)
		{
	case  SR_STASYNC    : return "SR_STASYNC / SR_POLLMODE";
	case  SR_MTASYNC      : return "SR_MTASYNC / SR_SIGMODE";
	case  SR_MTSYNC       : return "SR_MTSYNC";
	default				: return "Unknown SRLMODE";
		}
	}

char *CPTerm2Str(long lCPTermMask)
	{
	switch(lCPTermMask)
		{
	case  CR_BUSY       : return "CR_BUSY";
	case  CR_NOANS      : return "CR_NOANS";
	case  CR_NORB       : return "CR_NORB";
	case  CR_CNCT       : return "CR_CNCT";
	case  CR_CEPT       : return "CR_CEPT";
	case  CR_STOPD      : return "CR_STOPD";
	case  CR_NODIALTONE : return "CR_NODIALTONE";
	case  CR_FAXTONE    : return "CR_FAXTONE";
	case  CR_ERROR      : return "CR_ERROR";
	default				: return "Unknown CP termination";
		}
	}

char *CSTEvt2Str(unsigned short ushCSTEvt)
	{
	switch(ushCSTEvt)
		{
	case  DE_RINGS     : return "DE_RINGS";
	case  DE_SILON     : return "DE_SILON";
	case  DE_SILOF     : return "DE_SILOF";
	case  DE_LCON      : return "DE_LCON";
	case  DE_LCOF      : return "DE_LCOF";
	case  DE_WINK      : return "DE_WINK";
	case  DE_RNGOFF    : return "DE_RNGOFF";
	case  DE_DIGITS    : return "DE_DIGITS";
	case  DE_DIGOFF    : return "DE_DIGOFF";
	case  DE_LCREV     : return "DE_LCREV";
	case  DE_TONEON    : return "DE_TONEON";
	case  DE_TONEOFF   : return "DE_TONEOFF";
	case  DE_STOPRINGS : return "DE_STOPRINGS";
	default			   : return "Unknown CST event";
		}
	}
/*
char *CnfEventToStr(unsigned int iEvent)
{
	switch(iEvent)
	{
	case CNFEV_OPEN                    : return "CNFEV_OPEN";
	case CNFEV_OPEN_CONF               : return "CNFEV_OPEN_CONF";
	case CNFEV_OPEN_PARTY              : return "CNFEV_OPEN_PARTY";
	case CNFEV_ADD_PARTY               : return "CNFEV_ADD_PARTY";
	case CNFEV_REMOVE_PARTY            : return "CNFEV_REMOVE_PARTY";
	case CNFEV_GET_ATTRIBUTE           : return "CNFEV_GET_ATTRIBUTE";
	case CNFEV_SET_ATTRIBUTE           : return "CNFEV_SET_ATTRIBUTE";
	case CNFEV_ENABLE_EVENT            : return "CNFEV_ENABLE_EVENT";
	case CNFEV_DISABLE_EVENT           : return "CNFEV_DISABLE_EVENT";
	case CNFEV_GET_DTMF_CONTROL        : return "CNFEV_GET_DTMF_CONTROL";
	case CNFEV_SET_DTMF_CONTROL        : return "CNFEV_SET_DTMF_CONTROL";
	case CNFEV_GET_ACTIVE_TALKER       : return "CNFEV_GET_ACTIVE_TALKER";
	case CNFEV_GET_PARTY_LIST          : return "CNFEV_GET_PARTY_LIST";
	case CNFEV_GET_DEVICE_COUNT        : return "CNFEV_GET_DEVICE_COUNT";
	case CNFEV_CONF_OPENED             : return "CNFEV_CONF_OPENED";
	case CNFEV_CONF_CLOSED             : return "CNFEV_CONF_CLOSED";
	case CNFEV_PARTY_ADDED             : return "CNFEV_PARTY_ADDED";
	case CNFEV_PARTY_REMOVED           : return "CNFEV_PARTY_REMOVED";
	case CNFEV_DTMF_DETECTED           : return "CNFEV_DTMF_DETECTED";
	case CNFEV_ACTIVE_TALKER           : return "CNFEV_ACTIVE_TALKER";
	case CNFEV_ERROR                   : return "CNFEV_ERROR";
	case CNFEV_OPEN_FAIL               : return "CNFEV_OPEN_FAIL";
	case CNFEV_OPEN_CONF_FAIL          : return "CNFEV_OPEN_CONF_FAIL";
	case CNFEV_OPEN_PARTY_FAIL         : return "CNFEV_OPEN_PARTY_FAIL";
	case CNFEV_ADD_PARTY_FAIL          : return "CNFEV_ADD_PARTY_FAIL";
	case CNFEV_REMOVE_PARTY_FAIL       : return "CNFEV_REMOVE_PARTY_FAIL";
	case CNFEV_GET_ATTRIBUTE_FAIL      : return "CNFEV_GET_ATTRIBUTE_FAIL";
	case CNFEV_SET_ATTRIBUTE_FAIL      : return "CNFEV_SET_ATTRIBUTE_FAIL";
	case CNFEV_ENABLE_EVENT_FAIL       : return "CNFEV_ENABLE_EVENT_FAIL";
	case CNFEV_DISABLE_EVENT_FAIL      : return "CNFEV_DISABLE_EVENT_FAIL";
	case CNFEV_GET_DTMF_CONTROL_FAIL   : return "CNFEV_GET_DTMF_CONTROL_FAIL";
	case CNFEV_SET_DTMF_CONTROL_FAIL   : return "CNFEV_SET_DTMF_CONTROL_FAIL";
	case CNFEV_GET_ACTIVE_TALKER_FAIL  : return "CNFEV_GET_ACTIVE_TALKER_FAIL";
	case CNFEV_GET_PARTY_LIST_FAIL     : return "CNFEV_GET_PARTY_LIST_FAIL";
	case CNFEV_GET_DEVICE_COUNT_FAIL   : return "CNFEV_GET_DEVICE_COUNT_FAIL";
	case CNFEV_RESET_DEVICES		   : return "CNFEV_RESET_DEVICES";
	default							   : return "Unknown CNF event";
	}
}
*/
int Evt2Str(int hDev, long lEvtType, long *lEvtDatap, char *Str)
	{
	DX_CST *pCST;
	long lTermMask=0, lCPTermMask=0;

	if ((lEvtType >= TDX_PLAY) && (lEvtType <= TDX_GETR2MF))
		{
		lTermMask = ATDX_TERMMSK(hDev);
		}

	switch(lEvtType)
		{
	// dxxxlib events
		case TDX_PLAY:
			strcpy (Str,  "TDX_PLAY");
			sprintf(Str + strlen(Str), " Data 0x%lx %s",
									  lTermMask, DXTerm2Str(lTermMask));
			break;
		case TDX_RECORD:
			strcpy (Str,  "TDX_RECORD");
			sprintf(Str + strlen(Str), " Data 0x%lx %s",
									  lTermMask, DXTerm2Str(lTermMask));
			break;
		case TDX_GETDIG:
			strcpy (Str,  "TDX_GETDIG");
			sprintf(Str + strlen(Str), " Data 0x%lx %s",
									  lTermMask, DXTerm2Str(lTermMask));
			break;
		case TDX_DIAL:
			strcpy (Str,  "TDX_DIAL");
			sprintf(Str + strlen(Str), " Data 0x%lx %s",
									  lTermMask, DXTerm2Str(lTermMask));
			break;
		case TDX_CALLP:
			strcpy (Str,  "TDX_CALLP");
			lCPTermMask = ATDX_CPTERM(hDev);
			sprintf(Str + strlen(Str), " Data 0x%lx %s", 
								lCPTermMask, CPTerm2Str(lCPTermMask));
			if (lCPTermMask == CR_ERROR)
				sprintf(Str + strlen(Str), " 0x%lx", ATDX_CPERROR(hDev));
			break;
		case TDX_CST:
			strcpy (Str, "TDX_CST");
			pCST = (DX_CST *) lEvtDatap;
			if(pCST == NULL)
			break;
			sprintf(Str + strlen(Str), "Data 0x%x %s", 
			  	(BYTE) (pCST->cst_data & 0xff), CSTEvt2Str(pCST->cst_event));
			break;
		case TDX_SETHOOK:
			strcpy (Str,  "TDX_SETHOOK");
			pCST = (DX_CST *) lEvtDatap;
			if(pCST == NULL)
				break;
			switch(pCST->cst_event)
			{
			case DX_OFFHOOK:
				sprintf(Str + strlen(Str), "Data 0x%x Offhook", pCST->cst_event);
				break;
			case DX_ONHOOK:
				sprintf(Str + strlen(Str), "Data 0x%x Onhook", pCST->cst_event);
				break;
			} // End switch(pCST->cst_event)
			break;
		case TDX_WINK:
			strcpy (Str,  "TDX_WINK");
			break;
		case TDX_PLAYTONE:
			strcpy (Str,  "TDX_PLAYTONE");
			sprintf(Str + strlen(Str), " Data 0x%lx %s",
									  lTermMask, DXTerm2Str(lTermMask));
			break;
		case TDX_GETR2MF:
			strcpy (Str,  "TDX_GETR2MF");
			break;
		case TDX_ERROR:
			strcpy (Str,  "TDX_ERROR");
			sprintf(Str + strlen(Str), " %s", ATDV_ERRMSGP(hDev));
			break;
		case TDX_BARGEIN:
			strcpy (Str,  "TDX_BARGEIN");
			break;
		case DX_ATOMIC_ERR:
			strcpy (Str,  "DX_ATOMIC_ERR");
			break;

	// ISDN events
		case CCEV_TASKFAIL      :
			strcpy (Str,  "CCEV_TASKFAIL");
			break;
		case CCEV_ANSWERED      :
			strcpy (Str,  "CCEV_ANSWERED");
			break;
		case CCEV_CALLPROGRESS  :
			strcpy (Str,  "CCEV_CALLPROGRESS");
			break;
		case CCEV_ACCEPT        :
			strcpy (Str,  "CCEV_ACCEPT");
			break;
		case CCEV_DROPCALL      :
			strcpy (Str,  "CCEV_DROPCALL");
			break;
		case CCEV_RESTART       :
			strcpy (Str,  "CCEV_RESTART");
			break;
		case CCEV_CALLINFO      :
			strcpy (Str,  "CCEV_CALLINFO");
			break;
		case CCEV_REQANI        :
			strcpy (Str,  "CCEV_REQANI");
			break;
		case CCEV_SETCHANSTATE  :
			strcpy (Str,  "CCEV_SETCHANSTATE");
			break;
		case CCEV_FACILITY_ACK  :
			strcpy (Str,  "CCEV_FACILITY_ACK");
			break;
		case CCEV_FACILITY_REJ  :
			strcpy (Str,  "CCEV_FACILITY_REJ");
			break;
		case CCEV_MOREDIGITS    :
			strcpy (Str,  "CCEV_MOREDIGITS");
			break;
		case CCEV_SETBILLING    :
			strcpy (Str,  "CCEV_SETBILLING");
			break;
		case CCEV_ALERTING      :
			strcpy (Str,  "CCEV_ALERTING");
			break;
		case CCEV_CONNECTED     :
			strcpy (Str,  "CCEV_CONNECTED");
			break;
		case CCEV_ERROR         :
			strcpy (Str,  "CCEV_ERROR");
			break;
		case CCEV_OFFERED       :
			strcpy (Str,  "CCEV_OFFERED");
			break;
		case CCEV_DISCONNECTED  :
			strcpy (Str,  "CCEV_DISCONNECTED");
			break;
		case CCEV_PROCEEDING    :
			strcpy (Str,  "CCEV_PROCEEDING");
			break;

		case CCEV_USRINFO       :
			strcpy (Str,  "CCEV_USRINFO");
			break;
		case CCEV_FACILITY      :
			strcpy (Str,  "CCEV_FACILITY");
			break;
		case CCEV_CONGESTION    :
			strcpy (Str,  "CCEV_CONGESTION");
			break;
		case CCEV_D_CHAN_STATUS :
			strcpy (Str,  "CCEV_D_CHAN_STATUS");
			break;
		case CCEV_NOUSRINFOBUF  :
			strcpy (Str,  "CCEV_NOUSRINFOBUF");
			break;
		case CCEV_NOFACILITYBUF :
			strcpy (Str,  "CCEV_NOFACILITYBUF");
			break;
		case CCEV_BLOCKED       :
			strcpy (Str,  "CCEV_BLOCKED");
			break;
		case CCEV_UNBLOCKED     :
			strcpy (Str,  "CCEV_UNBLOCKED");
			break;
		case CCEV_ISDNMSG       :
			strcpy (Str,  "CCEV_ISDNMSG");
			break;
		case CCEV_NOTIFY        :
			strcpy (Str,  "CCEV_NOTIFY");
			break;
		case CCEV_L2FRAME       :
			strcpy (Str,  "CCEV_L2FRAME");
			break;
		case CCEV_L2BFFRFULL    :
			strcpy (Str,  "CCEV_L2BFFRFULL");
			break;
		case CCEV_L2NOBFFR      :
			strcpy (Str,  "CCEV_L2NOBFFR");
			break;

		case CCEV_SETUP_ACK:
			strcpy (Str,  "CCEV_SETUP_ACK");
			break;

		case CCEV_DIVERTED:
			strcpy (Str,  "CCEV_DIVERTED");
			break;
		case CCEV_HOLDCALL:
			strcpy (Str,  "CCEV_HOLDCALL");
			break;
		case CCEV_HOLDACK:
			strcpy (Str,  "CCEV_HOLDACK");
			break;
		case CCEV_HOLDREJ:
			strcpy (Str,  "CCEV_HOLDREJ");
			break;
		case CCEV_NSI:
			strcpy (Str,  "CCEV_NSI");
			break;
		case CCEV_RETRIEVECALL:
			strcpy (Str,  "CCEV_RETRIEVECALL");
			break;
		case CCEV_RETRIEVEACK:
			strcpy (Str,  "CCEV_RETRIEVEACK");
			break;
		case CCEV_RETRIEVEREJ:
			strcpy (Str,  "CCEV_RETRIEVEREJ");
			break;
		case CCEV_TRANSFERACK:
			strcpy (Str,  "CCEV_TRANSFERACK");
			break;
		case CCEV_TRANSFERREJ:
			strcpy (Str,  "CCEV_TRANSFERREJ");
			break;
		case CCEV_TRANSIT:
			strcpy (Str,  "CCEV_TRANSIT");
			break;
		case CCEV_RESTARTFAIL:
			strcpy (Str,  "CCEV_RESTARTFAIL");
			break;
		case CCEV_TERM_REGISTER :
			strcpy(Str,"CCEV_TERM_REGISTER");
			break;
		case CCEV_RCVTERMREG_ACK :
			strcpy(Str,"CCEV_RCVTERMREG_ACK");
			break;
		case CCEV_RCVTERMREG_NACK :
			strcpy(Str,"CCEV_RCVTERMREG_NACK");
			break;


		case CCEV_FACILITYNULL   :
			strcpy(Str,"NULL (Dummy) CRN Facility IE");
			break;
		case CCEV_INFOGLOBAL  :
			strcpy(Str,"GLOBAL CRN Information IE");
			break;
		case CCEV_NOTIFYGLOBAL  :
			strcpy(Str,"GLOBAL CRN Notify IE");
			break;
		case CCEV_FACILITYGLOBAL :
			strcpy(Str,"GLOBAL CRN Facility IE");
			break;
		case CCEV_DROPACK  :
			strcpy(Str,"DROP Request Acknowledgement message");
			break;

		//these events are from gclib.h
		case GCEV_ATTACH:
			strcpy(Str,"media device successfully attached");
			break;
		case GCEV_ATTACH_FAIL:
			strcpy(Str,"failed to attach media device");
			break;
		case GCEV_DETACH:
			strcpy(Str,"media device successfully detached");
			break;
		case GCEV_DETACH_FAIL:
			strcpy(Str,"failed to detach media device");
			break;
		case GCEV_MEDIA_REQ:
			strcpy(Str,"Remote end is requesting media channel");
			break;
		case GCEV_STOPMEDIA_REQ:
			strcpy(Str,"Remote end is requesting media streaming stop");
			break;
		case GCEV_MEDIA_ACCEPT:
			strcpy(Str,"Media channel established with peer");
			break;
		case GCEV_MEDIA_REJ:
			strcpy(Str,"Failed to established media channel with peer");
			break;
		case GCEV_OPENEX:
			strcpy(Str,"Device Opened successfully");
			break;
		case GCEV_OPENEX_FAIL:
			strcpy(Str,"Device failed to Open");
			break;

		case GCEV_CALLSTATUS :
			strcpy(Str,"End of call status");
			break;
		case GCEV_MEDIADETECTED :
			strcpy(Str,"CPA: Media detected");
			break;

		case GCEV_ACKCALL      :
			strcpy (Str,  "Termination event for callack");
			break;
		case GCEV_SETUPTRANSFER:
			strcpy(Str,"Ready for making consultation call");
			break;
		case GCEV_COMPLETETRANSFER:
			strcpy(Str,"Transfer completed successfully");
			break;
		case GCEV_SWAPHOLD: 
			strcpy(Str,"Call on hold swapped with active call");
			break;
		case GCEV_BLINDTRANSFER:
			strcpy(Str,"Call transferred to consultation call");
			break;
		case GCEV_LISTEN:
			strcpy(Str,"Channel (listen) connected to SCbus timeslot");
			break;
		case GCEV_UNLISTEN:
			strcpy(Str,"Channel (unlisten) disconnected from SCbus timeslot");
			break;
		case GCEV_DETECTED:
			strcpy(Str,"ISDN NOTIFYNULL message received/GC Incoming call detected");
			break;
		case GCEV_FATALERROR:
			strcpy(Str,"Fatal error has occurred");
			break;
		case GCEV_RELEASECALL :
			strcpy(Str,"Call released");
			break;
		case GCEV_RELEASECALL_FAIL :
			strcpy(Str,"Failed to release call");
			break;
		case GCEV_DIALTONE:
			strcpy(Str,"The call has transitioned to GCST_DialTone state");
			break;
		case GCEV_DIALING:
			strcpy(Str,"The call has transitioned to GCST_Dialing state");
			break;
		case GCEV_ALARM:
			strcpy(Str,"An alarm occurred");
			break;
		case GCEV_MOREINFO:
            strcpy(Str, "Status of information requested\received");
			break;
		case GCEV_SENDMOREINFO:
			strcpy(Str, "More information sent to the network");
			break;
		case GCEV_CALLPROC:
            strcpy(Str, "Call acknowledged to indicate that it is now proceeding");
			break;
		case GCEV_NODYNMEM:
			strcpy(Str,"No dynamic memory available");
			break;
		case GCEV_EXTENSION:
			strcpy(Str,"Extension event");
			break;
		case GCEV_EXTENSIONCMPLT:
			strcpy(Str,"Termination event for gc_Extension()");
			break;
		case GCEV_GETCONFIGDATA:
			strcpy(Str,"Configuration data successfully retrieved");
			break;
		case GCEV_GETCONFIGDATA_FAIL:
			strcpy(Str,"Failed to retrieve configuration data");
			break;
		case GCEV_SETCONFIGDATA:
			strcpy(Str,"Configuration data successfully set");
			break;
		case GCEV_SETCONFIGDATA_FAIL:
			strcpy(Str,"Failed to set configuration data");
			break;
		case GCEV_SERVICEREQ:
			strcpy(Str,"Service Request received");
			break;
		case GCEV_SERVICERESP:
			strcpy(Str,"Service Response received");
			break;
		case GCEV_SERVICERESPCMPLT:
			strcpy(Str,"Service Response sent");
			break;

		case GCEV_INVOKE_XFER_ACCEPTED:
			strcpy(Str,"Invoke transfer accepted by the remote party");
			break;
		case GCEV_INVOKE_XFER_REJ:
			strcpy(Str,"Invoke transfer rejected by the remote party");
			break;
		case GCEV_INVOKE_XFER:
			strcpy(Str,"Successful completion of invoke transfer");
			break;
		case GCEV_INVOKE_XFER_FAIL:
			strcpy(Str,"Failure in invoke transfer");
			break;
		case GCEV_REQ_XFER:
			strcpy(Str,"Receiving a call transfer request");
			break;
		case GCEV_ACCEPT_XFER:
			strcpy(Str,"Successfully accept the transfer request from remote party");
			break;
		case GCEV_ACCEPT_XFER_FAIL:
			strcpy(Str,"Failure to accept the transfer request from remote party");
			break;
		case GCEV_REJ_XFER:
			strcpy(Str,"Successfully reject the transfer request from remote party");
			break;
		case GCEV_REJ_XFER_FAIL:
			strcpy(Str,"Failure to reject the transfer request");
			break;
		case GCEV_XFER_CMPLT:
			strcpy(Str,"Successful completion of call transfer at the party receiving the request");
			break;
		case GCEV_XFER_FAIL:
			strcpy(Str,"Failure to reroute a transferred call");
			break;
		case GCEV_INIT_XFER:
			strcpy(Str,"Successful completion of transfer initiate");
			break;
		case GCEV_INIT_XFER_REJ:
			strcpy(Str,"Transfer initiate rejected by the remote party");
			break;
		case GCEV_INIT_XFER_FAIL:
			strcpy(Str,"Failure in transfer initiate");
			break;
		case GCEV_REQ_INIT_XFER:
			strcpy(Str,"Receiving a transfer initiate request");
			break;
		case GCEV_ACCEPT_INIT_XFER:
			strcpy(Str,"Successfully accept the transfer initiate request");
			break;
		case GCEV_ACCEPT_INIT_XFER_FAIL:
			strcpy(Str,"Failure to accept the transfer initiate request");
			break;
		case GCEV_REJ_INIT_XFER:
			strcpy(Str,"Successfully reject the transfer initiate request");
			break;
		case GCEV_REJ_INIT_XFER_FAIL:
			strcpy(Str,"Failure to reject the transfer initiate request");
			break;
		case GCEV_TIMEOUT:
			strcpy(Str,"Notification of generic time out");
			break;

		case GCEV_FACILITYREQ:
			strcpy(Str,"A facility request is made by CO");
			break;

		case GCEV_TRACEDATA:
			strcpy(Str,"Tracing Data");
			break;
/*
		//CNF EVENTS
		case CNFEV_OPEN:                    
		case CNFEV_OPEN_CONF  :
		case CNFEV_OPEN_PARTY :
		case CNFEV_ADD_PARTY:
		case CNFEV_REMOVE_PARTY:
		case CNFEV_GET_ATTRIBUTE:
		case CNFEV_SET_ATTRIBUTE:
		case CNFEV_ENABLE_EVENT:
		case CNFEV_DISABLE_EVENT:
		case CNFEV_GET_DTMF_CONTROL:
		case CNFEV_SET_DTMF_CONTROL:
		case CNFEV_GET_ACTIVE_TALKER :
		case CNFEV_GET_PARTY_LIST:
		case CNFEV_GET_DEVICE_COUNT:
		case CNFEV_CONF_OPENED:
		case CNFEV_CONF_CLOSED :
		case CNFEV_PARTY_ADDED:
		case CNFEV_PARTY_REMOVED:
		case CNFEV_DTMF_DETECTED:
		case CNFEV_ACTIVE_TALKER:
		case CNFEV_ERROR:
		case CNFEV_OPEN_FAIL  :
		case CNFEV_OPEN_CONF_FAIL :
		case CNFEV_OPEN_PARTY_FAIL:
		case CNFEV_ADD_PARTY_FAIL:
		case CNFEV_REMOVE_PARTY_FAIL :
		case CNFEV_GET_ATTRIBUTE_FAIL:
		case CNFEV_SET_ATTRIBUTE_FAIL:
		case CNFEV_ENABLE_EVENT_FAIL :
		case CNFEV_DISABLE_EVENT_FAIL:
		case CNFEV_GET_DTMF_CONTROL_FAIL:
		case CNFEV_SET_DTMF_CONTROL_FAIL :
		case CNFEV_GET_ACTIVE_TALKER_FAIL:
		case CNFEV_GET_PARTY_LIST_FAIL:
		case CNFEV_GET_DEVICE_COUNT_FAIL:
			strcpy (Str,  CnfEventToStr((unsigned int)lEvtType));
			break;
*/
		// Device management
		case DMEV_CONNECT:
			strcpy (Str,  "DMEV_CONNECT");
			break;
		case DMEV_CONNECT_FAIL:                      
			strcpy (Str,  "DMEV_CONNECT_FAIL");
			break;
		case DMEV_DISCONNECT:                        
			strcpy (Str,  "DMEV_DISCONNECT");
			break;
		case DMEV_DISCONNECT_FAIL:                   
			strcpy (Str,  "DMEV_DISCONNECT_FAIL");
			break;
		case DMEV_GET_RESOURCE_RESERVATIONINFO:      
			strcpy (Str,  "DMEV_GET_RESOURCE_RESERVATIONINFO");
			break;
		case DMEV_GET_RESOURCE_RESERVATIONINFO_FAIL: 
			strcpy (Str,  "DMEV_GET_RESOURCE_RESERVATIONINFO_FAIL");
			break;

		case DMEV_PORT_CONNECT:
			strcpy (Str,  "DMEV_PORT_CONNECT");
			break;
		case DMEV_PORT_CONNECT_FAIL:
			strcpy (Str,  "DMEV_PORT_CONNECT_FAIL");
			break;
			
		case DMEV_PORT_DISCONNECT:
			strcpy (Str,  "DMEV_PORT_DISCONNECT");
			break;
		case DMEV_PORT_DISCONNECT_FAIL:
			strcpy (Str,  "DMEV_PORT_DISCONNECT_FAIL");
			break;

		case DMEV_GET_TX_PORT_INFO:
			strcpy (Str,  "DMEV_GET_TX_PORT_INFO");
			break;
		case DMEV_GET_TX_PORT_INFO_FAIL:
			strcpy (Str,  "DMEV_GET_TX_PORT_INFO_FAIL");
			break;
		case DMEV_GET_RX_PORT_INFO:
			strcpy (Str,  "DMEV_GET_RX_PORT_INFO");
			break;
		case DMEV_GET_RX_PORT_INFO_FAIL:
			strcpy (Str,  "DMEV_GET_RX_PORT_INFO_FAIL");
			break;

			// ipm events
		case IPMEV_OPEN:
			strcpy(Str, "IPMEV_OPEN");
			break;
		case IPMEV_STARTMEDIA:
			strcpy(Str, "IPMEV_STARTMEDIA");
			break;
		case IPMEV_STOPPED:
			strcpy(Str, "IPMEV_STOPPED");
			break;
		case IPMEV_GET_LOCAL_MEDIA_INFO:
			strcpy(Str, "IPMEV_GET_LOCAL_MEDIA_INFO");
			break;
		case IPMEV_GET_SESSION_INFO:
			strcpy(Str, "IPMEV_GET_SESSION_INFO");
			break;
		case IPMEV_LISTEN:
			strcpy(Str, "IPMEV_LISTEN");
			break;
		case IPMEV_UNLISTEN:
			strcpy(Str, "IPMEV_UNLISTEN");
			break;
		case IPMEV_GET_XMITTS_INFO:
			strcpy(Str, "IPMEV_GET_XMITTS_INFO");
			break;
		case IPMEV_SEND_DIGITS:
			strcpy(Str, "IPMEV_SEND_DIGITS");
			break;
		case IPMEV_RECEIVE_DIGITS:
			strcpy(Str, "IPMEV_RECEIVE_DIGITS");
			break;
		case IPMEV_DIGITS_RECEIVED:
			strcpy(Str, "IPMEV_DIGITS_RECEIVED");
			break;
		case IPMEV_EVENT_ENABLED:
			strcpy(Str, "IPMEV_EVENT_ENABLED");
			break;
		case IPMEV_EVENT_DISABLED:
			strcpy(Str, "IPMEV_EVENT_DISABLED");
			break;
		case IPMEV_GET_QOS_ALARM_STATUS:
			strcpy(Str, "IPMEV_GET_QOS_ALARM_STATUS");
			break;
		case IPMEV_RESET_QOS_ALARM_STATUS:
			strcpy(Str, "IPMEV_RESET_QOS_ALARM_STATUS");
			break;
		case IPMEV_SET_QOS_THRESHOLD_INFO:
			strcpy(Str, "IPMEV_SET_QOS_THRESHOLD_INFO");
			break;
		case IPMEV_GET_QOS_THRESHOLD_INFO:
			strcpy(Str, "IPMEV_GET_QOS_THRESHOLD_INFO");
			break;
		case IPMEV_QOS_ALARM:
			strcpy(Str, "IPMEV_QOS_ALARM");
			break;
		case IPMEV_SET_PARM:
			strcpy(Str, "IPMEV_SET_PARM");
			break;
		case IPMEV_GET_PARM:
			strcpy(Str, "IPMEV_GET_PARM");
			break;
		case IPMEV_PING:
			strcpy(Str, "IPMEV_PING");
			break;
		case IPMEV_SEND_SIGNAL_TO_IP:
			strcpy(Str, "IPMEV_SEND_SIGNAL_TO_IP");
			break;
		case IPMEV_TELEPHONY_EVENT:// RFC2833
			strcpy(Str, "IPMEV_TELEPHONY_EVENT");
			break;
		case IPMEV_FAXTONE:
			strcpy(Str, "IPMEV_FAXTONE");
			break;
		case IPMEV_T38CALLSTATE:
			strcpy(Str, "IPMEV_T38CALLSTATE");
			break;
		case IPMEV_T38CAPFRAME_STATUS:
			strcpy(Str, "IPMEV_T38CAPFRAME_STATUS");
			break;
		case IPMEV_T38INFOFRAME_STATUS:
			strcpy(Str, "IPMEV_T38INFOFRAME_STATUS");
			break;
		case IPMEV_T38HDLCFRAME_STATUS:
			strcpy(Str, "IPMEV_T38HDLCFRAME_STATUS");
			break;
		case IPMEV_T38POSTPGCODE_STATUS:
			strcpy(Str, "IPMEV_T38POSTPGCODE_STATUS");
			break;
		case IPMEV_ERROR:// for compatibility
			strcpy(Str, "IPMEV_ERROR");
			break;
		case IPMEV_T38POSTPGRESP_STATUS:
			strcpy(Str, "IPMEV_T38POSTPGRESP_STATUS");
			break;
		case IPMEV_GET_CTINFO:
			strcpy(Str, "IPMEV_GET_CTINFO");
			break;
		case IPMEV_MODIFYMEDIA:
			strcpy(Str, "IPMEV_MODIFYMEDIA");
			break;
		case IPMEV_GET_CAPABILITIES:
			strcpy(Str, "IPMEV_GET_CAPABILITIES");
			break;
		case IPMEV_SEC_NOTIFY_EXPIRE_KEY_AUDIO:
			strcpy(Str, "IPMEV_SEC_NOTIFY_EXPIRE_KEY_AUDIO");
			break;
		case IPMEV_SEC_NOTIFY_EXPIRE_KEY_VIDEO:
			strcpy(Str, "IPMEV_SEC_NOTIFY_EXPIRE_KEY_VIDEO");
			break;
		case IPMEV_GENERATEIFRAME:
			strcpy(Str, "IPMEV_GENERATEIFRAME");
			break;
		case IPMEV_MODIFYMEDIA_FAIL:
			strcpy(Str, "IPMEV_MODIFYMEDIA_FAIL");
			break;
		case IPMEV_GET_CAPABILITIES_FAIL:
			strcpy(Str, "IPMEV_GET_CAPABILITIES_FAIL");
			break;
		case IPMEV_GENERATEIFRAME_FAIL:
			strcpy(Str, "IPMEV_GENERATEIFRAME_FAIL");
			break;
		case IPMEV_GET_SESSION_INFOEX:
			strcpy(Str, "IPMEV_GET_SESSION_INFOEX");
			break;
		case IPMEV_RTCP_NOTIFY_RECEIVED:
			strcpy(Str, "IPMEV_RTCP_NOTIFY_RECEIVED");
			break;
		case IPMEV_RTCP_NOTIFY_SENT:
			strcpy(Str, "IPMEV_RTCP_NOTIFY_SENT");
			break;
		case IPMEV_NOTIFY_ENDPOINTID:
			strcpy(Str, "IPMEV_NOTIFY_ENDPOINTID");
			break;
		case IPMEV_SEND_STUN_MESSAGE:
			strcpy(Str, "IPMEV_SEND_STUN_MESSAGE");
			break;
		case IPMEV_SEND_STUN_MESSAGE_FAIL:
			strcpy(Str, "IPMEV_SEND_STUN_MESSAGE_FAIL");
			break;
		case IPMEV_STUN_MESSAGE_RECEIVED:
			strcpy(Str, "IPMEV_STUN_MESSAGE_RECEIVED");
			break;
		case IPMEV_PAYLOAD_TYPE_UPDATE:
			strcpy(Str, "IPMEV_PAYLOAD_TYPE_UPDATE");
			break;
		case IPMEV_GET_FINGERPRINT:
			strcpy(Str, "IPMEV_GET_FINGERPRINT");
			break;
		case IPMEV_GET_FINGERPRINT_FAIL:
			strcpy(Str, "IPMEV_GET_FINGERPRINT_FAIL");
			break;
		case IPMEV_SEND_TELEPHONY_SIGNALS:
			strcpy(Str, "IPMEV_SEND_TELEPHONY_SIGNALS");
			break;
		case IPMEV_SEND_TELEPHONY_SIGNALS_FAIL:
			strcpy(Str, "IPMEV_SEND_TELEPHONY_SIGNALS_FAIL");
			break;
		case IPMEV_PARSE_OVERLAY_XML_FILE:
			strcpy(Str, "IPMEV_PARSE_OVERLAY_XML_FILE");
			break;
		case IPMEV_PARSE_OVERLAY_XML_FILE_FAIL:
			strcpy(Str, "IPMEV_PARSE_OVERLAY_XML_FILE_FAIL");
			break;
		case IPMEV_OVERLAY_CONTENT_EXPIRED:
			strcpy(Str, "IPMEV_OVERLAY_CONTENT_EXPIRED");
			break;

		// MM events

		case  MMEV_OPEN:
			strcpy (Str,  "MMEV_OPEN");
			break;
		case  MMEV_PLAY_ACK:
			strcpy (Str,  "MMEV_PLAY_ACK");
			break;
		case  MMEV_RECORD_ACK:
			strcpy (Str,  "MMEV_RECORD_ACK");
			break;
		case  MMEV_STOP_ACK:
			strcpy (Str,  "MMEV_STOP_ACK");
			break;
		case  MMEV_RESET_ACK:
			strcpy (Str,  "MMEV_RESET_ACK");
			break;
		case  MMEV_ENABLEEVENTS:
			strcpy (Str,  "MMEV_ENABLEEVENTS");
			break;
		case  MMEV_DISABLEEVENTS:
			strcpy (Str,  "MMEV_DISABLEEVENTS");
			break;
		case  MMEV_PLAY:
			strcpy (Str,  "MMEV_PLAY");
			break;
		case  MMEV_RECORD:
			strcpy (Str,  "MMEV_RECORD");
			break;
		case  MMEV_RESET:
			strcpy (Str,  "MMEV_RESET");
			break;
		case  MMEV_GET_CHAN_STATE:
			strcpy (Str,  "MMEV_GET_CHAN_STATE");
			break;
		case  MMEV_SETPARM:
			strcpy (Str,  "MMEV_SETPARM");
			break;
		case  MMEV_GETPARM:
			strcpy (Str,  "MMEV_GETPARM");
			break;
		case  MMEV_PLAY_VIDEO_LOWWATER:
			strcpy (Str,  "MMEV_PLAY_VIDEO_LOWWATER");
			break;
		case  MMEV_PLAY_VIDEO_HIGHWATER:
			strcpy (Str,  "MMEV_PLAY_VIDEO_HIGHWATER");
			break;
		case  MMEV_PLAY_AUDIO_LOWWATER:
			strcpy (Str,  "MMEV_PLAY_AUDIO_LOWWATER");
			break;
		case  MMEV_PLAY_AUDIO_HIGHWATER:
			strcpy (Str,  "MMEV_PLAY_AUDIO_HIGHWATER");
			break;
		case  MMEV_RECORD_VIDEO_LOWWATER:
			strcpy (Str,  "MMEV_RECORD_VIDEO_LOWWATER");
			break;
		case  MMEV_RECORD_VIDEO_HIGHWATER:
			strcpy (Str,  "MMEV_RECORD_VIDEO_HIGHWATER");
			break;
		case  MMEV_RECORD_AUDIO_LOWWATER:
			strcpy (Str,  "MMEV_RECORD_AUDIO_LOWWATER");
			break;
		case  MMEV_RECORD_AUDIO_HIGHWATER:
			strcpy (Str,  "MMEV_RECORD_AUDIO_HIGHWATER");
			break;
		case  MMEV_PAUSE:
			strcpy (Str,  "MMEV_PAUSE");
			break;
		case  MMEV_RESUME:
			strcpy (Str,  "MMEV_RESUME");
			break;
		case  MMEV_SEEK:
			strcpy (Str,  "MMEV_SEEK");
			break;
		case  MMEV_GETDURATION:
			strcpy (Str,  "MMEV_GETDURATION");
			break;
		case  MMEV_VIDEO_RECORD_STARTED:
			strcpy (Str,  "MMEV_VIDEO_RECORD_STARTED");
			break;
		case  MMEV_LAST_EVENT:
			strcpy (Str,  "MMEV_LAST_EVENT");
			break;
		case  MMEV_ERROR:
			strcpy (Str,  "MMEV_ERROR");
			break;
		case  MMEV_OPEN_FAIL:
			strcpy (Str,  "MMEV_OPEN_FAIL");
			break;
		case  MMEV_PLAY_ACK_FAIL:
			strcpy (Str,  "MMEV_PLAY_ACK_FAIL");
			break;
		case  MMEV_RECORD_ACK_FAIL:
			strcpy (Str,  "MMEV_RECORD_ACK_FAIL");
			break;
		case  MMEV_STOP_ACK_FAIL:
			strcpy (Str,  "MMEV_STOP_ACK_FAIL");
			break;
		case  MMEV_RESET_ACK_FAIL:
			strcpy (Str,  "MMEV_RESET_ACK_FAIL");
			break;
		case  MMEV_ENABLEEVENTS_FAIL:
			strcpy (Str,  "MMEV_ENABLEEVENTS_FAIL");
			break;
		case  MMEV_DISABLEEVENTS_FAIL:
			strcpy (Str,  "MMEV_DISABLEEVENTS_FAIL");
			break;
		case  MMEV_PLAY_FAIL:
			strcpy (Str,  "MMEV_PLAY_FAIL");
			break;
		case  MMEV_RECORD_FAIL:
			strcpy (Str,  "MMEV_RECORD_FAIL");
			break;
		case  MMEV_RESET_FAIL:
			strcpy (Str,  "MMEV_RESET_FAIL");
			break;
		case  MMEV_GET_CHAN_STATE_FAIL:
			strcpy (Str,  "MMEV_GET_CHAN_STATE_FAIL");
			break;
		case  MMEV_SETPARM_FAIL:
			strcpy (Str,  "MMEV_SETPARM_FAIL");
			break;
		case  MMEV_GETPARM_FAIL :
			strcpy (Str,  "MMEV_GETPARM_FAIL");
			break;

		case  MMEV_PAUSE_FAIL :
			strcpy (Str,  "MMEV_PAUSE_FAIL");
			break;
		case  MMEV_RESUME_FAIL :
			strcpy (Str,  "MMEV_RESUME_FAIL");
			break;
		case  MMEV_SEEK_FAIL :
			strcpy (Str,  "MMEV_SEEK_FAIL");
			break;

		case  MMEV_GETDURATION_FAIL :
			strcpy (Str,  "MMEV_GETDURATION_FAIL");
			break;

		case  MMEV_VIDEO_RECORD_STARTED_FAIL:
			strcpy (Str,  "MMEV_VIDEO_RECORD_STARTED_FAIL");
			break;
		case  MMEV_LAST_EVENT_ERROR :
			strcpy (Str,  "MMEV_LAST_EVENT_ERROR");
			break;

		default:
			sprintf(Str, "Unknown Event (0x%x)",lEvtType);
			return -1;
			break;
		}	// switch

		return 0;
}
int Str2LogLevel( char *str){

	if(strcmp("ERROR",str) == 0)	return(ERROR);
	else if(strcmp("EVENT",str) == 0)	return(EVENT);
	else if(strcmp("API",str) == 0)	return(API);
	else if(strcmp("INFO",str) == 0)	return(INFO);
	else if(strcmp("ENTRY",str) == 0)	return(ENTRY);
	else if(strcmp("EXIT",str) == 0)	return(EXIT);
	else if(strcmp("DEBUG",str) == 0)	return(DEBUG);
	else if(strcmp("ALL",str) == 0)	return(LOGALL);

	return(LOGALL);
	
}
char *LogLevel2Str(int level){

switch(level){
	case ERROR:
		return("[ERROR]");
		break;
	case EVENT:
		return("[EVENT]");
		break;
	case API:
		return("[ API ]");
		break;
	case INFO:
		return("[INFO ]");
		break;
	case ENTRY:
		return("[ENTRY]");
		break;
	case EXIT:
		return("[EXIT ]");
		break;
	case DEBUG:
		return("[DEBUG]");
		break;
	case LOGALL:
		return("[ ALL ]");
		break;
	default:
		return("[     ]");
		break;

	}

}


class CFuncLogger  
{
public:
	CFuncLogger(char *funcname,int index);
	virtual ~CFuncLogger();

private:
	char mFuncName[MAX_LOG_STR_SIZE];
	int mIndex;
	DWORD mEnterTime;
};


CFuncLogger::CFuncLogger(char *funcname="FUNCTION",int index=NOINDEX):
mIndex(index),
mEnterTime(0)
{
	mEnterTime=GetTickCount();
	strcpy(mFuncName,funcname);
	Log(ENTRY,mIndex,"Entering %s",mFuncName);
}

CFuncLogger::~CFuncLogger()
{
	Log(EXIT,mIndex,"Exiting %s, function time=%d",mFuncName,GetTickCount()-mEnterTime);

}