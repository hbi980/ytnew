#pragma once

#include <windows.h>

// ������Ͷ���
enum ENum_SockSel
{
	SOCKSEL_READ		= 0x01,		// ��
	SOCKSEL_WRITE		= 0x02,		// д
	SOCKSEL_READWRITE	= 0x03,		// ��д
	SOCKSEL_EXCEPTION	= 0x04,		// �쳣
	SOCKSEL_ALL			= 0x07,		// ȫ��
};

// �������Ͷ���
enum ENum_SockProxy
{
	SOCKPROXY_NONE		= 0x00,		// ��ʹ�ô���
	SOCKPROXY_SOCK4		= 0x01,		// sock4����
	SOCKPROXY_SOCK5		= 0x02,		// sock5����
	SOCKPROXY_HTTP		= 0x03,		// http����
};

// ����ṹ����
#pragma pack(1)

typedef struct
{
	char 	ip[64];					// ���������ip
	int 	port;					// ����������˿�
	bool	auth;					// �Ƿ���Ҫ��֤
	char	name[255];				// �û���
	int 	namelen;				// �û�������
	char	pwd[255];				// ����
	int		pwdlen;					// ���볤��
} tagProxy;

#pragma pack()

// ͨѶsocket��
class CYTSocket
{
public:
	CYTSocket(void);
	~CYTSocket(void);

public:
	// ��̬��������
	static int InitCondition(void);										// ��ʼ������socket����
	static void ReleaseCondition(void);									// �ͷ�����socket����
	
public:
	// ������������
	void SetProxyInfo(int proxytype, tagProxy proxyinfo);				// ���ô�����Ϣ

	int  Create(void);													// ����socket
	int  Connect(char * ip, int port, int timeout = 0);					// ��������
	int  Close(void);													// �ر�����

	int  Renew(int sock, int type, int status);							// ����һ�����е�socket	
	int  Reset(void);													// ���ö���ע�⣺�������ý������ͷ���Դ���������ر�socket��

	int  SetOption(int level, int name, const char * value, int len);	// �����������
	int  Select(int type, int timeout);									// ��д�쳣��״̬���
	bool IsValid(void);													// �ж�socket�Ƿ���Ч
	bool IsClose(void);													// select�ɶ����ж��Ƿ������ѹرգ�����select�ɶ������̵���
	
	int  SetType(int type);												// ����socket����
	int  GetType(void);													// ��ȡsocket����
	int  GetStatus(void);												// ��ȡsocket״̬
	int  GetSocket(void);												// ��ȡsocket����

	int  Send(char * data, int len, int timeout = 10);					// ��������
	int  Recv(char * data, int len, int timeout = 10, int type = 0);	// ��������

private:
	// ��������
	int  atoSockaddr(char * ip, int port, struct sockaddr_in * addr);	// ��ַ�ṹת��

	unsigned char Chr2Base(char c);
	char Base2Chr(unsigned char c);
	int  Base64Encode(const char * inbuf, char * outbuf);				// base64����
	int  Base64Decode(const char * inbuf, char * outbuf);				// base64����
	
	int  ConnectSock4(char * ip, int port, int timeout = 0);			// sock4��������
	int  ConnectSock5(char * ip, int port, int timeout = 0);			// sock5��������
	int  ConnectHttp(char * ip, int port, int timeout = 0);				// http��������

public:
	// ���г�Ա
	int			m_socket;		// socket����

protected:
	// �ܱ������ʶ���
	int			m_proxytype;	// ��������
	tagProxy	m_proxyinfo;	// ������Ϣ

public:
	// ˽�г�Ա
	int			m_type;			// socket���ͣ�0-ͬ�� 1-�첽
	int			m_status;		// socket״̬��0-δ���� 1-����
};

