#include "YTCom.h"
#include <time.h>

CYTCom::CYTCom(void)
{
	m_sysstatus				= 0;
	m_currserverid			= -1;

	m_chunnelid				= (unsigned int)-1;
	m_userid				= (unsigned int)-1;

	m_loadcfg				= false;
	m_conncreatetimeout		= 1000;
	m_connclosetimeout		= 100;
	m_sendandrecvtimeout	= 10;
	m_transtimeout			= 20;
}

CYTCom::~CYTCom(void)
{
	CloseConnect();
	m_serveraddr.clear();
}

// ��ʼ�����ã����󴴽���ֻ�������һ�Σ�
int CYTCom::Init(CYTIniFile * cfgfile)
{
	// ����Ѿ����ع��˾Ͳ������ټ���
	if(cfgfile==NULL || m_loadcfg)
	{
		GlobalLogout(LT_SYS, LOG_ERROR, "[0x%.8x]���Ӷ����ʼ��ʧ�ܣ�û�������ļ�\r\n", this);
		return -1;
	}

	// ��ȡ��������ַ
	const char * addrs = cfgfile->ReadString("SERVER", "serveraddrs", "");
	if(addrs==NULL || strlen(addrs)==0)
	{
		GlobalLogout(LT_SYS, LOG_ERROR, "[0x%.8x]���Ӷ����ʼ��ʧ�ܣ�û�з�������ַ\r\n", this);
		return -1;
	}
	char szAddrs[1024] = {0};
	sprintf_s(szAddrs, sizeof(szAddrs), "%s;", addrs);

	tagServer tmpserver;
	char buffer[256];
	char * psplit;
	char * pstart = (char *)szAddrs;
	char * paddr = strstr(pstart, ";");
	while(paddr != NULL)
	{
		// �õ�һ����ַ
		memset(buffer, 0, sizeof(buffer));
		memcpy(buffer, pstart, min(sizeof(buffer)-1, paddr-pstart));
		if(buffer[0] == '\0')
		{
			// �յ�ַ����Ҫ����ֱ��ת����һ��
			pstart = paddr+1;
			paddr = strstr(pstart, ";");
			continue;
		}

		// �������ַ�Ͷ˿�
		psplit = strstr(buffer, ":");
		if(psplit != NULL)
		{
			memset(&tmpserver, 0, sizeof(tagServer));
			memcpy(tmpserver.address, buffer, min(sizeof(tmpserver.address)-1, psplit-buffer));
			tmpserver.port = strtol(psplit+1, NULL, 10);
			if(tmpserver.address[0]!='\0' && tmpserver.port>0)
			{
				m_serveraddr.push_back(tmpserver);
			}
		}

		// ����һ��������
		pstart = paddr+1;
		paddr = strstr(pstart, ";");
	}
	if(m_serveraddr.size() == 0)
	{
		GlobalLogout(LT_SYS, LOG_ERROR, "[0x%.8x]���Ӷ����ʼ��ʧ�ܣ�û����Ч�ķ�������ַ\r\n", this);
		return -1;
	}

	// ��ȡ���������Ϣ
	int proxytype = cfgfile->ReadInteger("PROXY", "type", 0);
	if(proxytype != 0)
	{
		const char * ip = cfgfile->ReadString("PROXY", "ip", "");
		int port = cfgfile->ReadInteger("PROXY", "port", 0);
		if(ip!=NULL && strlen(ip)>0 && port>0)
		{
			tagProxy proxy;
			memset(&proxy, 0, sizeof(proxy));
			memcpy(proxy.ip, ip, min(sizeof(proxy.ip)-1, strlen(ip)));
			proxy.port = port;

			const char * name = cfgfile->ReadString("PROXY", "name", "");
			if(name!=NULL && strlen(name)>0)
			{
				proxy.auth = true;
				memcpy(proxy.name, name, min(sizeof(proxy.name)-1, strlen(name)));
				proxy.namelen = strlen(proxy.name);
			}

			const char * pwd = cfgfile->ReadString("PROXY", "pwd", "");
			if(pwd!=NULL && strlen(pwd)>0)
			{
				proxy.auth = true;
				memcpy(proxy.pwd, pwd, min(sizeof(proxy.pwd)-1, strlen(pwd)));
				proxy.pwdlen = strlen(proxy.pwd);
			}

			CYTSocket::SetProxyInfo(proxytype, proxy);
		}
	}

	// ��ȡ��ʱ�����Ϣ
	m_conncreatetimeout	= cfgfile->ReadInteger("TIMEOUT", "createtimeout", 1000);
	if(m_conncreatetimeout <= 0)
	{
		m_conncreatetimeout = 1000;
	}

	m_connclosetimeout	= cfgfile->ReadInteger("TIMEOUT", "closetimeout", 100);
	if(m_connclosetimeout <= 0)
	{
		m_connclosetimeout = 100;
	}

	m_sendandrecvtimeout	= cfgfile->ReadInteger("TIMEOUT", "sendandrecvtimeout", 10);
	if(m_sendandrecvtimeout <= 0)
	{
		m_sendandrecvtimeout = 10;
	}

	m_transtimeout	= cfgfile->ReadInteger("TIMEOUT", "translatetimeout", 20);
	if(m_transtimeout <= 0)
	{
		m_transtimeout = 20;
	}

	GlobalLogout(LT_SYS, LOG_SUCCESS, "[0x%.8x]���Ӷ����ʼ���ɹ�\r\n", this);
	m_loadcfg = true;

	return 0;
}

