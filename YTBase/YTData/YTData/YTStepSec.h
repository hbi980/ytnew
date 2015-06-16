#pragma once

#include "../../../YTPublic/YTInclude/YTUnit/YTUnit.h"
#include <algorithm>

#include <string>
#include <vector>
using namespace std;

#pragma warning(disable : 4200)

// 字段值结构信息
typedef struct
{
    unsigned short  count;
    char 			data[];
} DataUnit;

// 字段值比较类
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

// 记录集信息
class CYTStepSec
{
friend class CYTStep;

public:
	CYTStepSec(void);
	~CYTStepSec(void);

// 公共导出函数
public:
	int GetBytesCount();												// 获取具体数据长度
	int	GetRecordCount();												// 获取记录个数

	int CopyData(CYTStepSec * sourcestep, int startindex, int endindex);// 数据包copy
	int AppendData(CYTStepSec * sourcestep, int targetindex);			// 数据包合并

	bool IsNullValue(int fieldindex);									// 判断字段值是否为空

	int InsertField(int fieldindex);									// 在指定位置添加一条记录
	int DeleteField(int fieldindex);									// 删除一条指定记录

	int GetFieldString(int fieldindex, char * outbuf, int len);			// 获取字符串型数据
	char GetFieldChar(int fieldindex);									// 获取字符型数据
	int GetFieldInt(int fieldindex);									// 获取整型数据
	double GetFieldDouble(int fieldindex);								// 获取浮点型数据

	int AddFieldString(int fieldindex, const char * value);				// 添加字符串型数据
	int AddFieldChar(int fieldindex, const char value);					// 添加字符型数据
	int AddFieldInt(int fieldindex, int value);							// 添加整型数据
	int AddFieldDouble(int fieldindex, double value);					// 添加浮点型数据

	int SearchField(const char * value);								// 根据字段值查找指定记录

// 受保护访问对象
protected:
	void AddData(DataUnit * data)										// 添加数据
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

	int					m_bytescount;	// 具体数据总长度
	vector<DataUnit *>	m_vectvalue;	// 字段值信息
	CYTLock				m_section;		// 数据控制锁
};