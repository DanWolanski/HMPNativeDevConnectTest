// portConnectTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#include "srllib.h"
#include "dtilib.h" 
#include "gclib.h"
#include "gcerr.h"
#include "gcip.h"
#include "cnflib.h"
#include "devmgmt.h"
#include "ipmlib.h"
#include "cclib.h"
#include "mmlib.h"
#include "port_connect.h"

#include "Log.h"

#define MAX_IPMDEVCOUNT	250
#define MAX_MMDEVCOUNT	250
#define MAX_VOXDEVCOUNT	250

#define DEFAULTDEVCOUNT 10
int IPMDEVCOUNT = DEFAULTDEVCOUNT;
int MMDEVCOUNT = DEFAULTDEVCOUNT;
int VOXDEVCOUNT = DEFAULTDEVCOUNT;

int	ipmh[MAX_IPMDEVCOUNT];
int	ipmtxslot[MAX_IPMDEVCOUNT];
int	mmh[MAX_MMDEVCOUNT];
int	voxh[MAX_VOXDEVCOUNT];
int	voxtxslot[MAX_VOXDEVCOUNT];

#define DIALINDEX 0
#define ININDEX 1
#define OUTINDEX 2
#define DETECTINDEX 3

#define MULAW 0
#define AMRNB 1
#define AMRWB 2

#define DEFAULTRFCPTYPE 101
int audiocodec=MULAW;
int defaultrfcptype=DEFAULTRFCPTYPE;
char ipaddr[32] = "127.0.0.1";

DM_PORT_INFO_LIST ipmTxPortInfo[MAX_IPMDEVCOUNT];
DM_PORT_INFO_LIST mmTxPortInfo[MAX_MMDEVCOUNT];

DM_PORT_INFO_LIST ipmRxPortInfo[MAX_IPMDEVCOUNT];
DM_PORT_INFO_LIST mmRxPortInfo[MAX_MMDEVCOUNT];

DM_PORT_CONNECT_INFO_LIST mmConnectList[MAX_MMDEVCOUNT];
DM_PORT_CONNECT_INFO_LIST ipmConnectList[MAX_IPMDEVCOUNT];


int keeplooping=1;

#define CUSTOMEV_TEST_READY 0x7777

int sleeptime = 0;