// ���ӷ�����
int CYTCom::ConnectServer(void)
{
	// �ж������ǰ��������״̬��Ҫ�ظ�����
	if(m_sysstatus == 1)
	{
		return -1;
	}
	else
	{
		StopThread();
	}

	// ����������
	m_currserverid	= -1;
	m_chunnelid		= (unsigned int)-1;
	m_userid		= (unsigned int)-1;
	
	m_reqmaplock.Lock();
	m_reqmap.clear();
	m_reqmaplock.UnLock();

	// ���ӷ�����
	if(Connect_Muti() < 0)
	{
		GlobalLogout(LT_SYS, LOG_ERROR, "[0x%.8x]��������ʧ�ܣ���������ʧ��\r\n", this);
		return -1;
	}

	// ���������߳�
	if(!BeginThread())
	{
		GlobalLogout(LT_SYS, LOG_ERROR, "[0x%.8x]��������ʧ�ܣ����������߳�ʧ��\r\n", this);
		CloseConnect();
		return -1;
	}
	else
	{
		return 0;
	}
}

// �ر��������������
int CYTCom::CloseConnect()
{
	StopThread();
	CYTSocket::Close();

	m_currserverid	= -1;
	m_chunnelid		= (unsigned int)-1;
	m_userid		= (unsigned int)-1;
	
	m_reqmaplock.Lock();
	m_reqmap.clear();
	m_reqmaplock.UnLock();

	GlobalLogout(LT_RUN, LOG_SUCCESS, "[0x%.8x]�ر����ӳɹ�\r\n", this);
	return 0;
}

// ��ȡ����״̬
int CYTCom::GetStatus()
{
	bool linkstatus = false;

	m_sockrecvlock.Lock();

	linkstatus = CYTSocket::IsValid();

	m_sockrecvlock.UnLock();
	
	if(!linkstatus)
	{
		// �Ͽ�
		return CS_CLOSE;
	}
	else
	{
		// ������
		return CS_CONNED;
	}
}

