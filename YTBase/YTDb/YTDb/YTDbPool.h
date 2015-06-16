#pragma once

#include "YTDbObject.h"

class CYTDbPool
{
public:
	CYTDbPool(void);
	~CYTDbPool(void);

// 公共导出函数
public:
	int Init(int index, DBInfo dbinfo, int maxconn, int iniconn);	// 设置数据库信息
	int GetDBInfo(char * serverip, int serveriplen, int * port, 
				  char * dbname, int dbnamelen, 
				  int * maxconn, int * usedconn, int * waitconn);	// 获取当前活动的数据库信息

	CYTDbObject * PopDB(int timeout = 5000);						// 获取一个DB连接
	int PushDB(CYTDbObject * pdb);									// 释放一个DB连接，这里仅仅是设置个状态并不删除连接
	
	bool CheckConn(void);											// 检测数据库是否连通
	void RemoveIdle(int timeout = 300);								// 清除空闲不用的DB连接

// 私有成员
private:
	int						m_index;		// 数据库编号

	DBInfo					m_dbinfo;		// 数据库信息
	int						m_maxconn;		// 允许的最大连接数

	map<CYTDbObject *, int>	m_useddb;		// 使用的连接
	map<CYTDbObject *, int>	m_nouseddb;		// 未使用的连接

	CYTLock					m_section;		// 连接对象控制锁
	CYTSemaphore			m_semaphore;	// 最大连接控制信号量

	CYTSocket				m_socket;		// 连通性检测用
	OCI_Connection *		m_ociconnect;	// 连通性检测用
};

