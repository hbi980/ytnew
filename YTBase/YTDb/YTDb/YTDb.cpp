#include "../../../YTPublic/YTInclude/YTBase/YTDb.h"
#include "YTDbPool.h"

// 每次修改定义版本信息，版本记录修改信息在YTDb.h文件头部写明
#define MAIN_VER	"1.0"

// 静态全局变量，用于放置重复初始化和析构
static bool g_IsExist = false;

// 全局变量
CYTDbPool *		g_DbPool		= NULL;	// 数据库连接池
int				g_DbCount		= 0;	// 数据库个数
int				g_CurrDbIndex	= -1;	// 当前数据库索引

CYTThread		g_CheckDBThread;		// 数据库检测线程
int				g_SysStatus		= 0;	// 系统状态，0-停止 1-运行

CYTLogFile		g_LogFile;				// 日志文件

// 全局函数，日志信息
// type				日志类别，参见ENum_LogType定义
// level			日志级别，参见ENum_LogLevel定义
// log				日志信息
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
		sprintf_s(szBuffer, sizeof(szBuffer), "[基本]");
		break;
	case LOG_WARNING:
		sprintf_s(szBuffer, sizeof(szBuffer), "[警告]");
		break;
	case LOG_ERROR:
		sprintf_s(szBuffer, sizeof(szBuffer), "[错误]");
		break;
	case LOG_SUCCESS:
		sprintf_s(szBuffer, sizeof(szBuffer), "[成功]");
		break;
	}
	va_list arg_ptr;
	va_start(arg_ptr, format);
	_vsnprintf_s(szBuffer + strlen(szBuffer), sizeof(szBuffer) - strlen(szBuffer), _TRUNCATE, format, arg_ptr);
	va_end(arg_ptr);

	g_LogFile.WriteLogFormat_Time(type, szBuffer);
}

