#pragma once

#include "../../../YTPublic/YTInclude/YTUnit/YTUnit.h"

#include "../../../YTPublic/YTInclude/YTBase/YTData.h"
#pragma comment(lib, "../../../YTPublic/YTInclude/YTBase/YTData.lib")

#include "../../../YTPublic/YTInclude/YTBase/ocilib.h"
#pragma comment(lib, "../../../YTPublic/YTInclude/YTBase/ociliba.lib")

// ȫ�ֺ������壨��ȡoracle������Ϣ��
void GetOracleErr(const char * errmsg, int len);

enum ENum_LogLevel					// ��־������
{
	LOG_NORMAL	= 0,				// ������־
	LOG_WARNING	= 1,				// ������־
	LOG_ERROR	= 2,				// ������־
	LOG_SUCCESS	= 3,				// ĳ���ض��ɹ���Ϣ
};

// ȫ�ֺ�������־��Ϣ
// type				��־��𣬲μ�ENum_LogType����
// level			��־���𣬲μ�ENum_LogLevel����
// log				��־��Ϣ
void GlobalLogout(int type, int level, const char * format, ...);

// ���ݿ���Ϣ�ṹ
struct DBInfo
{
	char	serverip[128];	// ���ݿ������IP��ַ
	int		serverport;		// ���ݿ�������˿�
	char	dbname[128];	// ���ݿ�����
	char	uid[128];		// �����û���
	char	pwd[128];		// ��������
};

// ���ݿ������
class CYTDbObject
{
public:
	CYTDbObject(int index, DBInfo dbinfo);
	~CYTDbObject(void);

// ������������
public:
	int GetIndex(void);																		// ��ȡ���ݿ���

	void SetLastUseTime(int lastusetime);													// �������һ��ʹ��ʱ��
	int GetLastUseTime(void);																// ��ȡ���һ��ʹ��ʱ��
	bool IsUsed(void);																		// �ж��Ƿ�ʹ�ù�

	bool IsOpen(void);																		// �ж����Ӿ���Ƿ���Ч
	void Close(void);																		// �ر�����
	bool Connect(void);																		// ���������ݿ������

	int ExecuteSql(const char * sqlcmd, const char * errmsg, int msglen);					// ִ��һ��SQL���޷��ؽ����
	int OpenSql(const char * sqlcmd, const char * errmsg, int msglen, void * datahandle);	// ִ��һ��SQL�����ؽ����¼��

	int ExecuteProc(const char * procname, void * prochandle, 
					const char * errmsg, int msglen);										// ִ��һ���洢���̣��޷��ؽ����
	int OpenProc(const char * procname, void * prochandle, 
				 const char * errmsg, int msglen, void * datahandle);						// ִ��һ���洢���̣����ؽ����¼��
	
	int ExecuteProcTable(const char * procname, const char * paraname, void * parahandle, 
						 const char * errmsg, int msglen);									// ִ��һ���洢���̣��޷��ؽ������table��Σ�

// ˽�г�Ա
private:
	int					m_index;		// ���ݿ���
	DBInfo				m_dbinfo;		// ���ݿ���Ϣ
	bool				m_bused;		// �Ƿ�ʹ�ù����ñ�־��Ҫ���Ʒ�������ʼ��ʱ��ܳ�ʱ��ɾ���ѽ��������ݿ����ӵ����⣩
	int					m_lastusetime;	// �ϴ�ʹ��ʱ��
	OCI_Connection *	m_ociconnect;	// ���Ӷ���ָ��
};

extern	CYTLogFile	g_LogFile;			// ��־�ļ�
extern	int			g_CurrDbIndex;		// ���ݿ�״̬����

