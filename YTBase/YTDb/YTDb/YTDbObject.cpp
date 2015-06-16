#include "YTDbObject.h"
#include <time.h>

#define MAXPARAM_PROC_INT		30	// 存储过程整型入参个数限制
#define MAXPARAM_PROC_DOUBLE	30	// 存储过程浮点型形入参个数限制
#define MAXPARAM_PROC_STRING	30	// 存储过程字符型入参个数限制

// 全局函数：获取oracle错误信息
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

	// 初始化的时候就先建立一个与数据库的连接，可以节省后续请求创建连接的时间
	if(g_CurrDbIndex>=0 && !Connect())
	{
		char szerrmsg[512] = {0};
		sprintf_s(szerrmsg, sizeof(szerrmsg), "初始化创建于数据库[%s,%d,%s,%s,%s]的连接失败：", m_dbinfo.serverip, m_dbinfo.serverport, m_dbinfo.dbname, m_dbinfo.uid, m_dbinfo.pwd);
		GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
		sprintf_s(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg), "\r\n");
		GlobalLogout(LT_SYS, LOG_ERROR, szerrmsg);
	}
}

CYTDbObject::~CYTDbObject(void)
{
	Close();
}

// 获取数据库编号
int CYTDbObject::GetIndex(void)
{
	return m_index;
}

// 设置最后一次使用时间
void CYTDbObject::SetLastUseTime(int lastusetime)
{
	m_bused = true;
	m_lastusetime = lastusetime;
}

// 获取最后一次使用时间
int CYTDbObject::GetLastUseTime(void)
{
	return m_lastusetime;
}

// 判断是否被使用过
bool CYTDbObject::IsUsed(void)
{
	return m_bused;
}

// 判断连接句柄是否有效
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

// 关闭连接
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

// 建立与数据库的连接
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
			// 设置自动提交
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

// 执行一个SQL，无返回结果集
int CYTDbObject::ExecuteSql(const char * sqlcmd, const char * errmsg, int msglen)
{
	// 检测参数是否有效
	if(sqlcmd == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "传入sql为空", min(msglen-1, 11));
		}
		return -1;
	}

	try
	{
		// 判断状态，如果断开了则重新建立连接
		if(!IsOpen())
		{
			Close();
			if(!Connect())
			{
				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "连接数据库失败：");
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				return -1;
			}
		}

		// 创建一个statement
		OCI_Statement * ocistatement = OCI_StatementCreate(m_ociconnect);
		if(ocistatement == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "分配statement失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			Close();
			return -1;
		}

		// 执行sql
		if(OCI_ExecuteStmt(ocistatement, sqlcmd) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "执行sql失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;

			return -1;
		}
		else
		{
			// 释放statement
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

// 执行一个SQL，返回结果记录集
int CYTDbObject::OpenSql(const char * sqlcmd, const char * errmsg, int msglen, void * datahandle)
{
	// 检测参数是否有效
	if(sqlcmd==NULL || datahandle==NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "传入sql或数据集为空", min(msglen-1, 19));
		}
		return -1;
	}
	YTData_ResetData(datahandle);

	try
	{
		// 判断状态，如果断开了则重新建立连接
		if(!IsOpen())
		{
			Close();
			if(!Connect())
			{
				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "连接数据库失败：");
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				return -1;
			}
		}
		
		// 创建一个statement
		OCI_Statement * ocistatement = OCI_StatementCreate(m_ociconnect);
		if(ocistatement == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "分配statement失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			Close();
			return -1;
		}
		
		// 执行sql
		if(OCI_ExecuteStmt(ocistatement, sqlcmd) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "执行sql失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}
		
		// 获取结果集
		OCI_Resultset * ociresultset = OCI_GetResultset(ocistatement);
		if(ociresultset != NULL)
		{
			// 轮寻获取每条记录
			char			name[128];
			int				columncount;
			int				i;
			OCI_Column *	ocicolumn;
			const char *	columnname;
			const char *	columnvalue;
			while(OCI_FetchNext(ociresultset) == TRUE)
			{
				// 增加一条记录
				YTData_AppendRow(datahandle);

				// 设置字段信息
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

			// 释放结果集
			OCI_ReleaseResultsets(ocistatement);
			ociresultset = NULL;
		}

		// 释放statement
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

// 执行一个存储过程，无返回结果集
int CYTDbObject::ExecuteProc(const char * procname, void * prochandle, const char * errmsg, int msglen)
{
	// 检测参数是否有效
	if(procname == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "传入存储过程名为空", min(msglen-1, 18));
		}
		return -1;
	}

	try
	{
		// 判断状态，如果断开了则重新建立连接
		if(!IsOpen())
		{
			Close();
			if(!Connect())
			{
				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "连接数据库失败：");
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				return -1;
			}
		}
		
		// 创建一个statement
		OCI_Statement * ocistatement = OCI_StatementCreate(m_ociconnect);
		if(ocistatement == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "分配statement失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			Close();
			return -1;
		}

		// 构造执行的sql
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
			sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程准备失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// 设置存储过程各个入参
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
			// 转到指定记录
			YTData_GotoFixRow(prochandle, i);

			// 轮寻不同的字段类型
			sprintf_s(szKey, sizeof(szKey), ":v%d", i);
			switch(YTData_GetFieldInt(prochandle, "fieldtype"))
			{
			default:
			case FT_STRING:	// 字符串型
			case FT_CHAR:	// 字符型
				memset(szBuffer, 0, sizeof(szBuffer));
				YTData_GetFieldString(prochandle, "fieldvalue", szBuffer, sizeof(szBuffer));
				paramstring[index_string] = (string)szBuffer;
				if(OCI_BindString(ocistatement, szKey, (char *)paramstring[index_string].c_str(), paramstring[index_string].length()) == FALSE)
				{
					finderr = true;
				}
				index_string++;
				break;
			case FT_INTEGER:// 整型
				paramint[index_int] = YTData_GetFieldInt(prochandle, "fieldvalue");
				if(OCI_BindInt(ocistatement, szKey, &paramint[index_int]) == FALSE)
				{
					finderr = true;
				}
				index_int++;
				break;
			case FT_DOUBLE:	// 浮点型
				paramdouble[index_double] = YTData_GetFieldDouble(prochandle, "fieldvalue");
				if(OCI_BindDouble(ocistatement, szKey, &paramdouble[index_double]) == FALSE)
				{
					finderr = true;
				}
				index_double++;
				break;
			}

			// 如果有错误则直接返回
			if(finderr)
			{
				memset(szBuffer, 0, sizeof(szBuffer));
				YTData_GetFieldString(prochandle, "fieldname", szBuffer, sizeof(szBuffer));

				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程参数[%s]添加失败：", szBuffer);
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				
				// 释放statement
				OCI_StatementFree(ocistatement);
				ocistatement = NULL;
				
				return -1;
			}
		}
		
		// 添加存储过程返回结果出参
		int retcode = 0;
		if(OCI_BindInt(ocistatement, ":errcode", &retcode) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程参数[o_ErrorCode]添加失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// 添加存储过程返回错误信息出参
		char retmsg[512] = {0};
		if(OCI_BindString(ocistatement, ":errmsg", retmsg, sizeof(retmsg)) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程参数[o_ErrorCode]添加失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// 执行sql
		if(OCI_Execute(ocistatement) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "执行存储过程失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// 获取返回结果看是否成功
		if(retcode < 0)
		{
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, retmsg, min(msglen-1, (int)strlen(retmsg)));
			}

			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}
		else
		{
			// 释放statement
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

// 执行一个存储过程，返回结果记录集
int CYTDbObject::OpenProc(const char * procname, void * prochandle, const char * errmsg, int msglen, void * datahandle)
{
	// 检测参数是否有效
	if(procname==NULL || datahandle==NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "传入存储过程名或数据集为空", min(msglen-1, 26));
		}
		return -1;
	}
	YTData_ResetData(datahandle);

	try
	{
		// 判断状态，如果断开了则重新建立连接
		if(!IsOpen())
		{
			Close();
			if(!Connect())
			{
				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "连接数据库失败：");
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				return -1;
			}
		}
		
		// 创建一个statement
		OCI_Statement * ocistatement = OCI_StatementCreate(m_ociconnect);
		if(ocistatement == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "分配statement失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			Close();
			return -1;
		}

		// 构造执行的sql
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
			sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程准备失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// 设置存储过程各个入参
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
			// 转到指定记录
			YTData_GotoFixRow(prochandle, i);
			
			// 轮寻不同的字段类型
			sprintf_s(szKey, sizeof(szKey), ":v%d", i);
			switch(YTData_GetFieldInt(prochandle, "fieldtype"))
			{
			default:
			case FT_STRING:	// 字符串型
			case FT_CHAR:	// 字符型
				memset(szBuffer, 0, sizeof(szBuffer));
				YTData_GetFieldString(prochandle, "fieldvalue", szBuffer, sizeof(szBuffer));
				paramstring[index_string] = (string)szBuffer;
				if(OCI_BindString(ocistatement, szKey, (char *)paramstring[index_string].c_str(), paramstring[index_string].length()) == FALSE)
				{
					finderr = true;
				}
				index_string++;
				break;
			case FT_INTEGER:// 整型
				paramint[index_int] = YTData_GetFieldInt(prochandle, "fieldvalue");
				if(OCI_BindInt(ocistatement, szKey, &paramint[index_int]) == FALSE)
				{
					finderr = true;
				}
				index_int++;
				break;
			case FT_DOUBLE:	// 浮点型
				paramdouble[index_double] = YTData_GetFieldDouble(prochandle, "fieldvalue");
				if(OCI_BindDouble(ocistatement, szKey, &paramdouble[index_double]) == FALSE)
				{
					finderr = true;
				}
				index_double++;
				break;
			}
			
			// 如果有错误则直接返回
			if(finderr)
			{
				memset(szBuffer, 0, sizeof(szBuffer));
				YTData_GetFieldString(prochandle, "fieldname", szBuffer, sizeof(szBuffer));
				
				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程参数[%s]添加失败：", szBuffer);
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				
				// 释放statement
				OCI_StatementFree(ocistatement);
				ocistatement = NULL;
				
				return -1;
			}
		}

		// 添加存储过程返回游标出参
		OCI_Statement * ocistatementref = OCI_StatementCreate(m_ociconnect);
		if(ocistatementref == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程参数[o_resultset]创建失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			Close();
			return -1;
		}
		if(OCI_BindStatement(ocistatement, ":resultset", ocistatementref) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程参数[o_resultset]添加失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// 释放游标statement
			OCI_StatementFree(ocistatementref);
			ocistatementref = NULL;
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// 添加存储过程返回结果出参
		int retcode = 0;
		if(OCI_BindInt(ocistatement, ":errcode", &retcode) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程参数[o_ErrorCode]添加失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// 释放游标statement
			OCI_StatementFree(ocistatementref);
			ocistatementref = NULL;
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// 添加存储过程返回错误信息出参
		char retmsg[512] = {0};
		if(OCI_BindString(ocistatement, ":errmsg", retmsg, sizeof(retmsg)) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程参数[o_ErrorCode]添加失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// 释放游标statement
			OCI_StatementFree(ocistatementref);
			ocistatementref = NULL;
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// 执行sql
		if(OCI_Execute(ocistatement) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "执行存储过程失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// 释放游标statement
			OCI_StatementFree(ocistatementref);
			ocistatementref = NULL;
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// 获取返回结果看是否成功
		if(retcode < 0)
		{
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, retmsg, min(msglen-1, (int)strlen(retmsg)));
			}

			// 释放游标statement
			OCI_StatementFree(ocistatementref);
			ocistatementref = NULL;

			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}
		else
		{
			// 获取结果集
			OCI_Resultset * ociresultset = OCI_GetResultset(ocistatementref);
			if(ociresultset != NULL)
			{
				// 轮寻获取每条记录
				char			name[128];
				int				columncount;
				int				i;
				OCI_Column *	ocicolumn;
				const char *	columnname;
				const char *	columnvalue;
				while(OCI_FetchNext(ociresultset) == TRUE)
				{
					// 增加一条记录
					YTData_AppendRow(datahandle);
					
					// 设置字段信息
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
				
				// 释放结果集
				OCI_ReleaseResultsets(ocistatementref);
				ociresultset = NULL;
			}

			// 释放游标statement
			OCI_StatementFree(ocistatementref);
			ocistatementref = NULL;

			// 释放statement
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

// 执行一个存储过程，无返回结果集（table入参）
int CYTDbObject::ExecuteProcTable(const char * procname, const char * paraname, void * parahandle, const char * errmsg, int msglen)
{
	// 检测参数是否有效
	if(procname == NULL)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "传入存储过程名为空", min(msglen-1, 18));
		}
		return -1;
	}

	if(paraname==NULL || parahandle==NULL || YTData_GetRowCount(parahandle)<=0)
	{
		if(errmsg!=NULL && msglen>0)
		{
			memset((char *)errmsg, 0, msglen);
			memcpy((char *)errmsg, "传入存储过程table参数为空", min(msglen-1, 25));
		}
		return -1;
	}

	try
	{
		// 判断状态，如果断开了则重新建立连接
		if(!IsOpen())
		{
			Close();
			if(!Connect())
			{
				char szerrmsg[512] = {0};
				sprintf_s(szerrmsg, sizeof(szerrmsg), "连接数据库失败：");
				GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
				if(errmsg!=NULL && msglen>0)
				{
					memset((char *)errmsg, 0, msglen);
					memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
				}
				return -1;
			}
		}
		
		// 创建一个statement
		OCI_Statement * ocistatement = OCI_StatementCreate(m_ociconnect);
		if(ocistatement == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "分配statement失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			Close();
			return -1;
		}

		// 创建一个table对象
		int paracount = YTData_GetRowCount(parahandle);
		char szparaname[128] = {0};
		memcpy(szparaname, paraname, min(sizeof(szparaname)-1, strlen(paraname)));
		_strupr(szparaname);

		OCI_TypeInfo * ocitypeinfo = OCI_TypeInfoGet(m_ociconnect, szparaname, OCI_TIF_TYPE);
		if(ocitypeinfo == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "创建OCI_TypeInfo对象[%s,%d]失败：", szparaname, paracount);
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}
		
		OCI_Object ** ociobject = OCI_ObjectArrayCreate(m_ociconnect, ocitypeinfo, paracount);
		if(ociobject == NULL)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "分配objectarray[%s,%d]失败：", szparaname, paracount);
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			Close();
			return -1;
		}
		
		// 构造table入参
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
			// 转到指定记录
			YTData_GotoFixRow(parahandle, i);

			// 轮寻写入所有值
			iter = fields.begin();
			while(iter != fields.end())
			{
				// 获取字段名（获取后转为大写到数据库）
				memset(szfieldname, 0, sizeof(szfieldname));
				memcpy(szfieldname, iter->first.c_str(), iter->first.length());
				_strupr(szfieldname);
				
				// 轮寻不同的字段类型
				switch(iter->second)
				{
				default:
				case FT_STRING:	// 字符串型
				case FT_CHAR:	// 字符型
					memset(szfieldvalue, 0, sizeof(szfieldvalue));
					YTData_GetFieldString(parahandle, iter->first.c_str(), szfieldvalue, sizeof(szfieldvalue));
					if(OCI_ObjectSetString(ociobject[i-1], szfieldname, szfieldvalue) == FALSE)
					{
						finderr = true;
					}
					break;
				case FT_INTEGER:// 整型
					if(OCI_ObjectSetInt(ociobject[i-1], szfieldname, YTData_GetFieldInt(parahandle, iter->first.c_str())) == FALSE)
					{
						finderr = true;
					}
					break;
				case FT_DOUBLE:	// 浮点型
					if(OCI_ObjectSetDouble(ociobject[i-1], szfieldname, YTData_GetFieldDouble(parahandle, iter->first.c_str())) == FALSE)
					{
						finderr = true;
					}
					break;
				}
				
				// 如果有错误则直接返回
				if(finderr)
				{
					char szerrmsg[512] = {0};
					sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程table[%s,%d,%d]参数[%s]添加失败：", szparaname, paracount, i, szfieldname);
					GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
					if(errmsg!=NULL && msglen>0)
					{
						memset((char *)errmsg, 0, msglen);
						memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
					}
					
					// 释放map
					fields.clear();
					
					// 释放object
					OCI_ObjectArrayFree(ociobject);
					ociobject = NULL;
					
					// 释放statement
					OCI_StatementFree(ocistatement);
					ocistatement = NULL;
					
					return -1;
				}
				
				iter++;
			}
		}

		// 构造执行的sql
		char sqlcmd[512];
		sprintf_s(sqlcmd, sizeof(sqlcmd), "begin %s(:v1,:errcode,:errmsg); end;", procname);
		if(OCI_Prepare(ocistatement, sqlcmd) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程准备失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// 释放map
			fields.clear();

			// 释放object
			OCI_ObjectArrayFree(ociobject);
			ociobject = NULL;
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// 添加存储过程table入参
		if(OCI_BindArraySetSize(ocistatement, paracount) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程table大小[%d]设置失败：", paracount);
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// 释放map
			fields.clear();
			
			// 释放object
			OCI_ObjectArrayFree(ociobject);
			ociobject = NULL;
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}
		if(OCI_BindArrayOfObjects(ocistatement, ":v1", ociobject, OCI_TypeInfoGet(m_ociconnect, szparaname, OCI_TIF_TYPE), 0) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程objectarray绑定失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}
			
			// 释放map
			fields.clear();

			// 释放object
			OCI_ObjectArrayFree(ociobject);
			ociobject = NULL;
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}
		
		// 添加存储过程返回结果出参
		int * retcode = new int[paracount];
		if(OCI_BindArrayOfInts(ocistatement, ":errcode", retcode, 0) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程参数[o_ErrorCode]添加失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// 删除变量
			delete []retcode;

			// 释放map
			fields.clear();

			// 释放object
			OCI_ObjectArrayFree(ociobject);
			ociobject = NULL;
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// 添加存储过程返回错误信息出参
		char * retmsg = new char[128*paracount];
		memset(retmsg, 0, sizeof(retmsg));
		if(OCI_BindArrayOfStrings(ocistatement, ":errmsg", retmsg, 127, 0) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "存储过程参数[o_ErrorCode]添加失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// 删除变量
			delete []retcode;
			delete []retmsg;

			// 释放map
			fields.clear();

			// 释放object
			OCI_ObjectArrayFree(ociobject);
			ociobject = NULL;
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// 执行sql
		if(OCI_Execute(ocistatement) == FALSE)
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "执行存储过程失败：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			if(errmsg!=NULL && msglen>0)
			{
				memset((char *)errmsg, 0, msglen);
				memcpy((char *)errmsg, szerrmsg, min(msglen-1, (int)strlen(szerrmsg)));
			}

			// 删除变量
			delete []retcode;
			delete []retmsg;

			// 释放map
			fields.clear();

			// 释放object
			OCI_ObjectArrayFree(ociobject);
			ociobject = NULL;
			
			// 释放statement
			OCI_StatementFree(ocistatement);
			ocistatement = NULL;
			
			return -1;
		}

		// 获取返回结果看是否成功
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

		// 删除变量
		delete []retcode;
		delete []retmsg;

		// 释放map
		fields.clear();

		// 释放object
		OCI_ObjectArrayFree(ociobject);
		ociobject = NULL;

		// 释放statement
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