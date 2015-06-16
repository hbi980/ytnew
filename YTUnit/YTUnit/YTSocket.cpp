#include "../../YTPublic/YTInclude/YTUnit/YTSocket.h"
#include <time.h>
#include <stdio.h>

#include "winsock.h"
#pragma comment(lib,"ws2_32.lib")

CYTSocket::CYTSocket(void)
{
	m_socket	= -1;
	m_type		= 0;
	m_status	= 0;

	m_proxytype = SOCKPROXY_NONE;
	memset(&m_proxyinfo, 0, sizeof(tagProxy));
}

CYTSocket::~CYTSocket(void)
{
	Close();
}

// ��ʼ������socket����
int CYTSocket::InitCondition(void)
{
	WORD Version = MAKEWORD(1, 1);
	WSADATA	WsaData;
	if(::WSAStartup(Version, &WsaData) != 0)
	{
		return -1;
	}
	
	return 0;
}

// �ͷ�����socket����
void CYTSocket::ReleaseCondition(void)
{
	::WSACleanup();
}

// ���ô�����Ϣ
void CYTSocket::SetProxyInfo(int proxytype, tagProxy proxyinfo)
{
	if(proxytype>=SOCKPROXY_NONE && proxytype<=SOCKPROXY_HTTP)
	{
		m_proxytype = proxytype;
		memcpy(&m_proxyinfo, &proxyinfo, sizeof(tagProxy));
	}
}

// ����socket
int CYTSocket::Create()
{
	Close();

	m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if(m_socket != -1)
	{
		m_status = 1;
		return 0;
	}
	else
	{
		m_status = 0;
		return -1;
	}
}

// ��������
int CYTSocket::Connect(char * ip, int port, int timeout)
{
	if(ip==NULL || strlen(ip)==0 || m_socket==-1 || m_status==0)
	{
		return -1;
	}

	char szIP[256];
	int nPort;
	if(m_proxytype != SOCKPROXY_NONE)
	{
		sprintf_s(szIP, sizeof(szIP), m_proxyinfo.ip);
		nPort = m_proxyinfo.port;
	}
	else
	{
		sprintf_s(szIP, sizeof(szIP), ip);
		nPort = port;
	}

	int sockopt = 1;
	::setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&sockopt, sizeof(sockopt));

	struct sockaddr_in serverAddr;
	if(atoSockaddr(szIP, nPort, &serverAddr) < 0)
	{
		return -1;
	}

	int type = GetType();
	SetType(1);
	::connect(m_socket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr_in));
	int checkstatus = Select(SOCKSEL_ALL, timeout);
	if(checkstatus<=0 || ((checkstatus & SOCKSEL_EXCEPTION) == SOCKSEL_EXCEPTION))
	{
		// ��ʱ�����쳣
		SetType(type);
		return -1;
	}
	else
	{
		SetType(type);

		// ����д���Ļ���ת��������������
		switch(m_proxytype)
		{
		case SOCKPROXY_SOCK4:	// sock4����
			return ConnectSock4(ip, port, timeout);
		case SOCKPROXY_SOCK5:	// sock5����
			return ConnectSock5(ip, port, timeout);
		case SOCKPROXY_HTTP:	// http����
			return ConnectHttp(ip, port, timeout);
		case SOCKPROXY_NONE:	// ��ʹ�ô���
		default:
			return 0;
		}
	}
}

// �ر�����
int CYTSocket::Close()
{
	if(m_socket >= 0)
	{
		::closesocket(m_socket);
	}
	m_socket = -1;
	m_status = 0;

	return 0;
}

// ����һ�����е�socket	
int CYTSocket::Renew(int sock, int type, int status)
{
	if(sock<=0 || (type!=0 && type!=1) || (status!=0 && status!=1))
	{
		return -1;
	}

	Close();

	m_socket	= sock;
	m_type		= type;
	m_status	= status;

	return 0;
}

// ���ö���ע�⣺�������ý������ͷ���Դ���������ر�socket��
int CYTSocket::Reset()
{
	m_socket = -1;
	m_type	 = 0;
	m_status = 0;

	return 0;
}

// �����������
int CYTSocket::SetOption(int level, int name, const char * value, int len)
{
	return ::setsockopt(m_socket, level, name, value, len);
}