// �������ݣ������ڲ����������
int CYTCom::SendData(unsigned int funcid, const char * data, unsigned int len)
{
	if(funcid==0 || (data==NULL && len>0))
	{
		return -1;
	}

	// ��֤������Ӳſ����շ�ҵ������
	if(m_chunnelid==(unsigned int)-1 && m_userid==(unsigned int)-1 && funcid!=150001)
	{
		GlobalLogout(LT_RUN, LOG_ERROR, "[0x%.8x]������ʧ�ܣ�û��֤���շ�����(chunnelid:%u,userid:%u,funcid:%u)\r\n", this, m_chunnelid, m_userid, funcid);
		return -1;
	}

	// �ж�����״̬
	if(GetStatus() == 0)
	{
		GlobalLogout(LT_RUN, LOG_ERROR, "[0x%.8x]������ʧ�ܣ�����״̬����(chunnelid:%u,userid:%u,funcid:%u)\r\n", this, m_chunnelid, m_userid, funcid);
		return -1;
	}

	// ���췢�ͻ���
	unsigned int sendbuflen = 0;
	if(len > 0)
	{
		sendbuflen = sizeof(USDataHeader)+4+len;
	}
	else
	{
		sendbuflen = sizeof(USDataHeader);
	}
	char * sendbuf = new char[sendbuflen];
	if(sendbuf == NULL)
	{
		GlobalLogout(LT_RUN, LOG_ERROR, "[0x%.8x]������ʧ�ܣ��ڴ����%d���ȴ���(chunnelid:%u,userid:%u,funcid:%u)\r\n", this, sendbuflen, m_chunnelid, m_userid, funcid);
		return -1;
	}

	// ��ȡ������
	unsigned int reqno = GlobGetReqNo();

	// �鷢�Ͱ�
	USDataHeader ushead;
	ushead.ver = 1;
	ushead.len = sendbuflen;
	ushead.funcid = funcid;
	ushead.chunnelid = 0;
	ushead.serailid = reqno;
	ushead.checksum = 0;
	if(m_userid == (unsigned int)-1)
	{
		ushead.userid = 0;
	}
	else
	{
		ushead.userid = m_userid;
	}
	ushead.result = 0;
	ushead.reqserailid = 0;
	memcpy(sendbuf, (char *)&ushead, sizeof(USDataHeader));
	if(len > 0)
	{
		memcpy(sendbuf+sizeof(USDataHeader), &len, 4);
		memcpy(sendbuf+sizeof(USDataHeader)+4, data, len);
	}

	// ���ͻ�������
	int ret = 0;
	m_socksendlock.Lock();
	ret = CYTSocket::Send(sendbuf, (int)sendbuflen, m_sendandrecvtimeout);
	m_socksendlock.UnLock();
	delete sendbuf;
	sendbuf = NULL;

	// �жϷ��ͽ��������ɹ��Ļ���д�������
	if(ret != (int)sendbuflen)
	{
		GlobalLogout(LT_RUN, LOG_ERROR, "[0x%.8x]������ʧ�ܣ�(chunnelid:%u,userid:%u,funcid:%u,reqno:%u,sendlen:%d,ret:%d)\r\n", this, m_chunnelid, m_userid, funcid, reqno, sendbuflen, ret);
		return -1;
	}
	else
	{
		// �������󵽼�¼����map��
		tagReq reqinfo;
		reqinfo.reqno		= reqno;
		reqinfo.funcid		= funcid;
		reqinfo.sendtime	= (int)time(NULL);

		m_reqmaplock.Lock();
		m_reqmap[reqno] = reqinfo;
		m_reqmaplock.UnLock();
		
		return reqno;
	}
}

// ��ȡ��ǰ���ӵķ�������ַ
const char * CYTCom::GetServer(int * port)
{
	if(m_currserverid>=0 && m_currserverid<(int)m_serveraddr.size())
	{
		*port = m_serveraddr[m_currserverid].port;
		return m_serveraddr[m_currserverid].address;
	}
	else
	{
		return NULL;
	}
}

// �������ݴ�������δ���������һ�������Ӧ���������Ӧ�����ݻ��������
int CYTCom::Pack(tagYTConnPackHead * phead, const char * data, int len, const char * outbuf, int * outlen)
{
	if(phead==NULL || (data==NULL && len>0))
	{
		return 0;
	}

	// �жϷ��ͻ����Ƿ��㹻�������Ļ�������
	int sendbuflen = 0;
	if(len > 0)
	{
		sendbuflen = sizeof(USDataHeader)+4+len;
	}
	else
	{
		sendbuflen = sizeof(USDataHeader);
	}
	if(outlen==NULL || *outlen<sendbuflen)
	{
		*outlen = sendbuflen;
		return -1;
	}
	if(outbuf == NULL)
	{
		*outlen = sendbuflen;
		return -1;
	}

	// ����Ӧ������
	USDataHeader ushead;
	ushead.ver = 1;
	ushead.len = sendbuflen;
	ushead.funcid = phead->funcid;
	ushead.chunnelid = phead->chunnelid;
	ushead.serailid = phead->reqserailid;
	ushead.checksum = 0;
	ushead.userid = phead->userid;
	ushead.result = 0;
	ushead.reqserailid = phead->reqserailid;
	memcpy((char *)outbuf, (char *)&ushead, sizeof(USDataHeader));
	if(len > 0)
	{
		memcpy((char *)outbuf+sizeof(USDataHeader), &len, 4);
		memcpy((char *)outbuf+sizeof(USDataHeader)+4, data, len);
	}
	return sendbuflen;
}

