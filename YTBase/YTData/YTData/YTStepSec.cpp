#include "YTStepSec.h"

CYTStepSec::CYTStepSec(void)
{
	m_bytescount = 0;
}

CYTStepSec::~CYTStepSec(void)
{
	m_section.Lock();
	vector<DataUnit *>::iterator iter = m_vectvalue.begin();
	while(iter != m_vectvalue.end())
	{
		if(*iter != (DataUnit *)NULL)
		{
			free(*iter);
			*iter = (DataUnit *)NULL;
		}
		iter = m_vectvalue.erase(iter);
	}
	m_bytescount = 0;
	m_section.UnLock();
}

// 获取具体数据长度
int CYTStepSec::GetBytesCount()
{
	return m_bytescount;
}

// 获取记录个数
int	CYTStepSec::GetRecordCount()
{
	return m_vectvalue.size();
}

// 数据包copy
int CYTStepSec::CopyData(CYTStepSec * sourcestep, int startindex, int endindex)
{
	if(sourcestep == NULL)
	{
		return -1;
	}

	// 加锁
	m_section.Lock();
	sourcestep->m_section.Lock();

	// 清空原始数据
	vector<DataUnit *>::iterator iter = m_vectvalue.begin();
	while(iter != m_vectvalue.end())
	{
		if(*iter != (DataUnit *)NULL)
		{
			free(*iter);
			*iter = (DataUnit *)NULL;
		}
		iter = m_vectvalue.erase(iter);
	}
	m_bytescount = 0;

	// 判断是否超出限制
	if(startindex >= (int)sourcestep->m_vectvalue.size())
	{
		sourcestep->m_section.UnLock();
		m_section.UnLock();
		return -1;
	}
	
	// 数据copy
	int tmpcount;
	DataUnit * pData;
	int count = sourcestep->m_vectvalue.size();
	if(endindex >= 0)
	{
		count = min((int)sourcestep->m_vectvalue.size(), endindex+1);
	}
	for(int i=startindex; i<count; i++)
	{
		if(sourcestep->m_vectvalue[i] != (DataUnit *)NULL)
		{
			tmpcount = sizeof(DataUnit)+sourcestep->m_vectvalue[i]->count;
			pData = (DataUnit *)malloc(tmpcount);
			if(pData != (DataUnit *)NULL)
			{
				m_bytescount += tmpcount;
				memcpy((char *)pData, (char *)sourcestep->m_vectvalue[i], tmpcount);
				m_vectvalue.push_back(pData);
			}
			else
			{
				m_bytescount += sizeof(DataUnit);
				m_vectvalue.push_back((DataUnit *)NULL);
			}
		}
		else
		{
			m_bytescount += sizeof(DataUnit);
			m_vectvalue.push_back((DataUnit *)NULL);
		}
	}
	
	sourcestep->m_section.UnLock();
	m_section.UnLock();
	
	return 0;
}

// 数据包合并
int CYTStepSec::AppendData(CYTStepSec * sourcestep, int targetindex)
{
	if(sourcestep == NULL)
	{
		return -1;
	}
	if(targetindex < 0)
	{
		targetindex = 0;
	}
	
	// 加锁
	m_section.Lock();
	sourcestep->m_section.Lock();
	
	// 获取要添加的数据长度
	int count = sourcestep->m_vectvalue.size();

	// 补足现有的数据长度
	int needcount = count;
	if(targetindex < (int)m_vectvalue.size())
	{
		needcount -= m_vectvalue.size()-targetindex;
	}
	else
	{
		needcount += targetindex-m_vectvalue.size();
	}
	if(needcount > 0)
	{
		for(int i=0; i<needcount; i++)
		{
			m_vectvalue.push_back((DataUnit *)NULL);
		}
		m_bytescount += needcount*sizeof(DataUnit);
	}

	// 添加数据
	int tmpcount;
	DataUnit * pData;
	for(int i=0; i<count; i++)
	{
		if(m_vectvalue[targetindex+i] != (DataUnit *)NULL)
		{
			m_bytescount -= m_vectvalue[targetindex+i]->count;
			free(m_vectvalue[targetindex+i]);
			m_vectvalue[targetindex+i] = (DataUnit *)NULL;
		}

		tmpcount = sizeof(DataUnit) + sourcestep->m_vectvalue[i]->count;
		pData = (DataUnit *)malloc(tmpcount);
		if(pData != (DataUnit *)NULL)
		{
			memcpy((char *)pData, (char *)sourcestep->m_vectvalue[i], tmpcount);
			m_vectvalue[targetindex+i] = pData;
			m_bytescount += tmpcount-sizeof(DataUnit);
		}
	}

	// 解锁
	sourcestep->m_section.UnLock();
	m_section.UnLock();
	
	return 0;
}

// 判断字段值是否为空
bool CYTStepSec::IsNullValue(int fieldindex)
{
	bool ret = true;
	
	m_section.Lock();

	if(fieldindex>=0 && fieldindex<(int)m_vectvalue.size())
	{
		if(m_vectvalue[fieldindex]!=(DataUnit *)NULL || m_vectvalue[fieldindex]->count>0)
		{
			ret = false;
		}
	}
	
	m_section.UnLock();
	
	return ret;
}

