#include "YTCom.h"
#include <time.h>

// ÿ���޸Ķ���汾��Ϣ���汾��¼�޸���Ϣ��YTConn.h�ļ�ͷ��д��
#define MAIN_VER	"V1.0"

// ��̬ȫ�ֱ��������ڷ����ظ���ʼ��������
static bool g_IsExist = false;

// �ص�ȫ�ֱ���
tagYTConnCallBackFunc	g_CallBackFList;			// �ص�����

// ��־�ļ�ȫ�ֱ���
char					g_szProcessPath[512] = {0};	// ����·��
CYTLogFile				g_LogFile;					// ��־�ļ�

// ȫ��������
unsigned int			g_reqno = 0;				// �����ţ��ͻ���ÿ����������͵�����ˣ�
CYTLock					g_reqnolock;				// �����ſ�����

// ȫ�ֺ�������ȡ������
int GlobGetReqNo(void)
{
	int ret = 0;
	
	g_reqnolock.Lock();
	g_reqno++;
	ret = g_reqno;
	g_reqnolock.UnLock();
	
	return ret;
}

// ȫ�ֺ�������־��Ϣ
// type				��־��𣬲μ�ENum_LogType����
// level			��־���𣬲μ�ENum_LogLevel����
// log				��־��Ϣ
void GlobalLogout(int type, int level, const char * format, ...)
{
	if(level < 0)
	{
		return;
	}

	char szBuffer[10240] = {0};
	switch(level)
	{
	default:
	case LOG_NORMAL:
		sprintf_s(szBuffer, sizeof(szBuffer), "[����]");
		break;
	case LOG_WARNING:
		sprintf_s(szBuffer, sizeof(szBuffer), "[����]");
		break;
	case LOG_ERROR:
		sprintf_s(szBuffer, sizeof(szBuffer), "[����]");
		break;
	case LOG_SUCCESS:
		sprintf_s(szBuffer, sizeof(szBuffer), "[�ɹ�]");
		break;
	}
	va_list arg_ptr;
	va_start(arg_ptr, format);
	_vsnprintf_s(szBuffer + strlen(szBuffer), sizeof(szBuffer) - strlen(szBuffer), _TRUNCATE, format, arg_ptr);
	va_end(arg_ptr);

	g_LogFile.WriteLogFormat_Time(type, szBuffer);
}

// ��ȡ�汾����V1.0����ʽ
const char * YTConn_GetVer(void)
{
	return MAIN_VER;
}

// DLL��ʼ��
// processpath		����·���������ļ�����·����λ��
// callbackflist	�ص�����ָ���б�
// logpath			��־·�������ΪNULL�Ļ���Ĭ��ȡ��ǰ·����logĿ¼
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTConn_Init(const char * processpath, tagYTConnCallBackFunc * callbackflist, const char * logpath)
{
	if(g_IsExist)
	{
		return 0;
	}

	// �������·��
	if(processpath!=NULL && strlen(processpath)>0)
	{
		memcpy(g_szProcessPath, processpath, min(sizeof(g_szProcessPath)-1, strlen(processpath)));
	}

	// ������־·��
	char szFullLogPath[512];
	if(logpath!=NULL && strlen(logpath)>0)
	{
		if(processpath!=NULL && strlen(processpath)>0 && strstr(logpath, ":")==NULL)
		{
			sprintf_s(szFullLogPath, sizeof(szFullLogPath), "%s\\%s", processpath, logpath);
		}
		else
		{
			sprintf_s(szFullLogPath, sizeof(szFullLogPath), "%s", logpath);
		}
	}
	else
	{
		if(processpath!=NULL && strlen(processpath)>0)
		{
			sprintf_s(szFullLogPath, sizeof(szFullLogPath), "%s\\log", processpath);
		}
		else
		{
			sprintf_s(szFullLogPath, sizeof(szFullLogPath), "log");
		}
	}
	g_LogFile.SetFileName("ytconn");
	g_LogFile.SetLogType(LT_RUN);

	// ��ʼ��ʼ��
	GlobalLogout(LT_SYS, LOG_NORMAL, "ͨѶģ���ʼ����ʼ\r\n");

	YTData_Init();
	CYTSocket::InitCondition();

	if(callbackflist != NULL)
	{
		memcpy(&g_CallBackFList, callbackflist, sizeof(tagYTConnCallBackFunc));
	}

	g_reqno	= 0;

	GlobalLogout(LT_SYS, LOG_SUCCESS, "ͨѶģ���ʼ���ɹ�\r\n");

	g_IsExist = true;
	return 0;
}