// Ӧ�����ݽ����ÿ�ν����һ��ҵ�������
int CYTCom::UnPack(const char * data, int len, tagYTConnPackHead * phead, const char * outbuf, int * outlen)
{
	// �����жϴ�������Ƿ���Ч
	if(data==NULL || len<=0 || phead==NULL || outlen==NULL)
	{
		return 0;
	}

	// �������ͷ����
	if(len < sizeof(USDataHeader))
	{
		GlobalLogout(LT_RUN, LOG_WARNING, "UnPack[%d]������һ����ͷ\r\n", len);
		*outlen = 0;
		return 0;
	}
	USDataHeader ushead;
	memcpy((char *)&ushead, data, sizeof(USDataHeader));

	// �жϰ��������Ƿ��㹻
	if((int)ushead.len > len)
	{
		GlobalLogout(LT_RUN, LOG_WARNING, "UnPack[%d][%d]������һ������\r\n", len, 0);
		*outlen = 0;
		return 0;
	}

	int datalen = 0;
	if(ushead.len != sizeof(USDataHeader))
	{
		memcpy(&datalen, data+sizeof(USDataHeader), 4);
	}
	if(*outlen < datalen)
	{
		GlobalLogout(LT_RUN, LOG_WARNING, "UnPack[%d][%d][%d]��������泤�Ȳ���\r\n", len, datalen, *outlen);
		*outlen = datalen;
		return -1;
	}
	if(outbuf == NULL)
	{
		GlobalLogout(LT_RUN, LOG_WARNING, "UnPack[%d][%d][%d]���������Ϊ��\r\n", len, datalen, *outlen);
		*outlen = datalen;
		return -1;
	}
	*outlen = datalen;

	// ��֯Ӧ������
	phead->chunnelid = ushead.chunnelid;
	phead->userid = ushead.userid;
	phead->funcid = ushead.funcid;
	phead->reqserailid = ushead.serailid;
	phead->result = ushead.result;

	if(datalen > 0)
	{
		memcpy((char *)outbuf, data+sizeof(USDataHeader)+4, datalen);
	}
	return ushead.len;
}

// ���������߳�
bool CYTCom::BeginThread()
{
	m_sysstatus = 1;
	if(m_thread.BeginThread(RecvThread, this, " ") < 0)
	{
		m_sysstatus = 0;
		return false;
	}
	else
	{
		return true;
	}
}

// ֹͣ�����߳�
void CYTCom::StopThread()
{
	m_sysstatus = 0;
	m_thread.StopThread(m_connclosetimeout);
}

// ��˳�����ӷ�����
int CYTCom::Connect_Single()
{
	if(m_serveraddr.size() == 0)
	{
		return -1;
	}

	int index = 0;
	vector<tagServer>::iterator iter = m_serveraddr.begin();
	while(iter != m_serveraddr.end())
	{
		if(CYTSocket::Connect((*iter).address, (*iter).port, m_conncreatetimeout) >= 0)
		{
			m_currserverid = index;
			return 1;
		}
		else
		{
			CYTSocket::Close();
		}

		index++;
		iter++;
	}

	return -1;
}

