#if !defined(BYSERVER_COMPLUS_H)
#define BYSERVER_COMPLUS_H



//////��ʼ��ģ��
DWORD BYInitModule(IN char *InitParam);

//////ģ��ر�
DWORD BYReleaseModule();

//////��ȡģ��״̬
DWORD BYGetModuleStatus(OUT void *pStatus);

//////�ص���ģ��Ŀͻ��������ݽӿ�
struct stSendInfo
{
	void* pSendParam;
	char *pSendBuf;
	DWORD SendLen;
};
typedef DWORD (*BYSendClientDataFun)(IN void* pInfo,IN  char *pSendBuf,IN  DWORD SendLen);

//////��ʼ���ͻ�����
void* BYInitClientUserInfo(IN char *UserParam,IN  BYSendClientDataFun pSendFun);

//////�رտͻ�
DWORD BYCloseClientUser(IN void* pInfo);

//////���յ��ͻ�����
long BYReceiveClientData(IN void* pInfo,IN  char *pRecvBuf,IN  DWORD RecvLen);



#endif