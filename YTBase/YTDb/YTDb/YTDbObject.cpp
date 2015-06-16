#include "YTDbObject.h"
#include <time.h>

#define MAXPARAM_PROC_INT		30	// �洢����������θ�������
#define MAXPARAM_PROC_DOUBLE	30	// �洢���̸���������θ�������
#define MAXPARAM_PROC_STRING	30	// �洢�����ַ�����θ�������

// ȫ�ֺ�������ȡoracle������Ϣ
void GetOracleErr(const char * errmsg, int len)
{
	try
	{
		OCI_Error * ocierr = OCI_GetLastError();
		if(ocierr != NULL)
		{
			const char * ocierrmsg = OCI_ErrorGetString(ocierr);
			if(ocierrmsg != NULL)
			{
				if(errmsg!=NULL && len>0)
				{
					memset((char *)errmsg, 0, len);
					memcpy((char *)errmsg, ocierrmsg, min(len-1, (int)strlen(ocierrmsg)));
				}
			}
			else
			{
				if(errmsg!=NULL && len>0)
				{
					memset((char *)errmsg, 0, len);
				}
			}
		}
		else
		{
			if(errmsg!=NULL && len>0)
			{
				memset((char *)errmsg, 0, len);
			}
		}
	}
	catch(...)
	{
		if(errmsg!=NULL && len>0)
		{
			memset((char *)errmsg, 0, len);
			memcpy((char *)errmsg, "GetOracleErr exception", min(len-1, 22));
		}
	}
}

CYTDbObject::CYTDbObject(int index, DBInfo dbinfo)
{
	m_index			= index;
	memcpy(&m_dbinfo, &dbinfo, sizeof(DBInfo));
	m_bused			= false;
	m_lastusetime	= (int)time(NULL);
	m_ociconnect	= NULL;

	// ��ʼ����ʱ����Ƚ���һ�������ݿ�����ӣ����Խ�ʡ�������󴴽����ӵ�ʱ��
	if(g_CurrDbIndex>=0 && !Connect())
	{
		char szerrmsg[512] = {0};
		sprintf_s(szerrmsg, sizeof(szerrmsg), "��ʼ�����������ݿ�[%s,%d,%s,%s,%s]������ʧ�ܣ�", m_dbinfo.serverip, m_dbinfo.serverport, m_dbinfo.dbname, m_dbinfo.uid, m_dbinfo.pwd);
		GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
		sprintf_s(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg), "\r\n");
		GlobalLogout(LT_SYS, LOG_ERROR, szerrmsg);
	}
}

CYTDbObject::~CYTDbObject(void)
{
	Close();
}

// ��ȡ���ݿ���
int CYTDbObject::GetIndex(void)
{
	return m_index;
}

// �������һ��ʹ��ʱ��
void CYTDbObject::SetLastUseTime(int lastusetime)
{
	m_bused = true;
	m_lastusetime = lastusetime;
}

// ��ȡ���һ��ʹ��ʱ��
int CYTDbObject::GetLastUseTime(void)
{
	return m_lastusetime;
}

// �ж��Ƿ�ʹ�ù�
bool CYTDbObject::IsUsed(void)
{
	return m_bused;
}

