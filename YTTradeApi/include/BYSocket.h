/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYSocket.h
* �ļ�ժҪ��ͨѶsocket����
*
* �޸ļ�¼��2014-05-22 saimen����
*/
#ifndef __BYSOCKET_H__
#define __BYSOCKET_H__

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
	long 	port;					// ����������˿�
	bool	auth;					// �Ƿ���Ҫ��֤
	char	name[255];				// �û���
	long 	namelen;				// �û�������
	char	pwd[255];				// ����
	long	pwdlen;					// ���볤��
} tagProxy;

#pragma pack()

// ͨѶsocket��
class CBYSocket
{
public:
	CBYSocket();
	virtual ~CBYSocket();

public:
	// ��̬��������
	static long InitCondition();										// ��ʼ������socket����
	static void ReleaseCondition();										// �ͷ�����socket����
	
public:
	// ������������
	void SetProxyInfo(long proxytype, tagProxy proxyinfo);				// ���ô�����Ϣ

	long Create();														// ����socket
	long Connect(char * ip, long port, long timeout = 0);				// ��������
	long Close();														// �ر�����

	long Renew(long sock, long type, long status);						// ����һ�����е�socket	
	long Reset();														// ���ö���ע�⣺�������ý������ͷ���Դ���������ر�socket��

	long SetOption(int level, int name, const char * value, int len);	// �����������
	long Select(long type, long timeout);								// ��д�쳣��״̬���
	bool IsValid();														// �ж�socket�Ƿ���Ч
	bool IsClose();														// select�ɶ����ж��Ƿ������ѹرգ�����select�ɶ������̵���
	
	long SetType(long type);											// ����socket����
	long GetType();														// ��ȡsocket����
	long GetStatus();													// ��ȡsocket״̬
	long GetSocket();													// ��ȡsocket����

	long Send(char * data, long len, long timeout = 10);				// ��������
	long Recv(char * data, long len, long timeout = 10, long type = 0);	// ��������

private:
	// ��������
	long atoSockaddr(char * ip, long port, struct sockaddr_in * addr);	// ��ַ�ṹת��

	unsigned char Chr2Base(char c);
	char Base2Chr(unsigned char c);
	long Base64Encode(const char * inbuf, char * outbuf);				// base64����
	long Base64Decode(const char * inbuf, char * outbuf);				// base64����
	
	long ConnectSock4(char * ip, long port, long timeout = 0);			// sock4��������
	long ConnectSock5(char * ip, long port, long timeout = 0);			// sock5��������
	long ConnectHttp(char * ip, long port, long timeout = 0);			// http��������

public:
	// ���г�Ա
	long		m_socket;		// socket����

protected:
	// �ܱ������ʶ���
	long		m_proxytype;	// ��������
	tagProxy	m_proxyinfo;	// ������Ϣ

public:
	// ˽�г�Ա
	long		m_type;			// socket���ͣ�0-ͬ�� 1-�첽
	long		m_status;		// socket״̬��0-δ���� 1-����
};

#endif