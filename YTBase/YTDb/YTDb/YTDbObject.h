#pragma once

#include "../../../YTPublic/YTInclude/YTUnit/YTUnit.h"

#include "../../../YTPublic/YTInclude/YTBase/YTData.h"
#pragma comment(lib, "../../../YTPublic/YTInclude/YTBase/YTData.lib")

#include "../../../YTPublic/YTInclude/YTBase/ocilib.h"
#pragma comment(lib, "../../../YTPublic/YTInclude/YTBase/ociliba.lib")

// 全局函数定义（获取oracle错误信息）
void GetOracleErr(const char * errmsg, int len);

enum ENum_LogLevel					// 日志级别定义
{
	LOG_NORMAL	= 0,				// 基本日志
	LOG_WARNING	= 1,				// 警告日志
	LOG_ERROR	= 2,				// 错误日志
	LOG_SUCCESS	= 3,				// 某个特定成功消息
};

// 全局函数，日志信息
// type				日志类别，参见ENum_LogType定义
// level			日志级别，参见ENum_LogLevel定义
// log				日志信息
void GlobalLogout(int type, int level, const char * format, ...);

// 数据库信息结构
struct DBInfo
{
	char	serverip[128];	// 数据库服务器IP地址
	int		serverport;		// 数据库服务器端口
	char	dbname[128];	// 数据库名称
	char	uid[128];		// 访问用户名
	char	pwd[128];		// 访问密码
};

// 数据库操作类
class CYTDbObject
{
public:
	CYTDbObject(int index, DBInfo dbinfo);
	~CYTDbObject(void);

// 公共导出函数
public:
	int GetIndex(void);																		// 获取数据库编号

	void SetLastUseTime(int lastusetime);													// 设置最后一次使用时间
	int GetLastUseTime(void);																// 获取最后一次使用时间
	bool IsUsed(void);																		// 判断是否被使用过

	bool IsOpen(void);																		// 判断连接句柄是否有效
	void Close(void);																		// 关闭连接
	bool Connect(void);																		// 建立与数据库的连接

	int ExecuteSql(const char * sqlcmd, const char * errmsg, int msglen);					// 执行一个SQL，无返回结果集
	int OpenSql(const char * sqlcmd, const char * errmsg, int msglen, void * datahandle);	// 执行一个SQL，返回结果记录集

	int ExecuteProc(const char * procname, void * prochandle, 
					const char * errmsg, int msglen);										// 执行一个存储过程，无返回结果集
	int OpenProc(const char * procname, void * prochandle, 
				 const char * errmsg, int msglen, void * datahandle);						// 执行一个存储过程，返回结果记录集
	
	int ExecuteProcTable(const char * procname, const char * paraname, void * parahandle, 
						 const char * errmsg, int msglen);									// 执行一个存储过程，无返回结果集（table入参）

// 私有成员
private:
	int					m_index;		// 数据库编号
	DBInfo				m_dbinfo;		// 数据库信息
	bool				m_bused;		// 是否使用过（该标志主要控制服务器初始化时间很长时会删除已建立的数据库连接的问题）
	int					m_lastusetime;	// 上次使用时间
	OCI_Connection *	m_ociconnect;	// 连接对象指针
};

extern	CYTLogFile	g_LogFile;			// 日志文件
extern	int			g_CurrDbIndex;		// 数据库状态索引

