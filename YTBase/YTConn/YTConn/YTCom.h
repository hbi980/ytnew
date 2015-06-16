#pragma once

#include "../../../YTPublic/YTInclude/YTUnit/YTUnit.h"

#include "../../../YTPublic/YTInclude/YTBase/YTData.h"
#pragma comment(lib, "../../../YTPublic/YTInclude/YTBase/YTData.lib")

#include "../../../YTPublic/YTInclude/YTBase/YTConn.h"

#include <stdint.h>
#include <vector>
using namespace std;

#ifndef _YTData
# define _YTData void
#endif

// 通讯数据
#pragma pack(1)
typedef struct
{
    uint16_t	ver;			//协议版本，同时用于检测大小端，默认为1
    uint32_t	len;			//数据长度，包括协议版本号
    uint32_t	funcid;			//功能号
    uint32_t	chunnelid;		//通道号
    uint32_t	serailid;		//通信序号
    uint32_t	checksum;		//校验和，主要用于检验转发、加密的消息校验
    uint32_t	userid;			//用户编号
    uint32_t	result;			//系统处理结果
    uint32_t	reqserailid;	//请求消息功能
 } USDataHeader;				// 消息头
#pragma pack()

// 服务器地址结构定义
typedef struct 
{
	char			address[64];	// 服务器地址（支持域名地址）
	int				port;			// 服务器端口
} tagServer;

// 请求结构信息
typedef struct 
{
	int				reqno;			// 请求编号
	unsigned int	funcid;			// 功能号
	int				sendtime;		// 请求发送时间
} tagReq;

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

// 全局函数，获取请求编号，在YTConn.cpp中实现
int GlobGetReqNo(void);

// 传输通讯管理控制类
class CYTCom : CYTSocket
{
public:
	CYTCom(void);
	~CYTCom(void);

// 公共导出函数
public:
	int Init(CYTIniFile * cfgfile);													// 初始化配置（对象创建后只允许调用一次）
	int ConnectServer(void);														// 连接服务器
	int CloseConnect(void);															// 关闭与服务器的连接
	int GetStatus(void);															// 获取连接状态
	int SendData(unsigned int funcid, const char * data, unsigned int len);			// 发送数据（函数内部进行组包）
	const char * GetServer(int * port);												// 获取当前连接的服务器地址

// 静态导出函数
public:
	static int Pack(tagYTConnPackHead * phead, const char * data, int len, const char * outbuf, int * outlen);	// 请求数据打包：本次打包仅仅针对一个请求的应答打包，多个应答数据会产生错误
	static int UnPack(const char * data, int len, tagYTConnPackHead * phead, const char * outbuf, int * outlen);// 应答数据解包：每次解包出一个业务请求的

// 辅助函数
private:
	bool BeginThread(void);															// 开启接收线程
	void StopThread(void);															// 停止接收线程

	int Connect_Single(void);														// 按顺序连接服务器
	int Connect_Muti(void);															// 同时连接多个服务器

	int ProvConn(char * data, int len, const char * cryptkey, int keylen);			// 连接认证处理
	int ProvData(char * data, int len);												// 数据处理
	int DataRet(int reqno, unsigned int funcid);									// 应答返回更新请求map
	int CheckTimeOut(void);															// 监测超时的请求
	void CloseAllReq(void);															// 关闭所有请求信息，目前提示客户端超时

// 静态私有函数
private:
	static unsigned int __stdcall RecvThread(void * pParam);						// 接收线程

// 私有成员
private:
	int					m_sysstatus;			// 系统状态（0-关闭 1-正常）
	int					m_currserverid;			// 当前连接的服务器编号
	CYTThread			m_thread;				// 接收线程

	unsigned int		m_chunnelid;			// 通道号（在第一个登录包中返回，除非连接断开以后每次都用该通道号）
	unsigned int		m_userid;				// 用户编号（在第一个登录包中返回，除非连接断开以后每次都用该用户编号）
	
	map<int, tagReq>	m_reqmap;				// 保存请求还没有接到应答的记录
	CYTLock				m_reqmaplock;			// 记录控制锁

	CYTLock				m_socksendlock;			// socket发送控制锁（多个请求发送时需要对socket进行控制）
	CYTLock				m_sockrecvlock;			// socket接收控制锁（多个地方需要接收数据或者监测socket时需要控制）

private:
	// 配置文件中的和连接相关的信息
	bool				m_loadcfg;				// 是否加载过配置
	vector<tagServer>	m_serveraddr;			// 服务器地址信息
	int					m_conncreatetimeout;	// 连接建立超时时间（毫秒）
	int					m_connclosetimeout;		// 连接关闭超时时间（毫秒）
	int					m_sendandrecvtimeout;	// 数据传输超时时间（秒）
	int					m_transtimeout;			// 业务超时时间（秒）
};

extern tagYTConnCallBackFunc	g_CallBackFList;	// 回调函数
extern CYTLogFile				g_LogFile;			// 日志文件

