/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：YTData.h
* 文件摘要：数据对接接口
*
* Dll修改： 2015-05-29 saimen修改 V1.0
*
*/
#pragma once

#include <windows.h>

#if defined(WIN32)
	#ifdef LIB_YTDATA_API_EXPORT
		#define YTDATA_API_EXPORT extern "C" __declspec(dllexport)
	#else
		#define YTDATA_API_EXPORT extern "C" __declspec(dllimport)
	#endif
#else
	#define YTDATA_API_EXPORT extern "C" 
#endif

//---------------------------------------相关定义---------------------------------------
enum ENum_FieldType
{
	FT_STRING	= 0,	// 字符串型
	FT_CHAR		= 1,	// 字符型
	FT_INTEGER	= 2,	// 整型
	FT_DOUBLE	= 3,	// 浮点型
	FT_PASSWORD	= 4,	// 密码数据（采用blowfish加密后存储）
	FT_BINARY	= 5,	// 二进制数据（二进制数据采用Base64转换后存储）
};

//--------------------------------------DLL基础函数-------------------------------------
// 获取版本：“V1.0”格式
YTDATA_API_EXPORT const char * YTData_GetVer(void);

// DLL初始化
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_Init(void);

// DLL关闭
YTDATA_API_EXPORT void YTData_Release(void);

// ASCI转成UNICODE
// YTDATA操作取出的数据可以通过本函数转换成宽字符，主要针对fieldname还有string、char方式的fieldvalue
// 返回值：>=0转换后的长度 <0长度不够（返回-100表示长度不够需要100）
YTDATA_API_EXPORT int YTData_MByteToWChar(LPCSTR cszstr, LPWSTR wszstr, int wsize); 

// UNICODE转成ASCI
// 如果源数据未UNICODE，可以通过本函数转换成ASCI后通过YTData操作，主要针对fieldname还有string、char方式的fieldvalue
// 返回值：>=0转换后的长度 <0长度不够（返回-100表示长度不够需要100）
YTDATA_API_EXPORT int YTData_WCharToMByte(LPWSTR wszstr, LPSTR cszstr, int csize);

//--------------------------------------数据包函数--------------------------------------
// 创建句柄
// 1、该句柄标志数据信息，下面所有的和数据相关的函数请求参数中都需要传入该句柄信息
// 2、如果需要将数据包的数据全部清空做初始化则调用ResetData函数即可
// 3、返回的句柄外围不用的话则调用下面的DeleteData函数释放，外围不要做delete操作
// 返回值：			NULL失败，其他成功
YTDATA_API_EXPORT void * YTData_NewData(void);

// 重置数据
YTDATA_API_EXPORT void YTData_ResetData(void * handle);

// 删除句柄
YTDATA_API_EXPORT void YTData_DeleteData(void * handle);

// 数据包copy，入参sourcehandle表示源数据包句柄
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_CopyData(void * handle, void * sourcehandle, int startindex = 1, int endindex = 0);

// 数据合并，入参sourcehandle表示源数据包句柄
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_AppendData(void * handle, void * sourcehandle);

//----------------------------------数据与通讯缓存互转----------------------------------
// 转换数据缓存：该函数主要将传入缓存数据转换成标准数据结构
// 通讯数据传输后得到的缓存通过本函数转换成指定的数据结构
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_ConvertData(void * handle, const char * data, int len);

// 生成数据缓存：该函数主要将数据结构转换成缓存信息以便发送
// 注意：这里返回的数据缓存DLL内部管理，外围不要做delete操作
// 返回值：			返回缓存指针，为NULL表示失败，len返回缓存长度
YTDATA_API_EXPORT const char * YTData_MakeSendData(void * handle, int * len);

//--------------------------------------有效性控制--------------------------------------
// 设置返回值
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_SetRetCode(void * handle, int retcode);

// 设置返回信息
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_SetRetMsg(void * handle, const char* retmsg);

// 获取返回值
YTDATA_API_EXPORT int YTData_GetRetCode(void * handle);

// 获取返回信息
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_GetRetMsg(void * handle, char * outbuf, int len);

//-------------------------------------记录条数控制-------------------------------------
// 获取记录条数
// 返回值：			>=0记录个数 <0失败
YTDATA_API_EXPORT int YTData_GetRowCount(void * handle);

// 在指定位置插入一条记录
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_InsertRow(void * handle, int rowindex);

// 删除一条指定记录
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_DeleteRow(void * handle, int rowindex);

// 在最后增加一条记录
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_AppendRow(void * handle);

// 转到第一条记录
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_GotoBeginRow(void * handle);

// 转到最后一条记录
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_GotoEndRow(void * handle);

// 转到上一条记录
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_GotoPreRow(void * handle);

// 转到下一条记录
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_GotoNextRow(void * handle);

// 转到指定的某条记录
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_GotoFixRow(void * handle, int rowindex);

//---------------------------------------字段控制---------------------------------------
// 获取字段个数
// 返回值：			>=0字段个数 <0失败
YTDATA_API_EXPORT int YTData_GetFieldCount(void * handle);

// 获取字段类型
// 返回值：			>=0字段类型（参见上面ENum_FieldType定义） <0失败
YTDATA_API_EXPORT int YTData_GetFieldType(void * handle, int fieldindex);

// 获取字段名
// 返回值：			NULL失败，其他标示字段名
YTDATA_API_EXPORT const char * YTData_GetFieldName(void * handle, int fieldindex);

// 添加一个字段
// fieldtype为字段类型，具体参见上面ENum_FieldType定义
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_AddField(void * handle, const char * fieldname, int fieldtype = FT_STRING);

// 删除一个字段
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_DeleteField(void * handle, const char * fieldname);

//--------------------------------------字段值控制--------------------------------------
// 判断字段值是否为空
YTDATA_API_EXPORT bool YTData_IsNullValue(void * handle, const char * fieldname);

// 获取字符串型数据
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_GetFieldString(void * handle, const char * fieldname, char * outbuf, int len);

// 获取字符型数据
YTDATA_API_EXPORT char YTData_GetFieldChar(void * handle, const char * fieldname);

// 获取整型数据
YTDATA_API_EXPORT int YTData_GetFieldInt(void * handle, const char * fieldname);

// 获取浮点型数据
YTDATA_API_EXPORT double YTData_GetFieldDouble(void * handle, const char * fieldname);

// 获取密码数据
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_GetFieldPwd(void * handle, const char * fieldname, char * outpwd, int pwdlen);

// 获取二进制数据
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_GetFieldBinary(void * handle, const char * fieldname, char * outbuf, int * outlen);

// 添加字符串型数据
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_AddFieldString(void * handle, const char * fieldname, const char * value);

// 添加字符型数据
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_AddFieldChar(void * handle, const char * fieldname, const char value);

// 添加整型数据
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_AddFieldInt(void * handle, const char * fieldname, int value);

// 添加浮点型数据
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_AddFieldDouble(void * handle, const char * fieldname, double value);

// 添加密码数据
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_AddFieldPwd(void * handle, const char * fieldname, const char * value);

// 添加二进制数据
// 返回值：			>=0成功 <0失败
YTDATA_API_EXPORT int YTData_AddFieldBinary(void * handle, const char * fieldname, const char * value, int len);

//-------------------------------------相关辅助功能-------------------------------------
// 根据字段值查找指定数据
// 返回值：			>=0表示指定记录位置 <0未找到对应的记录
YTDATA_API_EXPORT int YTData_SearchByField(void * handle, const char * fieldname, const char * value);
