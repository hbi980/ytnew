#include "../../../YTPublic/YTInclude/YTBase/YTDb.h"
#include "YTDbPool.h"

// ÿ���޸Ķ���汾��Ϣ���汾��¼�޸���Ϣ��YTDb.h�ļ�ͷ��д��
#define MAIN_VER	"1.0"

// ��̬ȫ�ֱ��������ڷ����ظ���ʼ��������
static bool g_IsExist = false;

// ȫ�ֱ���
CYTDbPool *		g_DbPool		= NULL;	// ���ݿ����ӳ�
int				g_DbCount		= 0;	// ���ݿ����
int				g_CurrDbIndex	= -1;	// ��ǰ���ݿ�����

CYTThread		g_CheckDBThread;		// ���ݿ����߳�
int				g_SysStatus		= 0;	// ϵͳ״̬��0-ֹͣ 1-����

CYTLogFile		g_LogFile;				// ��־�ļ�

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

// ����߳�
unsigned int __stdcall CheckDBThread(void * pParam)
{
	int i;
	while(g_SysStatus != 0)
	{
		// ��⵱ǰʹ�õ����ݿ��״̬
		if(g_CurrDbIndex<0 || !g_DbPool[g_CurrDbIndex].CheckConn())
		{
			// �����ǰ���ݿⲻͨ��������һ��������ͨ�����ݿ���Ϊ���µ�
			for(i=0; i<g_DbCount; i++)
			{
				if(i!=g_CurrDbIndex && g_DbPool[i].CheckConn())
				{
					g_CurrDbIndex = i;
					break;
				}
			}
			if(i == g_DbCount)
			{
				g_CurrDbIndex = -1;
			}
		}

		// �����������
		for(i=0; i<g_DbCount; i++)
		{
			g_DbPool[i].RemoveIdle();
		}

		// ˯��һ��ʱ��
		::Sleep(10*1000);
	}

	return 0;	
}

// ��ȡ�汾����V1.0����ʽ
const char * YTDb_GetVer(void)
{
	return MAIN_VER;
}