// �ж����Ӿ���Ƿ���Ч
bool CYTDbObject::IsOpen(void)
{
	try
	{
		if(m_ociconnect!=NULL && OCI_IsConnected(m_ociconnect)==TRUE)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	catch(...)
	{
		return false;
	}
}

// �ر�����
void CYTDbObject::Close(void)
{
	try
	{
		if(m_ociconnect != NULL)
		{
			OCI_ConnectionFree(m_ociconnect);
			m_ociconnect = NULL;
		}
	}
	catch(...)
	{
		return;
	}
}

// ���������ݿ������
bool CYTDbObject::Connect(void)
{
	if(g_CurrDbIndex < 0)
	{
		return false;
	}

	try
	{
		char szdb[256];
		sprintf_s(szdb, sizeof(szdb), "%s:%d/%s", m_dbinfo.serverip, m_dbinfo.serverport, m_dbinfo.dbname);
		m_ociconnect = OCI_ConnectionCreate(szdb, m_dbinfo.uid, m_dbinfo.pwd, OCI_SESSION_DEFAULT);
		if(m_ociconnect != NULL)
		{
			// �����Զ��ύ
			if(OCI_SetAutoCommit(m_ociconnect, TRUE) == TRUE)
			{
				return true;
			}
			else
			{
				OCI_ConnectionFree(m_ociconnect);
				m_ociconnect = NULL;
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	catch(...)
	{
		return false;
	}
}

// ִ��һ��SQL���޷��ؽ����
int CYTDbObject::ExecuteSql(const char * sqlcmd, const char * errmsg, int msglen)
{
	// �������Ƿ���Ч
	if(sqlcmd == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "����sqlΪ��", min(msglen-1, 11));
		}
		return -1;
	}

	try
	{
		// �ж�״̬������Ͽ��������½�������
		if(!IsOpen())
		{
			Close();
			if(!Connect())
			{
				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "�������ݿ�ʧ�ܣ�");
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				return -1;
			}
		}

		// ����һ��statement
		OCI_Statement * ocistatement = OCI_StatementCreate(m_ociconnect);
		if(ocistatement == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "����statementʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			Close();
			return -1;
		}

		// ִ��sql
		if(OCI_ExecuteStmt(ocistatement, sqlcmd) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "ִ��sqlʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;

			return -1;
		}
		else
		{
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return 0;
		}
	}
	catch(...)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "ExecuteSql exception", min(msglen-1, 20));
		}
		return -1;
	}
}

// ִ��һ��SQL�����ؽ����¼��
int CYTDbObject::OpenSql(const char * sqlcmd, const char * errmsg, int msglen, void * datahandle)
{
	// �������Ƿ���Ч
	if(sqlcmd==NULL || datahandle==NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "����sql�����ݼ�Ϊ��", min(msglen-1, 19));
		}
		return -1;
	}
	YTData_ResetData(datahandle);

	try
	{
		// �ж�״̬������Ͽ��������½�������
		if(!IsOpen())
		{
			Close();
			if(!Connect())
			{
				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "�������ݿ�ʧ�ܣ�");
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				return -1;
			}
		}
		
		// ����һ��statement
		OCI_Statement * ocistatement = OCI_StatementCreate(m_ociconnect);
		if(ocistatement == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "����statementʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			Close();
			return -1;
		}
		
		// ִ��sql
		if(OCI_ExecuteStmt(ocistatement, sqlcmd) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "ִ��sqlʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}
		
		// ��ȡ�����
		OCI_Resultset * ociresultset = OCI_GetResultset(ocistatement);
		if(ociresultset != NULL)
		{
			// ��Ѱ��ȡÿ����¼
			char			name[128];
			int				columncount;
			int				i;
			OCI_Column *	ocicolumn;
			const char *	columnname;
			const char *	columnvalue;
			while(OCI_FetchNext(ociresultset) == TRUE)
			{
				// ����һ����¼
				YTData_AppendRow(datahandle);

				// �����ֶ���Ϣ
				columncount = OCI_GetColumnCount(ociresultset);
				for(i=1; i<=columncount; i++)
				{
					ocicolumn = OCI_GetColumn(ociresultset, i);
					if(ocicolumn != NULL)
					{
						columnname = OCI_ColumnGetName(ocicolumn);
						if(columnname != NULL)
						{
							sprintf_s(name, sizeof(name), columnname);
							_strlwr(name);

							if(OCI_IsNull(ociresultset, i) == TRUE)
							{
								YTData_AddFieldString(datahandle, name, "");
							}
							else
							{
								columnvalue	= OCI_GetString(ociresultset, i);
								YTData_AddFieldString(datahandle, name, columnvalue);
							}
						}
					}
				}
			}

			// �ͷŽ����
			OCI_ReleaseResultsets(ocistatement);
			ociresultset = NULL;
		}

		// �ͷ�statement
		OCI_StatementFree(ocistatement);
		ocistatement = NULL;
		
		return 0;
	}
	catch(...)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "OpenSql exception", min(msglen-1, 17));
		}
		return -1;
	}
}