// DLL�ر�
void YTConn_Release(void)
{
	if(!g_IsExist)
	{
		return;
	}
	else
	{
		g_IsExist = false;
	}
	
	GlobalLogout(LT_SYS, LOG_NORMAL, "ͨѶģ��رտ�ʼ\r\n");

	YTData_Release();
	CYTSocket::ReleaseCondition();

	GlobalLogout(LT_SYS, LOG_SUCCESS, "ͨѶģ��رճɹ�\r\n");
}

// �������
// 1���þ����־����������Ϣ���������к�������صĺ�����������ж���Ҫ����þ����Ϣ
// 2�����صľ����Χ���õĻ�����������DelConfig�����ͷţ���Χ��Ҫ��delete����
// ����ֵ��			NULLʧ�ܣ������ɹ�
void * YTConn_NewConfig(void)
{
	CYTIniFile * ret = new CYTIniFile;
	return ret;
}

// ɾ�����
void YTConn_DelConfig(void * cfghandle)
{
	CYTIniFile * pYTIniFile = (CYTIniFile *)cfghandle;
	if(pYTIniFile != NULL)
	{
		delete pYTIniFile;
		pYTIniFile = NULL;
	}
}

// ���ļ��м�����������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTConn_LoadConfig(void * cfghandle, const char * filename)
{
	CYTIniFile * pYTIniFile = (CYTIniFile *)cfghandle;
	if(pYTIniFile != NULL)
	{
		// ��ȡȫ·��
		char szFullPath[512];
		if(strlen(g_szProcessPath)>0 && strstr(filename, ":")==NULL)
		{
			sprintf_s(szFullPath, sizeof(szFullPath), "%s\\%s", g_szProcessPath, filename);
		}
		else
		{
			sprintf_s(szFullPath, sizeof(szFullPath), "%s", filename);
		}
		return pYTIniFile->Open(szFullPath);
	}
	else
	{
		return -1;
	}
}

// �����������õ��ļ���
// ���filenameΪNULL���ʾ���浽֮ǰ����ʱ������ļ�
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTConn_SaveConfig(void * cfghandle, const char * filename)
{
	CYTIniFile * pYTIniFile = (CYTIniFile *)cfghandle;
	if(pYTIniFile != NULL)
	{
		return pYTIniFile->Save(filename);
	}
	else
	{
		return -1;
	}
}

// ��ȡ�ַ���������
// ����ֵ��			>=0�ɹ� <0ʧ��
const char * YTConn_ReadStrConfig(void * cfghandle, const char * section, const char * key, const char * defaultvalue)
{
	CYTIniFile * pYTIniFile = (CYTIniFile *)cfghandle;
	if(pYTIniFile != NULL)
	{
		return pYTIniFile->ReadString(section, key, defaultvalue);
	}
	else
	{
		return NULL;
	}
}

// ��ȡ��������
int YTConn_ReadIntConfig(void * cfghandle, const char * section, const char * key, int defaultvalue)
{
	CYTIniFile * pYTIniFile = (CYTIniFile *)cfghandle;
	if(pYTIniFile != NULL)
	{
		return pYTIniFile->ReadInteger(section, key, defaultvalue);
	}
	else
	{
		return 0;
	}
}

// д���ַ���������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTConn_WriteStrConfig(void * cfghandle, const char * section, const char * key, const char * value)
{
	CYTIniFile * pYTIniFile = (CYTIniFile *)cfghandle;
	if(pYTIniFile != NULL)
	{
		return pYTIniFile->WriteString(section, key, value);
	}
	else
	{
		return -1;
	}
}

// д����������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTConn_WriteIntConfig(void * cfghandle, const char * section, const char * key, int value)
{
	CYTIniFile * pYTIniFile = (CYTIniFile *)cfghandle;
	if(pYTIniFile != NULL)
	{
		return pYTIniFile->WriteInteger(section, key, value);
	}
	else
	{
		return -1;
	}
}

