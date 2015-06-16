#include "YTStep.h"

CYTStep::CYTStep(void)
{
	m_currrow	= 0;
	m_totalrow	= 0;
	memset(m_crypekey, 0, sizeof(m_crypekey));
	memcpy(m_crypekey, "YTGJKGGF", 8);
	m_retcode	= 0;
	m_pbuffer	= NULL;
}

CYTStep::~CYTStep(void)
{
	m_section.Lock();

	m_currrow	= 0;
	m_totalrow	= 0;

	m_mapfield.clear();

	vector<DataColumn *>::iterator iter_column = m_vectfield.begin();
	while(iter_column != m_vectfield.end())
	{
		if(*iter_column != NULL)
		{
			free(*iter_column);
			*iter_column = NULL;
		}
		iter_column = m_vectfield.erase(iter_column);
	}

	vector<CYTStepSec *>::iterator iter = m_vectfieldvalue.begin();
	while(iter != m_vectfieldvalue.end())
	{
		if(*iter != NULL)
		{
			delete *iter;
			*iter = NULL;
		}
		iter = m_vectfieldvalue.erase(iter);
	}

	if(m_pbuffer != NULL)
	{
		free(m_pbuffer);
		m_pbuffer = NULL;
	}

	m_section.UnLock();
}

// 重置数据
void CYTStep::ResetData(void)
{
	m_section.Lock();

	m_currrow	= 0;
	m_totalrow	= 0;

	m_mapfield.clear();

	vector<DataColumn *>::iterator iter_column = m_vectfield.begin();
	while(iter_column != m_vectfield.end())
	{
		if(*iter_column != NULL)
		{
			free(*iter_column);
			*iter_column = NULL;
		}
		iter_column = m_vectfield.erase(iter_column);
	}

	vector<CYTStepSec *>::iterator iter = m_vectfieldvalue.begin();
	while(iter != m_vectfieldvalue.end())
	{
		if(*iter != NULL)
		{
			delete *iter;
			*iter = NULL;
		}
		iter = m_vectfieldvalue.erase(iter);
	}

	m_retcode = 0;
	m_retmsg = "";

	if(m_pbuffer != NULL)
	{
		free(m_pbuffer);
		m_pbuffer = NULL;
	}

	m_section.UnLock();
}

// 数据包copy
int CYTStep::CopyData(CYTStep * sourcestep, int startindex, int endindex)
{
	// 获取索引号
	if(sourcestep==NULL || (startindex>endindex && endindex>0))
	{
		return -1;
	}
	if(startindex <= 0)
	{
		startindex = 1;
	}

	// 加锁
	m_section.Lock();
	sourcestep->m_section.Lock();

	// 清空原有数据
	m_currrow	= 0;
	m_totalrow	= 0;

	m_mapfield.clear();

	vector<DataColumn *>::iterator iter_column = m_vectfield.begin();
	while(iter_column != m_vectfield.end())
	{
		if(*iter_column != NULL)
		{
			free(*iter_column);
			*iter_column = NULL;
		}
		iter_column = m_vectfield.erase(iter_column);
	}

	vector<CYTStepSec *>::iterator iter = m_vectfieldvalue.begin();
	while(iter != m_vectfieldvalue.end())
	{
		if(*iter != NULL)
		{
			delete *iter;
			*iter = NULL;
		}
		iter = m_vectfieldvalue.erase(iter);
	}

	// 添加新的数据
	bool bAddData = false;
	DataColumn * pDataColumn;
	int tmpcount;
	int columnindex = 0;
	CYTStepSec * pYTStepSec;
	int fieldcount = sourcestep->m_vectfield.size();
	for(int i=0; i<fieldcount; i++)
	{
		tmpcount = sizeof(DataColumn)+sourcestep->m_vectfield[i]->count;
		pDataColumn = (DataColumn *)malloc(tmpcount);
		if(pDataColumn != NULL)
		{
			memcpy((char *)pDataColumn, (char *)sourcestep->m_vectfield[i], tmpcount);
			m_vectfield.push_back(pDataColumn);
			m_mapfield[pDataColumn->data] = columnindex;
			columnindex++;
			pYTStepSec = new CYTStepSec;
			if(pYTStepSec!=NULL && startindex<=sourcestep->m_totalrow)
			{
				pYTStepSec->CopyData(sourcestep->m_vectfieldvalue[i], startindex-1, endindex-1);
			}
			m_vectfieldvalue.push_back(pYTStepSec);
			bAddData = true;
		}
	}

	if(bAddData)
	{
		if(startindex < sourcestep->m_totalrow)
		{
			if(endindex<=0 || endindex>=sourcestep->m_totalrow)
			{
				m_totalrow = sourcestep->m_totalrow - startindex + 1;
			}
			else
			{
				m_totalrow = endindex - startindex + 1;
			}
		}
	}

	// 解锁
	sourcestep->m_section.UnLock();
	m_section.UnLock();

	return 0;
}

// 数据包合并
int CYTStep::AppendData(CYTStep * sourcestep)
{
	if(sourcestep == NULL)
	{
		return -1;
	}
	
	// 加锁
	m_section.Lock();
	sourcestep->m_section.Lock();

	// 添加信息
	bool bAddData = false;
	hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter;
	DataColumn * pDataColumn;
	int tmpcount;
	int columncount = m_vectfield.size();
	CYTStepSec * pYTStepSec;
	int fieldcount = sourcestep->m_vectfield.size();
	for(int i=0; i<fieldcount; i++)
	{
		iter = m_mapfield.find(sourcestep->m_vectfield[i]->data);
		if(iter != m_mapfield.end())
		{
			// 找到了则添加数据
			if(m_vectfieldvalue[iter->second] == NULL)
			{
				m_vectfieldvalue[iter->second] = new CYTStepSec;
			}
			if(m_vectfieldvalue[iter->second] != NULL)
			{
				m_vectfieldvalue[iter->second]->AppendData(sourcestep->m_vectfieldvalue[i], m_totalrow);
				bAddData = true;
			}
		}
		else
		{
			// 没有找到则新增加一个数据
			tmpcount = sizeof(DataColumn)+sourcestep->m_vectfield[i]->count;
			pDataColumn = (DataColumn *)malloc(tmpcount);
			if(pDataColumn != NULL)
			{
				memcpy((char *)pDataColumn, (char *)sourcestep->m_vectfield[i], tmpcount);
				m_vectfield.push_back(pDataColumn);
				m_mapfield[pDataColumn->data] = columncount;
				columncount++;
				pYTStepSec = new CYTStepSec;
				if(pYTStepSec != NULL)
				{
					pYTStepSec->AppendData(sourcestep->m_vectfieldvalue[i], m_totalrow);
					bAddData = true;
				}
				m_vectfieldvalue.push_back(pYTStepSec);
			}
		}
	}

	m_currrow = 0;
	if(bAddData)
	{
		m_totalrow += sourcestep->m_totalrow;
	}

	// 解锁
	sourcestep->m_section.UnLock();
	m_section.UnLock();
	
	return 0;
}

// 转换数据缓存：该函数主要将传入缓存数据结构转换成标准数据结构
// 数据格式为：retcode(4bytes)+retmsglen(包括结尾的'\0')+retmsg('\0'结尾)+columncount(4bytes)+columnlen(4bytes)+columndata ...+rowcount(4bytes)+rowlen(4bytes)+rowdata...
int CYTStep::ConvertData(const char * data, int len)
{
	if(data==NULL || len<8)
	{
		return -1;
	}

	// 清空原来数据
	ResetData();

	// 加锁
	m_section.Lock();

	// 读取返回值【标志业务是否成功，>=0成功 <0失败】
	int provlen = 0;
	memcpy(&m_retcode, data+provlen, sizeof(int));
	provlen += sizeof(int);

	// 读取返回消息【业务失败时有效，为失败信息】
	int tmplen;
	memcpy(&tmplen, data+provlen, sizeof(int));
	provlen += sizeof(int);
	if(tmplen<0 || (len-provlen)<tmplen)
	{
		m_section.UnLock();
		return -1;
	}
	m_retmsg = data+provlen;
	provlen += tmplen;

	// 读取columncount
	if((len-provlen) < 4)
	{
		m_section.UnLock();
		return -1;
	}
	int columncount = 0;
	memcpy(&columncount, data+provlen, sizeof(int));
	provlen += sizeof(int);
	if(columncount <= 0)
	{
		m_section.UnLock();
		return 0;
	}

	// 读取columnlen
	if((len-provlen) < 4)
	{
		m_section.UnLock();
		return -1;
	}
	int columnlen = 0;
	memcpy(&columnlen, data+provlen, sizeof(int));
	provlen += sizeof(int);
	if(columnlen<=0 || (len-provlen)<columnlen)
	{
		m_section.UnLock();
		return -1;
	}

	// 读取columndata
	DataColumn * pDataColumn;
	DataColumn * pNewDataColumn;
	CYTStepSec * pYTStepSec;
	int i;
	for(i=0; i<columncount; i++)
	{
		pDataColumn = (DataColumn *)(data+provlen);
		if(pDataColumn->count == 0)
		{
			m_section.UnLock();
			return -1;
		}
		else
		{
			pNewDataColumn = (DataColumn *)malloc(sizeof(DataColumn)+pDataColumn->count);
			if(pNewDataColumn == NULL)
			{
				m_section.UnLock();
				return -1;
			}
			memcpy((char *)pNewDataColumn, data+provlen, sizeof(DataColumn)+pDataColumn->count);
			m_vectfield.push_back(pNewDataColumn);
			m_mapfield[pNewDataColumn->data] = m_vectfield.size()-1;
			pYTStepSec = new CYTStepSec;
			if(pYTStepSec == NULL)
			{
				m_section.UnLock();
				return -1;
			}
			m_vectfieldvalue.push_back(pYTStepSec);
		}
		provlen += sizeof(DataColumn)+pDataColumn->count;
	}

	// 读取rowcount
	if((len-provlen) < 4)
	{
		m_section.UnLock();
		return -1;
	}
	memcpy(&m_totalrow, data+provlen, sizeof(int));
	provlen += sizeof(int);
	if(m_totalrow <= 0)
	{
		m_totalrow = 0;
		m_section.UnLock();
		return 0;
	}

	// 读取rowlen
	if((len-provlen) < 4)
	{
		m_section.UnLock();
		return -1;
	}
	int rowlen = 0;
	memcpy(&rowlen, data+provlen, sizeof(int));
	provlen += sizeof(int);
	if(rowlen<=0 || (len-provlen)<rowlen)
	{
		m_section.UnLock();
		return -1;
	}

	// 读取rowdata
	DataUnit * pUdata;
	DataUnit * pNewData;
	int j;
	for(i=0; i<m_totalrow; i++)
	{
		for(j=0; j<columncount; j++)
		{
			pUdata = (DataUnit *)(data+provlen);
			if(pUdata->count == 0)
			{
				m_vectfieldvalue[j]->AddData((DataUnit *)NULL);
			}
			else
			{
				pNewData = (DataUnit *)malloc(sizeof(DataUnit)+pUdata->count);
				if(pNewData == (DataUnit *)NULL)
				{
					m_section.UnLock();
					return -1;
				}
				memcpy(pNewData, data+provlen, pUdata->count + sizeof(DataUnit));
				m_vectfieldvalue[j]->AddData(pNewData);
			}
			provlen += sizeof(DataUnit)+pUdata->count;
		}
	}

	// 解锁
	m_section.UnLock();
	
	return 0;
}

// 生成数据缓存：该函数主要将数据结构转换成缓存信息以便发送
// 数据格式为：retcode(4bytes)+retmsglen(包括结尾的'\0')+retmsg('\0'结尾)+columncount(4bytes)+columnlen(4bytes)+columndata ...+rowcount(4bytes)+rowlen(4bytes)+rowdata...
const char * CYTStep::MakeSendData(int * len)
{
	if(len == NULL)
	{
		return NULL;
	}

	// 加锁
	m_section.Lock();

	// 获取生成后的数据长度并分配好缓存
	int totallen = 0;
	int columnlen = 0;
	int rowlen = 0;
	GetDataBytesCount(totallen, columnlen, rowlen);

	*len = totallen;
	if(m_pbuffer != NULL)
	{
		free(m_pbuffer);
	}
	m_pbuffer = (char *)malloc(*len);
	if(m_pbuffer == NULL)
	{
		*len = 0;
		return NULL;
	}
	memset(m_pbuffer, 0, *len);
	char * pTempBuffer = m_pbuffer;

	// 写入retcode(4bytes)
	memcpy(pTempBuffer, &m_retcode, sizeof(int));
	pTempBuffer += sizeof(int);

	// 写入retmsg
	int tmplen = m_retmsg.length()+1;
	memcpy(pTempBuffer, &tmplen, sizeof(int));
	pTempBuffer += sizeof(int);
	memcpy(pTempBuffer, m_retmsg.c_str(), m_retmsg.length());
	pTempBuffer += tmplen;

	// 写入columncount(4bytes)
	int columncount = m_vectfield.size();
	memcpy(pTempBuffer, &columncount, sizeof(int));
	pTempBuffer += sizeof(int);

	// 写入columnlen(4bytes)
	memcpy(pTempBuffer, &columnlen, sizeof(int));
	pTempBuffer += sizeof(int);

	// 写入columndata
	int i;
	for(i=0; i<columncount; i++)
	{
		memcpy(pTempBuffer, (char *)m_vectfield[i], sizeof(DataColumn)+m_vectfield[i]->count);
		pTempBuffer += sizeof(DataColumn)+m_vectfield[i]->count;
	}

	// 写入rowcount(4bytes)
	memcpy(pTempBuffer, &m_totalrow, sizeof(int));
	pTempBuffer += sizeof(int);

	// 写入rowlen(4bytes)
	memcpy(pTempBuffer, &rowlen, sizeof(int));
	pTempBuffer += sizeof(int);

	// 写入rowdata
	DataUnit ZeroDataUnit = {0};
	int fieldvaluesize = m_vectfieldvalue.size();
	int j;
	for(i=0; i<m_totalrow; i++)
	{
		for(j=0; j<columncount; j++)
		{
			if(j<fieldvaluesize && m_vectfieldvalue[j]!=NULL && i<(int)m_vectfieldvalue[j]->m_vectvalue.size() && m_vectfieldvalue[j]->m_vectvalue[i]!=(DataUnit *)NULL)
			{
				memcpy(pTempBuffer, m_vectfieldvalue[j]->m_vectvalue[i], sizeof(DataUnit)+m_vectfieldvalue[j]->m_vectvalue[i]->count);
				pTempBuffer += sizeof(DataUnit)+m_vectfieldvalue[j]->m_vectvalue[i]->count;
			}
			else
			{
				memcpy(pTempBuffer, &ZeroDataUnit, sizeof(DataUnit));
				pTempBuffer += sizeof(DataUnit);
			}
		}
	}

	// 解锁
	m_section.UnLock();

	return m_pbuffer;
}

// 设置retcode返回值，用于标志业务是否成功
void CYTStep::SetRetCode(int retcode)
{
	m_retcode = retcode;
}

// 获取retcode返回值
int CYTStep::GetRetCode(void)
{
	return m_retcode;
}

// 设置comment返回值，当业务失败时有效，为失败信息
void CYTStep::SetRetMsg(const char* pMsg)
{
	m_retmsg = pMsg;
}

// 获取comment返回值
int CYTStep::GetRetMsg(char * outbuf, int len)
{
	if(outbuf==NULL || len<=0)
	{
		return -1;
	}

	memset(outbuf, 0, len);
	memcpy(outbuf, m_retmsg.c_str(), min(len-1, (int)m_retmsg.length()));
	return 0;
}

// 获取记录条数
int CYTStep::GetRowCount(void)
{
	int ret = 0;

	m_section.Lock();

	ret = m_totalrow;

	m_section.UnLock();

	return ret;
}

// 在指定位置插入一条记录
int CYTStep::InsertRow(int rowindex)
{
	if(rowindex <= 0)
	{
		return -1;
	}
	rowindex--;

	int ret = -1;

	m_section.Lock();

	if(rowindex < m_totalrow)
	{
		int fieldcount = m_vectfieldvalue.size();
		for(int i=0; i<fieldcount; i++)
		{
			if(m_vectfieldvalue[i] != NULL)
			{
				m_vectfieldvalue[i]->InsertField(rowindex);
			}
		}

		m_currrow = rowindex;
		m_totalrow++;
		ret = 0;
	}
	
	m_section.UnLock();

	return ret;
}

// 删除一条指定记录
int CYTStep::DeleteRow(int rowindex)
{
	if(rowindex <= 0)
	{
		return -1;
	}
	rowindex--;
	
	int ret = -1;
	
	m_section.Lock();
	
	if(rowindex < m_totalrow)
	{
		int fieldcount = m_vectfieldvalue.size();
		for(int i=0; i<fieldcount; i++)
		{
			if(m_vectfieldvalue[i] != NULL)
			{
				m_vectfieldvalue[i]->DeleteField(rowindex);
			}
		}

		if(rowindex <= m_currrow)
		{
			m_currrow--;
		}
		m_totalrow--;
		ret = 0;
	}
	
	m_section.UnLock();
	
	return ret;
}

// 在最后增加一条记录
int CYTStep::AppendRow(void)
{
	m_section.Lock();

	m_totalrow++;
	m_currrow = m_totalrow-1;
	
	m_section.UnLock();
	
	return 0;
}

// 转到第一条记录
int CYTStep::GotoBeginRow(void)
{
	m_section.Lock();

	m_currrow = 0;

	m_section.UnLock();

	return 0;
}

// 转到最后一条记录
int CYTStep::GotoEndRow(void)
{
	m_section.Lock();
	
	if(m_totalrow == 0)
	{
		m_currrow = 0;
	}
	else
	{
		m_currrow = m_totalrow-1;
	}
	
	m_section.UnLock();
	
	return 0;
}

// 转到上一条记录
int CYTStep::GotoPreRow(void)
{
	int ret = 0;
	
	m_section.Lock();
	
	if(m_currrow == 0)
	{
		ret = -1;
	}
	else
	{
		m_currrow--;
	}
	
	m_section.UnLock();
	
	return ret;
}

// 转到下一条记录
int CYTStep::GotoNextRow(void)
{
	int ret = 0;
	
	m_section.Lock();
	
	if(m_totalrow==0 || (m_currrow==m_totalrow-1))
	{
		ret = -1;
	}
	else
	{
		m_currrow++;
	}
	
	m_section.UnLock();
	
	return ret;
}

// 转到指定的某条记录
int CYTStep::GotoFixRow(int rowindex)
{
	int ret = 0;
	
	m_section.Lock();
	
	if(rowindex>=1 && rowindex<=m_totalrow)
	{
		m_currrow = rowindex-1;
	}
	else
	{
		ret = -1;
	}
	
	m_section.UnLock();
	
	return ret;
}

// 获取字段个数
int CYTStep::GetFieldCount(void)
{
	int ret = 0;
	
	m_section.Lock();
	
	ret = m_vectfield.size();
	
	m_section.UnLock();
	
	return ret;
}

// 获取字段类型
int CYTStep::GetFieldType(int fieldindex)
{
	if(fieldindex <= 0)
	{
		return -1;
	}
	
	int ret = -1;
	
	m_section.Lock();
	
	if(fieldindex <= (int)m_vectfield.size())
	{
		ret = m_vectfield[fieldindex-1]->FieldType;
	}
	
	m_section.UnLock();
	
	return ret;
}

// 获取字段名
const char * CYTStep::GetFieldName(int fieldindex)
{
	if(fieldindex <= 0)
	{
		return NULL;
	}

	const char * ret = NULL;
	
	m_section.Lock();

	if(fieldindex <= (int)m_vectfield.size())
	{
		ret = m_vectfield[fieldindex-1]->data;
	}
	
	m_section.UnLock();
	
	return (const char *)ret;
}