// DLL��ʼ��
// processpath		����·���������ļ�����·����λ��
// cfgfile			���ݿ������ļ�
// logpath			��־·�������ΪNULL�Ļ���Ĭ��ȡ��ǰ·����logĿ¼
// errmsg			��ʼ��������Ϣ����
// len				���󻺴泤��
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTDb_Init(const char * processpath, const char * cfgfile, const char * logpath, char * errmsg, int len)
{
	if(g_IsExist)
	{
		return 0;
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
	g_LogFile.SetFileName("ytdb");
	g_LogFile.SetLogType(LT_RUN);
	
	// ��ʼ��ʼ��
	GlobalLogout(LT_SYS, LOG_NORMAL, "���ݿ�ģ���ʼ����ʼ\r\n");

	// ��ʼ������ģ��
	YTData_Init();

	// ��������ļ��Ƿ����
	if(cfgfile == NULL)
	{
		YTData_Release();
		GlobalLogout(LT_SYS, LOG_ERROR, "���ݿ�ģ���ʼ��ʧ�ܣ�û�������ļ�\r\n");
		if(errmsg!=NULL && len>0)
		{
			strncpy((char *)errmsg, "���ݿ�ģ���ʼ��ʧ�ܣ�û�������ļ�\r\n", len-1);
		}
		return -1;
	}

	// �õ������ļ�·��
	char szFullCfgFilePath[512];
	if(processpath!=NULL && strlen(processpath)>0 && strstr(cfgfile, ":")==NULL)
	{
		sprintf_s(szFullCfgFilePath, sizeof(szFullCfgFilePath), "%s\\%s", processpath, cfgfile);
	}
	else
	{
		sprintf_s(szFullCfgFilePath, sizeof(szFullCfgFilePath), "%s", cfgfile);
	}

	// �������ļ�
	CYTIniFile file;
	if(file.Open(szFullCfgFilePath) < 0)
	{
		YTData_Release();
		GlobalLogout(LT_SYS, LOG_ERROR, "���ݿ�ģ���ʼ��ʧ�ܣ������ļ�[%s]��ʧ��\r\n", cfgfile);
		if(errmsg!=NULL && len>0)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "���ݿ�ģ���ʼ��ʧ�ܣ������ļ�[%s]��ʧ��\r\n", cfgfile);
			memcpy(errmsg, szerrmsg, min(len-1, (int)strlen(szerrmsg)));
		}
		return -1;
	}

	// ��ȡ���ݿ����
	g_DbCount = file.ReadInteger("DB_SET", "count", 0);
	if(g_DbCount <= 0)
	{
		YTData_Release();
		GlobalLogout(LT_SYS, LOG_ERROR, "���ݿ�ģ���ʼ��ʧ�ܣ����ݿ����Ϊ0\r\n");
		if(errmsg!=NULL && len>0)
		{
			strncpy((char *)errmsg, "���ݿ�ģ���ʼ��ʧ�ܣ����ݿ����Ϊ0\r\n", len-1);
		}
		g_DbCount = 0;
		file.Close();
		return -1;
	}
	g_DbPool = new CYTDbPool[g_DbCount];

	// ��ʼ��oracle����
	if(OCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT|OCI_ENV_THREADED|OCI_ENV_CONTEXT) == FALSE)
	{
		YTData_Release();

		char szerrmsg[512] = {0};
		sprintf_s(szerrmsg, sizeof(szerrmsg), "���ݿ�ģ���ʼ��ʧ�ܣ�oci��ʼ��ʧ��[");
		GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
		sprintf_s(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg), "]\r\n");
		GlobalLogout(LT_SYS, LOG_ERROR, szerrmsg);
		if(errmsg!=NULL && len>0)
		{
			strncpy((char *)errmsg, szerrmsg, len-1);
		}

		delete []g_DbPool;
		g_DbPool = NULL;
		return -1;
	}

	// ѭ����ȡÿ�����ݿ���Ϣ
	DBInfo	info;
	int		maxconn;
	int		iniconn;
	char	szKey[64];
	for(int i=0; i<g_DbCount; i++)
	{
		memset(&info, 0, sizeof(info));

		// ���ݿ������IP��ַ
		sprintf_s(szKey, sizeof(szKey), "serverip%d", i+1);
		sprintf_s(info.serverip, file.ReadString("DB_SET", szKey, ""));

		// ���ݿ�������˿�
		sprintf_s(szKey, sizeof(szKey), "serverport%d", i+1);
		info.serverport = file.ReadInteger("DB_SET", szKey, 1521);
		
		// ���ݿ�����
		sprintf_s(szKey, sizeof(szKey), "dbname%d", i+1);
		sprintf_s(info.dbname, file.ReadString("DB_SET", szKey, ""));

		// �����û���
		sprintf_s(szKey, sizeof(szKey), "uid%d", i+1);
		sprintf_s(info.uid, file.ReadString("DB_SET", szKey, ""));

		// ��������
		sprintf_s(szKey, sizeof(szKey), "pwd%d", i+1);
		sprintf_s(info.pwd, file.ReadString("DB_SET", szKey, ""));

		// ���������
		sprintf_s(szKey, sizeof(szKey), "maxcount%d", i+1);
		maxconn = file.ReadInteger("DB_SET", szKey, 0);

		// ��ʼ��������������
		sprintf_s(szKey, sizeof(szKey), "inicount%d", i+1);
		iniconn = file.ReadInteger("DB_SET", szKey, 0);

		g_DbPool[i].Init(i, info, maxconn, iniconn);
	}
	file.Close();

	// ��������߳�
	g_SysStatus = 1;
	if(g_CheckDBThread.BeginThread(CheckDBThread, NULL, " ") < 0)
	{
		YTData_Release();

		GlobalLogout(LT_SYS, LOG_ERROR, "���ݿ�ģ���ʼ��ʧ�ܣ���������߳�ʧ��\r\n");
		if(errmsg!=NULL && len>0)
		{
			strncpy((char *)errmsg, "���ݿ�ģ���ʼ��ʧ�ܣ���������߳�ʧ��\r\n", len-1);
		}

		delete []g_DbPool;
		g_DbPool = NULL;

		OCI_Cleanup();
	}

	GlobalLogout(LT_SYS, LOG_SUCCESS, "���ݿ�ģ���ʼ���ɹ�\r\n");
	g_IsExist = true;
	return 0;
}

// DLL�ر�
void YTDb_Release(void)
{
	__try
	{
		if(!g_IsExist)
		{
			return;
		}
		else
		{
			g_IsExist = false;
		}
		
		GlobalLogout(LT_SYS, LOG_NORMAL, "���ݿ�ģ��رտ�ʼ\r\n");
		
		YTData_Release();
		
		bool bneedcleanoci = true;
		if(g_SysStatus == 0)
		{
			bneedcleanoci = false;
		}
		g_SysStatus = 0;
		g_CheckDBThread.StopThread();
		
		if(g_DbPool != NULL)
		{
			delete [] g_DbPool;
			g_DbPool = NULL;
		}
		
		// �ͷ�oracle����
		if(bneedcleanoci)
		{
			OCI_Cleanup();
		}
		
		GlobalLogout(LT_SYS, LOG_SUCCESS, "���ݿ�ģ��رճɹ�\r\n");
	}
	__except(true)
	{
		return;
	}
}

// ��ȡ��ǰ������ݿ���Ϣ
// serverip			���ݿ��ַ
// serveriplen		���ݿ��ַ��󳤶�
// port				���ݿ�˿�
// dbname			���ݿ�����
// dbnamelen		���ݿ�������󳤶�
// maxconn			��������������
// usedconn			����ʹ�õ���������
// waitconn			�ȴ�ʹ�õ���������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTDb_GetCurrDBInfo(char * serverip, int serveriplen, int * port, char * dbname, int dbnamelen, int * maxconn, int * usedconn, int * waitconn)
{
	if(g_DbPool!=NULL && g_CurrDbIndex>=0)
	{
		return g_DbPool[g_CurrDbIndex].GetDBInfo(serverip, serveriplen, port, dbname, dbnamelen, maxconn, usedconn, waitconn);
	}
	else
	{
		return -1;
	}
}

// ������ݿ��Ƿ���ͨ
// ����ֵ��			>=0��ͨ <0�Ͽ�
int YTDb_CheckDB()
{
	if(g_CurrDbIndex >= 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

// ִ��һ��SQL���޷��ؽ����
// sqlcmd			���ݿ�sql���
// errmsg			������Ϣ���棬ִ��ʧ��ʱ����
// msglen			������Ϣ���泤��
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTDb_ExecuteSql(const char * sqlcmd, const char * errmsg, int msglen)
{
	// Ԥ���
	if(sqlcmd == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "����sqlΪ��", min(msglen-1, 11));
		}
		return -1;
	}
	DWORD dwthreadid = GetCurrentThreadId();

	// ��ȡdb���
	int tick_beginpopdb = GetTickCount();
	CYTDbObject * pdb = NULL;
	if(g_DbPool!=NULL && g_CurrDbIndex>=0)
	{
		pdb = g_DbPool[g_CurrDbIndex].PopDB();
	}
	if(pdb == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "��ȡdbʧ��", min(msglen-1, 10));
		}
		GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteSql[%s] err:��ȡdbʧ��\r\n", dwthreadid, sqlcmd);
		return -1;
	}

	// ִ�й���
	int tick_endpopdb = GetTickCount();
	char szerrmsg[1024] = {0};
	int errmsglen = 1024;
	int ret = pdb->ExecuteSql(sqlcmd, szerrmsg, errmsglen);
	int tick_endprov = GetTickCount();
	if(errmsg!=NULL && msglen>0)
	{
		memset((char *)errmsg, 0, msglen);
		memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
	}

	// �����ӷŻ����ӳ�
	g_DbPool[pdb->GetIndex()].PushDB(pdb);
	int tick_endpushdb = GetTickCount();

	// ��¼��־
	int totaltick = tick_endpushdb-tick_beginpopdb;
	int nLevel = 0;
	if(totaltick >= 2000)
	{
		nLevel = 2;
	}
	else if(totaltick >= 1000)
	{
		nLevel = 1;
	}

	if(nLevel == 2)
	{
		GlobalLogout(LT_SYS, LOG_WARNING, "[%u]YTDb_ExecuteSql[%s] errmsg=%s (Level:%d)(poptime:%d)(excutetime:%d)(pushtime:%d)\r\n", dwthreadid, sqlcmd, szerrmsg, nLevel, tick_endpopdb-tick_beginpopdb, tick_endprov-tick_endpopdb, tick_endpushdb-tick_endprov);
	}
	else
	{
		GlobalLogout(LT_RUN, LOG_NORMAL, "[%u]YTDb_ExecuteSql[%s] errmsg=%s (Level:%d)(poptime:%d)(excutetime:%d)(pushtime:%d)\r\n", dwthreadid, sqlcmd, szerrmsg, nLevel, tick_endpopdb-tick_beginpopdb, tick_endprov-tick_endpopdb, tick_endpushdb-tick_endprov);
	}
	
	if(ret < 0)
	{
		GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteSql[%s] err:%s\r\n", dwthreadid, sqlcmd, szerrmsg);
	}
	return ret;
}

// ִ��һ��SQL�����ؽ����¼��
// sqlcmd			���ݿ�sql���
// errmsg			������Ϣ���棬ִ��ʧ��ʱ����
// msglen			������Ϣ���泤��
// datahandle		���ؽ�����ϣ�ͨ��YTData_NewData�õ��������ͨ��YTData_DeleteData�Żأ�
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTDb_OpenSql(const char * sqlcmd, const char * errmsg, int msglen, void * datahandle)
{
	// Ԥ���
	if(sqlcmd==NULL || datahandle==NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "����sql�����ݼ�Ϊ��", min(msglen-1, 19));
		}
		return -1;
	}
	DWORD dwthreadid = GetCurrentThreadId();

	// ��ȡdb���
	int tick_beginpopdb = GetTickCount();
	CYTDbObject * pdb = NULL;
	if(g_DbPool!=NULL && g_CurrDbIndex>=0)
	{
		pdb = g_DbPool[g_CurrDbIndex].PopDB();
	}
	if(pdb == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "��ȡdbʧ��", min(msglen-1, 10));
		}
		GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_OpenSql[%s] err:��ȡdbʧ��\r\n", dwthreadid, sqlcmd);
		return -1;
	}

	// ִ�й���
	int tick_endpopdb = GetTickCount();
	char szerrmsg[1024] = {0};
	int errmsglen = 1024;
	int ret = pdb->OpenSql(sqlcmd, szerrmsg, errmsglen, datahandle);
	int tick_endprov = GetTickCount();
	if(errmsg!=NULL && msglen>0)
	{
		memset((char *)errmsg, 0, msglen);
		memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
	}
	
	// �����ӷŻ����ӳ�
	g_DbPool[pdb->GetIndex()].PushDB(pdb);
	int tick_endpushdb = GetTickCount();

	// ��¼��־
	int totaltick = tick_endpushdb-tick_beginpopdb;
	int nLevel = 0;
	if(totaltick >= 2000)
	{
		nLevel = 2;
	}
	else if(totaltick >= 1000)
	{
		nLevel = 1;
	}

	if(nLevel == 2)
	{
		GlobalLogout(LT_SYS, LOG_WARNING, "[%u]YTDb_OpenSql[%s] errmsg=%s (Level:%d)(poptime:%d)(excutetime:%d)(pushtime:%d)\r\n", dwthreadid, sqlcmd, szerrmsg, nLevel, tick_endpopdb-tick_beginpopdb, tick_endprov-tick_endpopdb, tick_endpushdb-tick_endprov);
	}
	else
	{
		GlobalLogout(LT_RUN, LOG_NORMAL, "[%u]YTDb_OpenSql[%s] errmsg=%s (Level:%d)(poptime:%d)(excutetime:%d)(pushtime:%d)\r\n", dwthreadid, sqlcmd, szerrmsg, nLevel, tick_endpopdb-tick_beginpopdb, tick_endprov-tick_endpopdb, tick_endpushdb-tick_endprov);
	}
	
	if(ret < 0)
	{
		GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_OpenSql[%s] err:%s\r\n", dwthreadid, sqlcmd, szerrmsg);
	}
	return ret;
}

// ִ��һ���洢���̣��޷��ؽ����
// procname			�洢��������
// prochandle		�洢������Σ��������ͨ��YTData���룬��������Ļ����밴˳����
// errmsg			������Ϣ���棬ִ��ʧ��ʱ����
// msglen			������Ϣ���泤��
// ����ֵ��			>=0�ɹ� <0ʧ��
// ����˵����		prochandleͨ��YTData_NewData���أ���ʽΪfieldname, fieldtype��fieldvalue��fieldtype�μ�YTData�е�ENum_FieldType��prochandle�����ͨ��YTData_DeleteData�Ż�
int YTDb_ExecuteProc(const char * procname, void * prochandle, const char * errmsg, int msglen)
{
	// Ԥ���
	if(procname == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "����洢������Ϊ��", min(msglen-1, 18));
		}
		return -1;
	}
	DWORD dwthreadid = GetCurrentThreadId();
	
	// ��ȡdb���
	int tick_beginpopdb = GetTickCount();
	CYTDbObject * pdb = NULL;
	if(g_DbPool!=NULL && g_CurrDbIndex>=0)
	{
		pdb = g_DbPool[g_CurrDbIndex].PopDB();
	}
	if(pdb == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "��ȡdbʧ��", min(msglen-1, 10));
		}
		if(prochandle == NULL)
		{
			GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProc[%s][] err:��ȡdbʧ��\r\n", dwthreadid, procname);
		}
		else
		{
			int reqparamlen = 0;
			const char * reqparam = YTData_MakeSendData(prochandle, &reqparamlen);
			if(reqparam == NULL)
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProc[%s][] err:��ȡdbʧ��\r\n", dwthreadid, procname);
			}
			else
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProc[%s][%s] err:��ȡdbʧ��\r\n", dwthreadid, procname, reqparam);
			}
		}
		return -1;
	}
	
	// ִ�й���
	int tick_endpopdb = GetTickCount();
	char szerrmsg[1024] = {0};
	int errmsglen = 1024;
	int ret = pdb->ExecuteProc(procname, prochandle, szerrmsg, errmsglen);
	int tick_endprov = GetTickCount();
	if(errmsg!=NULL && msglen>0)
	{
		memset((char *)errmsg, 0, msglen);
		memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
	}
	
	// �����ӷŻ����ӳ�
	g_DbPool[pdb->GetIndex()].PushDB(pdb);
	int tick_endpushdb = GetTickCount();

	// ��¼��־
	int totaltick = tick_endpushdb-tick_beginpopdb;
	int nLevel = 0;
	if(totaltick >= 2000)
	{
		nLevel = 2;
	}
	else if(totaltick >= 1000)
	{
		nLevel = 1;
	}

	if(nLevel == 2)
	{
		GlobalLogout(LT_SYS, LOG_WARNING, "[%u]YTDb_ExecuteProc[%s] errmsg=%s (Level:%d)(poptime:%d)(excutetime:%d)(pushtime:%d)\r\n", dwthreadid, procname, szerrmsg, nLevel, tick_endpopdb-tick_beginpopdb, tick_endprov-tick_endpopdb, tick_endpushdb-tick_endprov);
	}
	else
	{
		GlobalLogout(LT_RUN, LOG_NORMAL, "[%u]YTDb_ExecuteProc[%s] errmsg=%s (Level:%d)(poptime:%d)(excutetime:%d)(pushtime:%d)\r\n", dwthreadid, procname, szerrmsg, nLevel, tick_endpopdb-tick_beginpopdb, tick_endprov-tick_endpopdb, tick_endpushdb-tick_endprov);
	}
	
	if(ret < 0)
	{
		if(prochandle == NULL)
		{
			GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProc[%s][] err:%s\r\n", dwthreadid, procname, szerrmsg);
		}
		else
		{
			int reqparamlen = 0;
			const char * reqparam = YTData_MakeSendData(prochandle, &reqparamlen);
			if(reqparam == NULL)
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProc[%s][] err:%s\r\n", dwthreadid, procname, szerrmsg);
			}
			else
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProc[%s][%s] err:%s\r\n", dwthreadid, procname, reqparam, szerrmsg);
			}
		}
	}
	return ret;
}

// ִ��һ���洢���̣����ؽ����
// procname			�洢��������
// prochandle		�洢������Σ��������ͨ��YTData���룬��������Ļ����밴˳����
// errmsg			������Ϣ���棬ִ��ʧ��ʱ����
// msglen			������Ϣ���泤��
// datahandle		���ؽ�����ϣ�ͨ��YTData_NewData�õ��������ͨ��YTData_DeleteData�Żأ�
// ����ֵ��			>=0�ɹ� <0ʧ��
// ����˵����		prochandleͨ��YTData_NewData���أ���ʽΪfieldname, fieldtype��fieldvalue��fieldtype�μ�YTData�е�ENum_FieldType��prochandle�����ͨ��YTData_DeleteData�Ż�
int YTDb_OpenProc(const char * procname, void * prochandle, const char * errmsg, int msglen, void * datahandle)
{
	// Ԥ���
	if(procname==NULL || datahandle==NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "����洢�����������ݼ�Ϊ��", min(msglen-1, 26));
		}
		return -1;
	}
	DWORD dwthreadid = GetCurrentThreadId();
	
	// ��ȡdb���
	int tick_beginpopdb = GetTickCount();
	CYTDbObject * pdb = NULL;
	if(g_DbPool!=NULL && g_CurrDbIndex>=0)
	{
		pdb = g_DbPool[g_CurrDbIndex].PopDB();
	}
	if(pdb == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "��ȡdbʧ��", min(msglen-1, 10));
		}
		if(prochandle == NULL)
		{
			GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_OpenProc[%s][] err:��ȡdbʧ��\r\n", dwthreadid, procname);
		}
		else
		{
			int reqparamlen = 0;
			const char * reqparam = YTData_MakeSendData(prochandle, &reqparamlen);
			if(reqparam == NULL)
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_OpenProc[%s][] err:��ȡdbʧ��\r\n", dwthreadid, procname);
			}
			else
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_OpenProc[%s][%s] err:��ȡdbʧ��\r\n", dwthreadid, procname, reqparam);
			}
		}
		return -1;
	}
	
	// ִ�й���
	int tick_endpopdb = GetTickCount();
	char szerrmsg[1024] = {0};
	int errmsglen = 1024;
	int ret = pdb->OpenProc(procname, prochandle, szerrmsg, errmsglen, datahandle);
	int tick_endprov = GetTickCount();
	if(errmsg!=NULL && msglen>0)
	{
		memset((char *)errmsg, 0, msglen);
		memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
	}
	
	// �����ӷŻ����ӳ�
	g_DbPool[pdb->GetIndex()].PushDB(pdb);
	int tick_endpushdb = GetTickCount();

	// ��¼��־
	int totaltick = tick_endpushdb-tick_beginpopdb;
	int nLevel = 0;
	if(totaltick >= 2000)
	{
		nLevel = 2;
	}
	else if(totaltick >= 1000)
	{
		nLevel = 1;
	}
	if(nLevel == 2)
	{
		GlobalLogout(LT_SYS, LOG_WARNING, "[%u]YTDb_OpenProc[%s] errmsg=%s (Level:%d)(poptime:%d)(excutetime:%d)(pushtime:%d)\r\n", dwthreadid, procname, szerrmsg, nLevel, tick_endpopdb-tick_beginpopdb, tick_endprov-tick_endpopdb, tick_endpushdb-tick_endprov);
	}
	else
	{
		GlobalLogout(LT_RUN, LOG_NORMAL, "[%u]YTDb_OpenProc[%s] errmsg=%s (Level:%d)(poptime:%d)(excutetime:%d)(pushtime:%d)\r\n", dwthreadid, procname, szerrmsg, nLevel, tick_endpopdb-tick_beginpopdb, tick_endprov-tick_endpopdb, tick_endpushdb-tick_endprov);
	}
	
	if(ret < 0)
	{
		if(prochandle == NULL)
		{
			GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_OpenProc[%s][] err:%s\r\n", dwthreadid, procname, szerrmsg);
		}
		else
		{
			int reqparamlen = 0;
			const char * reqparam = YTData_MakeSendData(prochandle, &reqparamlen);
			if(reqparam == NULL)
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_OpenProc[%s][] err:%s\r\n", dwthreadid, procname, szerrmsg);
			}
			else
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_OpenProc[%s][%s] err:%s\r\n", dwthreadid, procname, reqparam, szerrmsg);
			}
		}
	}

	return ret;
}