// ͬʱ���Ӷ��������
int CYTCom::Connect_Muti()
{
	if(m_serveraddr.size() == 0)
	{
		return -1;
	}

	// �д���Ļ�����˳������ģʽ
	if(CYTSocket::m_proxytype != SOCKPROXY_NONE)
	{
		return Connect_Single();
	}

	FD_SET		fdwrite;
	CYTSocket	pSocket[100];
	int			count = m_serveraddr.size();
	for(int i=0; i<count; i++)
	{
		pSocket[i].Create();
		pSocket[i].Connect(m_serveraddr[i].address, m_serveraddr[i].port, 0);
	}

	timeval stimeout;
	stimeout.tv_sec = m_conncreatetimeout/1000;
	stimeout.tv_usec = 1000*(m_conncreatetimeout%1000);

	int maxsocket, j;
	for(int i=0; i<count; i++)
	{
		FD_ZERO(&fdwrite);
		maxsocket = 0;
		for(j=0; j<count; j++)
		{
			if(pSocket[j].m_socket >= 0)
			{
				FD_SET(pSocket[j].m_socket, &fdwrite);
				maxsocket = max(maxsocket, pSocket[j].m_socket);
			}
		}
		if(::select(maxsocket+1, NULL, &fdwrite, NULL, &stimeout) <= 0)
		{
			// �رռ���socket
			for(j=0; j<count; j++)
			{
				pSocket[i].Close();
			}
			return -1;
		}

		for(j=0; j<count; j++)
		{
			if(pSocket[j].m_socket>=0 && FD_ISSET(pSocket[j].m_socket, &fdwrite))
			{
				m_currserverid = j;

				CYTSocket::Renew(pSocket[j].m_socket, pSocket[j].m_type, pSocket[j].m_status);
				pSocket[j].m_socket = -1;
				
				// �رռ���socket
				for(i=j+1; i<count; i++)
				{
					pSocket[i].Close();
				}

				return 1;
			}
			else
			{
				pSocket[j].Close();
			}
		}
	}

	return -1;
}

// ���ݴ���
int CYTCom::ProvData(char * data, int len)
{
	// �ж�Ӧ�������Ƿ�Ϊ���û���
	USDataHeader * pUSHead	= (USDataHeader *)data;

	// ����ǵ�¼Ӧ��Ļ��򱣴�ͨ���ź��û����
	if(pUSHead->funcid != 150001)
	{
		// У���Ƿ��û��ͱ����ӵ�
		if(m_chunnelid == (unsigned int)-1)
		{
			GlobalLogout(LT_RUN, LOG_ERROR, "[0x%.8x]ͨ���Ų�ƥ�䣺(chunnelid:%u,userid:%u,funcid:%u,reqno:%u,result:%u)��memchunnelid:%d��\r\n", 
				this, pUSHead->chunnelid, pUSHead->userid, pUSHead->funcid, pUSHead->reqserailid, pUSHead->result, m_chunnelid);

			DataRet(pUSHead->reqserailid, pUSHead->funcid);
			if(g_CallBackFList.Func_YTConnDataAllRet != NULL)
			{
				char szResultBuffer[32];
				sprintf_s(szResultBuffer, sizeof(szResultBuffer), "�Ǳ�����Ӧ��");
				g_CallBackFList.Func_YTConnDataAllRet(this,  pUSHead->reqserailid, pUSHead->funcid, EC_DATAILLEGAL, szResultBuffer, strlen(szResultBuffer));
			}
			return -1;
		}
		else if(m_userid != pUSHead->userid)
		{
			GlobalLogout(LT_RUN, LOG_ERROR, "[0x%.8x]�û���Ų�ƥ�䣺(chunnelid:%u,userid:%u,funcid:%u,reqno:%u,result:%u)��memuserid:%d��\r\n", 
				this, pUSHead->chunnelid, pUSHead->userid, pUSHead->funcid, pUSHead->reqserailid, pUSHead->result, m_userid);

			DataRet(pUSHead->reqserailid, pUSHead->funcid);
			if(g_CallBackFList.Func_YTConnDataAllRet != NULL)
			{
				char szResultBuffer[32];
				sprintf_s(szResultBuffer, sizeof(szResultBuffer), "�Ǳ��û�Ӧ��");
				g_CallBackFList.Func_YTConnDataAllRet(this,  pUSHead->reqserailid, pUSHead->funcid, EC_DATAILLEGAL, szResultBuffer, strlen(szResultBuffer));
			}
			return -1;
		}
	}

	// ҵ��ɹ��Ļ����ȡҵ��������Ϣ
	int datalen = 0;
	if(pUSHead->len > sizeof(USDataHeader))
	{
		memcpy(&datalen, data+sizeof(USDataHeader), 4);
	}
  char sztmpretmsg[128] = {0};

	DataRet(pUSHead->reqserailid, pUSHead->funcid);
	if(pUSHead->funcid==150001 && pUSHead->result==0)
	{
		// ����ǵ�¼Ӧ��Ļ������⴦����Ҫ�����userid
		void * datahandle = YTData_NewData();
		if(YTData_ConvertData(datahandle, data+sizeof(USDataHeader)+4, datalen) >= 0)
		{
			if(YTData_GetRetCode(datahandle) >= 0)
			{
				m_chunnelid = pUSHead->chunnelid;
				m_userid = pUSHead->userid;
				if(m_userid == 0)
				{
					m_userid = YTData_GetFieldInt(datahandle, "userid");
				}
			}
			else
			{
				
				YTData_GetRetMsg(datahandle, sztmpretmsg, sizeof(sztmpretmsg));
			}
		}
		YTData_DeleteData(datahandle);
	}
	if(g_CallBackFList.Func_YTConnDataAllRet != NULL)
	{
		if(datalen > 0)
		{
			g_CallBackFList.Func_YTConnDataAllRet(this, pUSHead->reqserailid, pUSHead->funcid, pUSHead->result, data+sizeof(USDataHeader)+4, datalen);
		}
		else
		{
			g_CallBackFList.Func_YTConnDataAllRet(this, pUSHead->reqserailid, pUSHead->funcid, pUSHead->result, NULL, datalen);
		}
	}
	return 0;
}

