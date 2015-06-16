/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYIniFile.h
* �ļ�ժҪ�������ļ�����
*
* �޸ļ�¼��2014-05-22 saimen����
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
	// ������������
	long	Open(const char * filename);														// ���ļ�
	long	Create(const char * filename);														// �����ļ�
	void	Close();																			// �ر��ļ�
	long	Save(const char * filename = NULL);													// �����ļ�

	const char * ReadString(const char * section, const char * key, const char * defaulevalue);	// ��ȡ�ַ���������
	long	ReadInteger(const char * section, const char * key, int defaulevalue);				// ��ȡ��������
	double	ReadDouble(const char * section, const char * key, double defaulevalue);			// ��ȡ����������
	bool	ReadBool(const char * section, const char * key, bool defaulevalue);				// ��ȡbool������

	long	WriteString(const char * section, const char * key, const char * value);			// д���ַ���������
	long	WriteInteger(const char * section, const char * key, int value);					// д����������
	long	WriteDouble(const char * section, const char * key, double value);					// д�븡��������
	long	WriteBool(const char * section, const char * key, bool value);						// д��bool������

	long	AddSection(const char * section);													// ���һ����
	long	DelSection(const char * section);													// ɾ��һ����
	long	DelKey(const char * section, const char * key);										// ɾ��һ��Key��
	
private:
	// ˽�г�Ա
	char	m_filename[512];							// �ļ���
	map<string, map<string, string> >	m_mapvalue;		// �ֶκ��ֶ�ֵ��Ϣ
	bool	m_bModify;									// �ļ��Ƿ������޸�
};

#endif