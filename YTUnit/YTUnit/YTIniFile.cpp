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

// ���ļ�
int CYTIniFile::Open(const char * filename)
{
	if(filename == NULL)
	{
		return -1;
	}
	
	// �ر������ļ�
	Close();
	
	// ��ָ���ļ������仺��洢�ļ�
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

	// ѭ����ȡÿ�����ݽ��з���
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
		// �õ�һ������
		memset(szline, 0, sizeof(szline));
		memcpy(szline, start, min(sizeof(szline)-10, podoa-start));
		if(szline[0] == '\0')
		{
			// ���в���Ҫ����ֱ������һ��
			start = podoa+2;
			podoa = strstr(start, "\r\n");
			continue;
		}

		// ȥ��������������ע�͵ĵط�
		pcontent = strstr(szline, "//");
		if(pcontent!=NULL && pcontent!=szline && *(pcontent-1)!=':')
		{
			*pcontent = '\0';
		}
		if(szline[0] == '\0')
		{
			// ���в���Ҫ����ֱ������һ��
			start = podoa+2;
			podoa = strstr(start, "\r\n");
			continue;
		}

		// �����Ƿ��жα�־���жϱ�׼�ǵ�һ���ֽ�Ϊ[�����Һ�����]��
		psection = strstr(szline, "]");
		if(szline[0]=='[' && psection!=NULL)
		{
			// ����жα�־�Ļ��򱣴���һ���α�־��Ϣ
			if(!strsection.empty() && mapvalue.size()>0)
			{
				m_mapvalue.insert(map<string, map<string, string> >::value_type(strsection.c_str(), mapvalue));
				mapvalue.clear();
			}

			// ��¼������
			*psection = '\0';
			strsection = szline+1;
		}
		else if(!strsection.empty())
		{
			// û�жα�־�������ݱ�־
			pequal = strstr(szline, "=");
			if(pequal != NULL)
			{
				// �������ֶ���
				*pequal = '\0';
				beginnospace = 0;
				while(szline[beginnospace]==' ' || szline[beginnospace]=='\t')
				{
					// ����ֶ���ͷ���ո���Ʊ��
					beginnospace++;
				}
				if(beginnospace < (pequal-szline))
				{
					// ����ֶ���β���ո���Ʊ��
					endnospace = pequal-szline-1;
					while(szline[endnospace]==' ' || szline[endnospace]=='\t')
					{
						endnospace--;
					}
					szline[endnospace+1] = '\0';

					// ����ҵ��ֶ�ֵ
					endnospace = pequal-szline+1;
					while(szline[endnospace]==' ' || szline[endnospace]=='\t')
					{
						// ����ֶ�ֵͷ���ո���Ʊ��
						endnospace++;
					}
					if(szline[endnospace] != '\0')
					{
						start = szline+endnospace;

						// ����ֶ�ֵβ���ո���Ʊ��
						endnospace = strlen(start)-1;
						while(*(start+endnospace)==' ' || *(start+endnospace)=='\t')
						{
							endnospace--;
						}
						*(start+endnospace+1) = '\0';

						// ����ֶ������ֶ�ֵ��map��
						mapvalue.insert(map<string, string>::value_type(szline+beginnospace, start));
					}
				}
			}
		}
		
		// ����һ��
		start = podoa+2;
		podoa = strstr(start, "\r\n");
	}

	// �������һ��
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

// �����ļ�
int CYTIniFile::Create(const char * filename)
{
	if(filename == NULL)
	{
		return -1;
	}
	
	// �ر������ļ���������ļ���
	Close();
	memcpy(m_filename, filename, min(sizeof(m_filename)-1, strlen(filename)));

	return 0;
}

// �ر��ļ�
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

// �����ļ�
int CYTIniFile::Save(const char * filename)
{
	// ���û���ļ������ļ�û���޸Ĺ��򲻴���
	if((filename==NULL) && (!m_bModify || m_filename[0]=='\0'))
	{
		return -1;
	}

	// ��ָ���ļ�����ȡ�ļ�����
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

	// ѭ��д������Ρ��ֶ������ֶ�ֵ��Ϣ
	unsigned int totallen = 0;
	unsigned int writelen = 0;
	char buffer[4096*10];
	map<string, string>::iterator iter2;
	map<string, map<string, string> >::iterator iter = m_mapvalue.begin();
	while(iter != m_mapvalue.end())
	{
		// д���
		if(iter->second.size() > 0)
		{
			sprintf_s(buffer, sizeof(buffer), "[%s]\r\n", iter->first.c_str());
			writelen = (unsigned int)(strlen(buffer));
			CYTFile::Seek(FILESEEK_BEGIN, totallen);
			CYTFile::Write(buffer, writelen);
			totallen += writelen;

			// д���ֶκ��ֶ�ֵ
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

			// д��һ������������һ����
			sprintf_s(buffer, sizeof(buffer), "\r\n");
			writelen = (unsigned int)(strlen(buffer));
			CYTFile::Seek(FILESEEK_BEGIN, totallen);
			CYTFile::Write(buffer, writelen);
			totallen += writelen;
		}

		iter++;
	}

	// ��λ�ļ���СΪд��ĳ���
	CYTFile::SetLength(totallen);

	CYTFile::Close();
	return 0;
}

// ��ȡ�ַ���������
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

// ��ȡ��������
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

// ��ȡ����������
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

// ��ȡbool������
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

// д���ַ���������
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

// д����������
int CYTIniFile::WriteInteger(const char * section, const char * key, int value)
{
	char buffer[512];
	sprintf_s(buffer, sizeof(buffer), "%d", value);
	return WriteString(section, key, buffer);
}

// д�븡��������
int CYTIniFile::WriteDouble(const char * section, const char * key, double value)
{
	char buffer[512];
	sprintf_s(buffer, sizeof(buffer), "%f", value);
	return WriteString(section, key, buffer);
}

// д��bool������
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

// ���һ����
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

// ɾ��һ����
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

// ɾ��һ��Key��
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