// 检测线程
unsigned int __stdcall CheckDBThread(void * pParam)
{
	int i;
	while(g_SysStatus != 0)
	{
		// 检测当前使用的数据库的状态
		if(g_CurrDbIndex<0 || !g_DbPool[g_CurrDbIndex].CheckConn())
		{
			// 如果当前数据库不通了则找下一个可以连通的数据库作为最新的
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

		// 处理空闲连接
		for(i=0; i<g_DbCount; i++)
		{
			g_DbPool[i].RemoveIdle();
		}

		// 睡眠一段时间
		::Sleep(10*1000);
	}

	return 0;	
}

// 获取版本：“V1.0”格式
const char * YTDb_GetVer(void)
{
	return MAIN_VER;
}

// DLL初始化
// processpath		进程路径（用于文件绝对路径定位）
// cfgfile			数据库配置文件
// logpath			日志路径，如果为NULL的话则默认取当前路径的log目录
// errmsg			初始化错误信息描述
// len				错误缓存长度
// 返回值：			>=0成功 <0失败
int YTDb_Init(const char * processpath, const char * cfgfile, const char * logpath, char * errmsg, int len)
{
	if(g_IsExist)
	{
		return 0;
	}
	
	// 设置日志路径
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
	
	// 开始初始化
	GlobalLogout(LT_SYS, LOG_NORMAL, "数据库模块初始化开始\r\n");

	// 初始化数据模块
	YTData_Init();

	// 检测配置文件是否存在
	if(cfgfile == NULL)
	{
		YTData_Release();
		GlobalLogout(LT_SYS, LOG_ERROR, "数据库模块初始化失败：没有配置文件\r\n");
		if(errmsg!=NULL && len>0)
		{
			strncpy((char *)errmsg, "数据库模块初始化失败：没有配置文件\r\n", len-1);
		}
		return -1;
	}

	// 得到配置文件路径
	char szFullCfgFilePath[512];
	if(processpath!=NULL && strlen(processpath)>0 && strstr(cfgfile, ":")==NULL)
	{
		sprintf_s(szFullCfgFilePath, sizeof(szFullCfgFilePath), "%s\\%s", processpath, cfgfile);
	}
	else
	{
		sprintf_s(szFullCfgFilePath, sizeof(szFullCfgFilePath), "%s", cfgfile);
	}

	// 打开配置文件
	CYTIniFile file;
	if(file.Open(szFullCfgFilePath) < 0)
	{
		YTData_Release();
		GlobalLogout(LT_SYS, LOG_ERROR, "数据库模块初始化失败：配置文件[%s]打开失败\r\n", cfgfile);
		if(errmsg!=NULL && len>0)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "数据库模块初始化失败：配置文件[%s]打开失败\r\n", cfgfile);
			memcpy(errmsg, szerrmsg, min(len-1, (int)strlen(szerrmsg)));
		}
		return -1;
	}

	// 读取数据库个数
	g_DbCount = file.ReadInteger("DB_SET", "count", 0);
	if(g_DbCount <= 0)
	{
		YTData_Release();
		GlobalLogout(LT_SYS, LOG_ERROR, "数据库模块初始化失败：数据库个数为0\r\n");
		if(errmsg!=NULL && len>0)
		{
			strncpy((char *)errmsg, "数据库模块初始化失败：数据库个数为0\r\n", len-1);
		}
		g_DbCount = 0;
		file.Close();
		return -1;
	}
	g_DbPool = new CYTDbPool[g_DbCount];

	// 初始化oracle环境
	if(OCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT|OCI_ENV_THREADED|OCI_ENV_CONTEXT) == FALSE)
	{
		YTData_Release();

		char szerrmsg[512] = {0};
		sprintf_s(szerrmsg, sizeof(szerrmsg), "数据库模块初始化失败：oci初始化失败[");
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

	// 循环读取每个数据库信息
	DBInfo	info;
	int		maxconn;
	int		iniconn;
	char	szKey[64];
	for(int i=0; i<g_DbCount; i++)
	{
		memset(&info, 0, sizeof(info));

		// 数据库服务器IP地址
		sprintf_s(szKey, sizeof(szKey), "serverip%d", i+1);
		sprintf_s(info.serverip, file.ReadString("DB_SET", szKey, ""));

		// 数据库服务器端口
		sprintf_s(szKey, sizeof(szKey), "serverport%d", i+1);
		info.serverport = file.ReadInteger("DB_SET", szKey, 1521);
		
		// 数据库名称
		sprintf_s(szKey, sizeof(szKey), "dbname%d", i+1);
		sprintf_s(info.dbname, file.ReadString("DB_SET", szKey, ""));

		// 访问用户名
		sprintf_s(szKey, sizeof(szKey), "uid%d", i+1);
		sprintf_s(info.uid, file.ReadString("DB_SET", szKey, ""));

		// 访问密码
		sprintf_s(szKey, sizeof(szKey), "pwd%d", i+1);
		sprintf_s(info.pwd, file.ReadString("DB_SET", szKey, ""));

		// 最大连接数
		sprintf_s(szKey, sizeof(szKey), "maxcount%d", i+1);
		maxconn = file.ReadInteger("DB_SET", szKey, 0);

		// 初始化建立的连接数
		sprintf_s(szKey, sizeof(szKey), "inicount%d", i+1);
		iniconn = file.ReadInteger("DB_SET", szKey, 0);

		g_DbPool[i].Init(i, info, maxconn, iniconn);
	}
	file.Close();

	// 开启检测线程
	g_SysStatus = 1;
	if(g_CheckDBThread.BeginThread(CheckDBThread, NULL, " ") < 0)
	{
		YTData_Release();

		GlobalLogout(LT_SYS, LOG_ERROR, "数据库模块初始化失败：开启检测线程失败\r\n");
		if(errmsg!=NULL && len>0)
		{
			strncpy((char *)errmsg, "数据库模块初始化失败：开启检测线程失败\r\n", len-1);
		}

		delete []g_DbPool;
		g_DbPool = NULL;

		OCI_Cleanup();
	}

	GlobalLogout(LT_SYS, LOG_SUCCESS, "数据库模块初始化成功\r\n");
	g_IsExist = true;
	return 0;
}

// DLL关闭
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
		
		GlobalLogout(LT_SYS, LOG_NORMAL, "数据库模块关闭开始\r\n");
		
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
		
		// 释放oracle环境
		if(bneedcleanoci)
		{
			OCI_Cleanup();
		}
		
		GlobalLogout(LT_SYS, LOG_SUCCESS, "数据库模块关闭成功\r\n");
	}
	__except(true)
	{
		return;
	}
}