void StartMedia(int index, char *RemoteIPAddr, unsigned int RemoteAudioPort, unsigned int RemoteVideoPort=0)
{
	LOG_ENTRYEXIT("StartMedia");
	
	// First, set up the IP Media Info struct using the Media info parsed from the incoming SDP
	// Codecs would also be set according to parsed info, but we're simplifying to just use G.711
	IPM_MEDIA_INFO MediaInfo;
	memset(&MediaInfo, 0, sizeof(IPM_MEDIA_INFO));
	int miCnt = 0;

	MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_AUDIO_REMOTE_RTP_INFO;
	MediaInfo.MediaData[miCnt].mediaInfo.PortInfo.unPortId = RemoteAudioPort;
	strcpy(MediaInfo.MediaData[miCnt].mediaInfo.PortInfo.cIPAddress, RemoteIPAddr);
	Log(INFO, index, "MediaInfo[%d] - REMOTE Audio RTP to %s:%d", miCnt, RemoteIPAddr, RemoteAudioPort);
	miCnt++;

	MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_AUDIO_REMOTE_RTCP_INFO;
	MediaInfo.MediaData[miCnt].mediaInfo.PortInfo.unPortId = RemoteAudioPort + 1;
	strcpy(MediaInfo.MediaData[miCnt].mediaInfo.PortInfo.cIPAddress, RemoteIPAddr);
	Log(INFO, index, "MediaInfo[%d] - REMOTE Audio RTCP to %s:%d", miCnt, RemoteIPAddr, RemoteAudioPort + 1);
	miCnt++;
	
	if (audiocodec == AMRNB){
	MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_AUDIO_REMOTE_CODER_INFO;
	MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.eCoderType = CODER_TYPE_AMRWB_23_85K;
	MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.eFrameSize = CODER_FRAMESIZE_20;
	MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.unFramesPerPkt = 1;
	MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.eVadEnable = CODER_VAD_ENABLE;
	MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.unCoderPayloadType = 97;
	MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.unRedPayloadType = 0;
	Log(INFO, index, "MediaInfo[%d] - REMOTE Audio CODER to CODER_TYPE_AMRWB_23_85K 20ms, Payload type of 97", miCnt);
	miCnt++;

	MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_AUDIO_LOCAL_CODER_INFO;
	MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.eCoderType = CODER_TYPE_AMRWB_23_85K;
	MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.eFrameSize = CODER_FRAMESIZE_20;
	MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.unFramesPerPkt = 1;
	MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.eVadEnable = CODER_VAD_ENABLE;
	MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.unCoderPayloadType = 97;
	MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.unRedPayloadType = 0;
	Log(INFO, index, "MediaInfo[%d] - LOCAL Audio CODER to CODER_TYPE_AMRWB_23_85K 20ms, Payload type of 97", miCnt);
	miCnt++;
	
	MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_AUDIO_LOCAL_CODER_OPTIONS_INFO;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderOptionsInfo.unVersion =
		IPM_AUDIO_CODER_OPTIONS_INFO_VERSION;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderOptionsInfo.unCoderOptions =
		 CODER_OPT_AMR_EFFICIENT;
	Log(INFO, index, "MediaInfo[%d] - LOCAL Audio CODER to CODER_OPT_AMR_EFFICIENT", miCnt);
	miCnt++;
	MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_AUDIO_REMOTE_CODER_OPTIONS_INFO;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderOptionsInfo.unVersion =
		IPM_AUDIO_CODER_OPTIONS_INFO_VERSION;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderOptionsInfo.unCoderOptions =
		CODER_OPT_AMR_CMR_TRACK | CODER_OPT_AMR_EFFICIENT;
	Log(INFO, index, "MediaInfo[%d] - REMOTE Audio CODER to CODER_OPT_AMR_EFFICIENT", miCnt);
	
	miCnt++;
	}
	else if (audiocodec == AMRNB){
	// Local Audio Coder
	MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_AUDIO_LOCAL_CODER_INFO;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderInfo.eCoderType = CODER_TYPE_AMRNB_7_95k;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderInfo.eFrameSize = CODER_FRAMESIZE_20;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderInfo.unFramesPerPkt = 1;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderInfo.eVadEnable = CODER_VAD_ENABLE;
		MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderInfo.unCoderPayloadType = 96;
		MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderInfo.unRedPayloadType = 0;
		Log(INFO, index, "MediaInfo[%d] - LOCAL Audio CODER to CODER_TYPE_AMRNB_NONE 20ms, Payload type of 96", miCnt);
		miCnt++;
	// Remote Audio Coder
	MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_AUDIO_REMOTE_CODER_INFO;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderInfo.eCoderType = CODER_TYPE_AMRNB_7_95k;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderInfo.eFrameSize = CODER_FRAMESIZE_20;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderInfo.unFramesPerPkt = 1;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderInfo.eVadEnable = CODER_VAD_ENABLE;
		MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderInfo.unCoderPayloadType = 96;
		MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderInfo.unRedPayloadType = 0;
		Log(INFO, index, "MediaInfo[%d] - REMOTE Audio CODER to CODER_TYPE_AMRNB_NONE 20ms, Payload type of 96", miCnt);
		miCnt++;
	MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_AUDIO_LOCAL_CODER_OPTIONS_INFO;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderOptionsInfo.unVersion =
		IPM_AUDIO_CODER_OPTIONS_INFO_VERSION;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderOptionsInfo.unCoderOptions =
		CODER_OPT_AMR_EFFICIENT;
	miCnt++;
	MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_AUDIO_REMOTE_CODER_OPTIONS_INFO;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderOptionsInfo.unVersion =
		IPM_AUDIO_CODER_OPTIONS_INFO_VERSION;
	MediaInfo.MediaData[miCnt].mediaInfo.AudioCoderOptionsInfo.unCoderOptions =
		CODER_OPT_AMR_CMR_TRACK | CODER_OPT_AMR_EFFICIENT;
	miCnt++;
	}
	else{
		MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_AUDIO_REMOTE_CODER_INFO;
		MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.eCoderType = CODER_TYPE_G711ULAW64K;
		MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.eFrameSize = CODER_FRAMESIZE_20;
		MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.unFramesPerPkt = 1;
		MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.eVadEnable = CODER_VAD_DISABLE;
		MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.unCoderPayloadType = 0;
		MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.unRedPayloadType = 0;

		Log(INFO, index, "MediaInfo[%d] - REMOTE Audio CODER to CODER_TYPE_G711ULAW64K 20ms, Payload type of 0", miCnt);
		miCnt++;

		MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_AUDIO_LOCAL_CODER_INFO;
		MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.eCoderType = CODER_TYPE_G711ULAW64K;
		MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.eFrameSize = CODER_FRAMESIZE_20;
		MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.unFramesPerPkt = 1;
		MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.eVadEnable = CODER_VAD_ENABLE;
		MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.unCoderPayloadType = 0;
		MediaInfo.MediaData[miCnt].mediaInfo.CoderInfo.unRedPayloadType = 0;
		Log(INFO, index, "MediaInfo[%d] - LOCAL Audio CODER to CODER_TYPE_G711ULAW64K 20ms, Payload type of 0", miCnt);
		miCnt++;
	} 
	if (RemoteVideoPort) {
		// remote video RTP port 
		MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_VIDEO_REMOTE_RTP_INFO;
		MediaInfo.MediaData[miCnt].mediaInfo.PortInfo.unPortId = RemoteVideoPort;
		strcpy(MediaInfo.MediaData[miCnt].mediaInfo.PortInfo.cIPAddress, RemoteIPAddr);
		Log(INFO, index, "MediaInfo[%d] - REMOTE Video RTP to %s:%d", miCnt, RemoteIPAddr, RemoteVideoPort);
		miCnt++;

		// remote video RTCP port
		MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_VIDEO_REMOTE_RTCP_INFO;
		MediaInfo.MediaData[miCnt].mediaInfo.PortInfo.unPortId = RemoteVideoPort + 1;
		strcpy(MediaInfo.MediaData[miCnt].mediaInfo.PortInfo.cIPAddress, RemoteIPAddr);
		Log(INFO, index, "MediaInfo[%d] - REMOTE Video RTCP to %s:%d", miCnt, RemoteIPAddr, RemoteVideoPort + 1);
		miCnt++;

		// remote video codec
		MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_VIDEO_REMOTE_CODER_INFO;
		MediaInfo.MediaData[miCnt].mediaInfo.VideoCoderInfo.eCoderType = CODER_TYPE_H264;
		MediaInfo.MediaData[miCnt].mediaInfo.VideoCoderInfo.unVersion = 0;
		MediaInfo.MediaData[miCnt].mediaInfo.VideoCoderInfo.unCoderPayloadType = 96;
		IPM_VIDEO_CODER_INFO_EX local_codeex;
		INIT_IPM_VIDEO_CODER_INFO_EX(&local_codeex);
		local_codeex.eProfile = VIDEO_PROFILE_BASELINE_H264;
		local_codeex.eLevel = VIDEO_LEVEL_DEFAULT;
		local_codeex.unBitRate = EMM_VIDEO_BITRATE_DEFAULT;
		local_codeex.eFramesPerSec = VIDEO_FRAMESPERSEC_DEFAULT;
		local_codeex.eSamplingRate = VIDEO_SAMPLING_RATE_90000;
		MediaInfo.MediaData[miCnt].mediaInfo.VideoCoderInfo.pExtraCoderInfo = &local_codeex;
		Log(INFO, index, "MediaInfo[%d] - REMOTE Video Coder H264 payload 96", miCnt);
		miCnt++;

		// local video codec
		MediaInfo.MediaData[miCnt].eMediaType = MEDIATYPE_VIDEO_LOCAL_CODER_INFO;
		MediaInfo.MediaData[miCnt].mediaInfo.VideoCoderInfo.eCoderType = CODER_TYPE_H264;
		MediaInfo.MediaData[miCnt].mediaInfo.VideoCoderInfo.unVersion = 0;
		MediaInfo.MediaData[miCnt].mediaInfo.VideoCoderInfo.unCoderPayloadType = 96;
		IPM_VIDEO_CODER_INFO_EX remote_codeex;
		INIT_IPM_VIDEO_CODER_INFO_EX(&remote_codeex);
		remote_codeex.eProfile = VIDEO_PROFILE_BASELINE_H264;
		remote_codeex.eLevel = VIDEO_LEVEL_DEFAULT;
		remote_codeex.unBitRate = EMM_VIDEO_BITRATE_DEFAULT;
		remote_codeex.eFramesPerSec = VIDEO_FRAMESPERSEC_DEFAULT;
		remote_codeex.eSamplingRate = VIDEO_SAMPLING_RATE_90000;
		MediaInfo.MediaData[miCnt].mediaInfo.VideoCoderInfo.pExtraCoderInfo = &remote_codeex;
		Log(INFO, index, "MediaInfo[%d] - LOCAL Video Coder H264 payload 96", miCnt);
		miCnt++;
	}
	MediaInfo.unCount = miCnt;


	Log(DEBUG, index, "Final size of MediaInfo is %d", miCnt);
	//Log(INFO, index, "Starting Media.  Remote ip=%s, audioport=%d,videoport=%d", RemoteIPAddr, RemoteAudioPort, RemoteVideoPort);

	// Strictly speaking, this ought to be done async, but this simplifies the demo
	if (ipm_StartMedia(ipmh[index], &MediaInfo, DATA_IP_TDM_BIDIRECTIONAL, EV_ASYNC) != 0) {
		Log(ERROR, index, "ipm_StartMedia() on device %d Failed - %s", ipmh[index], ATDV_ERRMSGP(ipmh[index]));
	}
	else {
		//        mediaStreaming = true;
		Log(INFO, index, "********* ipm_StartMedia success (devh=%d,str=%s)******** ", ipmh[index], ATDV_NAMEP(ipmh[index]));
	}
}
void SetRFC2833Parms(int index, unsigned long ptype) {
	LOG_ENTRYEXIT("SetRFC2833Parms");

	IPM_PARM_INFO ParmInfo;
	unsigned long ulParmValue = DTMFXFERMODE_RFC2833;
	ParmInfo.eParm = PARMCH_DTMFXFERMODE;
	ParmInfo.pvParmValue = &ulParmValue;
	Log(INFO, index, "Enabling RFC2833 Digits on Device");
	if (ipm_SetParm(ipmh[index], &ParmInfo, EV_SYNC) == -1)
	{
		Log(INFO, index, "ipm_SetParm failed for device name %s with error = %d\n",
			ATDV_NAMEP(ipmh[index]), ATDV_LASTERR(ipmh[index]));
	}
	ulParmValue = ptype;
	ParmInfo.eParm = PARMCH_RFC2833EVT_TX_PLT;
	ParmInfo.pvParmValue = &ulParmValue;
	Log(INFO, index, "Enabling RFC2833 TX Payload to %d",ptype);
	if (ipm_SetParm(ipmh[index], &ParmInfo, EV_SYNC) == -1)
	{
		Log(INFO, index, "ipm_SetParm failed for device name %s with error = %d\n",
			ATDV_NAMEP(ipmh[index]), ATDV_LASTERR(ipmh[index]));
	}
	ulParmValue = ptype;
	ParmInfo.eParm = PARMCH_RFC2833EVT_RX_PLT;
	ParmInfo.pvParmValue = &ulParmValue;
	Log(INFO, index, "Enabling RFC2833 RX Payload to %d",ptype);
	if (ipm_SetParm(ipmh[index], &ParmInfo, EV_SYNC) == -1)
	{
		Log(INFO, index, "ipm_SetParm failed for device name %s with error = %d\n",
			ATDV_NAMEP(ipmh[index]), ATDV_LASTERR(ipmh[index]));
	}
}
void OpenIpm(int index){
	LOG_ENTRYEXIT("OpenIpm");
	char ipmname[20];

	sprintf(ipmname,"ipmB1C%d",index+1);
	Log(INFO,index,"********* ipm_Open ******** Opening %s...",ipmname);
	ipmh[index]=ipm_Open(ipmname,NULL,EV_SYNC);
	if(ipmh[index] == -1){
		printf("Error in opening %s",ipmname);
		keeplooping=0;
	}
	Log(INFO,index,"********* IPM device %s Open *********, ipmh[%d]=%d",ipmname,index,ipmh[index]);
	SetRFC2833Parms(index, defaultrfcptype);
	if (ipm_GetXmitSlot(ipmh[index], NULL, EV_ASYNC) == -1) {
		Log(INFO, index, "ipm_GetXmitSlot failed for device name %s with error = %d\n",
			ATDV_NAMEP(ipmh[index]), ATDV_LASTERR(ipmh[index]));
	}
}

