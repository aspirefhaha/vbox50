#include "effilelist.h"
#include <Windows.h>
#include <qdebug.h>
//#include "exfatserver.h"

#pragma comment(lib,"ws2_32.lib")
#define EXFATSERVERPORT 6543	//����˿ں�
#define EXFAT_NAME_MAX 255
/* UTF-16 encodes code points up to U+FFFF as single 16-bit code units.
   UTF-8 uses up to 3 bytes (i.e. 8-bit code units) to encode code points
   up to U+FFFF. One additional character is for null terminator. */
#define EXFAT_UTF8_NAME_BUFFER_MAX (EXFAT_NAME_MAX * 3 + 1)

enum TCPEXFAT_CMD {	//TCP Exfat Command
	TECMD_NEGSIZE=0,	//negotiate variable length or constant length ,default 0=constant length,1=variable length
	TECMD_EFFileOpen,	// 
	TECMD_EFFileClose,
	TECMD_EFPathQueryInfoEx,
	TECMD_EFFileSeek,
	TECMD_EFFileRead,
	TECMD_EFFileWrite,
	TECMD_EFFileFlush,
	TECMD_EFFileGetSize,
	TECMD_EFFileIsValid,
	TECMD_EFFileQueryInfo,
	TECMD_EFFileDelete,
	TECMD_EFFsQuerySizes,
	TECMD_EFFileSetSize,
	TECMD_EFDirList,
	TECMD_MAX
};

//unsigned char tcpexfat_param_bytes[] ={
//TCP EXFAT COMMAND
#define	TEC_NEGSIZE  2 	//TECMD_NEGSIZE=1,	//negotiate variable length or constant length ,default constant length
#define	TEC_EFFILEOPEN (1+4+EXFAT_UTF8_NAME_BUFFER_MAX)	//TECMD_EFFileOpen, (EXFAT_UTF8_NAME_BUFFER_MAX + 8 bytes flag) = 255+8 = 263 bytes
#define TEC_EFFILECLOSE	(1+8)	//TECMD_EFFileClose, 64 bit pointer = 8 bytes
#define TEC_EFPATHQUERYINFOEX 	(1+EXFAT_UTF8_NAME_BUFFER_MAX)	//TECMD_EFPathQueryInfoEx, EXFAT_NAME_MAX + 4(fFlags) = 255 + 4 = 259
#define TEC_EFFILESEEK	(1+8+8+4)	//TECMD_EFFileSeek,
#define TEC_EFFILEREAD	(1+8+8)	//TECMD_EFFileRead, file handle and cbToRead
#define TEC_EFFILEWRITE	(1+8+8)	//TECMD_EFFileWrite,file handle and cbToWrite
#define TEC_EFFILEFLUSH	(1+8)	//TECMD_EFFileFlush,
#define TEC_EFFILEGETSIZE	(1+8)	//TECMD_EFFileGetSize,
#define TEC_EFFILEISVALID	(1+8)	//TECMD_EFFileIsValid,
#define TEC_EFFILEQUERYINFO	(1+8)	//TECMD_EFFileQueryInfo,
#define TEC_EFFILEDELETE	(1+EXFAT_UTF8_NAME_BUFFER_MAX)	//TECMD_EFFileDelete,
#define TEC_EFFSQUERYSIZES	(1)
#define TEC_EFFILESETSIZE	(1+8+8)	//
#define TEC_EFDIRLIST		(1+EXFAT_UTF8_NAME_BUFFER_MAX)
#define TEC_CMDMAX	1	//TECMD_MAX
//};

//TCP EXFAT ACK
//unsigned char tcpexfat_return_bytes[] ={
#define TEA_NEGSIZE  (2)	//TECMD_NEGSIZE=1,	//negotiate variable length or constant length ,default constant length
#define TEA_EFFILEOPEN 	(1+1+8)	//TECMD_EFFileOpen,HANDLE as 64 bit pointer
#define TEA_EFFILECLOSE	2	//TECMD_EFFileClose, 64 bit pointer = 8 bytes
#define TEA_EFPATHQUERYINFOEX	(1+sizeof(EFFSOBJINFO))	//TECMD_EFPathQueryInfoEx, EXFAT_NAME_MAX + 4(fFlags) = 255 + 4 = 259
#define TEA_EFFILESEEK	(1+8)	//TECMD_EFFileSeek,
#define TEA_EFFILEREAD	(1+8)	//TECMD_EFFileRead,
#define TEA_EFFILEWRITE	(1+8)	//TECMD_EFFileWrite,
#define TEA_EFFILEFLUSH	(1+4)	//TECMD_EFFileFlush,
#define TEA_EFFILEGETSIZE	(1+8)	//TECMD_EFFileGetSize,
#define TEA_EFFILEISVALID	(1+4)	//TECMD_EFFileIsValid,
#define TEA_EFFILEQUERYINFO	(1+sizeof(EFFSOBJINFO))	//TECMD_EFFileQueryInfo,
#define TEA_EFFILEDELETE	(1+4)	//TECMD_EFFileDelete,
#define TEA_EFFSQUERYSIZES	(1+8+8+4+4)
#define TEA_EFFILESETSIZE	(1+8+4)
#define TEA_EFDIRLIST		(1+EXFAT_UTF8_NAME_BUFFER_MAX)
#define TEA_CMDMAX	1	//TECMD_MAX