// ִ��һ���洢���̣��޷��ؽ����
int CYTDbObject::ExecuteProc(const char * procname, void * prochandle, const char * errmsg, int msglen)
{
	// �������Ƿ���Ч
	if(procname == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "����洢������Ϊ��", min(msglen-1, 18));
		}
		return -1;
	}

	try
	{
		// �ж�״̬������Ͽ��������½�������
		if(!IsOpen())
		{
			Close();
			if(!Connect())
			{
				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "�������ݿ�ʧ�ܣ�");
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				return -1;
			}
		}
		
		// ����һ��statement
		OCI_Statement * ocistatement = OCI_StatementCreate(m_ociconnect);
		if(ocistatement == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "����statementʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			Close();
			return -1;
		}

		// ����ִ�е�sql
		char sqlcmd[512] = {0};
		int count = 0;
		if(prochandle != NULL)
		{
			count = YTData_GetRowCount(prochandle);
		}
		if(count <= 0)
		{
			sprintf_s(sqlcmd+strlen(sqlcmd), sizeof(sqlcmd)-strlen(sqlcmd), "begin %s(:errcode,:errmsg); end;", procname);
		}
		else
		{
			sprintf_s(sqlcmd+strlen(sqlcmd), sizeof(sqlcmd)-strlen(sqlcmd), "begin %s(", procname);
			for(int i=1; i<=count; i++)
			{
				sprintf_s(sqlcmd+strlen(sqlcmd), sizeof(sqlcmd)-strlen(sqlcmd), ":v%d,", i);
			}
			sprintf_s(sqlcmd+strlen(sqlcmd), sizeof(sqlcmd)-strlen(sqlcmd), ":errcode,:errmsg); end;");
		}
		if(OCI_Prepare(ocistatement, sqlcmd) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢����׼��ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// ���ô洢���̸������
		int		paramint[MAXPARAM_PROC_INT];
		double	paramdouble[MAXPARAM_PROC_DOUBLE];
		string	paramstring[MAXPARAM_PROC_STRING];
		int		index_int = 0;
		int		index_double = 0;
		int		index_string = 0;
		char	szBuffer[512];
		char	szKey[64];
		bool	finderr = false;
		for(int i=1; i<=count; i++)
		{
			// ת��ָ����¼
			YTData_GotoFixRow(prochandle, i);

			// ��Ѱ��ͬ���ֶ�����
			sprintf_s(szKey, sizeof(szKey), ":v%d", i);
			switch(YTData_GetFieldInt(prochandle, "fieldtype"))
			{
			default:
			case FT_STRING:	// �ַ�����
			case FT_CHAR:	// �ַ���
				memset(szBuffer, 0, sizeof(szBuffer));
				YTData_GetFieldString(prochandle, "fieldvalue", szBuffer, sizeof(szBuffer));
				paramstring[index_string] = (string)szBuffer;
				if(OCI_BindString(ocistatement, szKey, (char *)paramstring[index_string].c_str(), paramstring[index_string].length()) == FALSE)
				{
					finderr = true;
				}
				index_string++;
				break;
			case FT_INTEGER:// ����
				paramint[index_int] = YTData_GetFieldInt(prochandle, "fieldvalue");
				if(OCI_BindInt(ocistatement, szKey, &paramint[index_int]) == FALSE)
				{
					finderr = true;
				}
				index_int++;
				break;
			case FT_DOUBLE:	// ������
				paramdouble[index_double] = YTData_GetFieldDouble(prochandle, "fieldvalue");
				if(OCI_BindDouble(ocistatement, szKey, &paramdouble[index_double]) == FALSE)
				{
					finderr = true;
				}
				index_double++;
				break;
			}

			// ����д�����ֱ�ӷ���
			if(finderr)
			{
				memset(szBuffer, 0, sizeof(szBuffer));
				YTData_GetFieldString(prochandle, "fieldname", szBuffer, sizeof(szBuffer));

				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢���̲���[%s]���ʧ�ܣ�", szBuffer);
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				
				// �ͷ�statement
				OCI_StatementFree(ocistatement);
				ocistatement = NULL;
				
				return -1;
			}
		}
		
		// ��Ӵ洢���̷��ؽ������
		int retcode = 0;
		if(OCI_BindInt(ocistatement, ":errcode", &retcode) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢���̲���[o_ErrorCode]���ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// ��Ӵ洢���̷��ش�����Ϣ����
		char retmsg[512] = {0};
		if(OCI_BindString(ocistatement, ":errmsg", retmsg, sizeof(retmsg)) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢���̲���[o_ErrorCode]���ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// ִ��sql
		if(OCI_Execute(ocistatement) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "ִ�д洢����ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// ��ȡ���ؽ�����Ƿ�ɹ�
		if(retcode < 0)
		{
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, retmsg, min(msglen-1, (int)strlen(retmsg)));
			}

			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}
		else
		{
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return 0;
		}
	}
	catch(...)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "ExecuteProc exception", min(msglen-1, 21));
		}
		return -1;
	}
}

