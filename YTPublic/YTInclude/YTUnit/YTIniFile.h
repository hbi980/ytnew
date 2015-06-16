#pragma once

#pragma warning(disable : 4786)
#pragma warning(disable : 4503)

#include "YTFile.h"

#include <string>
#include <map>
using namespace std;

class CYTIniFile : public CYTFile
{
public:
	CYTIniFile(void);
	~CYTIniFile(void);

public:
	// ������������
	int		Open(const char * filename);														// ���ļ�
	int		Create(const char * filename);														// �����ļ�
	void	Close(void);																			// �ر��ļ�
	int		Save(const char * filename = NULL);													// �����ļ�

	const char * ReadString(const char * section, const char * key, const char * defaulevalue);	// ��ȡ�ַ���������
	int		ReadInteger(const char * section, const char * key, int defaulevalue);				// ��ȡ��������
	double	ReadDouble(const char * section, const char * key, double defaulevalue);			// ��ȡ����������
	bool	ReadBool(const char * section, const char * key, bool defaulevalue);				// ��ȡbool������

	int		WriteString(const char * section, const char * key, const char * value);			// д���ַ���������
	int		WriteInteger(const char * section, const char * key, int value);					// д����������
	int		WriteDouble(const char * section, const char * key, double value);					// д�븡��������
	int		WriteBool(const char * section, const char * key, bool value);						// д��bool������

	int		AddSection(const char * section);													// ���һ����
	int		DelSection(const char * section);													// ɾ��һ����
	int		DelKey(const char * section, const char * key);										// ɾ��һ��Key��
	
private:
	// ˽�г�Ա
	char	m_filename[512];							// �ļ���
	map<string, map<string, string> >	m_mapvalue;		// �ֶκ��ֶ�ֵ��Ϣ
	bool	m_bModify;									// �ļ��Ƿ������޸�
};

