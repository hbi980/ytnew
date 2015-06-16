#pragma once

#include "YTDbObject.h"

class CYTDbPool
{
public:
	CYTDbPool(void);
	~CYTDbPool(void);

// ������������
public:
	int Init(int index, DBInfo dbinfo, int maxconn, int iniconn);	// �������ݿ���Ϣ
	int GetDBInfo(char * serverip, int serveriplen, int * port, 
				  char * dbname, int dbnamelen, 
				  int * maxconn, int * usedconn, int * waitconn);	// ��ȡ��ǰ������ݿ���Ϣ

	CYTDbObject * PopDB(int timeout = 5000);						// ��ȡһ��DB����
	int PushDB(CYTDbObject * pdb);									// �ͷ�һ��DB���ӣ�������������ø�״̬����ɾ������
	
	bool CheckConn(void);											// ������ݿ��Ƿ���ͨ
	void RemoveIdle(int timeout = 300);								// ������в��õ�DB����

// ˽�г�Ա
private:
	int						m_index;		// ���ݿ���

	DBInfo					m_dbinfo;		// ���ݿ���Ϣ
	int						m_maxconn;		// ��������������

	map<CYTDbObject *, int>	m_useddb;		// ʹ�õ�����
	map<CYTDbObject *, int>	m_nouseddb;		// δʹ�õ�����

	CYTLock					m_section;		// ���Ӷ��������
	CYTSemaphore			m_semaphore;	// ������ӿ����ź���

	CYTSocket				m_socket;		// ��ͨ�Լ����
	OCI_Connection *		m_ociconnect;	// ��ͨ�Լ����
};

