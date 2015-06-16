#include "YTCom.h"
#include <time.h>

// 每次修改定义版本信息，版本记录修改信息在YTConn.h文件头部写明
#define MAIN_VER	"V1.0"

// 静态全局变量，用于放置重复初始化和析构
static bool g_IsExist = false;

// 回调全局变量
tagYTConnCallBackFunc	g_CallBackFList;			// 回调函数

// 日志文件全局变量
char					g_szProcessPath[512] = {0};	// 进程路径
CYTLogFile				g_LogFile;					// 日志文件

// 全局请求编号
unsigned int			g_reqno = 0;				// 请求编号（客户端每次请求递增送到服务端）
CYTLock					g_reqnolock;				// 请求编号控制锁

// 全局函数，获取请求编号
int GlobGetReqNo(void)
{
	int ret = 0;
	
	g_reqnolock.Lock();
	g_reqno++;
	ret = g_reqno;
	g_reqnolock.UnLock();
	
	return ret;
}

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

// 获取版本：“V1.0”格式
const char * YTConn_GetVer(void)
{
	return MAIN_VER;
}

// DLL初始化
// processpath		进程路径（用于文件绝对路径定位）
// callbackflist	回调函数指针列表
// logpath			日志路径，如果为NULL的话则默认取当前路径的log目录
// 返回值：			>=0成功 <0失败
int YTConn_Init(const char * processpath, tagYTConnCallBackFunc * callbackflist, const char * logpath)
{
	if(g_IsExist)
	{
		return 0;
	}

	// 保存进程路径
	if(processpath!=NULL && strlen(processpath)>0)
	{
		memcpy(g_szProcessPath, processpath, min(sizeof(g_szProcessPath)-1, strlen(processpath)));
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
	g_LogFile.SetFileName("ytconn");
	g_LogFile.SetLogType(LT_RUN);

	// 开始初始化
	GlobalLogout(LT_SYS, LOG_NORMAL, "通讯模块初始化开始\r\n");

	YTData_Init();
	CYTSocket::InitCondition();

	if(callbackflist != NULL)
	{
		memcpy(&g_CallBackFList, callbackflist, sizeof(tagYTConnCallBackFunc));
	}

	g_reqno	= 0;

	GlobalLogout(LT_SYS, LOG_SUCCESS, "通讯模块初始化成功\r\n");

	g_IsExist = true;
	return 0;
}

// DLL关闭
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
	
	GlobalLogout(LT_SYS, LOG_NORMAL, "通讯模块关闭开始\r\n");

	YTData_Release();
	CYTSocket::ReleaseCondition();

	GlobalLogout(LT_SYS, LOG_SUCCESS, "通讯模块关闭成功\r\n");
}

// 创建句柄
// 1、该句柄标志连接配置信息，下面所有和配置相关的函数请求参数中都需要传入该句柄信息
// 2、返回的句柄外围不用的话则调用下面的DelConfig函数释放，外围不要做delete操作
// 返回值：			NULL失败，其他成功
void * YTConn_NewConfig(void)
{
	CYTIniFile * ret = new CYTIniFile;
	return ret;
}

// 删除句柄
void YTConn_DelConfig(void * cfghandle)
{
	CYTIniFile * pYTIniFile = (CYTIniFile *)cfghandle;
	if(pYTIniFile != NULL)
	{
		delete pYTIniFile;
		pYTIniFile = NULL;
	}
}

// 从文件中加载已有配置
// 返回值：			>=0成功 <0失败
int YTConn_LoadConfig(void * cfghandle, const char * filename)
{
	CYTIniFile * pYTIniFile = (CYTIniFile *)cfghandle;
	if(pYTIniFile != NULL)
	{
		// 获取全路径
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

// 保存已有配置到文件中
// 如果filename为NULL则表示保存到之前加载时传入的文件
// 返回值：			>=0成功 <0失败
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

// 读取字符串型数据
// 返回值：			>=0成功 <0失败
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

// 读取整型数据
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

// 写入字符串型数据
// 返回值：			>=0成功 <0失败
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

// 写入整型数据
// 返回值：			>=0成功 <0失败
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

// 创建连接句柄
// 1、该句柄标志连接对象信息，下面所有和连接相关的函数请求参数中都需要传入该句柄信息
// 2、返回的句柄外围不用的话则调用下面的DelConn函数释放，外围不要做delete操作
// confighandle		连接配置句柄，由YTConn_NewConfig产生，和连接一一对应（含有代理、IP地址等信息）
// 返回值：			NULL失败，其他成功
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

// 删除连接
void YTConn_DelConn(void * connhandle)
{
	CYTCom * pYTCom = (CYTCom *)connhandle;
	if(pYTCom != NULL)
	{
		delete pYTCom;
		pYTCom = NULL;
	}
}

// 建立连接
// 返回值：			>=0成功 <0失败
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

// 关闭连接
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

// 获取连接状态
// 返回值：			参见上面的ENum_ConnStatus结构定义
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

// 获取当前连接的服务器
// 返回值：			NULL没有连接，其他标示连接服务器地址
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

// 发送请求（数据交互采用异步方式，数据收到后会通过上面的各个回调函数返回）
// funcid			功能号（参见协议文档）
// data				要发送的数据（注意：这里传入仅仅是数据体，包头本DLL自己内部会做控制）
// len				数据长度
// 返回值：			>=0请求编号 <0失败
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

// 请求数据打包：本次打包仅仅针对一个请求的应答打包
// phead			打包解包器包头部分
// data				要发送的数据（注意：这里传入仅仅是数据体）
// len				数据长度
// outbuf			传出数据：打包后的发送数据
// outlen			传出数据：缓存长度，如果长度不够的话则该值作为传出数据需要的长度
// 返回值:			>0 打包后的数据长度
//					=0 打包错误
//					<0 传入的打包后缓存长度不足，此时outlen返回需要的业务数据长度
int YTConn_Pack(tagYTConnPackHead * phead, const char * data, int len, const char * outbuf, int * outlen)
{
	return CYTCom::Pack(phead, data, len, outbuf, outlen);
}

// 应答数据解包
// data				接收到的数据（注意：这里的数据有包头和包体）
// len				数据长度
// phead			打包解包器包头部分数据
// outbuf			传出数据：解包后的业务数据（注意：这个出来的业务数据没有包头）
// outlen			传出数据：实际的业务数据的长度，如果为0则表示没有解包出数据
// 返回值:			>0 已经处理的接收数据长度，此时根据outlen返回解包出的业务数据长度
//					=0 还需要继续接收数据
//					<0 传入的业务数据长度不足，此时outlen返回需要的业务数据长度
int YTConn_UnPack(const char * data, int len, tagYTConnPackHead * phead, const char * outbuf, int * outlen)
{
	return CYTCom::UnPack(data, len, phead, outbuf, outlen);
}