// 添加一个字段
int CYTStep::AddField(const char * fieldname, int fieldtype)
{
	if(fieldname==NULL || fieldtype<0)
	{
		return -1;
	}

	int ret = -1;

	m_section.Lock();

	if(m_mapfield.find(fieldname) == m_mapfield.end())
	{
		int iCount = strlen(fieldname)+1;
		DataColumn * pDataColumn = (DataColumn *)malloc(sizeof(DataColumn)+iCount);
		if(pDataColumn != NULL)
		{
			memset(pDataColumn, 0, sizeof(DataColumn)+iCount);
			pDataColumn->FieldType = fieldtype;
			pDataColumn->count = iCount;
			memcpy(pDataColumn->data, fieldname, iCount-1);
			m_vectfield.push_back(pDataColumn);
			m_mapfield[pDataColumn->data] = m_vectfield.size()-1;

			m_vectfieldvalue.push_back(new CYTStepSec);
			ret = 0;
		}
	}

	m_section.UnLock();

	return ret;
}

// 删除一个字段
int CYTStep::DeleteField(const char * fieldname)
{
	if(fieldname == NULL)
	{
		return -1;
	}
	
	int ret = -1;
	
	m_section.Lock();
	
	hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
	if(iter != m_mapfield.end())
	{
		if(iter->second == m_vectfield.size()-1)
		{
			// 如果是最后一个字段的话则不需要移位
			m_mapfield.erase(iter);

			if(m_vectfield[m_vectfield.size()-1] != NULL)
			{
				free(m_vectfield[m_vectfield.size()-1]);
				m_vectfield[m_vectfield.size()-1] = NULL;
			}
			m_vectfield.pop_back();

			if(m_vectfieldvalue[m_vectfieldvalue.size()-1] != NULL)
			{
				delete m_vectfieldvalue[m_vectfieldvalue.size()-1];
				m_vectfieldvalue[m_vectfieldvalue.size()-1] = NULL;
			}
			m_vectfieldvalue.pop_back();
		}
		else
		{
			// 不是最后一个字段则需要移位
			int index = iter->second;
			m_mapfield.erase(iter);

			if(m_vectfield[index] != NULL)
			{
				free(m_vectfield[index]);
				m_vectfield[index] = NULL;
			}
			m_vectfield.erase(m_vectfield.begin()+index);

			if(m_vectfieldvalue[index] != NULL)
			{
				delete m_vectfieldvalue[index];
				m_vectfieldvalue[index] = NULL;
			}
			m_vectfieldvalue.erase(m_vectfieldvalue.begin()+index);

			iter = m_mapfield.begin();
			while(iter != m_mapfield.end())
			{
				if(iter->second > index)
				{
					iter->second--;
				}
				iter++;
			}
		}

		ret = 0;
	}
	
	m_section.UnLock();
	
	return ret;
}

// 判断字段值是否为空
bool CYTStep::IsNullValue(const char * fieldname)
{
	if(fieldname == NULL)
	{
		return true;
	}

	bool ret = true;

	m_section.Lock();

	hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
	if(iter!=m_mapfield.end() && m_vectfieldvalue[iter->second]!=NULL)
	{
		ret = m_vectfieldvalue[iter->second]->IsNullValue(m_currrow);
	}

	m_section.UnLock();

	return ret;
}

// 获取字段串型数据
int CYTStep::GetFieldString(const char * fieldname, char * outbuf, int len)
{
	if(fieldname==NULL || outbuf==NULL || len<=0)
	{
		return -1;
	}

	memset(outbuf, 0, len);

	int ret = -1;
	
	m_section.Lock();
	
	hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
	if(iter!=m_mapfield.end() && m_vectfieldvalue[iter->second]!=NULL)
	{
		ret = m_vectfieldvalue[iter->second]->GetFieldString(m_currrow, outbuf, len);
	}
	
	m_section.UnLock();
	
	return ret;
}

// 获取字符型数据
char CYTStep::GetFieldChar(const char * fieldname)
{
	if(fieldname == NULL)
	{
		return '\0';
	}
	
	char ret = '\0';
	
	m_section.Lock();
	
	hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
	if(iter!=m_mapfield.end() && m_vectfieldvalue[iter->second]!=NULL)
	{
		ret = m_vectfieldvalue[iter->second]->GetFieldChar(m_currrow);
	}
	
	m_section.UnLock();
	
	return ret;
}

// 获取整型数据
int CYTStep::GetFieldInt(const char * fieldname)
{
	if(fieldname == NULL)
	{
		return 0;
	}
	
	int ret = 0;
	
	m_section.Lock();
	
	hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
	if(iter!=m_mapfield.end() && m_vectfieldvalue[iter->second]!=NULL)
	{
		ret = m_vectfieldvalue[iter->second]->GetFieldInt(m_currrow);
	}
	
	m_section.UnLock();
	
	return ret;
}

// 获取浮点型数据
double CYTStep::GetFieldDouble(const char * fieldname)
{
	if(fieldname == NULL)
	{
		return 0;
	}

	double ret = 0;
	
	m_section.Lock();
	
	hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
	if(iter!=m_mapfield.end() && m_vectfieldvalue[iter->second]!=NULL)
	{
		ret = m_vectfieldvalue[iter->second]->GetFieldDouble(m_currrow);
	}
	
	m_section.UnLock();
	
	return ret;
}

// 获取密码数据
int CYTStep::GetFieldPwd(const char * fieldname, char * outpwd, int pwdlen)
{
	if(fieldname == NULL)
	{
		return -1;
	}

	memset(outpwd, 0, pwdlen);

	int ret = -1;
	
	m_section.Lock();

	hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
	if(iter!=m_mapfield.end() && m_vectfieldvalue[iter->second]!=NULL)
	{
		char value[4096*2] = {0};
		ret = m_vectfieldvalue[iter->second]->GetFieldString(m_currrow, value, sizeof(value));
		if(ret >= 0)
		{
			char buffer[512] = {0};
			int len = CYTBase64::DoDecode((const char *)value, strlen(value), (char *)buffer);
			memcpy(outpwd, buffer, min(pwdlen-1, len));
			
			if(m_crypekey[0] != '\0')
			{
				CYTBlowfish blowfish(m_crypekey, strlen((const char *)m_crypekey));
				blowfish.Decrypt((BYTE *)outpwd, len);
			}

			ret = 0;
		}
	}
	
	m_section.UnLock();
	
	return ret;
}

// 获取二进制数据
int CYTStep::GetFieldBinary(const char * fieldname, char * outbuf, int * outlen)
{
	if(fieldname==NULL || outbuf==NULL || outlen==NULL || *outlen<=0)
	{
		return -1;
	}
	
	memset(outbuf, 0, *outlen);
	
	int ret = -1;
	
	m_section.Lock();

	hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
	if(iter!=m_mapfield.end() && m_vectfieldvalue[iter->second]!=NULL)
	{
		char value[4096*2] = {0};
		ret = m_vectfieldvalue[iter->second]->GetFieldString(m_currrow, value, sizeof(value));
		if(ret >= 0)
		{
			char buffer[512] = {0};
			int len = CYTBase64::DoDecode((const char *)value, strlen(value), (char *)buffer);
			memcpy(outbuf, buffer, min(*outlen, len));
			if(*outlen > len)
			{
				*outlen = len;
			}

			ret = 0;
		}
	}
	
	m_section.UnLock();

	if(ret < 0)
	{
		*outlen = 0;
	}
	
	return ret;
}

// 添加字段串型数据
int CYTStep::AddFieldString(const char * fieldname, const char * value)
{
	if(fieldname==NULL || value==NULL)
	{
		return -1;
	}
	
	int ret = -1;
	
	m_section.Lock();

	if(m_totalrow > 0)
	{
		hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
		if(iter == m_mapfield.end())
		{
			// 没有字段则添加一个
			int iCount = strlen(fieldname)+1;
			DataColumn * pDataColumn = (DataColumn *)malloc(sizeof(DataColumn)+iCount);
			if(pDataColumn != NULL)
			{
				memset(pDataColumn, 0, sizeof(DataColumn)+iCount);
				pDataColumn->FieldType = FT_STRING;
				pDataColumn->count = iCount;
				memcpy(pDataColumn->data, fieldname, iCount-1);
				m_vectfield.push_back(pDataColumn);
				m_mapfield[pDataColumn->data] = m_vectfield.size()-1;

				CYTStepSec * pYTStepSec = new CYTStepSec;
				if(pYTStepSec != NULL)
				{
					ret = pYTStepSec->AddFieldString(m_currrow, value);
				}
				m_vectfieldvalue.push_back(pYTStepSec);
			}
		}
		else
		{
			if(m_vectfieldvalue[iter->second] == NULL)
			{
				m_vectfieldvalue[iter->second] = new CYTStepSec;
			}
			if(m_vectfieldvalue[iter->second] != NULL)
			{
				ret = m_vectfieldvalue[iter->second]->AddFieldString(m_currrow, value);
			}
		}
	}
	
	m_section.UnLock();
	
	return ret;
}

// 添加字符型数据
int CYTStep::AddFieldChar(const char * fieldname, const char value)
{
	if(fieldname==NULL || value=='\0')
	{
		return -1;
	}
	
	int ret = -1;
	
	m_section.Lock();

	if(m_totalrow > 0)
	{
		hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
		if(iter == m_mapfield.end())
		{
			// 没有字段则添加一个
			int iCount = strlen(fieldname)+1;
			DataColumn * pDataColumn = (DataColumn *)malloc(sizeof(DataColumn)+iCount);
			if(pDataColumn != NULL)
			{
				memset(pDataColumn, 0, sizeof(DataColumn)+iCount);
				pDataColumn->FieldType = FT_CHAR;
				pDataColumn->count = iCount;
				memcpy(pDataColumn->data, fieldname, iCount-1);
				m_vectfield.push_back(pDataColumn);
				m_mapfield[pDataColumn->data] = m_vectfield.size()-1;

				CYTStepSec * pYTStepSec = new CYTStepSec;
				if(pYTStepSec != NULL)
				{
					ret = pYTStepSec->AddFieldChar(m_currrow, value);
				}
				m_vectfieldvalue.push_back(pYTStepSec);
			}
		}
		else
		{
			if(m_vectfieldvalue[iter->second] == NULL)
			{
				m_vectfieldvalue[iter->second] = new CYTStepSec;
			}
			if(m_vectfieldvalue[iter->second] != NULL)
			{
				ret = m_vectfieldvalue[iter->second]->AddFieldChar(m_currrow, value);
			}
		}
	}
	
	m_section.UnLock();
	
	return ret;
}

// 添加整型数据
int CYTStep::AddFieldInt(const char * fieldname, int value)
{
	if(fieldname == NULL)
	{
		return -1;
	}
	
	int ret = -1;
	
	m_section.Lock();

	if(m_totalrow > 0)
	{
		hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
		if(iter == m_mapfield.end())
		{
			// 没有字段则添加一个
			int iCount = strlen(fieldname)+1;
			DataColumn * pDataColumn = (DataColumn *)malloc(sizeof(DataColumn)+iCount);
			if(pDataColumn != NULL)
			{
				memset(pDataColumn, 0, sizeof(DataColumn)+iCount);
				pDataColumn->FieldType = FT_INTEGER;
				pDataColumn->count = iCount;
				memcpy(pDataColumn->data, fieldname, iCount-1);
				m_vectfield.push_back(pDataColumn);
				m_mapfield[pDataColumn->data] = m_vectfield.size()-1;

				CYTStepSec * pYTStepSec = new CYTStepSec;
				if(pYTStepSec != NULL)
				{
					ret = pYTStepSec->AddFieldInt(m_currrow, value);
				}
				m_vectfieldvalue.push_back(pYTStepSec);
			}
		}
		else
		{
			if(m_vectfieldvalue[iter->second] == NULL)
			{
				m_vectfieldvalue[iter->second] = new CYTStepSec;
			}
			if(m_vectfieldvalue[iter->second] != NULL)
			{
				ret = m_vectfieldvalue[iter->second]->AddFieldInt(m_currrow, value);
			}
		}
	}
	
	m_section.UnLock();
	
	return ret;
}

// 添加浮点型数据
int CYTStep::AddFieldDouble(const char * fieldname, double value)
{
	if(fieldname == NULL)
	{
		return -1;
	}
	
	int ret = -1;
	
	m_section.Lock();

	if(m_totalrow > 0)
	{
		hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
		if(iter == m_mapfield.end())
		{
			// 没有字段则添加一个
			int iCount = strlen(fieldname)+1;
			DataColumn * pDataColumn = (DataColumn *)malloc(sizeof(DataColumn)+iCount);
			if(pDataColumn != NULL)
			{
				memset(pDataColumn, 0, sizeof(DataColumn)+iCount);
				pDataColumn->FieldType = FT_DOUBLE;
				pDataColumn->count = iCount;
				memcpy(pDataColumn->data, fieldname, iCount-1);
				m_vectfield.push_back(pDataColumn);
				m_mapfield[pDataColumn->data] = m_vectfield.size()-1;

				CYTStepSec * pYTStepSec = new CYTStepSec;
				if(pYTStepSec != NULL)
				{
					ret = pYTStepSec->AddFieldDouble(m_currrow, value);
				}
				m_vectfieldvalue.push_back(pYTStepSec);
			}
		}
		else
		{
			if(m_vectfieldvalue[iter->second] == NULL)
			{
				m_vectfieldvalue[iter->second] = new CYTStepSec;
			}
			if(m_vectfieldvalue[iter->second] != NULL)
			{
				ret = m_vectfieldvalue[iter->second]->AddFieldDouble(m_currrow, value);
			}
		}
	}
	
	m_section.UnLock();
	
	return ret;
}

// 添加密码数据
int CYTStep::AddFieldPwd(const char * fieldname, const char * value)
{
	if(fieldname==NULL || value==NULL)
	{
		return -1;
	}
	
	int ret = -1;
	
	m_section.Lock();
	
	if(m_totalrow > 0)
	{
		// 对密码进行blowfish加密
		char buffer[512] = {0};
		memcpy(buffer, value, min(sizeof(buffer), strlen(value)));
		int len = ((strlen(value)+7)/8)*8+8;
		
		if(m_crypekey[0] != '\0')
		{
			CYTBlowfish blowfish(m_crypekey, strlen((const char *)m_crypekey));
			blowfish.Encrypt((BYTE *)buffer, len);
		}

		// 进行base64加密
		char outbuffer[4096*2] = {0};
		CYTBase64::DoEncode(buffer, len, outbuffer);

		// 添加数据
		hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
		if(iter == m_mapfield.end())
		{
			// 没有字段则添加一个
			int iCount = strlen(fieldname)+1;
			DataColumn * pDataColumn = (DataColumn *)malloc(sizeof(DataColumn)+iCount);
			if(pDataColumn != NULL)
			{
				memset(pDataColumn, 0, sizeof(DataColumn)+iCount);
				pDataColumn->FieldType = FT_PASSWORD;
				pDataColumn->count = iCount;
				memcpy(pDataColumn->data, fieldname, iCount-1);
				m_vectfield.push_back(pDataColumn);
				m_mapfield[pDataColumn->data] = m_vectfield.size()-1;

				CYTStepSec * pYTStepSec = new CYTStepSec;
				if(pYTStepSec != NULL)
				{
					ret = pYTStepSec->AddFieldString(m_currrow, outbuffer);
				}
				m_vectfieldvalue.push_back(pYTStepSec);
			}
		}
		else
		{
			if(m_vectfieldvalue[iter->second] == NULL)
			{
				m_vectfieldvalue[iter->second] = new CYTStepSec;
			}
			if(m_vectfieldvalue[iter->second] != NULL)
			{
				ret = m_vectfieldvalue[iter->second]->AddFieldString(m_currrow, outbuffer);
			}
		}
	}
	
	m_section.UnLock();
	
	return ret;
}

// 添加二进制数据
int CYTStep::AddFieldBinary(const char * fieldname, const char * value, int len)
{
	if(fieldname==NULL || value==NULL || len<=0)
	{
		return -1;
	}
	
	int ret = -1;
	
	m_section.Lock();

	if(m_totalrow > 0)
	{
		// 进行base64加密
		char outbuffer[4096*2] = {0};
		CYTBase64::DoEncode(value, len, outbuffer);

		// 添加数据
		hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
		if(iter == m_mapfield.end())
		{
			// 没有字段则添加一个
			int iCount = strlen(fieldname)+1;
			DataColumn * pDataColumn = (DataColumn *)malloc(sizeof(DataColumn)+iCount);
			if(pDataColumn != NULL)
			{
				memset(pDataColumn, 0, sizeof(DataColumn)+iCount);
				pDataColumn->FieldType = FT_BINARY;
				pDataColumn->count = iCount;
				memcpy(pDataColumn->data, fieldname, iCount-1);
				m_vectfield.push_back(pDataColumn);
				m_mapfield[pDataColumn->data] = m_vectfield.size()-1;

				CYTStepSec * pYTStepSec = new CYTStepSec;
				if(pYTStepSec != NULL)
				{
					ret = pYTStepSec->AddFieldString(m_currrow, outbuffer);
				}
				m_vectfieldvalue.push_back(pYTStepSec);
			}
		}
		else
		{
			if(m_vectfieldvalue[iter->second] == NULL)
			{
				m_vectfieldvalue[iter->second] = new CYTStepSec;
			}
			if(m_vectfieldvalue[iter->second] != NULL)
			{
				ret = m_vectfieldvalue[iter->second]->AddFieldString(m_currrow, outbuffer);
			}
		}
	}
	
	m_section.UnLock();
	
	return ret;
}

// 根据字段值查找某条指定记录
int CYTStep::SearchByField(const char * fieldname, const char * value)
{
	if(fieldname==NULL || value==NULL)
	{
		return -1;
	}
	
	int ret = -1;
	
	m_section.Lock();

	hash_map<const char *, int, hash_compare<const char *, CharLess> >::iterator iter = m_mapfield.find(fieldname);
	if(iter!=m_mapfield.end() && m_vectfieldvalue[iter->second]!=NULL)
	{
		ret = m_vectfieldvalue[iter->second]->SearchField(value);
		if(ret >= 0)
		{
			ret++;
		}
	}
	
	m_section.UnLock();
	
	return ret;
}

// 获取字段信息长度
int CYTStep::GetColumnInfoLen(void)
{
	int len = 0;
	int count = m_vectfield.size();
	for(int i=0; i<count; i++)
	{
		len += sizeof(DataColumn) + m_vectfield[i]->count;
	}
	return len;
}

// 获取数值信息长度
int	CYTStep::GetRowInfoLen(void)
{
	int len = 0;
	int count = m_vectfieldvalue.size();
	for(int i=0; i<count; i++)
	{
		len += m_vectfieldvalue[i]->GetBytesCount() + (m_totalrow-m_vectfieldvalue[i]->GetRecordCount())*sizeof(DataColumn);
	}
	return len;
}

// 获取数值信息长度
// 数据格式为：retcode(4bytes)+retmsglen(包括结尾的'\0')+retmsg('\0'结尾)+columncount(4bytes)+columnlen(4bytes)+columndata ...+rowcount(4bytes)+rowlen(4bytes)+rowdata...
void CYTStep::GetDataBytesCount(int &totallen, int &columnlen, int &rowlen)
{
	columnlen = GetColumnInfoLen();
	rowlen = GetRowInfoLen();
	totallen = sizeof(int) + sizeof(int) + (m_retmsg.length()+1) + sizeof(int) + sizeof(int) + columnlen + sizeof(int) + sizeof(int) + rowlen;
}
