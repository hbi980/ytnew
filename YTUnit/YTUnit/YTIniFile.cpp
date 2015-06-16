#include "../../YTPublic/YTInclude/YTUnit/YTIniFile.h"


CYTIniFile::CYTIniFile(void)
{
	memset(m_filename, 0, sizeof(m_filename));
	m_bModify = false;
}

CYTIniFile::~CYTIniFile(void)
{
	Close();
}

// 打开文件
int CYTIniFile::Open(const char * filename)
{
	if(filename == NULL)
	{
		return -1;
	}
	
	// 关闭现有文件
	Close();
	
	// 打开指定文件并分配缓存存储文件
	if(CYTFile::Open(filename) < 0)
	{
		return -1;
	}
	unsigned int length = CYTFile::Length();
	if(length == 0xFFFFFFFF)
	{
		CYTFile::Close();
		return -1;
	}
	char * buffer = new char[length+10];
	memset(buffer, 0, length+10);
	CYTFile::Seek(FILESEEK_BEGIN, 0);
	if(CYTFile::Read(buffer, length) != length)
	{
		delete buffer;
		buffer = NULL;
		CYTFile::Close();
		return -1;
	}
	sprintf_s(buffer+length, sizeof(buffer), "\r\n");
	CYTFile::Close();

	// 循环读取每行数据进行分析
	map<string, string> mapvalue;
	string strsection = "";
	int  beginnospace;
	int  endnospace;
	char szline[4096*10];
	char * pcontent;
	char * psection;
	char * pequal;
	char * start = buffer;
	char * podoa = strstr(start, "\r\n");
	while(podoa != NULL)
	{
		// 得到一行数据
		memset(szline, 0, sizeof(szline));
		memcpy(szline, start, min(sizeof(szline)-10, podoa-start));
		if(szline[0] == '\0')
		{
			// 空行不需要处理直接找下一行
			start = podoa+2;
			podoa = strstr(start, "\r\n");
			continue;
		}

		// 去掉这行数据中有注释的地方
		pcontent = strstr(szline, "//");
		if(pcontent!=NULL && pcontent!=szline && *(pcontent-1)!=':')
		{
			*pcontent = '\0';
		}
		if(szline[0] == '\0')
		{
			// 空行不需要处理直接找下一行
			start = podoa+2;
			podoa = strstr(start, "\r\n");
			continue;
		}

		// 查找是否有段标志（判断标准是第一个字节为[，而且后面有]）
		psection = strstr(szline, "]");
		if(szline[0]=='[' && psection!=NULL)
		{
			// 如果有段标志的话则保存上一个段标志信息
			if(!strsection.empty() && mapvalue.size()>0)
			{
				m_mapvalue.insert(map<string, map<string, string> >::value_type(strsection.c_str(), mapvalue));
				mapvalue.clear();
			}

			// 记录段名称
			*psection = '\0';
			strsection = szline+1;
		}
		else if(!strsection.empty())
		{
			// 没有段标志则找数据标志
			pequal = strstr(szline, "=");
			if(pequal != NULL)
			{
				// 首先找字段名
				*pequal = '\0';
				beginnospace = 0;
				while(szline[beginnospace]==' ' || szline[beginnospace]=='\t')
				{
					// 清空字段名头部空格和制表符
					beginnospace++;
				}
				if(beginnospace < (pequal-szline))
				{
					// 清空字段名尾部空格和制表符
					endnospace = pequal-szline-1;
					while(szline[endnospace]==' ' || szline[endnospace]=='\t')
					{
						endnospace--;
					}
					szline[endnospace+1] = '\0';

					// 其次找到字段值
					endnospace = pequal-szline+1;
					while(szline[endnospace]==' ' || szline[endnospace]=='\t')
					{
						// 清空字段值头部空格和制表符
						endnospace++;
					}
					if(szline[endnospace] != '\0')
					{
						start = szline+endnospace;

						// 清空字段值尾部空格和制表符
						endnospace = strlen(start)-1;
						while(*(start+endnospace)==' ' || *(start+endnospace)=='\t')
						{
							endnospace--;
						}
						*(start+endnospace+1) = '\0';

						// 添加字段名和字段值到map中
						mapvalue.insert(map<string, string>::value_type(szline+beginnospace, start));
					}
				}
			}
		}
		
		// 找下一行
		start = podoa+2;
		podoa = strstr(start, "\r\n");
	}

	// 保存最后一个
	if(!strsection.empty() && mapvalue.size()>0)
	{
		m_mapvalue.insert(map<string, map<string, string> >::value_type(strsection.c_str(), mapvalue));
	}

	mapvalue.clear();
	delete buffer;
	buffer = NULL;
	memcpy(m_filename, filename, min(sizeof(m_filename)-1, strlen(filename)));
	return 0;
}