void CloseIpm(int index){
	LOG_ENTRYEXIT("CloseIpm");

	Log(INFO,index, "Stopping IPM");
	ipm_Stop(ipmh[index], STOP_ALL, EV_SYNC);

	Log(INFO,index,"Closing %s (ipmh[%d]=%d)",ATDV_NAMEP(ipmh[index]),index,ipmh[index]);
	ipm_Close(ipmh[index],NULL);

}

void OpenMM(int index){
	

	char name[20];

	sprintf(name,"mmB1C%d",index+1);
	Log(INFO,index,"Opening %s...",name);
	mmh[index]=mm_Open(name,NULL,NULL);
	if(mmh[index] == -1){
		Log(ERROR,index,"Error in opening %s",name);
		keeplooping=0;
	}
	Log(INFO,index,"MM %s device Open, mmh[%d]=%d",name,index,mmh[index]);
	
}
void CloseMM(int num) {

	Log(INFO, index, "Closing %s (mmh[%d]=%d)", ATDV_NAMEP(mmh[num]), num, mmh[num]);



}
void Dial(int index,char * dialstr) {
	LOG_ENTRYEXIT("Dial");
	Log(INFO, index, "Dialing %s", dialstr);
	CRC(dx_dial(voxh[index], dialstr, NULL, EV_ASYNC), "Dialing 1234567890");
}
void OpenVox(int index) {
	LOG_ENTRYEXIT("OpenVox");

	char name[20];

	sprintf(name, "dxxxB%dC%d", (index/4) + 1,(index%4)+1);
	Log(INFO, index, "Opening %s...", name);
	voxh[index] = dx_open(name, NULL);
	if (voxh[index] == -1) {
		Log(ERROR, index, "Error in opening %s", name);
		keeplooping = 0;
	}
	Log(INFO, index, "VOX %s device Open, voxh[%d]=%d", name, index, voxh[index]);
	Log(INFO, index, "Setting Event Mask to DM_DIGITS");
	CRC( dx_setevtmsk(voxh[index], DM_DIGITS) ,"dx_setevtmsk(DM_DIGITS)") {
		Log(ERROR, index, "Error in dx_setevtmsk");
	}
	dx_stopch(voxh[index], EV_SYNC);
	dx_clrdigbuf(voxh[index]);
}
void CloseVox(int num){
	LOG_ENTRYEXIT("CloseVox");
	Log(INFO,index,"Closing %s (voxh[%d]=%d)",ATDV_NAMEP(voxh[num]),num,voxh[num]);
	dx_close(voxh[num]);
	

}

int FindIndexByHandle( int hDev){

	for(int i=0;i<IPMDEVCOUNT;i++){
		if(hDev == ipmh[i]) return i;
	}
	for(int j=0;j<MMDEVCOUNT;j++){
		if(hDev == mmh[j]) return j;
	}
	for (int k = 0; k<VOXDEVCOUNT; k++) {
		if (hDev == voxh[k]) return k;
	}
	return -1;
}
int isMM(int hDev){
	for(int k=0;k<MMDEVCOUNT;k++){
		if(hDev == mmh[k]) return 1;
	}

	return 0;
}
int isVox(int hDev) {
	for (int k = 0; k<VOXDEVCOUNT; k++) {
		if (hDev == voxh[k]) return 1;
	}

	return 0;
}
static void RouteMedia(int index) {
	LOG_ENTRYEXIT("RouteMedia");
	SC_TSINFO      sc_tsinfo;        /* CTbus time slot structure */
	SC_TSINFO      tsinfo;        /* CTbus time slot structure */
	long scts;

	if (index < 0) {
		Log(ERROR, index, "Invalid index passed, %d", index);
		return;
	}

	
		Log(DEBUG, index, "Routing index is %d ( %s and %s)", index, ATDV_NAMEP(ipmh[index]), ATDV_NAMEP(voxh[index]));

		tsinfo.sc_numts = 1;
		tsinfo.sc_tsarrayp = &scts;
		scts = ipmtxslot[index];

		CRC(dx_listen(voxh[index], &tsinfo), "dx_listen");
		Log(API, index, "dx_listen() Success %s now listening to ts %d)", ATDV_NAMEP(voxh[index]),scts);

		sc_tsinfo.sc_numts = 1;
		sc_tsinfo.sc_tsarrayp = &scts;
		scts = 0;
		/* Get CTbus time slot connected to transmit of voice
		channel on board ...1 */
		CRC(dx_getxmitslot(voxh[index], &sc_tsinfo), "dx_getxmitslot")
		{
			return ;
		}
		Log(API, index, "%s is xmitting to %d, attaching to %s", ATDV_NAMEP(voxh[index]), scts, ATDV_NAMEP(ipmh[index]));

		
		CRC(ipm_Listen(ipmh[index], &sc_tsinfo, EV_ASYNC), "ipm_Listen") {
			//AppExit();
		}
	

}
void DoPortConnect(int sendindex, int recvindex);
void Dial(int index, char * dialstr);
void ProcessEvt(void){
	
	int hDev=sr_getevtdev(0);
	int lEvtType=sr_getevttype(0);
	long *lEvtDatap=(long *)sr_getevtdatap(0);
	int evt_len = sr_getevtlen(0);
	char Str[100];
	MM_METAEVENT mmMetaevt;
	SC_TSINFO* pTimeSlotInfo;


	int index=FindIndexByHandle(hDev);

	if( isMM(hDev) ){

		mm_GetMetaEvent(&mmMetaevt);
		lEvtDatap=(long *)mmMetaevt.evtdatap;
		hDev= mmMetaevt.evtdev;
		lEvtType=mmMetaevt.evttype;
		evt_len =mmMetaevt.evtlen;
	}
	Evt2Str(hDev,lEvtType,lEvtDatap,Str);
	Log(INFO,index,"Processing %s on devh=%d",Str,hDev);

	switch(lEvtType){

		case DMEV_GET_TX_PORT_INFO:
			Log(INFO,index,"Setting TXPortInfo struct");
			if(isMM(hDev) ){
				memcpy(&mmTxPortInfo[index], lEvtDatap, evt_len);
				Log(INFO,index, "Number of TX ports found: %d", mmTxPortInfo[index].unCount);
				
			}else{
				memcpy(&ipmTxPortInfo[index], lEvtDatap, evt_len);
				Log(INFO,index, "Number of TX ports found: %d", ipmTxPortInfo[index].unCount);
			}
			break;
		case DMEV_GET_RX_PORT_INFO:
			Log(INFO,index,"Setting RXPortInfo struct");
			if(isMM(hDev) ){
				memcpy(&mmRxPortInfo[index], lEvtDatap, evt_len);
				Log(INFO,index, "Number of RX ports found: %d", mmRxPortInfo[index].unCount);
				
			}else{
				memcpy(&ipmRxPortInfo[index], lEvtDatap, evt_len);
				Log(INFO,index, "Number of RX ports found: %d", ipmRxPortInfo[index].unCount);
			}
			
			break;
		case IPMEV_STARTMEDIA:
			Log(INFO, index, "Processing IPMEV_STARTMEDIA: %s(%d)", ATDV_NAMEP(hDev), hDev);
			if (index = ININDEX) {
				DoPortConnect(ININDEX, OUTINDEX);
			}
			if (index = OUTINDEX) {
				DoPortConnect(OUTINDEX, ININDEX);
			}
			break;
		case DMEV_PORT_CONNECT:
			Log(INFO,index,"Processing DMEV_PORT_CONNECT: %s(%d)",ATDV_NAMEP(hDev),hDev);
			break;
		case DMEV_PORT_DISCONNECT:
			Log(INFO,index,"Processing DMEV_PORT_DISCONNECT: %s(%d)",ATDV_NAMEP(hDev),hDev);
			break;
		case IPMEV_GET_XMITTS_INFO:
			pTimeSlotInfo = (SC_TSINFO*)lEvtDatap;
			Log(INFO,index,"Received IPMEV_GET_XMITTS_INFO for device = %s",
				ATDV_NAMEP(hDev));
			Log(INFO,index,"Timeslot number %d", *(pTimeSlotInfo->sc_tsarrayp));
			ipmtxslot[index] = *(pTimeSlotInfo->sc_tsarrayp);
			break;
		case IPMEV_LISTEN:
			if (index == DIALINDEX) {
				Dial(index, ",,,1,2,3,4,5,6,7,8,9,0,,,");
			}
			break;
		case TDX_DIAL:
			Dial(index, ",,,1,2,3,4,5,6,7,8,9,0,,,");
		break;
		case TDX_CST:
			DX_CST *cstp;
			cstp = (DX_CST *)sr_getevtdatap();
			switch (cstp->cst_event) {
			case DE_DIGITS:
				Log(EVENT, index, "DE_DIGIT event = Digit pressed = %c", cstp->cst_data);
				break;
			default:
				Log(EVENT, index, "No processing enabled for CST event");
				break;

			}

		break;
		case CUSTOMEV_TEST_READY:
			
			StartMedia(ININDEX, ipaddr, 49152, 0);
			StartMedia(OUTINDEX, ipaddr, 49158, 0);

			StartMedia(DIALINDEX, ipaddr, 49154, 0);
			StartMedia(DETECTINDEX, ipaddr, 49156, 0);
			RouteMedia(DETECTINDEX);
			RouteMedia(DIALINDEX);
			

			break;
	}

}

/***************************************************************************
 *        NAME: void intr_hdlr(), enable_int_handers()
 * DESCRIPTION: Handler called when one of the following signals is
 *              received: SIGHUP, SIGINT, SIGQUIT, SIGTERM.
 *              This function stops I/O activity on all channels and
 *              closes all the channels.
 ***************************************************************************/
#if WIN32
void intr_hdlr()
#else
void intr_hdlr(void)
#endif
{

keeplooping=0;	

 	
	
}

void enable_int_handers( void){

LOG_ENTRYEXIT("enable_int_handers");

keeplooping=1;
#ifdef WIN32
	signal(SIGINT, (void (__cdecl*)(int)) intr_hdlr);
	signal(SIGTERM, (void (__cdecl*)(int)) intr_hdlr);
#else
	sigset(SIGHUP, (void (*)()) intr_hdlr);
	sigset(SIGQUIT, (void (*)()) intr_hdlr);
	sigset(SIGINT, (void (*)()) intr_hdlr);
	sigset(SIGTERM, (void (*)()) intr_hdlr);
#endif
	
	
}
int CreateConnectInfoList(
PDM_PORT_CONNECT_INFO_LIST pconn_lst,
CPDM_PORT_INFO_LIST pport_lst1,
CPDM_PORT_INFO_LIST pport_lst2
)
{
	INIT_DM_PORT_CONNECT_INFO_LIST(pconn_lst);
	// Loop through all transmit ports of 1st device
	int k = 0;
	int i;
	for (i = 0; i < pport_lst1->unCount; ++i) {
		DM_PORT_MEDIA_TYPE type_tx =
		pport_lst1->port_info[i].port_media_type;
		// find appropriate RX port on 2nd device
		bool bFound = false;
		int j;
		for (j = 0; j < pport_lst2->unCount; ++j) {
			DM_PORT_MEDIA_TYPE type_rx =
			pport_lst2->port_info[j].port_media_type;
			if (type_tx == type_rx) {
				bFound = true;
				break;
			}
		}
		if (!bFound) {
			continue;
		}
		// create element of connect list
		DM_PORT_CONNECT_INFO& info =
		pconn_lst->port_connect_info[k];
		INIT_DM_PORT_CONNECT_INFO(&info);
		//info.unFlags = DMFL_TRANSCODE_ON;
		info.unFlags = DMFL_TRANSCODE_NATIVE;
		info.port_info_tx = pport_lst1->port_info[i];
		info.port_info_rx = pport_lst2->port_info[j];
		++k;
	}
	pconn_lst->unCount = k;
	return k;
}
void DoPortConnect(int sendindex, int recvindex) {
	LOG_ENTRYEXIT("DoPortConnect(index)");
	int index = sendindex;
	int other = recvindex;
	int num_portsMatched;


		num_portsMatched = CreateConnectInfoList(&ipmConnectList[index], &ipmTxPortInfo[index], &ipmRxPortInfo[other]);
		if (num_portsMatched < 1) {
			Log(ERROR, index, "Unable to match ports");
			keeplooping = 0;
			return;
		}
		Log(INFO, index, "dev_PortConnect Tx:%s(%d) to Rx:%s(%d)",
			ATDV_NAMEP(ipmh[index]),
			ipmh[index],
			ATDV_NAMEP(ipmh[other ])
			, ipmh[other]);
		//Log(INFO,index,"dev_PortConnect(%d) to %s(%d)",ipmh[index],ATDV_NAMEP(mmh[index]),mmh[index]);

		dev_PortConnect(ipmh[index], &ipmConnectList[index], NULL);
		

}