// ִ��һ���洢���̣����ؽ����¼��
int CYTDbObject::OpenProc(const char * procname, void * prochandle, const char * errmsg, int msglen, void * datahandle)
{
	// �������Ƿ���Ч
	if(procname==NULL || datahandle==NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "����洢�����������ݼ�Ϊ��", min(msglen-1, 26));
		}
		return -1;
	}
	YTData_ResetData(datahandle);

	try
	{
		// �ж�״̬������Ͽ��������½�������
		if(!IsOpen())
		{
			Close();
			if(!Connect())
			{
				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "�������ݿ�ʧ�ܣ�");
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				return -1;
			}
		}
		
		// ����һ��statement
		OCI_Statement * ocistatement = OCI_StatementCreate(m_ociconnect);
		if(ocistatement == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "����statementʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			Close();
			return -1;
		}

		// ����ִ�е�sql
		char sqlcmd[512] = {0};
		int count = 0;
		if(prochandle != NULL)
		{
			count = YTData_GetRowCount(prochandle);
		}
		if(count <= 0)
		{
			sprintf_s(sqlcmd+strlen(sqlcmd), sizeof(sqlcmd)-strlen(sqlcmd), "begin %s(:resultset,:errcode,:errmsg); end;", procname);
		}
		else
		{
			sprintf_s(sqlcmd+strlen(sqlcmd), sizeof(sqlcmd)-strlen(sqlcmd), "begin %s(", procname);
			for(int i=1; i<=count; i++)
			{
				sprintf_s(sqlcmd+strlen(sqlcmd), sizeof(sqlcmd)-strlen(sqlcmd), ":v%d,", i);
			}
			sprintf_s(sqlcmd+strlen(sqlcmd), sizeof(sqlcmd)-strlen(sqlcmd), ":resultset,:errcode,:errmsg); end;");
		}
		if(OCI_Prepare(ocistatement, sqlcmd) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢����׼��ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// ���ô洢���̸������
		int		paramint[MAXPARAM_PROC_INT];
		double	paramdouble[MAXPARAM_PROC_DOUBLE];
		string	paramstring[MAXPARAM_PROC_STRING];
		int		index_int = 0;
		int		index_double = 0;
		int		index_string = 0;
		char	szBuffer[512];
		char	szKey[64];
		bool	finderr = false;
		for(int i=1; i<=count; i++)
		{
			// ת��ָ����¼
			YTData_GotoFixRow(prochandle, i);
			
			// ��Ѱ��ͬ���ֶ�����
			sprintf_s(szKey, sizeof(szKey), ":v%d", i);
			switch(YTData_GetFieldInt(prochandle, "fieldtype"))
			{
			default:
			case FT_STRING:	// �ַ�����
			case FT_CHAR:	// �ַ���
				memset(szBuffer, 0, sizeof(szBuffer));
				YTData_GetFieldString(prochandle, "fieldvalue", szBuffer, sizeof(szBuffer));
				paramstring[index_string] = (string)szBuffer;
				if(OCI_BindString(ocistatement, szKey, (char *)paramstring[index_string].c_str(), paramstring[index_string].length()) == FALSE)
				{
					finderr = true;
				}
				index_string++;
				break;
			case FT_INTEGER:// ����
				paramint[index_int] = YTData_GetFieldInt(prochandle, "fieldvalue");
				if(OCI_BindInt(ocistatement, szKey, &paramint[index_int]) == FALSE)
				{
					finderr = true;
				}
				index_int++;
				break;
			case FT_DOUBLE:	// ������
				paramdouble[index_double] = YTData_GetFieldDouble(prochandle, "fieldvalue");
				if(OCI_BindDouble(ocistatement, szKey, &paramdouble[index_double]) == FALSE)
				{
					finderr = true;
				}
				index_double++;
				break;
			}
			
			// ����д�����ֱ�ӷ���
			if(finderr)
			{
				memset(szBuffer, 0, sizeof(szBuffer));
				YTData_GetFieldString(prochandle, "fieldname", szBuffer, sizeof(szBuffer));
				
				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢���̲���[%s]���ʧ�ܣ�", szBuffer);
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				
				// �ͷ�statement
				OCI_StatementFree(ocistatement);
				ocistatement = NULL;
				
				return -1;
			}
		}

		// ��Ӵ洢���̷����α����
		OCI_Statement * ocistatementref = OCI_StatementCreate(m_ociconnect);
		if(ocistatementref == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢���̲���[o_resultset]����ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			Close();
			return -1;
		}
		if(OCI_BindStatement(ocistatement, ":resultset", ocistatementref) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢���̲���[o_resultset]���ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// �ͷ��α�statement
			OCI_StatementFree(ocistatementref);
			ocistatementref = NULL;
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// ��Ӵ洢���̷��ؽ������
		int retcode = 0;
		if(OCI_BindInt(ocistatement, ":errcode", &retcode) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢���̲���[o_ErrorCode]���ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// �ͷ��α�statement
			OCI_StatementFree(ocistatementref);
			ocistatementref = NULL;
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// ��Ӵ洢���̷��ش�����Ϣ����
		char retmsg[512] = {0};
		if(OCI_BindString(ocistatement, ":errmsg", retmsg, sizeof(retmsg)) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢���̲���[o_ErrorCode]���ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// �ͷ��α�statement
			OCI_StatementFree(ocistatementref);
			ocistatementref = NULL;
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// ִ��sql
		if(OCI_Execute(ocistatement) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "ִ�д洢����ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// �ͷ��α�statement
			OCI_StatementFree(ocistatementref);
			ocistatementref = NULL;
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// ��ȡ���ؽ�����Ƿ�ɹ�
		if(retcode < 0)
		{
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, retmsg, min(msglen-1, (int)strlen(retmsg)));
			}

			// �ͷ��α�statement
			OCI_StatementFree(ocistatementref);
			ocistatementref = NULL;

			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}
		else
		{
			// ��ȡ�����
			OCI_Resultset * ociresultset = OCI_GetResultset(ocistatementref);
			if(ociresultset != NULL)
			{
				// ��Ѱ��ȡÿ����¼
				char			name[128];
				int				columncount;
				int				i;
				OCI_Column *	ocicolumn;
				const char *	columnname;
				const char *	columnvalue;
				while(OCI_FetchNext(ociresultset) == TRUE)
				{
					// ����һ����¼
					YTData_AppendRow(datahandle);
					
					// �����ֶ���Ϣ
					columncount = OCI_GetColumnCount(ociresultset);
					for(i=1; i<=columncount; i++)
					{
						ocicolumn = OCI_GetColumn(ociresultset, i);
						if(ocicolumn != NULL)
						{
							columnname = OCI_ColumnGetName(ocicolumn);
							if(columnname != NULL)
							{
								sprintf_s(name, sizeof(name), columnname);
								_strlwr(name);

								if(OCI_IsNull(ociresultset, i) == TRUE)
								{
									YTData_AddFieldString(datahandle, name, "");
								}
								else
								{
									columnvalue	= OCI_GetString(ociresultset, i);
									YTData_AddFieldString(datahandle, name, columnvalue);
								}
							}
						}
					}
				}
				
				// �ͷŽ����
				OCI_ReleaseResultsets(ocistatementref);
				ociresultset = NULL;
			}

			// �ͷ��α�statement
			OCI_StatementFree(ocistatementref);
			ocistatementref = NULL;

			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return 0;
		}
	}
	catch(...)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "OpenProc exception", min(msglen-1, 18));
		}
		return -1;
	}
}