// Ӧ�𷵻ظ�������map
int CYTCom::DataRet(int reqno, unsigned int funcid)
{
	int ret = 0;

	m_reqmaplock.Lock();

	map<int, tagReq>::iterator iter = m_reqmap.find(reqno);
	if(iter != m_reqmap.end())
	{
		if(iter->second.funcid == funcid)
		{
			m_reqmap.erase(iter);
		}
		else
		{
			ret = -1;
		}
	}
	else
	{
		ret = -1;
	}
	
	m_reqmaplock.UnLock();

	return ret;
}

// ��ⳬʱ������
int CYTCom::CheckTimeOut()
{
	m_reqmaplock.Lock();

	int currtime = (int)time(NULL);
	map<int, tagReq>::iterator iter = m_reqmap.begin();
	while(iter != m_reqmap.end())
	{
		if(currtime-iter->second.sendtime > m_transtimeout)
		{
			// ��ʱ����
			if(g_CallBackFList.Func_YTConnTimeOut != NULL)
			{
				g_CallBackFList.Func_YTConnTimeOut(this, iter->second.reqno, iter->second.funcid);
			}
			iter = m_reqmap.erase(iter);
		}
		else
		{
			iter++;
		}
	}

	m_reqmaplock.UnLock();

	return 0;
}

// �ر�����������Ϣ��Ŀǰ��ʾ�ͻ��˳�ʱ
void CYTCom::CloseAllReq()
{
	m_reqmaplock.Lock();

	map<int, tagReq>::iterator iter = m_reqmap.begin();
	while(iter != m_reqmap.end())
	{
		if(g_CallBackFList.Func_YTConnTimeOut != NULL)
		{
			g_CallBackFList.Func_YTConnTimeOut(this, iter->second.reqno, iter->second.funcid);
		}
		iter = m_reqmap.erase(iter);
	}

	m_reqmaplock.UnLock();
}