// �������Ӿ��
// 1���þ����־���Ӷ�����Ϣ���������к�������صĺ�����������ж���Ҫ����þ����Ϣ
// 2�����صľ����Χ���õĻ�����������DelConn�����ͷţ���Χ��Ҫ��delete����
// confighandle		�������þ������YTConn_NewConfig������������һһ��Ӧ�����д���IP��ַ����Ϣ��
// ����ֵ��			NULLʧ�ܣ������ɹ�
void * YTConn_NewConn(void * cfghandle)
{
	CYTCom * ret = new CYTCom;
	if(ret != NULL)
	{
		if(ret->Init((CYTIniFile *)cfghandle) < 0)
		{
			delete ret;
			ret = NULL;
			
			return NULL;
		}
		else
		{
			return ret;
		}
	}
	else
	{
		return NULL;
	}
}

// ɾ������
void YTConn_DelConn(void * connhandle)
{
	CYTCom * pYTCom = (CYTCom *)connhandle;
	if(pYTCom != NULL)
	{
		delete pYTCom;
		pYTCom = NULL;
	}
}

// ��������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTConn_Connect(void * connhandle)
{
	CYTCom * pYTCom = (CYTCom *)connhandle;
	if(pYTCom != NULL)
	{
		return pYTCom->ConnectServer();
	}
	else
	{
		return -1;
	}
}

// �ر�����
int YTConn_Close(void * connhandle)
{
	CYTCom * pYTCom = (CYTCom *)connhandle;
	if(pYTCom != NULL)
	{
		return pYTCom->CloseConnect();
	}
	else
	{
		return -1;
	}
}

// ��ȡ����״̬
// ����ֵ��			�μ������ENum_ConnStatus�ṹ����
int YTConn_GetStatus(void * connhandle)
{
	CYTCom * pYTCom = (CYTCom *)connhandle;
	if(pYTCom != NULL)
	{
		return pYTCom->GetStatus();
	}
	else
	{
		return -1;
	}
}

// ��ȡ��ǰ���ӵķ�����
// ����ֵ��			NULLû�����ӣ�������ʾ���ӷ�������ַ
const char * YTConn_GetServer(void * connhandle, int * port)
{
	CYTCom * pYTCom = (CYTCom *)connhandle;
	if(pYTCom != NULL)
	{
		return pYTCom->GetServer(port);
	}
	else
	{
		return NULL;
	}
}

// �����������ݽ��������첽��ʽ�������յ����ͨ������ĸ����ص��������أ�
// funcid			���ܺţ��μ�Э���ĵ���
// data				Ҫ���͵����ݣ�ע�⣺���ﴫ������������壬��ͷ��DLL�Լ��ڲ��������ƣ�
// len				���ݳ���
// ����ֵ��			>=0������ <0ʧ��
int YTConn_Send(void * connhandle, unsigned int funcid, const char * data, int len)
{
	CYTCom * pYTCom = (CYTCom *)connhandle;
	if(pYTCom != NULL)
	{
		return pYTCom->SendData(funcid, data, (int)len);
	}
	else
	{
		return -1;
	}
}

// �������ݴ�������δ���������һ�������Ӧ����
// phead			����������ͷ����
// data				Ҫ���͵����ݣ�ע�⣺���ﴫ������������壩
// len				���ݳ���
// outbuf			�������ݣ������ķ�������
// outlen			�������ݣ����泤�ȣ�������Ȳ����Ļ����ֵ��Ϊ����������Ҫ�ĳ���
// ����ֵ:			>0 ���������ݳ���
//					=0 �������
//					<0 ����Ĵ���󻺴泤�Ȳ��㣬��ʱoutlen������Ҫ��ҵ�����ݳ���
int YTConn_Pack(tagYTConnPackHead * phead, const char * data, int len, const char * outbuf, int * outlen)
{
	return CYTCom::Pack(phead, data, len, outbuf, outlen);
}

// Ӧ�����ݽ��
// data				���յ������ݣ�ע�⣺����������а�ͷ�Ͱ��壩
// len				���ݳ���
// phead			����������ͷ��������
// outbuf			�������ݣ�������ҵ�����ݣ�ע�⣺���������ҵ������û�а�ͷ��
// outlen			�������ݣ�ʵ�ʵ�ҵ�����ݵĳ��ȣ����Ϊ0���ʾû�н��������
// ����ֵ:			>0 �Ѿ�����Ľ������ݳ��ȣ���ʱ����outlen���ؽ������ҵ�����ݳ���
//					=0 ����Ҫ������������
//					<0 �����ҵ�����ݳ��Ȳ��㣬��ʱoutlen������Ҫ��ҵ�����ݳ���
int YTConn_UnPack(const char * data, int len, tagYTConnPackHead * phead, const char * outbuf, int * outlen)
{
	return CYTCom::UnPack(data, len, phead, outbuf, outlen);
}