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
	// �ͷ��������ݿ�����
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

// �������ݿ���Ϣ
int CYTDbPool::Init(int index, DBInfo dbinfo, int maxconn, int iniconn)
{
	// �������ݿ���
	m_index = index;

	// �������ݿ���Ϣ
	memcpy(&m_dbinfo, &dbinfo, sizeof(DBInfo));
	m_maxconn = maxconn;

	// ��ʼ���ź���
	m_semaphore.Init(m_maxconn, m_maxconn);

	// ��ʼ����ʱ����ȴ��������ӣ�����ִ�й����д����������ʱ��
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

// ��ȡ��ǰ������ݿ���Ϣ
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

// ��ȡһ��DB����
CYTDbObject * CYTDbPool::PopDB(int timeout)
{
	// �ȴ��ź���
	if(m_semaphore.Lock(timeout) < 0)
	{
		return NULL;
	}

	// ��ȡһ��û��ʹ�õ�DB����
	CYTDbObject * ret = NULL;

	m_section.Lock();

	map<CYTDbObject *, int>::iterator iter = m_nouseddb.begin();
	if(iter != m_nouseddb.end())
	{
		ret = iter->first;
		m_nouseddb.erase(iter);
	}

	// ���û��ȡ���Ļ����ж���������û�г������Ļ��򴴽�һ��
	if(ret==NULL && ((int)(m_useddb.size()+m_nouseddb.size()) < m_maxconn))
	{
		ret = new CYTDbObject(m_index, m_dbinfo);
	}

	// �����ӷŵ���ʹ�ó���
	if(ret != NULL)
	{
		m_useddb[ret] = 1;
	}
	else
	{
		// �����ź���
		m_semaphore.UnLock();
	}

	m_section.UnLock();

	return ret;
}

// �ͷ�һ��DB���ӣ�������������ø�״̬����ɾ������
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

		// �����ź���
		m_semaphore.UnLock();
	}
	else
	{
		ret = -1;
	}

	m_section.UnLock();

	return ret;
}

// ������ݿ��Ƿ���ͨ
bool CYTDbPool::CheckConn()
{
	// ʹ��socket���˿��Ƿ�ͨ
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
		// ���ԭ���Ѿ����������ݿ��������ж�����
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
		
		// ���ʧ�ܵĻ��������ٴ���һ�����ӿ���
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
			sprintf_s(szerrmsg, sizeof(szerrmsg), "��⵽���ݿ�Ͽ���");
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

// ������в��õ�DB����
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