// ��д�쳣��״̬���
int CYTSocket::Select(int type, int timeout)
{
	if(m_socket < 0)
	{
		return -1;
	}

	fd_set readfds;
	fd_set writefds;
	fd_set exceptionfds;

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptionfds);

	timeval stimeout;
	stimeout.tv_sec = timeout/1000;
	stimeout.tv_usec = (timeout%1000)*1000;

	int errcode = 0;
	switch(type)
	{
	case SOCKSEL_READ:
		FD_SET(m_socket, &readfds);
		errcode = ::select(m_socket+1, &readfds, NULL, NULL, &stimeout);
		break;
	case SOCKSEL_WRITE:
		FD_SET(m_socket, &writefds); //����ǲ������ӳɹ�
		errcode = ::select(m_socket+1, NULL, &writefds, NULL, &stimeout);
		break;
	case SOCKSEL_EXCEPTION:
		FD_SET(m_socket, &exceptionfds); //����ǲ������ӳɹ�
		errcode = ::select(m_socket+1, NULL, NULL, &exceptionfds, &stimeout);
		break;
	case SOCKSEL_READWRITE:
		FD_SET(m_socket, &readfds);
		FD_SET(m_socket, &writefds); //����ǲ������ӳɹ�
		errcode = ::select(m_socket+1, &readfds, &writefds, NULL, &stimeout);
		break;
	default:
		FD_SET(m_socket, &readfds);
		FD_SET(m_socket, &writefds); //����ǲ������ӳɹ�
		FD_SET(m_socket, &exceptionfds); //����ǲ������ӳɹ�
		errcode = ::select(m_socket+1, &readfds, &writefds, &exceptionfds, &stimeout);
		break;
	}

	if(errcode <= 0)
	{
		return 0;
	}

	int ret = 0;
	if(FD_ISSET(m_socket, &readfds)) 
	{
		ret |= SOCKSEL_READ;
	}
	if(FD_ISSET(m_socket, &writefds)) 
	{
		ret |= SOCKSEL_WRITE;
	}
	if(FD_ISSET(m_socket, &exceptionfds)) 
	{
		ret |= SOCKSEL_EXCEPTION;
	}

	return ret;
}

