/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYData_Interface.h
* 文件摘要：数据解析导出函数，具体协议结构参见cpp文件
*
* 简单的操作举例说明：
* ★ 接收到数据的一般操作
*	 1、通过BYData_NewData创建一个对象
*	 2、通过BYData_ConvertData函数将接收到的数据给1中的对象进行数据结构转换
*	 3、通过后续一系列的函数对1中的对象进行数据操作
*	 4、通过BYData_DeleteData函数删除对象
* ★ 需要建立发送数据
*	 1、通过BYData_NewData创建一个对象
*	 2、通过一系列的数据操作给1中的数据对象赋值
*	 3、通过BYData_MakeSendData函数生成数据发送缓存进行数据发送
*	 4、通过BYData_DeleteData函数删除对象（注意：一旦删除对象3中生成的数据缓存就自动消失了）
* ★ 简单的数据循环获取
*	 1、通过BYData_GetFieldString、BYData_GetFieldChar等函数获取字段值
*	 2、通过BYData_GotoNextRow转到下一条记录，然后回到1的操作，直到BYData_GotoNextRow失败则表示数据已经到最后
* ★ 简单的数据循环添加
*	 1、通过BYData_AppendRow添加一行
*	 2、通过BYData_AddFieldString、BYData_AddFieldChar等函数设置字段对应的数据，如有多条数据继续到1
*
* Dll修改： 2014-05-22 saimen创建 V1.0.0.3
*			2014-xx-xx saimen修改 V1.0.0.4	整体优化版，增加ASCI和UNICODE互转的函数
*/
#ifndef __BYDATA_INTERFACE_H__
#define __BYDATA_INTERFACE_H__

#include <windows.h>

#ifdef _WIN32
	#ifdef BYDATA_EXPORT
		#define BYDATA_CALL	extern "C" __declspec(dllexport)
	#else
		#define BYDATA_CALL extern "C" __declspec(dllimport)
	#endif
#else
	#define BYDATA_CALL extern "C"
#endif

//-------------------------------------结构体定义-------------------------------------
enum ENum_FieldType		// 字段类型定义
{
	FT_STRING	= 0,	// 字符串型
	FT_CHAR		= 1,	// 字符型
	FT_INTEGER	= 2,	// 整型
	FT_DOUBLE	= 3,	// 浮点型
	FT_PASSWORD	= 4,	// 密码数据（采用blowfish加密后存储）
	FT_BINARY	= 5,	// 二进制数据（二进制数据采用Base64转换后存储）
};

enum ENum_SortType		// 排序方式定义
{
	ST_NULL		= 0,	// 保留
	ST_DOWN		= 1,	// 降序
	ST_UP		= 2,	// 升序
};

//------------------------------------导出函数定义------------------------------------

//------------------------------------------------------------------------------------
// DLL基础函数

// 获取版本：“V1.0 B001”格式
const char * BYData_GetVer();

// DLL初始化
// errmsg	错误信息缓存
// len		缓存长度
// 返回值：	>=0成功 <0失败
long BYData_Init(char * errmsg, long len);

// DLL关闭
void BYData_Release();

// ASCI转成UNICODE
// BYDATA操作取出的数据可通过本函数转换成宽字符，主要针对fieldname还有string、char方式的fieldvalue
// 返回值：>=0转换后的长度 <0长度不够（返回-100就表示长度不够，需要100） 
long BYData_MByteToWChar(LPCSTR cszstr, LPWSTR wszstr, long wsize); 

// UNICODE转成ASCI
// 如果源数据为UNICODE，可通过本函数转换成ASCI后进行BYDATA操作，主要针对fieldname还有string、char方式的fieldvalue
// 返回值：>=0转换后的长度 <0长度不够（返回-100就表示长度不够，需要100） 
long BYData_WCharToMByte(LPWSTR wszstr, LPCSTR cszstr, long csize);


//------------------------------------------------------------------------------------
// 数据包基础函数

// 创建句柄
// 1、该句柄标志数据信息，下面所有和数据相关的函数请求参数中都需要传入该句柄信息
// 2、如果需要将数据包的数据全部清空做初始化则调用ResetData函数即可
// 3、返回的句柄外围不用的话则调用下面的DeleteData函数释放，外围不要做delete操作
// 返回值：NULL失败，其他成功
void * BYData_NewData();

// 重置数据
void BYData_ResetData(void * handle);

// 删除句柄
void BYData_DeleteData(void * handle);

// 数据包copy，入参sourcehandle表示源数据包句柄
// 返回值：>=0成功 <0失败
long BYData_CopyData(void * handle, void * sourcehandle, long index = 1);

// 数据合并，入参sourcehandle表示源数据包句柄
// 返回值：>=0成功 <0失败
long BYData_AppendData(void * handle, void * sourcehandle);

//------------------------------------------------------------------------------------
// 数据包与通讯缓存互转

