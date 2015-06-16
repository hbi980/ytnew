/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYConn_Interface.h
* 文件摘要：传输通讯导出函数，具体协议结构参见cpp文件
*
* 简单的操作举例说明：
* ★ 连接配置的一般操作
*	 1、通过BYConn_NewConfig创建一个对象
*	 2、通过BYConn_LoadConfig函数从文件加载相关配置
*	 3、通过后续一系列的函数对1中的对象进行数据操作
*	 4、如果需要保存到一个文件的话则通过BYConn_SaveConfig函数实现
*	 5、通过BYConn_DelConfig函数删除对象
* ★ 连接管理的一般操作
*	 1、通过BYConn_NewConn创建一个对象
*	 2、通过BYConn_Connect函数建立连接
*	 3、通过BYConn_Send发送数据
*	 4、通过BYConn_Close函数建立连接
*	 5、通过BYConn_DelConn函数删除对象
*
* 相关注意说明：
*	 1、连接的第一个包必须是通讯认证包，只有认证通过了才能发送后面的数据
*	 2、通讯认证成功后需要将服务器返回的密钥作为传输加密密钥
*	 3、通讯的应答信息全部通过回调的形式返回
*
* Dll修改： 2014-05-22 saimen创建 V1.0.0.3
*			2014-xx-xx saimen修改 V1.0.0.4	整体优化版
*/
#ifndef __BYCONN_INTERFACE_H__
#define __BYCONN_INTERFACE_H__

#include <windows.h>

//#ifdef _WIN32
//	#ifdef BYCONN_EXPORT
//		#define BYCONN_CALL	extern "C" __declspec(dllexport)
//	#else
//		#define BYCONN_CALL extern "C" __declspec(dllimport)
//	#endif
//#else
//	#define BYCONN_CALL extern "C"
//#endif

//-------------------------------------结构体定义-------------------------------------
enum ENum_ConnStatus	// 连接状态定义
{
	CS_CLOSE		= 0,	// 未连接或连接断开
	CS_CONNING		= 1,	// 连接中
	CS_CONNED		= 2,	// 连接上
};

enum ENum_ErrCode		// 通讯错误定义
{
	EC_SUCCESS		= 0,	// 正确
	EC_CONNERR		= -1,	// 通讯错误
	EC_CONNCLOSE	= -2,	// 连接异常或断开
	EC_DATAILLEGAL	= -3,	// 数据非法
};

// 数据完全返回回调定义
// 回调返回的数据retbuf需要立刻处理，出了回调函数后数据会自动在DLL内部删除
// pConn		连接对象
// reqno		请求编号
// funcid		功能号（参见协议文档）
// errcode		错误编号，注意：这里的错误编号仅仅是通讯层的错误，业务层的错误在retbuf中自己去解析
// retbuf		应答数据集缓存，这里仅返回缓存，外围可以自动通过BYData_Interface.h中的数据结构进行转换
// len			应答数据集缓存长度
// 特别注意：	如果是通讯层错误的话则错误描述就是返回的retbuf缓存，业务层的错误需要对retbuf数据进行解析后得到
typedef void (__stdcall * pFunc_DataAllRet)(void * pConn, long reqno, unsigned long funcid, long errcode, const char * retbuf, long len);

// 数据部分返回回调定义（注意：这里不完全返回可能是丢包产生）
// 回调返回的数据retbuf需要立刻处理，出了回调函数后数据会自动在DLL内部删除
// pConn		连接对象
// reqno		请求编号
// funcid		功能号（参见协议文档）
// errno		错误编号，注意：这里的错误编号仅仅是通讯层的错误，业务层的错误在retbuf中自己去解析
// retbuf		应答数据集缓存，这里仅返回缓存，外围可以自动通过BYData_Interface.h中的数据结构进行转换
// len			应答数据集缓存长度
// 特别注意：	如果是通讯层错误的话则错误描述就是返回的retbuf缓存，业务层的错误需要对retbuf数据进行解析后得到
typedef void (__stdcall * pFunc_DataPartRet)(void * pConn, long reqno, unsigned long funcid, long errcode, const char * retbuf, long len);

// 数据超时回调定义（针对请求发送后长时间得不到应答数据的情况）
// pConn		连接对象
// reqno		请求编号
// funcid		功能号（参见协议文档）
typedef void (__stdcall * pFunc_TimeOut)(void * pConn, long reqno, unsigned long funcid);

// 连接断开回调定义
// pConn		连接对象
typedef void (__stdcall * pFunc_Close)(void * pConn);

//------------------------------------导出函数定义------------------------------------

//------------------------------------------------------------------------------------
// DLL基础函数

// 获取版本：“V1.0 B001”格式
 const char * BYConn_GetVer();

// DLL初始化
// logpath	日志路径
// errmsg	错误信息缓存
// len		缓存长度
// 返回值：	>=0成功 <0失败
 long BYConn_Init(const char * logpath, char * errmsg, long len);

// DLL关闭
 void BYConn_Release();


//------------------------------------------------------------------------------------
// 连接配置文件操作函数

// 创建句柄
// 1、该句柄标志连接配置信息，下面所有和配置相关的函数请求参数中都需要传入该句柄信息
// 2、返回的句柄外围不用的话则调用下面的DelConfig函数释放，外围不要做delete操作
// 返回值：		NULL失败，其他成功
 void * BYConn_NewConfig();

// 删除句柄
 void BYConn_DelConfig(void * cfghandle);

// 从文件中加载已有配置
// 返回值：		>=0成功 <0失败
 long BYConn_LoadConfig(void * cfghandle, const char * filename);