// ִ��һ���洢���̣��޷��ؽ������table��Σ�
int CYTDbObject::ExecuteProcTable(const char * procname, const char * paraname, void * parahandle, const char * errmsg, int msglen)
{
	// �������Ƿ���Ч
	if(procname == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "����洢������Ϊ��", min(msglen-1, 18));
		}
		return -1;
	}

	if(paraname==NULL || parahandle==NULL || YTData_GetRowCount(parahandle)<=0)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "����洢����table����Ϊ��", min(msglen-1, 25));
		}
		return -1;
	}

	try
	{
		// �ж�״̬������Ͽ��������½�������
		if(!IsOpen())
		{
			Close();
			if(!Connect())
			{
				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "�������ݿ�ʧ�ܣ�");
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				return -1;
			}
		}
		
		// ����һ��statement
		OCI_Statement * ocistatement = OCI_StatementCreate(m_ociconnect);
		if(ocistatement == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "����statementʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			Close();
			return -1;
		}

		// ����һ��table����
		int paracount = YTData_GetRowCount(parahandle);
		char szparaname[128] = {0};
		memcpy(szparaname, paraname, min(sizeof(szparaname)-1, strlen(paraname)));
		_strupr(szparaname);

		OCI_TypeInfo * ocitypeinfo = OCI_TypeInfoGet(m_ociconnect, szparaname, OCI_TIF_TYPE);
		if(ocitypeinfo == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "����OCI_TypeInfo����[%s,%d]ʧ�ܣ�", szparaname, paracount);
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}
		
		OCI_Object ** ociobject = OCI_ObjectArrayCreate(m_ociconnect, ocitypeinfo, paracount);
		if(ociobject == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "����objectarray[%s,%d]ʧ�ܣ�", szparaname, paracount);
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			Close();
			return -1;
		}
		
		// ����table���
		map<string, int>	fields;
		int					fieldcount = YTData_GetFieldCount(parahandle);
		const char *		fieldname;
		int					fieldtype;
		for(int i=1; i<=fieldcount; i++)
		{
			fieldname = YTData_GetFieldName(parahandle, i);
			fieldtype = YTData_GetFieldType(parahandle, i);
			if(fieldname!=NULL && strlen(fieldname)!=0 && fieldtype>=0)
			{
				fields[fieldname] = fieldtype;
			}
		}

		char szfieldname[128];
		char szfieldvalue[512];
		bool finderr = false;
		map<string, int>::iterator iter;
		for(int i=1; i<=paracount; i++)
		{
			// ת��ָ����¼
			YTData_GotoFixRow(parahandle, i);

			// ��Ѱд������ֵ
			iter = fields.begin();
			while(iter != fields.end())
			{
				// ��ȡ�ֶ�������ȡ��תΪ��д�����ݿ⣩
				memset(szfieldname, 0, sizeof(szfieldname));
				memcpy(szfieldname, iter->first.c_str(), iter->first.length());
				_strupr(szfieldname);
				
				// ��Ѱ��ͬ���ֶ�����
				switch(iter->second)
				{
				default:
				case FT_STRING:	// �ַ�����
				case FT_CHAR:	// �ַ���
					memset(szfieldvalue, 0, sizeof(szfieldvalue));
					YTData_GetFieldString(parahandle, iter->first.c_str(), szfieldvalue, sizeof(szfieldvalue));
					if(OCI_ObjectSetString(ociobject[i-1], szfieldname, szfieldvalue) == FALSE)
					{
						finderr = true;
					}
					break;
				case FT_INTEGER:// ����
					if(OCI_ObjectSetInt(ociobject[i-1], szfieldname, YTData_GetFieldInt(parahandle, iter->first.c_str())) == FALSE)
					{
						finderr = true;
					}
					break;
				case FT_DOUBLE:	// ������
					if(OCI_ObjectSetDouble(ociobject[i-1], szfieldname, YTData_GetFieldDouble(parahandle, iter->first.c_str())) == FALSE)
					{
						finderr = true;
					}
					break;
				}
				
				// ����д�����ֱ�ӷ���
				if(finderr)
				{
					char szerrmsg[512] = {0};
					sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢����table[%s,%d,%d]����[%s]���ʧ�ܣ�", szparaname, paracount, i, szfieldname);
					GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
					if(errmsg!=NULL && msglen>0)
					{
						memset((char *)errmsg, 0, msglen);
						memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
					}
					
					// �ͷ�map
					fields.clear();
					
					// �ͷ�object
					OCI_ObjectArrayFree(ociobject);
					ociobject = NULL;
					
					// �ͷ�statement
					OCI_StatementFree(ocistatement);
					ocistatement = NULL;
					
					return -1;
				}
				
				iter++;
			}
		}

		// ����ִ�е�sql
		char sqlcmd[512];
		sprintf_s(sqlcmd, sizeof(sqlcmd), "begin %s(:v1,:errcode,:errmsg); end;", procname);
		if(OCI_Prepare(ocistatement, sqlcmd) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢����׼��ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// �ͷ�map
			fields.clear();

			// �ͷ�object
			OCI_ObjectArrayFree(ociobject);
			ociobject = NULL;
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// ��Ӵ洢����table���
		if(OCI_BindArraySetSize(ocistatement, paracount) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢����table��С[%d]����ʧ�ܣ�", paracount);
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// �ͷ�map
			fields.clear();
			
			// �ͷ�object
			OCI_ObjectArrayFree(ociobject);
			ociobject = NULL;
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}
		if(OCI_BindArrayOfObjects(ocistatement, ":v1", ociobject, OCI_TypeInfoGet(m_ociconnect, szparaname, OCI_TIF_TYPE), 0) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢����objectarray��ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// �ͷ�map
			fields.clear();

			// �ͷ�object
			OCI_ObjectArrayFree(ociobject);
			ociobject = NULL;
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}
		
		// ��Ӵ洢���̷��ؽ������
		int * retcode = new int[paracount];
		if(OCI_BindArrayOfInts(ocistatement, ":errcode", retcode, 0) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢���̲���[o_ErrorCode]���ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// ɾ������
			delete []retcode;

			// �ͷ�map
			fields.clear();

			// �ͷ�object
			OCI_ObjectArrayFree(ociobject);
			ociobject = NULL;
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// ��Ӵ洢���̷��ش�����Ϣ����
		char * retmsg = new char[128*paracount];
		memset(retmsg, 0, sizeof(retmsg));
		if(OCI_BindArrayOfStrings(ocistatement, ":errmsg", retmsg, 127, 0) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "�洢���̲���[o_ErrorCode]���ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// ɾ������
			delete []retcode;
			delete []retmsg;

			// �ͷ�map
			fields.clear();

			// �ͷ�object
			OCI_ObjectArrayFree(ociobject);
			ociobject = NULL;
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// ִ��sql
		if(OCI_Execute(ocistatement) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "ִ�д洢����ʧ�ܣ�");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// ɾ������
			delete []retcode;
			delete []retmsg;

			// �ͷ�map
			fields.clear();

			// �ͷ�object
			OCI_ObjectArrayFree(ociobject);
			ociobject = NULL;
			
			// �ͷ�statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// ��ȡ���ؽ�����Ƿ�ɹ�
		int ret = 0;
		for(int i=0; i<paracount; i++)
		{
			if(retcode[i] < 0)
			{
				ret = -1;
				if(errmsg!=NULL && msglen>0 && (msglen-strlen(errmsg)>strlen(retmsg+i*128)))
				{
					if(i == 0)
					{
						memset((char *)errmsg, 0, msglen);
					}
					memcpy((char *)(errmsg+strlen(errmsg)), retmsg+i*128, strlen(retmsg+i*128));
				}
			}
		}

		// ɾ������
		delete []retcode;
		delete []retmsg;

		// �ͷ�map
		fields.clear();

		// �ͷ�object
		OCI_ObjectArrayFree(ociobject);
		ociobject = NULL;

		// �ͷ�statement
		OCI_StatementFree(ocistatement);
		ocistatement = NULL;
		
		return ret;
	}
	catch(...)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "ExecuteProcTable exception", min(msglen-1, 26));
		}
		return -1;
	}
}