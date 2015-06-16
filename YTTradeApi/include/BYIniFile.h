/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：BYIniFile.h
* 文件摘要：配置文件控制
*
* 修改记录：2014-05-22 saimen创建
*/
#ifndef __BYINIFILE_H__
#define __BYINIFILE_H__

#pragma warning(disable : 4786)
#pragma warning(disable : 4503)

#include "BYFile.h"

#include <string>
#include <map>
using namespace std;

class CBYIniFile : public CBYFile
{
public:
	CBYIniFile();
	virtual ~CBYIniFile();
	
public:
	// 公共导出函数
	long	Open(const char * filename);														// 打开文件
	long	Create(const char * filename);														// 创建文件
	void	Close();																			// 关闭文件
	long	Save(const char * filename = NULL);													// 保存文件

	const char * ReadString(const char * section, const char * key, const char * defaulevalue);	// 读取字符串型数据
	long	ReadInteger(const char * section, const char * key, int defaulevalue);				// 读取整型数据
	double	ReadDouble(const char * section, const char * key, double defaulevalue);			// 读取浮点型数据
	bool	ReadBool(const char * section, const char * key, bool defaulevalue);				// 读取bool型数据

	long	WriteString(const char * section, const char * key, const char * value);			// 写入字符串型数据
	long	WriteInteger(const char * section, const char * key, int value);					// 写入整型数据
	long	WriteDouble(const char * section, const char * key, double value);					// 写入浮点型数据
	long	WriteBool(const char * section, const char * key, bool value);						// 写入bool型数据

	long	AddSection(const char * section);													// 添加一个段
	long	DelSection(const char * section);													// 删除一个段
	long	DelKey(const char * section, const char * key);										// 删除一个Key项
	
private:
	// 私有成员
	char	m_filename[512];							// 文件名
	map<string, map<string, string> >	m_mapvalue;		// 字段和字段值信息
	bool	m_bModify;									// 文件是否做过修改
};

#endif