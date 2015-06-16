#if !defined(BYSERVER_COMPLUS_H)
#define BYSERVER_COMPLUS_H



//////初始化模块
DWORD BYInitModule(IN char *InitParam);

//////模块关闭
DWORD BYReleaseModule();

//////获取模块状态
DWORD BYGetModuleStatus(OUT void *pStatus);

//////回调给模块的客户发送数据接口
struct stSendInfo
{
	void* pSendParam;
	char *pSendBuf;
	DWORD SendLen;
};
typedef DWORD (*BYSendClientDataFun)(IN void* pInfo,IN  char *pSendBuf,IN  DWORD SendLen);

//////初始化客户对象
void* BYInitClientUserInfo(IN char *UserParam,IN  BYSendClientDataFun pSendFun);

//////关闭客户
DWORD BYCloseClientUser(IN void* pInfo);

//////接收到客户数据
long BYReceiveClientData(IN void* pInfo,IN  char *pRecvBuf,IN  DWORD RecvLen);



#endif