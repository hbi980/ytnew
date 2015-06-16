/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：YTConn.h
* 文件摘要：通讯对接接口
*
* Dll修改： 2015-05-29 saimen修改 V1.0
*
*/
#pragma once

#include <windows.h>

#if defined(WIN32)
	#ifdef LIB_YTCONN_API_EXPORT
		#define YTCONN_API_EXPORT extern "C" __declspec(dllexport)
	#else
		#define YTCONN_API_EXPORT extern "C" __declspec(dllimport)
	#endif
#else
	#define YTCONN_API_EXPORT extern "C" 
#endif

//---------------------------------------相关定义---------------------------------------
enum ENum_ConnStatus		// 连接状态定义
{
	CS_CLOSE		= 0,	// 未连接或连接断开
	CS_CONNING		= 1,	// 连接中
	CS_CONNED		= 2,	// 连接上
};

enum ENum_ErrCode			// 通讯错误定义
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
// retbuf		应答数据集缓存，这里仅返回缓存，外围可以自动通过YTData.h中的数据结构进行转换
// len			应答数据集缓存长度
// 特别注意：如果是通讯层错误的话则错误描述就是返回的retbuf，业务层的错误需要对retbuf数据进行解析后得到
typedef void (__stdcall * pFunc_YTConnDataAllRet)(void * pConn, int reqno, unsigned int funcid, int errcode, const char * retbuf, int len);

// 数据部分返回回调定义（注意：这里不完全返回可能是丢包产生）
// 回调返回的数据retbuf需要立刻处理，出了回调函数后数据会自动在DLL内部删除
// pConn		连接对象
// reqno		请求编号
// funcid		功能号（参见协议文档）
// errcode		错误编号，注意：这里的错误编号仅仅是通讯层的错误，业务层的错误在retbuf中自己去解析
// retbuf		应答数据集缓存，这里仅返回缓存，外围可以自动通过YTData.h中的数据结构进行转换
// len			应答数据集缓存长度
// 特别注意：如果是通讯层错误的话则错误描述就是返回的retbuf，业务层的错误需要对retbuf数据进行解析后得到
typedef void (__stdcall * pFunc_YTConnDataPartRet)(void * pConn, int reqno, unsigned int funcid, int errcode, const char * retbuf, int len);

// 数据超时回调定义（针对请求发送后长时间得不到应答数据的情况）
// pConn		连接对象
// reqno		请求编号
// funcid		功能号（参见协议文档）
typedef void (__stdcall * pFunc_YTConnTimeOut)(void * pConn, int reqno, unsigned int funcid);

// 连接断开回调定义
// pConn		连接对象
typedef void (__stdcall * pFunc_YTConnClose)(void * pConn);

// 回调结构：列举所有回调函数指针
struct tagYTConnCallBackFunc
{
	pFunc_YTConnDataAllRet	Func_YTConnDataAllRet;
	pFunc_YTConnDataPartRet	Func_YTConnDataPartRet;
	pFunc_YTConnTimeOut		Func_YTConnTimeOut;
	pFunc_YTConnClose		Func_YTConnClose;
};

//--------------------------------------DLL基础函数-------------------------------------
// 获取版本：“V1.0”格式
YTCONN_API_EXPORT const char * YTConn_GetVer(void);

// DLL初始化
// processpath		进程路径（用于文件绝对路径定位）
// callbackflist	回调函数指针列表
// logpath			日志路径，如果为NULL的话则默认取当前路径的log目录
// 返回值：			>=0成功 <0失败
YTCONN_API_EXPORT int YTConn_Init(const char * processpath, tagYTConnCallBackFunc * callbackflist, const char * logpath = NULL);

// DLL关闭
YTCONN_API_EXPORT void YTConn_Release(void);

//---------------------------------连接配置文件操作函数---------------------------------
// 创建句柄
// 1、该句柄标志连接配置信息，下面所有和配置相关的函数请求参数中都需要传入该句柄信息
// 2、返回的句柄外围不用的话则调用下面的DelConfig函数释放，外围不要做delete操作
// 返回值：			NULL失败，其他成功
YTCONN_API_EXPORT void * YTConn_NewConfig(void);

// 删除句柄
YTCONN_API_EXPORT void YTConn_DelConfig(void * cfghandle);

// 从文件中加载已有配置
// 返回值：			>=0成功 <0失败
YTCONN_API_EXPORT int YTConn_LoadConfig(void * cfghandle, const char * filename);

