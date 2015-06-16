/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYSocket.h
* 文件摘要：通讯socket操作
*
* 修改记录：2014-05-22 saimen创建
*/
#ifndef __BYSOCKET_H__
#define __BYSOCKET_H__

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
	long 	port;					// 代理服务器端口
	bool	auth;					// 是否需要认证
	char	name[255];				// 用户名
	long 	namelen;				// 用户名长度
	char	pwd[255];				// 密码
	long	pwdlen;					// 密码长度
} tagProxy;

#pragma pack()

// 通讯socket类
class CBYSocket
{
public:
	CBYSocket();
	virtual ~CBYSocket();

public:
	// 静态导出函数
	static long InitCondition();										// 初始化整个socket环境
	static void ReleaseCondition();										// 释放整个socket环境
	
public:
	// 公共导出函数
	void SetProxyInfo(long proxytype, tagProxy proxyinfo);				// 设置代理信息

	long Create();														// 创建socket
	long Connect(char * ip, long port, long timeout = 0);				// 建立连接
	long Close();														// 关闭连接

	long Renew(long sock, long type, long status);						// 赋于一个已有的socket	
	long Reset();														// 重置对象（注意：这里重置仅仅是释放资源，而并不关闭socket）

	long SetOption(int level, int name, const char * value, int len);	// 设置相关属性
	long Select(long type, long timeout);								// 读写异常等状态检测
	bool IsValid();														// 判断socket是否有效
	bool IsClose();														// select可读后判断是否连接已关闭，需在select可读后立刻调用
	
	long SetType(long type);											// 设置socket类型
	long GetType();														// 获取socket类型
	long GetStatus();													// 获取socket状态
	long GetSocket();													// 获取socket对象

	long Send(char * data, long len, long timeout = 10);				// 发送数据
	long Recv(char * data, long len, long timeout = 10, long type = 0);	// 接收数据

private:
	// 辅助函数
	long atoSockaddr(char * ip, long port, struct sockaddr_in * addr);	// 地址结构转换

	unsigned char Chr2Base(char c);
	char Base2Chr(unsigned char c);
	long Base64Encode(const char * inbuf, char * outbuf);				// base64加密
	long Base64Decode(const char * inbuf, char * outbuf);				// base64解密
	
	long ConnectSock4(char * ip, long port, long timeout = 0);			// sock4代理连接
	long ConnectSock5(char * ip, long port, long timeout = 0);			// sock5代理连接
	long ConnectHttp(char * ip, long port, long timeout = 0);			// http代理连接

public:
	// 共有成员
	long		m_socket;		// socket对象

protected:
	// 受保护访问对象
	long		m_proxytype;	// 代理类型
	tagProxy	m_proxyinfo;	// 代理信息

public:
	// 私有成员
	long		m_type;			// socket类型，0-同步 1-异步
	long		m_status;		// socket状态，0-未创建 1-创建
};

#endif