// 获取当前活动的数据库信息
// serverip			数据库地址
// serveriplen		数据库地址最大长度
// port				数据库端口
// dbname			数据库名称
// dbnamelen		数据库名称最大长度
// maxconn			允许最大的连接数
// usedconn			正在使用的连接总数
// waitconn			等待使用的连接总数
// 返回值：			>=0成功 <0失败
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

// 检测数据库是否连通
// 返回值：			>=0连通 <0断开
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

// 执行一个SQL，无返回结果集
// sqlcmd			数据库sql语句
// errmsg			错误信息缓存，执行失败时返回
// msglen			错误信息缓存长度
// 返回值：			>=0成功 <0失败
int YTDb_ExecuteSql(const char * sqlcmd, const char * errmsg, int msglen)
{
	// 预检测
	if(sqlcmd == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "传入sql为空", min(msglen-1, 11));
		}
		return -1;
	}
	DWORD dwthreadid = GetCurrentThreadId();

	// 获取db句柄
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
			memcpy((char *)errmsg, "获取db失败", min(msglen-1, 10));
		}
		GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteSql[%s] err:获取db失败\r\n", dwthreadid, sqlcmd);
		return -1;
	}

	// 执行功能
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

	// 将连接放回连接池
	g_DbPool[pdb->GetIndex()].PushDB(pdb);
	int tick_endpushdb = GetTickCount();

	// 记录日志
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

// 执行一个SQL，返回结果记录集
// sqlcmd			数据库sql语句
// errmsg			错误信息缓存，执行失败时返回
// msglen			错误信息缓存长度
// datahandle		返回结果集合（通过YTData_NewData得到，用完后通过YTData_DeleteData放回）
// 返回值：			>=0成功 <0失败
int YTDb_OpenSql(const char * sqlcmd, const char * errmsg, int msglen, void * datahandle)
{
	// 预检测
	if(sqlcmd==NULL || datahandle==NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "传入sql或数据集为空", min(msglen-1, 19));
		}
		return -1;
	}
	DWORD dwthreadid = GetCurrentThreadId();

	// 获取db句柄
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
			memcpy((char *)errmsg, "获取db失败", min(msglen-1, 10));
		}
		GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_OpenSql[%s] err:获取db失败\r\n", dwthreadid, sqlcmd);
		return -1;
	}

	// 执行功能
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
	
	// 将连接放回连接池
	g_DbPool[pdb->GetIndex()].PushDB(pdb);
	int tick_endpushdb = GetTickCount();

	// 记录日志
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

// 执行一个存储过程，无返回结果集
// procname			存储过程名称
// prochandle		存储过程入参，这里参数通过YTData传入，多个参数的话必须按顺序传入
// errmsg			错误信息缓存，执行失败时返回
// msglen			错误信息缓存长度
// 返回值：			>=0成功 <0失败
// 补充说明：		prochandle通过YTData_NewData返回，格式为fieldname, fieldtype、fieldvalue，fieldtype参见YTData中的ENum_FieldType，prochandle用完后通过YTData_DeleteData放回
int YTDb_ExecuteProc(const char * procname, void * prochandle, const char * errmsg, int msglen)
{
	// 预检测
	if(procname == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "传入存储过程名为空", min(msglen-1, 18));
		}
		return -1;
	}
	DWORD dwthreadid = GetCurrentThreadId();
	
	// 获取db句柄
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
			memcpy((char *)errmsg, "获取db失败", min(msglen-1, 10));
		}
		if(prochandle == NULL)
		{
			GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProc[%s][] err:获取db失败\r\n", dwthreadid, procname);
		}
		else
		{
			int reqparamlen = 0;
			const char * reqparam = YTData_MakeSendData(prochandle, &reqparamlen);
			if(reqparam == NULL)
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProc[%s][] err:获取db失败\r\n", dwthreadid, procname);
			}
			else
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProc[%s][%s] err:获取db失败\r\n", dwthreadid, procname, reqparam);
			}
		}
		return -1;
	}
	
	// 执行功能
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
	
	// 将连接放回连接池
	g_DbPool[pdb->GetIndex()].PushDB(pdb);
	int tick_endpushdb = GetTickCount();

	// 记录日志
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

// 执行一个存储过程，返回结果集
// procname			存储过程名称
// prochandle		存储过程入参，这里参数通过YTData传入，多个参数的话必须按顺序传入
// errmsg			错误信息缓存，执行失败时返回
// msglen			错误信息缓存长度
// datahandle		返回结果集合（通过YTData_NewData得到，用完后通过YTData_DeleteData放回）
// 返回值：			>=0成功 <0失败
// 补充说明：		prochandle通过YTData_NewData返回，格式为fieldname, fieldtype、fieldvalue，fieldtype参见YTData中的ENum_FieldType，prochandle用完后通过YTData_DeleteData放回
int YTDb_OpenProc(const char * procname, void * prochandle, const char * errmsg, int msglen, void * datahandle)
{
	// 预检测
	if(procname==NULL || datahandle==NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "传入存储过程名或数据集为空", min(msglen-1, 26));
		}
		return -1;
	}
	DWORD dwthreadid = GetCurrentThreadId();
	
	// 获取db句柄
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
			memcpy((char *)errmsg, "获取db失败", min(msglen-1, 10));
		}
		if(prochandle == NULL)
		{
			GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_OpenProc[%s][] err:获取db失败\r\n", dwthreadid, procname);
		}
		else
		{
			int reqparamlen = 0;
			const char * reqparam = YTData_MakeSendData(prochandle, &reqparamlen);
			if(reqparam == NULL)
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_OpenProc[%s][] err:获取db失败\r\n", dwthreadid, procname);
			}
			else
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_OpenProc[%s][%s] err:获取db失败\r\n", dwthreadid, procname, reqparam);
			}
		}
		return -1;
	}
	
	// 执行功能
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
	
	// 将连接放回连接池
	g_DbPool[pdb->GetIndex()].PushDB(pdb);
	int tick_endpushdb = GetTickCount();

	// 记录日志
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

// 执行一个存储过程，无返回结果集（table型入参）
// procname			存储过程名称
// paraname			存储过程入参table的单个结构名称，一般为obj对象（注意：这里不是tbl对象）
// parahandle		存储过程入参table数值，对应字段为table定义字段
// errmsg			错误信息缓存，执行失败时返回
// msglen			错误信息缓存长度
// 返回值：			>=0成功 <0失败
// 补充说明：		parahandle通过YTData_NewData返回，用完后通过YTData_DeleteData放回
int YTDb_ExecuteProcTable(const char * procname, const char * paraname, void * parahandle, const char * errmsg, int msglen)
{
	// 预检测
	if(procname == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "传入存储过程名为空", min(msglen-1, 18));
		}
		return -1;
	}
	if(paraname==NULL || parahandle==NULL || YTData_GetRowCount(parahandle)<=0)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "传入存储过程table参数为空", min(msglen-1, 25));
		}
		return -1;
	}
	DWORD dwthreadid = GetCurrentThreadId();
	
	// 获取db句柄
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
			memcpy((char *)errmsg, "获取db失败", min(msglen-1, 10));
			if(parahandle == NULL)
			{
				GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProcTable[%s][%s][] err:获取db失败\r\n", dwthreadid, procname, paraname);
			}
			else
			{
				int reqparamlen = 0;
				const char * reqparam = YTData_MakeSendData(parahandle, &reqparamlen);
				if(reqparam == NULL)
				{
					GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProcTable[%s][%s][] err:获取db失败\r\n", dwthreadid, procname, paraname);
				}
				else
				{
					GlobalLogout(LT_SYS, LOG_ERROR, "[%u]YTDb_ExecuteProcTable[%s][%s][%s] err:获取db失败\r\n", dwthreadid, procname, paraname, reqparam);
				}
			}
		}
		return -1;
	}
	
	// 执行功能
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
	
	// 将连接放回连接池
	g_DbPool[pdb->GetIndex()].PushDB(pdb);
	int tick_endpushdb = GetTickCount();

	// 记录日志
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