// 保存已有配置到文件中
// 如果filename为NULL则表示保存到之前加载时传入的文件
 long BYConn_SaveConfig(void * cfghandle, const char * filename = NULL);

// 读取字符串型数据
// 返回值：		NULL失败，其他成功
 const char * BYConn_ReadStrConfig(void * cfghandle, const char * section, const char * key, const char * defaultvalue);

// 读取整型数据
 long BYConn_ReadIntConfig(void * cfghandle, const char * section, const char * key, long defaultvalue);

// 写入字符串型数据
// 返回值：		>=0成功 <0失败
 long BYConn_WriteStrConfig(void * cfghandle, const char * section, const char * key, const char * value);

// 写入整形数据
// 返回值：		>=0成功 <0失败
long BYConn_WriteIntConfig(void * cfghandle, const char * section, const char * key, long value);


//------------------------------------------------------------------------------------
// 通讯回调函数设置（注意：以下各个回调函数不允许重复设置）

// 设置数据完全返回回调函数
void BYConn_CallBackDataAllRet(pFunc_DataAllRet callbackfun);

// 设置数据部分返回回调函数
void BYConn_CallBackDataPartRet(pFunc_DataPartRet callbackfun);

// 设置数据超时回调函数
void BYConn_CallBackTimeOut(pFunc_TimeOut callbackfun);

// 设置连接断开回调函数
void BYConn_CallBackClose(pFunc_Close callbackfun);


//------------------------------------------------------------------------------------
// 连接管理相关函数

// 创建连接句柄
// 1、该句柄标志连接对象信息，下面所有和连接相关的函数请求参数中都需要传入该句柄信息
// 2、返回的句柄外围不用的话则调用下面的DelConn函数释放，外围不要做delete操作
// confighandle	连接配置句柄，由BYConn_NewConfig产生，和连接一一对应标志（含有代理、IP地址等信息）
// userid       用户编号，没有登录时送0，登录后建多个连接送用户编号
// 返回值：		NULL失败，其他成功
void * BYConn_NewConn(void * cfghandle, long userid);

// 删除连接
void BYConn_DelConn(void * connhandle);

// 建立连接 
// cryptkey		通讯密钥
// len			通讯密钥长度
// 返回值：		>=0成功 <0失败
long BYConn_Connect(void * connhandle, const char * cryptkey, long len);

// 关闭连接
long BYConn_Close(void * connhandle);

// 获取连接状态
// 返回值：		参见上面的ENum_ConnStatus结构定义
long BYConn_GetStatus(void * connhandle);

// 获取当前连接的服务器
// 返回值：		NULL没有连接，非NULL表示连接服务器地址
const char * BYConn_GetServer(void * connhandle, long * port);

// 发送请求（数据交互采用异步方式，数据接收到以后会通过上面的各个回调函数返回）
// funcid		功能号（参见协议文档）
// data			要发送的数据（注意：这里传入仅仅是数据体，包头本DLL自己内部会做控制）
// len			数据长度
// 返回值：		如果>0表示请求编号，<=0表示发送失败
long BYConn_Send(void * connhandle, unsigned long funcid, const char * data, long len);


//------------------------------------------------------------------------------------
// 服务器数据管理相关函数

// 打包解包器包头部分数据
#pragma pack(1)

typedef struct
{
	unsigned long	ConnID;			// 会话ID
	long			UserID;			// 用户编号，用于标识管理员编号
	unsigned long	FuncID;			// 功能号（参见协议文档）
	long			ReqNo;			// 请求编号（注意：0--999999为请求编号、1000000--1999999为推送编号，其他保留）
	long			ErrNo;			// 错误编号（参见数据字典）
} tagPackHead;

#pragma pack()

// 请求数据打包：本次打包仅仅针对一个请求的应答打包，多个应答数据会产生错误
// phead		打包解包器包头部分数据
// data			要发送的数据（注意：这里传入仅仅是数据体）
// len			数据长度
// key			传输加密密钥
// keylen		传输加密密钥长度
// outbuf		传出数据：打包后的发送数据
// outlen		传出数据：缓存长度，如果长度不够的话则该值作为传出数据需要的长度
// 返回值：		>0 打包后的数据长度
//				=0 打包错误
//				<0 传入的打包后缓存长度不足，此时outlen返回需要的业务数据长度
long BYConn_Pack(tagPackHead * phead, const char * data, long len, const char * key, long keylen,const char * outbuf, long * outlen);

// 应答数据解包：每次解包出一个业务请求的
// data			接收到的数据（注意：这里的数据有包头和包体）
// len			数据长度
// key			传输加密密钥
// keylen		传输加密密钥长度
// phead		打包解包器包头部分数据
// outbuf		传出数据：解包后的业务数据（注意：这个出来的业务数据没有包头）
// outlen		传出数据：实际的业务数据的长度，如果为0则表示没有解包出数据
// 返回值：		>0 已经处理的接收数据长度，此时根据outlen解包出的业务数据长度
//				=0 还需要继续接收数据
//				<0 传入的业务数据长度不足，此时outlen返回需要的业务数据长度
long BYConn_UnPack(const char * data, long len, const char * key, long keylen, tagPackHead * phead, const char * outbuf, long * outlen);

// 生成加密密钥
// key			密钥
// len			密钥长度
// 返回值：		>0	 密钥长度
//				=0  传入的密钥缓存长度不足，此时len返回需要的密钥长度
//				<0  失败
long BYConn_GenerateKey(const char * key, long * len);

#endif