// ִ��һ���洢���̣��޷��ؽ������table����Σ�
// procname			�洢��������
// paraname			�洢�������table�ĵ����ṹ���ƣ�һ��Ϊobj����ע�⣺���ﲻ��tbl����
// parahandle		�洢�������table��ֵ����Ӧ�ֶ�Ϊtable�����ֶ�
// errmsg			������Ϣ���棬ִ��ʧ��ʱ����
// msglen			������Ϣ���泤��
// ����ֵ��			>=0�ɹ� <0ʧ��
// ����˵����		parahandleͨ��YTData_NewData���أ������ͨ��YTData_DeleteData�Ż�
int YTDb_ExecuteProcTable(const char * procname, const char * paraname, void * parahandle, const char * errmsg, int msglen)
{
	// Ԥ���
	if(procname == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "����洢������Ϊ��", min(msglen-1, 18));
		}
		return -1;
	}
	if(paraname==NULL || parahandle==NULL || YTData_GetRowCount(parahandle)<=0)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "����洢����table����Ϊ��", min(msglen-1, 25));
		}
		return -1;
	}
	DWORD dwthreadid = GetCurrentThreadId();
	
	// ��ȡdb���
	int tick_beginpopdb = GetTickCount();
	CYTDbObject * pdb = NULL;
	if(g_DbPool!=NULL && g_CurrDbIndex>=0)
	{
		pdb = g_DbPool[g_CurrDbIndex].PopDB();
	}
	if(pdb == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "��ȡdbʧ��", min(msglen-1, 10));
			if(parahandle == NULL)
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProcTable[%s][%s][] err:��ȡdbʧ��\r\n", dwthreadid, procname, paraname);
			}
			else
			{
				int reqparamlen = 0;
				const char * reqparam = YTData_MakeSendData(parahandle, &reqparamlen);
				if(reqparam == NULL)
				{
					GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProcTable[%s][%s][] err:��ȡdbʧ��\r\n", dwthreadid, procname, paraname);
				}
				else
				{
					GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProcTable[%s][%s][%s] err:��ȡdbʧ��\r\n", dwthreadid, procname, paraname, reqparam);
				}
			}
		}
		return -1;
	}
	
	// ִ�й���
	int tick_endpopdb = GetTickCount();
	char szerrmsg[1024] = {0};
	int errmsglen = 1024;
	int ret = pdb->ExecuteProcTable(procname, paraname, parahandle, szerrmsg, errmsglen);
	int tick_endprov = GetTickCount();
	if(errmsg!=NULL && msglen>0)
	{
		memset((char *)errmsg, 0, msglen);
		memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
	}
	
	// �����ӷŻ����ӳ�
	g_DbPool[pdb->GetIndex()].PushDB(pdb);
	int tick_endpushdb = GetTickCount();

	// ��¼��־
	int totaltick = tick_endpushdb-tick_beginpopdb;
	int nLevel = 0;
	if(totaltick >= 2000)
	{
		nLevel = 2;
	}
	else if(totaltick >= 1000)
	{
		nLevel = 1;
	}

	if(nLevel == 2)
	{
		GlobalLogout(LT_SYS, LOG_WARNING, "[%u]YTDb_ExecuteProcTable[%s] errmsg=%s (Level:%d)(poptime:%d)(excutetime:%d)(pushtime:%d)\r\n", dwthreadid, procname, szerrmsg, nLevel, tick_endpopdb-tick_beginpopdb, tick_endprov-tick_endpopdb, tick_endpushdb-tick_endprov);
	}
	else
	{
		GlobalLogout(LT_RUN, LOG_NORMAL, "[%u]YTDb_ExecuteProcTable[%s] errmsg=%s (Level:%d)(poptime:%d)(excutetime:%d)(pushtime:%d)\r\n", dwthreadid, procname, szerrmsg, nLevel, tick_endpopdb-tick_beginpopdb, tick_endprov-tick_endpopdb, tick_endpushdb-tick_endprov);
	}
	
	if(ret < 0)
	{
		if(parahandle == NULL)
		{
			GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProcTable[%s][%s][] err:%s\r\n", dwthreadid, procname, paraname, szerrmsg);
		}
		else
		{
			int reqparamlen = 0;
			const char * reqparam = YTData_MakeSendData(parahandle, &reqparamlen);
			if(reqparam == NULL)
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProcTable[%s][%s][] err:%s\r\n", dwthreadid, procname, paraname, szerrmsg);
			}
			else
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProcTable[%s][%s][%s] err:%s\r\n", dwthreadid, procname, paraname, reqparam, szerrmsg);
			}
		}
	}
	return ret;
}