// 转换数据缓存：该函数主要将传入缓存数据转换成标准数据结构
// 通讯数据传输后得到的缓存通过本函数转换成指定的数据结构
// 返回值：>=0成功 <0失败
long BYData_ConvertData(void * handle, const char * data, long len);

// 生成数据缓存：该函数主要将数据结构转换成缓存信息以便发送
// 注意：这里返回的数据缓存DLL内部管理，外围不要做delete操作
// 返回值：返回缓存指针，为NULL表示失败，len返回缓存长度
const char * BYData_MakeSendData(void * handle, long * len);


//------------------------------------------------------------------------------------
// 记录条数控制

// 获取记录条数
// 返回值：>=0记录个数 <0失败
long BYData_GetRowCount(void * handle);

// 在指定位置插入一条记录
// 返回值：>=0成功 <0失败
long BYData_InsertRow(void * handle, long rowindex);

// 删除一条指定记录
// 返回值：>=0成功 <0失败
long BYData_DeleteRow(void * handle, long rowindex);

// 在最后增加一条记录
// 返回值：>=0成功 <0失败
long BYData_AppendRow(void * handle);

// 转到第一条记录
// 返回值：>=0成功 <0失败
long BYData_GotoBeginRow(void * handle);

// 转到最后一条记录
// 返回值：>=0成功 <0失败
long BYData_GotoEndRow(void * handle);

// 转到上一条记录
// 返回值：>=0成功 <0失败
long BYData_GotoPreRow(void * handle);

// 转到下一条记录
// 返回值：>=0成功 <0失败
long BYData_GotoNextRow(void * handle);

// 转到指定的某条记录
// 返回值：>=0成功 <0失败
long BYData_GotoFixRow(void * handle, long rowindex);


//------------------------------------------------------------------------------------
// 字段控制

// 获取字段个数
// 返回值：>=0成功 <0失败
long BYData_GetFieldCount(void * handle);

// 获取字段类型
// 返回值：>=0字段类型（参见上面ENum_FieldType定义） <0失败
long BYData_GetFieldType(void * handle, long fieldindex);

// 获取字段名
// 返回值：NULL失败，其他表示字段名
const char * BYData_GetFieldName(void * handle, long fieldindex);

// 添加一个字段
// fieldtype为字段类型，具体参见上面ENum_FieldType定义
// 返回值：>=0成功 <0失败
long BYData_AddField(void * handle, const char * fieldname, long fieldtype = FT_STRING);

// 删除一个字段
long BYData_DeleteField(void * handle, const char * fieldname);


//------------------------------------------------------------------------------------
// 字段值控制

// 判断字段值是否为空
bool BYData_IsNullValue(void * handle, const char * fieldname);

// 获取字符串型数据
long BYData_GetFieldString(void * handle, const char * fieldname, char * outbuf, long len);

// 获取字符型数据
char BYData_GetFieldChar(void * handle, const char * fieldname);

// 获取整型数据
long BYData_GetFieldInt(void * handle, const char * fieldname);

// 获取浮点型数据
double BYData_GetFieldDouble(void * handle, const char * fieldname);

// 获取密码数据
// 返回值：>=0成功 <0失败
long BYData_GetFieldPwd(void * handle, const char * fieldname, char * outpwd, long pwdlen);

// 获取二进制数据
// 返回值：>=0成功 <0失败
long BYData_GetFieldBinary(void * handle, const char * fieldname, char * outbuf, long * outlen);

// 添加字符串型数据
// 返回值：>=0成功 <0失败
long BYData_AddFieldString(void * handle, const char * fieldname, const char * value);

// 添加字符型数据
// 返回值：>=0成功 <0失败
long BYData_AddFieldChar(void * handle, const char * fieldname, const char value);

// 添加整型数据
// 返回值：>=0成功 <0失败
long BYData_AddFieldInt(void * handle, const char * fieldname, long value);

// 添加浮点型数据
// 返回值：>=0成功 <0失败
long BYData_AddFieldDouble(void * handle, const char * fieldname, double value);

// 添加密码数据
// 返回值：>=0成功 <0失败
long BYData_AddFieldPwd(void * handle, const char * fieldname, const char * value);

// 添加二进制数据
// 返回值：>=0成功 <0失败
long BYData_AddFieldBinary(void * handle, const char * fieldname, const char * value, long len);


//------------------------------------------------------------------------------------
// 辅助功能

// 根据字段值查找指定记录
// 返回置：>=0表示指定记录位置 <0未找到记录
long BYData_SearchByField(void * handle, const char * fieldname, const char * value);

// 根据某个字段排序（暂未实现）
// sorttype表示排序方式，具体参见上面的ENum_SortType
// 返回值：>=0成功 <0失败
long BYData_SortByField(void * handle, const char * fieldname, long sorttype = ST_DOWN);

#endif
