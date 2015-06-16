#pragma once

#include <windows.h>

// 检测类型定义
enum ENum_SockSel
{
	SOCKSEL_READ		= 0x01,		// 读
	SOCKSEL_WRITE		= 0x02,		// 写
	SOCKSEL_READWRITE	= 0x03,		// 读写
	SOCKSEL_EXCEPTION	= 0x04,		// 异常
	SOCKSEL_ALL			= 0x07,		// 全部
};

// 代理类型定义
enum ENum_SockProxy
{
	SOCKPROXY_NONE		= 0x00,		// 不使用代理
	SOCKPROXY_SOCK4		= 0x01,		// sock4代理
	SOCKPROXY_SOCK5		= 0x02,		// sock5代理
	SOCKPROXY_HTTP		= 0x03,		// http代理
};

// 代理结构定义
#pragma pack(1)

typedef struct
{
	char 	ip[64];					// 代理服务器ip
	int 	port;					// 代理服务器端口
	bool	auth;					// 是否需要认证
	char	name[255];				// 用户名
	int 	namelen;				// 用户名长度
	char	pwd[255];				// 密码
	int		pwdlen;					// 密码长度
} tagProxy;

#pragma pack()

// 通讯socket类
class CYTSocket
{
public:
	CYTSocket(void);
	~CYTSocket(void);

public:
	// 静态导出函数
	static int InitCondition(void);										// 初始化整个socket环境
	static void ReleaseCondition(void);									// 释放整个socket环境
	
public:
	// 公共导出函数
	void SetProxyInfo(int proxytype, tagProxy proxyinfo);				// 设置代理信息

	int  Create(void);													// 创建socket
	int  Connect(char * ip, int port, int timeout = 0);					// 建立连接
	int  Close(void);													// 关闭连接

	int  Renew(int sock, int type, int status);							// 赋于一个已有的socket	
	int  Reset(void);													// 重置对象（注意：这里重置仅仅是释放资源，而并不关闭socket）

	int  SetOption(int level, int name, const char * value, int len);	// 设置相关属性
	int  Select(int type, int timeout);									// 读写异常等状态检测
	bool IsValid(void);													// 判断socket是否有效
	bool IsClose(void);													// select可读后判断是否连接已关闭，需在select可读后立刻调用
	
	int  SetType(int type);												// 设置socket类型
	int  GetType(void);													// 获取socket类型
	int  GetStatus(void);												// 获取socket状态
	int  GetSocket(void);												// 获取socket对象

	int  Send(char * data, int len, int timeout = 10);					// 发送数据
	int  Recv(char * data, int len, int timeout = 10, int type = 0);	// 接收数据

private:
	// 辅助函数
	int  atoSockaddr(char * ip, int port, struct sockaddr_in * addr);	// 地址结构转换

	unsigned char Chr2Base(char c);
	char Base2Chr(unsigned char c);
	int  Base64Encode(const char * inbuf, char * outbuf);				// base64加密
	int  Base64Decode(const char * inbuf, char * outbuf);				// base64解密
	
	int  ConnectSock4(char * ip, int port, int timeout = 0);			// sock4代理连接
	int  ConnectSock5(char * ip, int port, int timeout = 0);			// sock5代理连接
	int  ConnectHttp(char * ip, int port, int timeout = 0);				// http代理连接

public:
	// 共有成员
	int			m_socket;		// socket对象

protected:
	// 受保护访问对象
	int			m_proxytype;	// 代理类型
	tagProxy	m_proxyinfo;	// 代理信息

public:
	// 私有成员
	int			m_type;			// socket类型，0-同步 1-异步
	int			m_status;		// socket状态，0-未创建 1-创建
};

