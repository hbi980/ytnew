#pragma once

#include "YTFile.h"
#include "YTLock.h"
#include "YTThread.h"

#define MAX_LOG_BUFFER	1024*20

// 日志类型定义
enum ENum_LogType
{
	LT_SYS		= 0x00,		// 系统日志，记录系统启动关闭以及关键位置的异常等信息
	LT_RUN		= 0x01,		// 运行日志，揭示用户登录关闭等情况
	LT_DEBUG	= 0x02,		// 调试日志，为了揭示程序异常打印一些的测试日志
};

class CYTLogFile : public CYTFile
{
public:
	CYTLogFile(int logwritetime = 20);
	~CYTLogFile(void);

public:
	// 公共导出函数
	void SetFileName(const char * filename);								// 设置日志文件名前缀
	void SetDirectory(const char * directory);								// 设置日志文件目录
	void SetLogType(int maxtype);											// 最大日志级别，设置后只打印<=设置的级别的日志

	void WriteLog(int logtype, const char * log, int loglen = 0);			// 写入一个字符串
	void WriteLog_Time(int logtype, const char * log, int loglen = 0);		// 写入一个字符串（带时间记录）
	void WriteLogFormat(int logtype, const char * format, ...);				// 写入一个带有格式的数据
	void WriteLogFormat_Time(int logtype, const char * format, ...);		// 写入一个带有格式的数据（带时间记录）

private:
	// 辅助函数
	bool CheckFile(void);													// 检测文件，如没有则创建
	void FlushLogBuffer(void);												// 将临时缓存中的信息写入文件
	void WriteToFile(const char * log, int loglen);							// 写入日志

	static unsigned int __stdcall LogWriteThread(void * pParam);			// 文件定期写入线程
	
private:
	// 私有成员
	int			m_maxtype;							// 最大日志级别，设置后只打印<=设置的级别的日志
	char		m_Name[128];						// 日志文件名前缀
	char		m_Directory[256];					// 日志文件目录
	
	int			m_logdate;							// 当前文件日期
	char		m_logbuffer[MAX_LOG_BUFFER+1];		// 日志临时缓存
	int			m_loglen;							// 日志临时缓存长度
	CYTLock		m_lock;								// 文件操作控制锁

	int			m_logwritetime;						// 写缓存到文件的时间控制
	int			m_stopflag;							// 线程退出标志
	CYTThread	m_thread;							// 定时写缓存到文件的控制线程
};