void DoPortDisconnect(int index){
	LOG_ENTRYEXIT("DoPortDisconnect(index)");
		
			Log(INFO,index,"dev_PortDisconnect(%d)",ipmh[index]);
			//Log(INFO,index,"dev_PortConnect(%d) to %s(%d)",ipmh[index],ATDV_NAMEP(mmh[index]),mmh[index]);
			dev_PortDisconnect(ipmh[index], &ipmConnectList[index], NULL);

		
}

void DoGetRxPortInfo(){
	LOG_ENTRYEXIT("DoGetRxPortInfo");
	for(int index=0;index<IPMDEVCOUNT;index++){
		Log(INFO,index,"dev_GetReceivePortInfo(%d)",ipmh[index]);
		if(dev_GetReceivePortInfo(ipmh[index],NULL) == -1){
			Log(ERROR,index,"Error in dev_GetReceivePortInfo(%d)",ipmh[index]);
		}
	}
	
	for(int index2=0;index2<MMDEVCOUNT;index2++){
		Log(INFO,index2,"dev_GetReceivePortInfo(%d)",mmh[index2]);
		if(dev_GetReceivePortInfo(mmh[index2],NULL) == -1){
			Log(ERROR,index2,"Error in dev_GetReceivePortInfo(%d)",mmh[index2]);
		}
	}

}
void DoGetTxPortInfo(){
	LOG_ENTRYEXIT("DoGetTxPortInfo");
	for(int index=0;index<IPMDEVCOUNT;index++){
		Log(INFO,index,"dev_GetTransmitPortInfo(%d)",ipmh[index]);
		if(dev_GetTransmitPortInfo(ipmh[index],NULL) == -1){
			Log(ERROR,index,"Error in dev_GetTransmitPortInfo(%d)",ipmh[index]);
		}
	}
	
	for(int index3=0;index3<MMDEVCOUNT;index3++){
		Log(INFO,index3,"dev_GetTransmitPortInfo(%d)",mmh[index3]);
		if(dev_GetTransmitPortInfo(mmh[index3],NULL) == -1){
			Log(ERROR,index3,"Error in dev_GetTransmitPortInfo(%d)",mmh[index3]);
		}
	}

}
void ProcessKey( int key){
	LOG_ENTRYEXIT("ProcessKey");
	static int index=0;
	switch(key){
		
		case 'q':
		case 'Q':
		case 27:
			intr_hdlr();
			break;
		
		default:
			Log(DEBUG,NOINDEX,"%c key pressed",key);
	}
}
int _tmain(int argc, _TCHAR* argv[])
{
	
	int rc;
	char key;

	if(argc == 2){
		strcpy(ipaddr,argv[1]);
	}
	else if(argc == 3){
		strcpy(ipaddr,argv[1]);
		
	}else if(argc == 4){
		strcpy(ipaddr,argv[1]);
		audiocodec=atoi(argv[2]);
		defaultrfcptype=atoi(argv[3]);
	}
	else{
		printf("Error on Command Line!!\n");
		printf("Usage:\n");
		printf("   %s <RTP IP addr> [codec] [rfc2833 ptype]\n",argv[0]);
		printf("Where:\n");
		printf("   RTP IP address:   Ip selected for the RTP in the DCM\n");
		printf("   Codec to use:     AMRNB = %d, AMRWB=%d, ULAW=%d (default ULAW)\n",AMRNB,AMRWB,MULAW);
		printf("   rfc2833 payload:  Number to use, (default %d)\n",defaultrfcptype);
		printf("Ex:\n");
		printf("   %s 192.168.1.1 %d\n",argv[0],MULAW);
		exit(0);

	}
	LOG_FILE_INIT("portConnectTest.txt");
	Log(ALWAYS,NOINDEX,"Parsing command line, ipaddr=%s, codec=%d, ptype=%d",ipaddr,audiocodec,defaultrfcptype);
	
	
	//IPMDEVCOUNT=devcount;
	//MMDEVCOUNT=devcount;
	MMDEVCOUNT = 0;
	IPMDEVCOUNT = 4;
	VOXDEVCOUNT = 4;

	enable_int_handers();

	for(int m=0; m<IPMDEVCOUNT;m++){
		OpenIpm(m);
	}
	for(int n=0; n<MMDEVCOUNT;n++){
		OpenMM(n);
	}
	for (int n = 0; n<VOXDEVCOUNT; n++) {
		OpenVox(n);
	}
	DoGetRxPortInfo();
	DoGetTxPortInfo();

	Sleep(1000);
	sr_putevt(SRL_DEVICE, CUSTOMEV_TEST_READY,0, NULL, 0);
	
	

	while(keeplooping){

		rc=sr_waitevt(100);
		if(rc >= 0){
			ProcessEvt();
		}
		if(kbhit()){
			key=getch();
			ProcessKey( key );
		}
	}
	for(int q=0; q<IPMDEVCOUNT;q++){
		CloseIpm(q);
	}
	for(int p=0; p<MMDEVCOUNT;p++){
		CloseMM(p);
	}
	for (int p = 0; p<VOXDEVCOUNT; p++) {
		CloseVox(p);
	}
	Sleep(4000);
	return 0;
}