static  SOCKET curSock = INVALID_SOCKET;
//static SOCKET sockClient = INVALID_SOCKET;

static void deinit_exfatfs(void){

    if(curSock != INVALID_SOCKET){
        closesocket(curSock);
        curSock = INVALID_SOCKET;
    }
}
static SOCKET init_exfatfs(void)
{

    if((getenv("FHAHADEBUG") != NULL && strcmp(getenv("FHAHADEBUG"),"1")==0)){
        return curSock;
    }
    
    if(curSock == INVALID_SOCKET){
		WORD wVerisonRequested;
		WSADATA wsaData;
		int err;
		wVerisonRequested = MAKEWORD(1, 1);
		err = WSAStartup(wVerisonRequested, &wsaData);
		if (err != 0)
		{
			return INVALID_SOCKET;
		}
		curSock = socket(AF_INET, SOCK_STREAM, 0);

		// connect server socket
		SOCKADDR_IN addrServer;
		addrServer.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		addrServer.sin_family = AF_INET;
		addrServer.sin_port = htons(EXFATSERVERPORT);
		if(connect(curSock, (SOCKADDR *)&addrServer, sizeof(addrServer))==SOCKET_ERROR){
			curSock = INVALID_SOCKET;
            //Assert(0);
		}
        else{
            
        }
        
    }
    return curSock;
    
}

void effilelist::sltFileSelect(QModelIndex idx)
{
	ui.pbOk->setEnabled(true);
}
void effilelist::sltSelectOk()
{
	this->selectFileName = QString("/VirtualBox VMs/%1/%2").arg(ui.lwDirs->currentItem()->text()).arg(ui.lwFiles->currentItem()->text());
	accept();
}
void effilelist::sltCancel()
{
	this->selectFileName = "";
	reject();
}

void effilelist::sltDirSelect(QModelIndex idx)
{
	QListWidgetItem * pItem = ui.lwDirs->item(idx.row());
	ui.pbOk->setEnabled(false);
	QString fileliststr = QString("/VirtualBox VMs/%1").arg(pItem->text()) ;
	if(curSock != INVALID_SOCKET){
		int rc = -1;
		do{
			char cmddata[TEC_EFDIRLIST]={0};
			cmddata[0]= TECMD_EFDirList;
			strcpy(&cmddata[1],fileliststr.toStdString().c_str());
			int sendret = send(curSock, cmddata, TEC_EFDIRLIST , 0);

			if(sendret != TEC_EFDIRLIST)
				break;
			int tmpRecvLen = 0;
			int curLen = 1;
			char recvdata[TEA_EFDIRLIST]={0};        
			while(tmpRecvLen < TEA_EFDIRLIST && curLen >0){
				curLen =  recv(curSock,recvdata+tmpRecvLen,TEA_EFDIRLIST-tmpRecvLen,0);
				tmpRecvLen += curLen;
			}
			if(recvdata[0]!= TECMD_EFDirList)
				break;
			ui.lwFiles->clear();
			int vmcount = 0;
			char * pstartname = &recvdata[5];
			memcpy(&vmcount,&recvdata[1],sizeof(int));
			for(int k = 0 ;k<vmcount;k++){
				char tmpname[64]={0};
				int curlen = strlen(pstartname);
				strncpy(tmpname,pstartname,63);
				pstartname+= curlen+1;
				tmpname[63]=0;
				QString subname = tmpname;
				if(subname.endsWith(".vdi"))
					ui.lwFiles->addItem(tmpname);
			}
		}while(0);
	}
}

effilelist::effilelist(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	init_exfatfs();
	ui.setupUi(this);
	if(curSock != INVALID_SOCKET){
		int rc = -1;
		do{
			char cmddata[TEC_EFDIRLIST]={0};
			cmddata[0]= TECMD_EFDirList;
			strcpy(&cmddata[1],"/VirtualBox VMs");
			int sendret = send(curSock, cmddata, TEC_EFDIRLIST , 0);

			if(sendret != TEC_EFDIRLIST)
				break;
			int tmpRecvLen = 0;
			int curLen = 1;
			char recvdata[TEA_EFDIRLIST]={0};        
			while(tmpRecvLen < TEA_EFDIRLIST && curLen >0){
				curLen =  recv(curSock,recvdata+tmpRecvLen,TEA_EFDIRLIST-tmpRecvLen,0);
				tmpRecvLen += curLen;
			}
			if(recvdata[0]!= TECMD_EFDirList)
				break;
			int vmcount = 0;
			char * pstartname = &recvdata[5];
			memcpy(&vmcount,&recvdata[1],sizeof(int));
			for(int k = 0 ;k<vmcount;k++){
				char tmpname[64]={0};
				int curlen = strlen(pstartname);
				strncpy(tmpname,pstartname,63);
				pstartname+= curlen+1;
				tmpname[63]=0;
				ui.lwDirs->addItem(tmpname);
			}
		}while(0);
	}
}

effilelist::~effilelist()
{
	deinit_exfatfs();
}
