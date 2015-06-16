#pragma once

#include "../../../YTPublic/YTInclude/YTUnit/YTUnit.h"
#include <algorithm>

#include <string>
#include <vector>
using namespace std;

#pragma warning(disable : 4200)

// �ֶ�ֵ�ṹ��Ϣ
typedef struct
{
    unsigned short  count;
    char 			data[];
} DataUnit;

// �ֶ�ֵ�Ƚ���
class FindDataUnit
{
public:
	FindDataUnit(DataUnit * pData)
	{
		m_pData = pData;
	}
	bool operator()(DataUnit * other)
	{
		if(m_pData==NULL || other==NULL)
		{
			return false;
		}
		else if(m_pData->count==other->count && memcmp(m_pData->data, other->data, m_pData->count) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
private:
	DataUnit * m_pData;
};

// ��¼����Ϣ
class CYTStepSec
{
friend class CYTStep;

public:
	CYTStepSec(void);
	~CYTStepSec(void);

// ������������
public:
	int GetBytesCount();												// ��ȡ�������ݳ���
	int	GetRecordCount();												// ��ȡ��¼����

	int CopyData(CYTStepSec * sourcestep, int startindex, int endindex);// ���ݰ�copy
	int AppendData(CYTStepSec * sourcestep, int targetindex);			// ���ݰ��ϲ�

	bool IsNullValue(int fieldindex);									// �ж��ֶ�ֵ�Ƿ�Ϊ��

	int InsertField(int fieldindex);									// ��ָ��λ�����һ����¼
	int DeleteField(int fieldindex);									// ɾ��һ��ָ����¼

	int GetFieldString(int fieldindex, char * outbuf, int len);			// ��ȡ�ַ���������
	char GetFieldChar(int fieldindex);									// ��ȡ�ַ�������
	int GetFieldInt(int fieldindex);									// ��ȡ��������
	double GetFieldDouble(int fieldindex);								// ��ȡ����������

	int AddFieldString(int fieldindex, const char * value);				// ����ַ���������
	int AddFieldChar(int fieldindex, const char value);					// ����ַ�������
	int AddFieldInt(int fieldindex, int value);							// �����������
	int AddFieldDouble(int fieldindex, double value);					// ��Ӹ���������

	int SearchField(const char * value);								// �����ֶ�ֵ����ָ����¼

// �ܱ������ʶ���
protected:
	void AddData(DataUnit * data)										// �������
	{
		if(data != (DataUnit *)NULL)
		{
			m_vectvalue.push_back(data);
			m_bytescount += sizeof(DataUnit)+data->count;
		}
		else
		{
			m_vectvalue.push_back((DataUnit *)NULL);
			m_bytescount += sizeof(DataUnit);
		}
	}

	int					m_bytescount;	// ���������ܳ���
	vector<DataUnit *>	m_vectvalue;	// �ֶ�ֵ��Ϣ
	CYTLock				m_section;		// ���ݿ�����
};