// 保存已有配置到文件中
// 如果filename为NULL则表示保存到之前加载时传入的文件
// 返回值：			>=0成功 <0失败
YTCONN_API_EXPORT int YTConn_SaveConfig(void * cfghandle, const char * filename = NULL);

// 读取字符串型数据
// 返回值：			>=0成功 <0失败
YTCONN_API_EXPORT const char * YTConn_ReadStrConfig(void * cfghandle, const char * section, const char * key, const char * defaultvalue);

// 读取整型数据
YTCONN_API_EXPORT int YTConn_ReadIntConfig(void * cfghandle, const char * section, const char * key, int defaultvalue);

// 写入字符串型数据
// 返回值：			>=0成功 <0失败
YTCONN_API_EXPORT int YTConn_WriteStrConfig(void * cfghandle, const char * section, const char * key, const char * value);

// 写入整型数据
// 返回值：			>=0成功 <0失败
YTCONN_API_EXPORT int YTConn_WriteIntConfig(void * cfghandle, const char * section, const char * key, int value);

//-----------------------------------连接管理相关函数-----------------------------------
// 创建连接句柄
// 1、该句柄标志连接对象信息，下面所有和连接相关的函数请求参数中都需要传入该句柄信息
// 2、返回的句柄外围不用的话则调用下面的DelConn函数释放，外围不要做delete操作
// confighandle		连接配置句柄，由YTConn_NewConfig产生，和连接一一对应（含有代理、IP地址等信息）
// 返回值：			NULL失败，其他成功
YTCONN_API_EXPORT void * YTConn_NewConn(void * cfghandle);

// 删除连接
YTCONN_API_EXPORT void YTConn_DelConn(void * connhandle);

// 建立连接
// 返回值：			>=0成功 <0失败
YTCONN_API_EXPORT int YTConn_Connect(void * connhandle);

// 关闭连接
YTCONN_API_EXPORT int YTConn_Close(void * connhandle);

// 获取连接状态
// 返回值：			参见上面的ENum_ConnStatus结构定义
YTCONN_API_EXPORT int YTConn_GetStatus(void * connhandle);

// 获取当前连接的服务器
// 返回值：			NULL没有连接，其他标示连接服务器地址
YTCONN_API_EXPORT const char * YTConn_GetServer(void * connhandle, int * port);

// 发送请求（数据交互采用异步方式，数据收到后会通过上面的各个回调函数返回）
// funcid			功能号（参见协议文档）
// data				要发送的数据（注意：这里传入仅仅是数据体，包头本DLL自己内部会做控制）
// len				数据长度
// 返回值：			>=0请求编号 <0失败
YTCONN_API_EXPORT int YTConn_Send(void * connhandle, unsigned int funcid, const char * data, int len);

//----------------------------------服务器管理相关函数----------------------------------
#pragma pack(1)
typedef struct 						// 打包解包器包头部分数据
{
	unsigned int	chunnelid;		// 通道号
	unsigned int	userid;			// 用户编号
	unsigned int	funcid;			// 功能号
	unsigned int	reqserailid;	// 请求消息编号
	unsigned int	result;			// 系统处理结果
} tagYTConnPackHead;
#pragma pack()

// 请求数据打包：本次打包仅仅针对一个请求的应答打包
// phead			打包解包器包头部分
// data				要发送的数据（注意：这里传入仅仅是数据体）
// len				数据长度
// outbuf			传出数据：打包后的发送数据
// outlen			传出数据：缓存长度，如果长度不够的话则该值作为传出数据需要的长度
// 返回值:			>0 打包后的数据长度
//					=0 打包错误
//					<0 传入的打包后缓存长度不足，此时outlen返回需要的业务数据长度
YTCONN_API_EXPORT int YTConn_Pack(tagYTConnPackHead * phead, const char * data, int len, const char * outbuf, int * outlen);

// 应答数据解包
// data				接收到的数据（注意：这里的数据有包头和包体）
// len				数据长度
// phead			打包解包器包头部分数据
// outbuf			传出数据：解包后的业务数据（注意：这个出来的业务数据没有包头）
// outlen			传出数据：实际的业务数据的长度，如果为0则表示没有解包出数据
// 返回值:			>0 已经处理的接收数据长度，此时根据outlen返回解包出的业务数据长度
//					=0 还需要继续接收数据
//					<0 传入的业务数据长度不足，此时outlen返回需要的业务数据长度
YTCONN_API_EXPORT int YTConn_UnPack(const char * data, int len, tagYTConnPackHead * phead, const char * outbuf, int * outlen);