// 在指定位置添加一条记录
int CYTStepSec::InsertField(int fieldindex)
{
	if(fieldindex < 0)
	{
		return -1;
	}
	
	m_section.Lock();
	
	if(fieldindex >= (int)m_vectvalue.size())
	{
		int count = fieldindex-m_vectvalue.size()+1;
		for(int i=0; i<count; i++)
		{
			m_vectvalue.push_back((DataUnit *)NULL);
		}
		m_bytescount += count*sizeof(DataUnit);
	}
	else
	{
		m_vectvalue.insert(m_vectvalue.begin()+fieldindex, (DataUnit *)NULL);
		m_bytescount += sizeof(DataUnit);
	}
	
	m_section.UnLock();
	
	return 0;
}

// 删除一条指定记录
int CYTStepSec::DeleteField(int fieldindex)
{
	if(fieldindex < 0)
	{
		return -1;
	}

	int ret = -1;

	m_section.Lock();
	
	if(fieldindex < (int)m_vectvalue.size())
	{
		if(m_vectvalue[fieldindex] != (DataUnit *)NULL)
		{
			m_bytescount -= sizeof(DataUnit)+m_vectvalue[fieldindex]->count;
		}
		else
		{
			m_bytescount -= sizeof(DataUnit);
		}
		m_vectvalue.erase(m_vectvalue.begin()+fieldindex);
		ret = 0;
	}
	
	m_section.UnLock();
	
	return ret;
}

// 获取字符串型数据
int CYTStepSec::GetFieldString(int fieldindex, char * outbuf, int len)
{
	if(outbuf==NULL || len<=0)
	{
		return -1;
	}

	memset(outbuf, 0, len);
	
	int ret = -1;
	
	m_section.Lock();

	if(fieldindex>=0 && fieldindex<(int)m_vectvalue.size() && m_vectvalue[fieldindex] != (DataUnit *)NULL)
	{
		memcpy(outbuf, m_vectvalue[fieldindex]->data, min(len-1, (int)m_vectvalue[fieldindex]->count));
		ret = 0;
	}
	
	m_section.UnLock();
	
	return ret;
}

// 获取字符型数据
char CYTStepSec::GetFieldChar(int fieldindex)
{
	char buffer[512] = {0};
	if(GetFieldString(fieldindex, buffer, sizeof(buffer)) >= 0)
	{
		return buffer[0];
	}
	else
	{
		return '\0';
	}
}

// 获取整型数据
int CYTStepSec::GetFieldInt(int fieldindex)
{
	char buffer[512] = {0};
	if(GetFieldString(fieldindex, buffer, sizeof(buffer)) >= 0)
	{
		return strtol(buffer, NULL, 10);
	}
	else
	{
		return 0;
	}
}

// 获取浮点型数据
double CYTStepSec::GetFieldDouble(int fieldindex)
{
	char buffer[512] = {0};
	if(GetFieldString(fieldindex, buffer, sizeof(buffer)) >= 0)
	{
		return strtod(buffer, NULL);
	}
	else
	{
		return 0;
	}
}

// 添加字符串型数据
int CYTStepSec::AddFieldString(int fieldindex, const char * value)
{
	if(fieldindex<0 || value==NULL)
	{
		return -1;
	}
	
	m_section.Lock();

	if(fieldindex >= (int)m_vectvalue.size())
	{
		int count = fieldindex-m_vectvalue.size()+1;
		for(int i=0; i<count; i++)
		{
			m_vectvalue.push_back((DataUnit *)NULL);
		}
		m_bytescount += count*sizeof(DataUnit);
	}
	
	if(m_vectvalue[fieldindex] != (DataUnit *)NULL)
	{
		m_bytescount -= m_vectvalue[fieldindex]->count;
		free(m_vectvalue[fieldindex]);
		m_vectvalue[fieldindex] = (DataUnit *)NULL;
	}

	int valuelen = strlen(value);
	DataUnit * pdata = (DataUnit*)malloc(sizeof(DataUnit)+valuelen);
	if(pdata != (DataUnit *)NULL)
	{
		pdata->count = valuelen;
		memcpy(pdata->data, value, valuelen);
		m_vectvalue[fieldindex] = pdata;
		m_bytescount += valuelen;
	}
	
	m_section.UnLock();
	
	return 0;
}

// 添加字符型数据
int CYTStepSec::AddFieldChar(int fieldindex, const char value)
{
	if(value == '\0')
	{
		return -1;
	}

	char buffer[2];
	buffer[0] = value;
	buffer[1] = '\0';
	return AddFieldString(fieldindex, (const char *)buffer);
}

// 添加整型数据
int CYTStepSec::AddFieldInt(int fieldindex, int value)
{
	char buffer[512];
	sprintf_s(buffer, sizeof(buffer), "%d", value);
	return AddFieldString(fieldindex, (const char *)buffer);
}

// 添加浮点型数据
int CYTStepSec::AddFieldDouble(int fieldindex, double value)
{
	char buffer[512];
	sprintf_s(buffer, sizeof(buffer), "%f", value);
	return AddFieldString(fieldindex, (const char *)buffer);
}

// 根据字段值查找指定记录
int CYTStepSec::SearchField(const char * value)
{
	if(value == NULL)
	{
		return -1;
	}

	int ret = -1;

	int valuelen = strlen(value);
	DataUnit * pData = (DataUnit*)malloc(sizeof(DataUnit)+valuelen);
	if(pData == (DataUnit *)NULL)
	{
		return -1;
	}
	pData->count = valuelen;
	memcpy(pData->data, value, valuelen);

	m_section.Lock();
	vector<DataUnit*>::iterator iter = find_if(m_vectvalue.begin(), m_vectvalue.end(), FindDataUnit(pData));
	if(iter != m_vectvalue.end())
	{
		ret = iter-m_vectvalue.begin();
	}
	m_section.UnLock();

	free(pData);

	return ret;
}