// �����߳�
unsigned int __stdcall CYTCom::RecvThread(void * pParam)
{
	CYTCom * pYTCom = (CYTCom *)pParam;

	int			recvlen		= 0;
	int			currlen		= 0;
	int			totallen	= sizeof(USDataHeader)+4;
	char *		recvbuf		= new char[totallen];
	if(recvbuf == NULL)
	{
		return -1;
	}
	USDataHeader * pUSHead	= (USDataHeader *)recvbuf;
	char *		newrecvbuf	= NULL;

	int			selectret	= 0;
	int			checkflag	= 1;
	bool		sockclose	= 0;
	while(pYTCom!=NULL && pYTCom->m_sysstatus==1)
	{
		// �����ж�socket״̬��������ӶϿ���ͨ���ص�֪ͨ��Χ
		if(pYTCom->GetStatus() == CS_CLOSE)
		{
			if(pYTCom->m_socket >= 0)
			{
				GlobalLogout(LT_RUN, LOG_WARNING, "[0x%.8x]�̼߳�⵽���ӶϿ�\r\n", pParam);
				pYTCom->CloseAllReq();
				pYTCom->m_sysstatus = 0;
				if(g_CallBackFList.Func_YTConnClose != NULL)
				{
					g_CallBackFList.Func_YTConnClose(pParam);
				}
				pYTCom->CYTSocket::Close();
			}
			::Sleep(100);
			continue;
		}
		else
		{
			checkflag++;
		}

		// ����Ƿ�ɶ�״̬
		selectret = pYTCom->CYTSocket::Select(SOCKSEL_READ, 500);
		if((selectret & SOCKSEL_READ) == SOCKSEL_READ)
		{
			// ��������Ƿ��Ѿ��رգ�������ӹر���ͨ���ص�֪ͨ��Χ
			sockclose = false;
			pYTCom->m_sockrecvlock.Lock();
			sockclose = pYTCom->CYTSocket::IsClose();
			pYTCom->m_sockrecvlock.UnLock();
			if(sockclose)
			{
				GlobalLogout(LT_RUN, LOG_WARNING, "[0x%.8x]�̼߳�⵽���ӹر�\r\n", pParam);
				pYTCom->CloseAllReq();
				pYTCom->m_sysstatus = 0;
				if(g_CallBackFList.Func_YTConnClose != NULL)
				{
					g_CallBackFList.Func_YTConnClose(pParam);
				}
				pYTCom->CYTSocket::Close();
				::Sleep(100);
				continue;			
			}

			// �������ݲ�����
			pYTCom->m_sockrecvlock.Lock();
			if(currlen < sizeof(USDataHeader))
			{
				// ���հ�ͷ
				recvlen = pYTCom->CYTSocket::Recv(recvbuf+currlen, sizeof(USDataHeader)-currlen, pYTCom->m_sendandrecvtimeout);
			}
			else
			{
				// �հ���
				recvlen = pYTCom->CYTSocket::Recv(recvbuf+currlen, pUSHead->len-currlen, pYTCom->m_sendandrecvtimeout);
			}
			pYTCom->m_sockrecvlock.UnLock();
			if(recvlen <= 0)
			{
				GlobalLogout(LT_RUN, LOG_WARNING, "[0x%.8x]Ӧ�����ʧ��[%d][%d][%d]\r\n", pParam, totallen, currlen, recvlen);
			}
			else 
			{
				currlen += recvlen;

				// �������һ����ͷ�Ļ������ҵ����
				if(currlen >= sizeof(USDataHeader))
				{
					// �ж����ݳ����Ƿ��㹻����������Ļ���׷�ӻ���
					if((int)pUSHead->len > totallen)
					{
						totallen = pUSHead->len;
						newrecvbuf = new char[totallen];
						if(newrecvbuf != NULL)
						{
							memcpy(newrecvbuf, recvbuf, currlen);
							delete recvbuf;
							recvbuf = newrecvbuf;
							newrecvbuf = NULL;
							pUSHead	= (USDataHeader *)recvbuf;
						}
						else
						{
							GlobalLogout(LT_RUN, LOG_WARNING, "[0x%.8x]�ڴ����ʧ��[%d][%d][%d]\r\n", pParam, totallen, currlen, recvlen);
						}
						continue;
					}
					else if((int)pUSHead->len > currlen)
					{
						// Ӧ�ճ���δ��ȫ
						continue;
					}

					// ����ҵ����
					pYTCom->ProvData(recvbuf, currlen);
					currlen = 0;
				}
			}
		}
		else
		{
			// ������10�ζ�û����Ϣ�Ļ��������������г�ʱ���
			if(checkflag%10 == 0)
			{
				checkflag = 1;
				pYTCom->CheckTimeOut();
			}
		}
	}

	if(recvbuf != NULL)
	{
		delete recvbuf;
		recvbuf = NULL;
	}

	return 0;	
}