// 创建文件
int CYTIniFile::Create(const char * filename)
{
	if(filename == NULL)
	{
		return -1;
	}
	
	// 关闭现有文件并保存好文件名
	Close();
	memcpy(m_filename, filename, min(sizeof(m_filename)-1, strlen(filename)));

	return 0;
}

// 关闭文件
void CYTIniFile::Close(void)
{
	memset(m_filename, 0, sizeof(m_filename));

	map<string, map<string, string> >::iterator iter = m_mapvalue.begin();
	while(iter != m_mapvalue.end())
	{
		iter->second.clear();
		iter++;
	}
	m_mapvalue.clear();

	m_bModify = false;
}

// 保存文件
int CYTIniFile::Save(const char * filename)
{
	// 如果没有文件或者文件没有修改过则不存盘
	if((filename==NULL) && (!m_bModify || m_filename[0]=='\0'))
	{
		return -1;
	}

	// 打开指定文件并获取文件长度
	if(filename == NULL)
	{
		if(CYTFile::Open(m_filename) < 0)
		{
			if(CYTFile::Create(m_filename) < 0)
			{
				return -1;
			}
		}
	}
	else
	{
		if(CYTFile::Open(filename) < 0)
		{
			if(CYTFile::Create(filename) < 0)
			{
				return -1;
			}
		}
	}
	unsigned int length = CYTFile::Length();
	if(length == 0xFFFFFFFF)
	{
		CYTFile::Close();
		return -1;
	}
	CYTFile::Seek(FILESEEK_BEGIN, 0);

	// 循环写入各个段、字段名和字段值信息
	unsigned int totallen = 0;
	unsigned int writelen = 0;
	char buffer[4096*10];
	map<string, string>::iterator iter2;
	map<string, map<string, string> >::iterator iter = m_mapvalue.begin();
	while(iter != m_mapvalue.end())
	{
		// 写入段
		if(iter->second.size() > 0)
		{
			sprintf_s(buffer, sizeof(buffer), "[%s]\r\n", iter->first.c_str());
			writelen = (unsigned int)(strlen(buffer));
			CYTFile::Seek(FILESEEK_BEGIN, totallen);
			CYTFile::Write(buffer, writelen);
			totallen += writelen;

			// 写入字段和字段值
			iter2 = iter->second.begin();
			while(iter2 != iter->second.end())
			{
				sprintf_s(buffer, sizeof(buffer), "%s=%s\r\n", iter2->first.c_str(), iter2->second.c_str());
				writelen = (unsigned int)(strlen(buffer));
				CYTFile::Seek(FILESEEK_BEGIN, totallen);
				CYTFile::Write(buffer, writelen);
				totallen += writelen;

				iter2++;
			}

			// 写入一个空行区分下一个段
			sprintf_s(buffer, sizeof(buffer), "\r\n");
			writelen = (unsigned int)(strlen(buffer));
			CYTFile::Seek(FILESEEK_BEGIN, totallen);
			CYTFile::Write(buffer, writelen);
			totallen += writelen;
		}

		iter++;
	}

	// 定位文件大小为写入的长度
	CYTFile::SetLength(totallen);

	CYTFile::Close();
	return 0;
}

// 读取字符串型数据
const char * CYTIniFile::ReadString(const char * section, const char * key, const char * defaulevalue)
{
	if(section==NULL || strlen(section)==0 || key==NULL || strlen(key)==0)
	{
		return defaulevalue;
	}

	map<string, map<string, string> >::iterator iter = m_mapvalue.find(section);
	if(iter != m_mapvalue.end())
	{
		map<string, string>::iterator iter2 = iter->second.find(key);
		if(iter2 != iter->second.end())
		{
			return iter2->second.c_str();
		}
	}

	return defaulevalue;
}

// 读取整型数据
int CYTIniFile::ReadInteger(const char * section, const char * key, int defaulevalue)
{
	if(section==NULL || strlen(section)==0 || key==NULL || strlen(key)==0)
	{
		return defaulevalue;
	}

	map<string, map<string, string> >::iterator iter = m_mapvalue.find(section);
	if(iter != m_mapvalue.end())
	{
		map<string, string>::iterator iter2 = iter->second.find(key);
		if(iter2 != iter->second.end())
		{
			return strtol(iter2->second.c_str(), NULL, 10);
		}
	}
	
	return defaulevalue;
}

// 读取浮点型数据
double CYTIniFile::ReadDouble(const char * section, const char * key, double defaulevalue)
{
	if(section==NULL || strlen(section)==0 || key==NULL || strlen(key)==0)
	{
		return defaulevalue;
	}
	
	map<string, map<string, string> >::iterator iter = m_mapvalue.find(section);
	if(iter != m_mapvalue.end())
	{
		map<string, string>::iterator iter2 = iter->second.find(key);
		if(iter2 != iter->second.end())
		{
			return strtod(iter2->second.c_str(), NULL);
		}
	}
	
	return defaulevalue;
}

// 读取bool型数据
bool CYTIniFile::ReadBool(const char * section, const char * key, bool defaulevalue)
{
	if(section==NULL || strlen(section)==0 || key==NULL || strlen(key)==0)
	{
		return defaulevalue;
	}
	
	map<string, map<string, string> >::iterator iter = m_mapvalue.find(section);
	if(iter != m_mapvalue.end())
	{
		map<string, string>::iterator iter2 = iter->second.find(key);
		if(iter2 != iter->second.end())
		{
			if((iter2->second.compare("yes") == 0)
				|| (iter2->second.compare("YES") == 0)
				|| (iter2->second.compare("true") == 0)
				||(iter2->second.compare("TRUE") == 0))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	
	return defaulevalue;
}

// 写入字符串型数据
int CYTIniFile::WriteString(const char * section, const char * key, const char * value)
{
	if(section==NULL || strlen(section)==0 || key==NULL || strlen(key)==0 || value==NULL)
	{
		return -1;
	}

	map<string, map<string, string> >::iterator iter = m_mapvalue.find(section);
	if(iter != m_mapvalue.end())
	{
		iter->second[key] = value;
	}
	else
	{
		map<string, string> mapvalue;
		mapvalue[key] = value;
		m_mapvalue[section] = mapvalue;
	}
	
	m_bModify = true;
	return 0;
}

// 写入整型数据
int CYTIniFile::WriteInteger(const char * section, const char * key, int value)
{
	char buffer[512];
	sprintf_s(buffer, sizeof(buffer), "%d", value);
	return WriteString(section, key, buffer);
}

// 写入浮点型数据
int CYTIniFile::WriteDouble(const char * section, const char * key, double value)
{
	char buffer[512];
	sprintf_s(buffer, sizeof(buffer), "%f", value);
	return WriteString(section, key, buffer);
}

// 写入bool型数据
int CYTIniFile::WriteBool(const char * section, const char * key, bool value)
{
	char buffer[512];
	if(value == true)
	{
		sprintf_s(buffer, sizeof(buffer), "true");
	}
	else
	{
		sprintf_s(buffer, sizeof(buffer), "false");
	}
	return WriteString(section, key, buffer);
}

// 添加一个段
int CYTIniFile::AddSection(const char * section)
{
	if(section == NULL)
	{
		return -1;
	}

	map<string, map<string, string> >::iterator iter = m_mapvalue.find(section);
	if(iter == m_mapvalue.end())
	{
		map<string, string> mapvalue;
		m_mapvalue[section] = mapvalue;
		m_bModify = true;
		return 0;
	}
	else
	{
		return -1;
	}
}

// 删除一个段
int CYTIniFile::DelSection(const char * section)
{
	if(section == NULL)
	{
		return -1;
	}
	
	map<string, map<string, string> >::iterator iter = m_mapvalue.find(section);
	if(iter != m_mapvalue.end())
	{
		iter->second.clear();
		m_mapvalue.erase(iter);
		m_bModify = true;
		return 0;
	}
	else
	{
		return -1;
	}
}

// 删除一个Key项
int CYTIniFile::DelKey(const char * section, const char * key)
{
	if(section==NULL || key==NULL)
	{
		return -1;
	}
	
	map<string, map<string, string> >::iterator iter = m_mapvalue.find(section);
	if(iter != m_mapvalue.end())
	{
		map<string, string>::iterator iter2 = iter->second.find(key);
		if(iter2 != iter->second.end())
		{
			iter->second.erase(iter2);
			m_bModify = true;
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}