// �ж�socket�Ƿ���Ч
bool CYTSocket::IsValid()
{	
	if((Select(SOCKSEL_EXCEPTION, 0) & SOCKSEL_EXCEPTION) == SOCKSEL_EXCEPTION)
	{
		m_status = 0;
		return false;
	}

	if((Select(SOCKSEL_READ, 0) & SOCKSEL_READ) == SOCKSEL_READ)
	{
		char buf[16];	
		if(recv(m_socket, buf, 1, MSG_PEEK) <= 0)
		{
			m_status = 0;
		}
	}

	if(m_status == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

// select�ɶ����ж��Ƿ������ѹرգ�����select�ɶ������̵���
bool CYTSocket::IsClose()
{
	if((Select(SOCKSEL_READ, 0) & SOCKSEL_READ) == SOCKSEL_READ)
	{
		char data[2] = {0};
		if(recv(m_socket, data, 1, MSG_PEEK) <= 0)
		{
			if(GetLastError() == 10060)
			{
				return false;
			}
			else
			{
				Close();
				return true;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

// ����socket����
int CYTSocket::SetType(int type)
{
	if(type!=0 && type!=1)
	{
		return -1;
	}
	
	if(type == 0)
	{
		unsigned long errcode = 0;
		if(::ioctlsocket(m_socket, FIONBIO, &errcode) == SOCKET_ERROR)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		unsigned long errcode = 1;
		if(::ioctlsocket(m_socket, FIONBIO, &errcode) == SOCKET_ERROR)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
}

// ��ȡsocket����
int CYTSocket::GetType()
{
	return m_type;
}

// ��ȡsocket״̬
int CYTSocket::GetStatus()
{
	return m_status;
}

// ��ȡsocket����
int CYTSocket::GetSocket()
{
	return m_socket;
}

// ��������
int CYTSocket::Send(char * data, int len, int timeout)
{
	int sendlen = 0;
	int ret;
	int startime = (int)time(NULL);

	while(sendlen < len)
	{
		if((Select(SOCKSEL_WRITE, 100) & SOCKSEL_WRITE) != SOCKSEL_WRITE)
		{
			if((time(NULL) - startime) <= timeout)
			{
				::Sleep(30);
				continue;
			}
			else
			{
				return sendlen;
			}
		}

		ret = send(m_socket, data+sendlen, len-sendlen, 0);
		if(ret == 0)
		{
			int ret = GetLastError();
			return -ret;
		}
		else if(ret < 0)
		{
			return sendlen;
		}
		else
		{
			startime = (int)time(NULL);
			sendlen += ret;
		}
	}

	return sendlen;
}

// ��������
int CYTSocket::Recv(char * data, int len, int timeout, int type)
{
	int recvlen = 0;
	int ret;
	int startime = (long)time(NULL);
	
	while(recvlen < len)
	{
		if((Select(SOCKSEL_READ, 100) & SOCKSEL_READ) != SOCKSEL_READ)
		{
			if((time(NULL) - startime) <= timeout)
			{
				::Sleep(30);
				continue;
			}
			else
			{
				return recvlen;
			}
		}
		
		ret = recv(m_socket, data+recvlen, len-recvlen, 0);
		if(ret == 0)
		{
			int ret = GetLastError();
			return -ret;
		}
		else if(ret < 0)
		{
			return recvlen;
		}
		else
		{
			startime = (int)time(NULL);
			recvlen += ret;
			if(type == 0)
			{
				return recvlen;
			}
		}
	}
	
	return recvlen;
}

// ��ַ�ṹת��
int CYTSocket::atoSockaddr(char * ip, int port, struct sockaddr_in * addr)
{
	unsigned int naddr = ::inet_addr(ip);
	if(naddr == INADDR_NONE)
	{
		hostent * hp = ::gethostbyname(ip);
		if(hp != NULL)
		{
			addr->sin_family = hp->h_addrtype;
			memcpy(&(addr->sin_addr.s_addr), hp->h_addr, hp->h_length);
		}
		else
		{
			return -1;
		}
	}
	else
	{
		addr->sin_family = AF_INET;
		addr->sin_addr.S_un.S_addr = naddr;
	}
	addr->sin_port = ::htons((unsigned short)port);

	return 0;
}

unsigned char CYTSocket::Chr2Base(char c)
{
	if(c>='A' && c<='Z')
	{
		return (unsigned char)(c - 'A');
	}
	else if (c>='a' && c<='z')
	{
	   return (unsigned char)(c - 'a' + 26);
	}
	else if (c>='0' && c<='9')
	{
		return (unsigned char)(c - '0' + 52);
	}
	else if(c == '+')
	{
		return 62;
	}
	else
	{
		return 63;
	}
}

char CYTSocket::Base2Chr(unsigned char c)
{
	c &= 0x3F;
	if(c < 26)
	{
		return (char)(c + 'A');
	}
	else if (c < 52)
	{
		return (char)(c - 26 + 'a');
	}
	else if(c < 62)
	{
		return ( char )(c - 52 + '0');
	}
	else if(c == 62)
	{
		return '+';
	}
	else
	{
		return '/';
	}
}

// base64����
int CYTSocket::Base64Encode(const char * inbuf, char * outbuf)
{
	unsigned int  x, y, z;
    unsigned int  i, j;
    unsigned char buf[3];

    x = strlen(inbuf)/3;
    y = strlen(inbuf)%3;
    i = 0;
    j = 0;
    for(z=0; z<x; z++)
    {
        outbuf[i]   = Base2Chr(inbuf[j]>>2);
        outbuf[i+1] = Base2Chr((inbuf[j] & 0x03)<<4 | inbuf[j+1]>>4);
        outbuf[i+2] = Base2Chr((inbuf[j+1] & 0x0F)<<2 | inbuf[j+2]>>6);
        outbuf[i+3] = Base2Chr(inbuf[j+2] & 0x3F);
        i += 4;
        j += 3;
    }
    if(y != 0)
    {
        buf[0] = 0x00;
        buf[1] = 0x00;
        buf[2] = 0x00;
        for(z=0; z<y; z++)
        {
            buf[z] = inbuf[j+z];
        }
        outbuf[i]   = Base2Chr(buf[0]>>2);
        outbuf[i+1] = Base2Chr((buf[0] & 0x03)<<4 | buf[1]>>4);
        outbuf[i+2] = Base2Chr((buf[1] & 0x0F)<<2 | buf[2]>>6);
        outbuf[i+3] = Base2Chr(buf[2] & 0x3F);
        i += 4;
        for(z=0; z<3-y; z++)
        {
            outbuf[i-z-1] = '=';
        }
    }

    return i;
}

// base64����
int CYTSocket::Base64Decode(const char * inbuf, char * outbuf)
{
	unsigned int  x, y, z;
    unsigned int  i, j;
    unsigned char bufa[4];
    unsigned char bufb[3];
	unsigned int  len = strlen(inbuf);
    x = (len-4)/4;
    i = 0;
    j = 0;
    for(z=0; z<x; z++)
    {
        for(y=0; y<4; y++)
        {
            bufa[y] = Chr2Base(inbuf[j+y]);
        }
        outbuf[i]   = bufa[0]<<2 | (bufa[1] & 0x30)>>4;
        outbuf[i+1] = (bufa[1] & 0x0F)<<4 | (bufa[2] & 0x3C)>>2;
        outbuf[i+2] = (bufa[2] & 0x03)<<6 | (bufa[3] & 0x3F);
        i += 3;
        j += 4;
    }
    for(z=0; z<4; z++)
    {
        bufa[z] = Chr2Base(inbuf[j+z]);
    }

    if(inbuf[len-2] == '=')
    {
        y = 2;
    }
    else if(inbuf[len-1] == '=')
    {
        y = 1;
    }
    else
    {
        y = 0;
    }

    for(z=0; z<y; z++)
    {
        bufa[4-z-1] = 0x00;
    }
    bufb[0] = bufa[0]<<2 | (bufa[1] & 0x30)>>4;
    bufb[1] = (bufa[1] & 0x0F)<<4 | (bufa[2] & 0x3C)>>2;
    bufb[2] = (bufa[2] & 0x03)<<6 | (bufa[3] & 0x3F);

    for(z=0; z<3-y; z++)
    {
        outbuf[i+z] = bufb[z];
    }

    i += z;
    return i;
}

// sock4��������
int CYTSocket::ConnectSock4(char * ip, int port, int timeout)
{
	// ��sock4��Ϣ��
	char buffer[256] = {0};
	buffer[0] = (char)0x04;
	buffer[1] = (char)0x01;

	struct sockaddr_in serverAddr;
	if(atoSockaddr(ip, port, &serverAddr) < 0)
	{
		return -1;
	}
	else
	{
		memcpy(&buffer[2], &serverAddr.sin_port, 2);	
		memcpy(&buffer[4], &(serverAddr.sin_addr.S_un.S_addr), 4);
	}

	// ����sock4����
	if(Send(buffer, 9) <= 0)
	{
		return -1;
	}

	// ����sock4Ӧ��
	memset(buffer, 0, sizeof(buffer));
	if(Select(SOCKSEL_READ, timeout) != SOCKSEL_READ)
	{
		return -1;
	}
	if(Recv(buffer, 8) <= 0)
	{
		return -1;
	}

	// �ж��Ƿ���Ч
	if(buffer[1] != 90)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

// sock5��������
int CYTSocket::ConnectSock5(char * ip, int port, int timeout)
{
	// �жϵ�ַ�Ƿ���Ч
	struct sockaddr_in serverAddr;
	if(atoSockaddr(ip, port, &serverAddr) < 0)
	{
		return -1;
	}

	// ��sock5��Ϣ��
	char buffer[256] = {0};
	buffer[0] = (char)0x05;
	buffer[1] = (char)0x01;
	if(m_proxyinfo.auth)
	{
		buffer[2] = (char)0x02;
	}
	else
	{
		buffer[2] = (char)0x00;
	}

	// ����sock5����
	if(Send(buffer, 3) <= 0)
	{
		return -1;
	}
	
	// ����sock5Ӧ��
	memset(buffer, 0, sizeof(buffer));
	if(Select(SOCKSEL_READ, timeout) != SOCKSEL_READ)
	{
		return -1;
	}
	if(Recv(buffer, 2) <= 0)
	{
		return -1;
	}

	// �ж��Ƿ���Ч
	if(buffer[0]!=(char)0x05 || buffer[1]==(char)0xFF)
	{
		return -1;
	}

	// �����Ҫ��֤�û����������������������У��
	int flag = 0;
	if(buffer[1] == (char)0x02)
	{
		// ����֤�����
		memset(buffer, 0, sizeof(buffer));
		buffer[0] = 0x01;
		buffer[1] = (char)m_proxyinfo.namelen;
		memcpy(&buffer[2], m_proxyinfo.name, m_proxyinfo.namelen);
		buffer[2+m_proxyinfo.namelen] = (char)m_proxyinfo.pwdlen;
		memcpy(&buffer[3+m_proxyinfo.namelen], m_proxyinfo.pwd, m_proxyinfo.pwdlen);
		int sendlen = m_proxyinfo.namelen + m_proxyinfo.pwdlen + 3;
		
		// ������֤����
		if(Send(buffer, sendlen) <= 0)
		{
			return -1;
		}
		
		// ������֤Ӧ��
		memset(buffer, 0, sizeof(buffer));
		if(Select(SOCKSEL_READ, timeout) != SOCKSEL_READ)
		{
			return -1;
		}
		if(Recv(buffer, 2) <= 0)
		{
			return -1;
		}
		
		// �ж��Ƿ���Ч
		if(buffer[0]!=(char)0x01 || buffer[1]!=(char)0x00)
		{
			return -1;
		}
		flag = 1;
	}
	else if(buffer[1] == (char)0x00)
	{
		flag = 1;
	}

	if(flag == 1)	// ����Զ�̷�����
	{
		// �����������
		memset(buffer, 0, sizeof(buffer));
		buffer[0] = (char)0x05;
		buffer[1] = (char)0x01;
		buffer[2] = (char)0x00;
		buffer[3] = (char)0x01;
		memcpy(&buffer[4], &(serverAddr.sin_addr.S_un.S_addr), 4);
		memcpy(&buffer[8], &serverAddr.sin_port, 2);	
		
		// ������֤����
		if(Send(buffer, 10) <= 0)
		{
			return -1;
		}
		
		// ������֤Ӧ��
		memset(buffer, 0, sizeof(buffer));
		if(Select(SOCKSEL_READ, timeout) != SOCKSEL_READ)
		{
			return -1;
		}
		if(Recv(buffer, 10) <= 0)
		{
			return -1;
		}
		
		// �ж��Ƿ���Ч
		if(buffer[1] != (char)0x00)
		{
			return -1;
		}

		if(buffer[0]!=(char)0x01 || buffer[1]!=(char)0x00)
		{
			return -1;
		}

		if(buffer[0]==(char)0x05 && buffer[3]==(char)0x01)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

// http��������
int CYTSocket::ConnectHttp(char * ip, int port, int timeout)
{
	// ��http�����
	char buffer[1024] = {0};
	char name[512] = {0};
	char encode[256] = {0};
	int  len = (60+2*strlen(ip));

	sprintf_s(name, sizeof(name), "%s:%s", m_proxyinfo.name, m_proxyinfo.pwd);
	if(m_proxyinfo.auth)
	{
		Base64Encode(name, encode);
		sprintf_s(buffer, sizeof(buffer), "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\nAuthorization: Basic %s\r\nProxy-Authorization: Basic %s\r\n\r\n",
			ip, port, ip, port, encode, encode);
	}
	else
	{
		sprintf_s(buffer, sizeof(buffer), "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\n\r\n", ip, port, ip, port);
	}
	len = (unsigned short)(strlen(buffer));

	// ����http����
	if(Send(buffer, len) <= 0)
	{
		return -1;
	}
	
	// ����httpӦ��
	memset(buffer, 0, sizeof(buffer));
	if(Select(SOCKSEL_READ, timeout) != SOCKSEL_READ)
	{
		return -1;
	}
	if(Recv(buffer, 1023) <= 0)
	{
		return -1;
	}

	// �ж��Ƿ���Ч
	if(buffer[0]=='\0' || strncmp(buffer, "HTTP/1.", 7)!=0)
	{
		return -1;
	}

	char head[1024] = {0};
	int pos = 0;
	int j=0;
	bool flag = false;
	len = strlen(buffer);
	for(int i=0; i<len; i++)
	{
		if(strncmp(buffer+i, "\r\n", 2) == 0)
		{
			pos++;
			i++; 
		}
		else
		{
			if(pos == 0)
			{
				flag = true;
				head[j] = buffer[i];
				if(j < 100)
				{
					j++;
				}
				else
				{
					break;
				}
			}
			else if(pos < 0)
			{
				continue;
			}
			else
			{
				flag = true;
				break;
			}
		}
	}
	head[j] = '\0';
	if(!flag || j<=0)
	{
		return -1;
	}

	if(strstr(head, "Unauthorized") != NULL)
	{
		return -1;
	}
	if(_stricmp(head, "Connection established") < 0)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}