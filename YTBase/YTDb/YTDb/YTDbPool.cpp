#include "YTDbPool.h"
#include <time.h>

CYTDbPool::CYTDbPool(void)
{
	m_index			= -1;
	memset(&m_dbinfo, 0, sizeof(DBInfo));
	m_maxconn		= 0;
	m_ociconnect	= NULL;
}

CYTDbPool::~CYTDbPool(void)
{
	// 释放所有数据库连接
	m_section.Lock();

	CYTDbObject * pDb = NULL;
	map<CYTDbObject *, int>::iterator iter = m_useddb.begin();
	while(iter != m_useddb.end())
	{
		pDb = iter->first;
		iter = m_useddb.erase(iter);
		if(pDb != NULL)
		{
			delete pDb;
			pDb = NULL;
		}
	}
	m_useddb.clear();

	iter = m_nouseddb.begin();
	while(iter != m_nouseddb.end())
	{
		pDb = iter->first;
		iter = m_useddb.erase(iter);
		if(pDb != NULL)
		{
			delete pDb;
			pDb = NULL;
		}
	}
	m_nouseddb.clear();

	m_section.UnLock();

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
	}
}

// 设置数据库信息
int CYTDbPool::Init(int index, DBInfo dbinfo, int maxconn, int iniconn)
{
	// 保存数据库编号
	m_index = index;

	// 保存数据库信息
	memcpy(&m_dbinfo, &dbinfo, sizeof(DBInfo));
	m_maxconn = maxconn;

	// 初始化信号量
	m_semaphore.Init(m_maxconn, m_maxconn);

	// 初始化的时候就先创建好连接，避免执行过程中创建连接损耗时间
	if(CheckConn())
	{
		CYTDbObject * pdb;
		for(int i=0; i<iniconn; i++)
		{
			pdb = new CYTDbObject(m_index, m_dbinfo);
			if(pdb != NULL)
			{
				m_nouseddb[pdb] = 1;
			}
		}
	}

	return 0;
}

// 获取当前活动的数据库信息
int CYTDbPool::GetDBInfo(char * serverip, int serveriplen, int * port, char * dbname, int dbnamelen, int * maxconn, int * usedconn, int * waitconn)
{
	if(serverip != NULL)
	{
		sprintf_s(serverip, serveriplen, "%s", m_dbinfo.serverip);
	}

	*port = m_dbinfo.serverport;

	if(dbname != NULL)
	{
		sprintf_s(dbname, dbnamelen, "%s", m_dbinfo.dbname);
	}

	m_section.Lock();

	*maxconn = m_maxconn;
	*usedconn = m_useddb.size();
	*waitconn = m_nouseddb.size();

	m_section.UnLock();

	return 0;
}

// 获取一个DB连接
CYTDbObject * CYTDbPool::PopDB(int timeout)
{
	// 等待信号量
	if(m_semaphore.Lock(timeout) < 0)
	{
		return NULL;
	}

	// 获取一个没有使用的DB连接
	CYTDbObject * ret = NULL;

	m_section.Lock();

	map<CYTDbObject *, int>::iterator iter = m_nouseddb.begin();
	if(iter != m_nouseddb.end())
	{
		ret = iter->first;
		m_nouseddb.erase(iter);
	}

	// 如果没有取到的话则判断连接数，没有超过最大的话则创建一个
	if(ret==NULL && ((int)(m_useddb.size()+m_nouseddb.size()) < m_maxconn))
	{
		ret = new CYTDbObject(m_index, m_dbinfo);
	}

	// 将连接放到已使用池中
	if(ret != NULL)
	{
		m_useddb[ret] = 1;
	}
	else
	{
		// 减少信号量
		m_semaphore.UnLock();
	}

	m_section.UnLock();

	return ret;
}

// 释放一个DB连接，这里仅仅是设置个状态并不删除连接
int CYTDbPool::PushDB(CYTDbObject * pdb)
{
	if(pdb == NULL)
	{
		return -1;
	}

	int ret = 0;

	m_section.Lock();

	map<CYTDbObject *, int>::iterator iter = m_useddb.find(pdb);
	if(iter != m_useddb.end())
	{
		m_useddb.erase(iter);

		pdb->SetLastUseTime((int)time(NULL));
		m_nouseddb[pdb] = 1;

		// 减少信号量
		m_semaphore.UnLock();
	}
	else
	{
		ret = -1;
	}

	m_section.UnLock();

	return ret;
}

// 检测数据库是否连通
bool CYTDbPool::CheckConn()
{
	// 使用socket检测端口是否通
	if(!m_socket.IsValid())
	{
		m_socket.Close();
		if(m_socket.Create()<0 || m_socket.Connect(m_dbinfo.serverip, m_dbinfo.serverport, 1000)<0)
		{
			m_socket.Close();
			return false;
		}
		int i;
		for(i=0; i<5; i++)
		{
			if(m_socket.Select(SOCKSEL_WRITE, 0) == SOCKSEL_WRITE)
			{
				break;
			}
			::Sleep(1000);
		}
		if(i == 5)
		{
			m_socket.Close();
			return false;
		}
	}

	__try
	{
		// 如果原来已经建立了数据库连接则判断属性
		if(m_ociconnect != NULL)
		{
			if(OCI_IsConnected(m_ociconnect) == TRUE)
			{
				return true;
			}
			else
			{
				OCI_ConnectionFree(m_ociconnect);
				m_ociconnect = NULL;
			}
		}
		
		// 如果失败的话则这里再创建一次连接看看
		char szdb[256];
		sprintf_s(szdb, sizeof(szdb), "%s:%d/%s", m_dbinfo.serverip, m_dbinfo.serverport, m_dbinfo.dbname);
		m_ociconnect = OCI_ConnectionCreate(szdb, m_dbinfo.uid, m_dbinfo.pwd, OCI_SESSION_DEFAULT);
		if(m_ociconnect != NULL)
		{
			return true;
		}
		else
		{
			char szerrmsg[512] = {0};
			sprintf_s(szerrmsg, sizeof(szerrmsg), "检测到数据库断开：");
			GetOracleErr(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg));
			sprintf_s(szerrmsg+strlen(szerrmsg), sizeof(szerrmsg)-strlen(szerrmsg), "\r\n");
			GlobalLogout(LT_SYS, LOG_ERROR, szerrmsg);
			return false;
		}
	}
	__except(true)
	{
		return false;
	}
}

// 清除空闲不用的DB连接
void CYTDbPool::RemoveIdle(int timeout)
{
	m_section.Lock();

	int currtime = (int)time(NULL);
	CYTDbObject * pdb;

	map<CYTDbObject *, int>::iterator iter = m_nouseddb.begin();
	while(iter != m_nouseddb.end())
	{
		pdb = iter->first;
		if(pdb == NULL)
		{
			iter = m_nouseddb.erase(iter);
			continue;
		}
		else if(pdb->IsUsed() && (currtime - pdb->GetLastUseTime() > timeout))
		{
			iter = m_nouseddb.erase(iter);
			delete pdb;
			pdb = NULL;
			continue;
		}

		iter++;
	}
	
	m_section.UnLock();
}