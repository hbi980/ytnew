#pragma once

#include "../../../YTPublic/YTInclude/YTUnit/YTUnit.h"

#include "../../../YTPublic/YTInclude/YTBase/YTData.h"
#pragma comment(lib, "../../../YTPublic/YTInclude/YTBase/YTData.lib")

#include "../../../YTPublic/YTInclude/YTBase/YTConn.h"

#include <stdint.h>
#include <vector>
using namespace std;

#ifndef _YTData
# define _YTData void
#endif

// ͨѶ����
#pragma pack(1)
typedef struct
{
    uint16_t	ver;			//Э��汾��ͬʱ���ڼ���С�ˣ�Ĭ��Ϊ1
    uint32_t	len;			//���ݳ��ȣ�����Э��汾��
    uint32_t	funcid;			//���ܺ�
    uint32_t	chunnelid;		//ͨ����
    uint32_t	serailid;		//ͨ�����
    uint32_t	checksum;		//У��ͣ���Ҫ���ڼ���ת�������ܵ���ϢУ��
    uint32_t	userid;			//�û����
    uint32_t	result;			//ϵͳ������
    uint32_t	reqserailid;	//������Ϣ����
 } USDataHeader;				// ��Ϣͷ
#pragma pack()

// ��������ַ�ṹ����
typedef struct 
{
	char			address[64];	// ��������ַ��֧��������ַ��
	int				port;			// �������˿�
} tagServer;

// ����ṹ��Ϣ
typedef struct 
{
	int				reqno;			// ������
	unsigned int	funcid;			// ���ܺ�
	int				sendtime;		// ������ʱ��
} tagReq;

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

// ȫ�ֺ�������ȡ�����ţ���YTConn.cpp��ʵ��
int GlobGetReqNo(void);

// ����ͨѶ���������
class CYTCom : CYTSocket
{
public:
	CYTCom(void);
	~CYTCom(void);

// ������������
public:
	int Init(CYTIniFile * cfgfile);													// ��ʼ�����ã����󴴽���ֻ�������һ�Σ�
	int ConnectServer(void);														// ���ӷ�����
	int CloseConnect(void);															// �ر��������������
	int GetStatus(void);															// ��ȡ����״̬
	int SendData(unsigned int funcid, const char * data, unsigned int len);			// �������ݣ������ڲ����������
	const char * GetServer(int * port);												// ��ȡ��ǰ���ӵķ�������ַ

// ��̬��������
public:
	static int Pack(tagYTConnPackHead * phead, const char * data, int len, const char * outbuf, int * outlen);	// �������ݴ�������δ���������һ�������Ӧ���������Ӧ�����ݻ��������
	static int UnPack(const char * data, int len, tagYTConnPackHead * phead, const char * outbuf, int * outlen);// Ӧ�����ݽ����ÿ�ν����һ��ҵ�������

// ��������
private:
	bool BeginThread(void);															// ���������߳�
	void StopThread(void);															// ֹͣ�����߳�

	int Connect_Single(void);														// ��˳�����ӷ�����
	int Connect_Muti(void);															// ͬʱ���Ӷ��������

	int ProvConn(char * data, int len, const char * cryptkey, int keylen);			// ������֤����
	int ProvData(char * data, int len);												// ���ݴ���
	int DataRet(int reqno, unsigned int funcid);									// Ӧ�𷵻ظ�������map
	int CheckTimeOut(void);															// ��ⳬʱ������
	void CloseAllReq(void);															// �ر�����������Ϣ��Ŀǰ��ʾ�ͻ��˳�ʱ

// ��̬˽�к���
private:
	static unsigned int __stdcall RecvThread(void * pParam);						// �����߳�

// ˽�г�Ա
private:
	int					m_sysstatus;			// ϵͳ״̬��0-�ر� 1-������
	int					m_currserverid;			// ��ǰ���ӵķ��������
	CYTThread			m_thread;				// �����߳�

	unsigned int		m_chunnelid;			// ͨ���ţ��ڵ�һ����¼���з��أ��������ӶϿ��Ժ�ÿ�ζ��ø�ͨ���ţ�
	unsigned int		m_userid;				// �û���ţ��ڵ�һ����¼���з��أ��������ӶϿ��Ժ�ÿ�ζ��ø��û���ţ�
	
	map<int, tagReq>	m_reqmap;				// ��������û�нӵ�Ӧ��ļ�¼
	CYTLock				m_reqmaplock;			// ��¼������

	CYTLock				m_socksendlock;			// socket���Ϳ����������������ʱ��Ҫ��socket���п��ƣ�
	CYTLock				m_sockrecvlock;			// socket���տ�����������ط���Ҫ�������ݻ��߼��socketʱ��Ҫ���ƣ�

private:
	// �����ļ��еĺ�������ص���Ϣ
	bool				m_loadcfg;				// �Ƿ���ع�����
	vector<tagServer>	m_serveraddr;			// ��������ַ��Ϣ
	int					m_conncreatetimeout;	// ���ӽ�����ʱʱ�䣨���룩
	int					m_connclosetimeout;		// ���ӹرճ�ʱʱ�䣨���룩
	int					m_sendandrecvtimeout;	// ���ݴ��䳬ʱʱ�䣨�룩
	int					m_transtimeout;			// ҵ��ʱʱ�䣨�룩
};

extern tagYTConnCallBackFunc	g_CallBackFList;	// �ص�����
extern CYTLogFile				g_LogFile;			// ��־�ļ�

