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

// 初始化配置（对象创建后只允许调用一次）
int CYTCom::Init(CYTIniFile * cfgfile)
{
	// 如果已经加载过了就不允许再加载
	if(cfgfile==NULL || m_loadcfg)
	{
		GlobalLogout(LT_SYS, LOG_ERROR, "[0x%.8x]连接对象初始化失败：没有配置文件\r\n", this);
		return -1;
	}

	// 获取服务器地址
	const char * addrs = cfgfile->ReadString("SERVER", "serveraddrs", "");
	if(addrs==NULL || strlen(addrs)==0)
	{
		GlobalLogout(LT_SYS, LOG_ERROR, "[0x%.8x]连接对象初始化失败：没有服务器地址\r\n", this);
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
		// 得到一个地址
		memset(buffer, 0, sizeof(buffer));
		memcpy(buffer, pstart, min(sizeof(buffer)-1, paddr-pstart));
		if(buffer[0] == '\0')
		{
			// 空地址不需要处理，直接转到下一个
			pstart = paddr+1;
			paddr = strstr(pstart, ";");
			continue;
		}

		// 分离出地址和端口
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

		// 找下一个服务器
		pstart = paddr+1;
		paddr = strstr(pstart, ";");
	}
	if(m_serveraddr.size() == 0)
	{
		GlobalLogout(LT_SYS, LOG_ERROR, "[0x%.8x]连接对象初始化失败：没有有效的服务器地址\r\n", this);
		return -1;
	}

	// 读取代理相关信息
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

	// 读取超时相关信息
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

	GlobalLogout(LT_SYS, LOG_SUCCESS, "[0x%.8x]连接对象初始化成功\r\n", this);
	m_loadcfg = true;

	return 0;
}

// 连接服务器
int CYTCom::ConnectServer(void)
{
	// 判断如果当前处于连接状态则不要重复创建
	if(m_sysstatus == 1)
	{
		return -1;
	}
	else
	{
		StopThread();
	}

	// 清空相关数据
	m_currserverid	= -1;
	m_chunnelid		= (unsigned int)-1;
	m_userid		= (unsigned int)-1;
	
	m_reqmaplock.Lock();
	m_reqmap.clear();
	m_reqmaplock.UnLock();

	// 连接服务器
	if(Connect_Muti() < 0)
	{
		GlobalLogout(LT_SYS, LOG_ERROR, "[0x%.8x]创建连接失败：连服务器失败\r\n", this);
		return -1;
	}

	// 开启接收线程
	if(!BeginThread())
	{
		GlobalLogout(LT_SYS, LOG_ERROR, "[0x%.8x]创建连接失败：开启接收线程失败\r\n", this);
		CloseConnect();
		return -1;
	}
	else
	{
		return 0;
	}
}

// 关闭与服务器的连接
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

	GlobalLogout(LT_RUN, LOG_SUCCESS, "[0x%.8x]关闭连接成功\r\n", this);
	return 0;
}

// 获取连接状态
int CYTCom::GetStatus()
{
	bool linkstatus = false;

	m_sockrecvlock.Lock();

	linkstatus = CYTSocket::IsValid();

	m_sockrecvlock.UnLock();
	
	if(!linkstatus)
	{
		// 断开
		return CS_CLOSE;
	}
	else
	{
		// 连接上
		return CS_CONNED;
	}
}

// 发送数据（函数内部进行组包）
int CYTCom::SendData(unsigned int funcid, const char * data, unsigned int len)
{
	if(funcid==0 || (data==NULL && len>0))
	{
		return -1;
	}

	// 认证后的连接才可以收发业务数据
	if(m_chunnelid==(unsigned int)-1 && m_userid==(unsigned int)-1 && funcid!=150001)
	{
		GlobalLogout(LT_RUN, LOG_ERROR, "[0x%.8x]请求发送失败：没认证就收发数据(chunnelid:%u,userid:%u,funcid:%u)\r\n", this, m_chunnelid, m_userid, funcid);
		return -1;
	}

	// 判断连接状态
	if(GetStatus() == 0)
	{
		GlobalLogout(LT_RUN, LOG_ERROR, "[0x%.8x]请求发送失败：连接状态错误(chunnelid:%u,userid:%u,funcid:%u)\r\n", this, m_chunnelid, m_userid, funcid);
		return -1;
	}

	// 构造发送缓存
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
		GlobalLogout(LT_RUN, LOG_ERROR, "[0x%.8x]请求发送失败：内存分配%d长度错误(chunnelid:%u,userid:%u,funcid:%u)\r\n", this, sendbuflen, m_chunnelid, m_userid, funcid);
		return -1;
	}

	// 获取请求编号
	unsigned int reqno = GlobGetReqNo();

	// 组发送包
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

	// 发送缓存数据
	int ret = 0;
	m_socksendlock.Lock();
	ret = CYTSocket::Send(sendbuf, (int)sendbuflen, m_sendandrecvtimeout);
	m_socksendlock.UnLock();
	delete sendbuf;
	sendbuf = NULL;

	// 判断发送结果，如果成功的话则写入队列中
	if(ret != (int)sendbuflen)
	{
		GlobalLogout(LT_RUN, LOG_ERROR, "[0x%.8x]请求发送失败：(chunnelid:%u,userid:%u,funcid:%u,reqno:%u,sendlen:%d,ret:%d)\r\n", this, m_chunnelid, m_userid, funcid, reqno, sendbuflen, ret);
		return -1;
	}
	else
	{
		// 加入请求到记录控制map中
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

// 获取当前连接的服务器地址
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

// 请求数据打包：本次打包仅仅针对一个请求的应答打包，多个应答数据会产生错误
int CYTCom::Pack(tagYTConnPackHead * phead, const char * data, int len, const char * outbuf, int * outlen)
{
	if(phead==NULL || (data==NULL && len>0))
	{
		return 0;
	}

	// 判断发送缓存是否足够，不够的话则增加
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

	// 构造应答数据
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

// 应答数据解包：每次解包出一个业务请求的
int CYTCom::UnPack(const char * data, int len, tagYTConnPackHead * phead, const char * outbuf, int * outlen)
{
	// 首先判断传入参数是否有效
	if(data==NULL || len<=0 || phead==NULL || outlen==NULL)
	{
		return 0;
	}

	// 分离出包头数据
	if(len < sizeof(USDataHeader))
	{
		GlobalLogout(LT_RUN, LOG_WARNING, "UnPack[%d]：不够一个包头\r\n", len);
		*outlen = 0;
		return 0;
	}
	USDataHeader ushead;
	memcpy((char *)&ushead, data, sizeof(USDataHeader));

	// 判断包体数据是否足够
	if((int)ushead.len > len)
	{
		GlobalLogout(LT_RUN, LOG_WARNING, "UnPack[%d][%d]：不够一个包体\r\n", len, 0);
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
		GlobalLogout(LT_RUN, LOG_WARNING, "UnPack[%d][%d][%d]：输出缓存长度不够\r\n", len, datalen, *outlen);
		*outlen = datalen;
		return -1;
	}
	if(outbuf == NULL)
	{
		GlobalLogout(LT_RUN, LOG_WARNING, "UnPack[%d][%d][%d]：输出缓存为空\r\n", len, datalen, *outlen);
		*outlen = datalen;
		return -1;
	}
	*outlen = datalen;

	// 组织应答数据
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

// 开启接收线程
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

// 停止接收线程
void CYTCom::StopThread()
{
	m_sysstatus = 0;
	m_thread.StopThread(m_connclosetimeout);
}

// 按顺序连接服务器
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

// 同时连接多个服务器
int CYTCom::Connect_Muti()
{
	if(m_serveraddr.size() == 0)
	{
		return -1;
	}

	// 有代理的话则走顺序连接模式
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
			// 关闭检测的socket
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
				
				// 关闭检测的socket
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

// 数据处理
int CYTCom::ProvData(char * data, int len)
{
	// 判断应答数据是否为本用户的
	USDataHeader * pUSHead	= (USDataHeader *)data;

	// 如果是登录应答的话则保存通道号和用户编号
	if(pUSHead->funcid != 150001)
	{
		// 校验是否本用户和本连接的
		if(m_chunnelid == (unsigned int)-1)
		{
			GlobalLogout(LT_RUN, LOG_ERROR, "[0x%.8x]通道号不匹配：(chunnelid:%u,userid:%u,funcid:%u,reqno:%u,result:%u)【memchunnelid:%d】\r\n", 
				this, pUSHead->chunnelid, pUSHead->userid, pUSHead->funcid, pUSHead->reqserailid, pUSHead->result, m_chunnelid);

			DataRet(pUSHead->reqserailid, pUSHead->funcid);
			if(g_CallBackFList.Func_YTConnDataAllRet != NULL)
			{
				char szResultBuffer[32];
				sprintf_s(szResultBuffer, sizeof(szResultBuffer), "非本连接应答");
				g_CallBackFList.Func_YTConnDataAllRet(this,  pUSHead->reqserailid, pUSHead->funcid, EC_DATAILLEGAL, szResultBuffer, strlen(szResultBuffer));
			}
			return -1;
		}
		else if(m_userid != pUSHead->userid)
		{
			GlobalLogout(LT_RUN, LOG_ERROR, "[0x%.8x]用户编号不匹配：(chunnelid:%u,userid:%u,funcid:%u,reqno:%u,result:%u)【memuserid:%d】\r\n", 
				this, pUSHead->chunnelid, pUSHead->userid, pUSHead->funcid, pUSHead->reqserailid, pUSHead->result, m_userid);

			DataRet(pUSHead->reqserailid, pUSHead->funcid);
			if(g_CallBackFList.Func_YTConnDataAllRet != NULL)
			{
				char szResultBuffer[32];
				sprintf_s(szResultBuffer, sizeof(szResultBuffer), "非本用户应答");
				g_CallBackFList.Func_YTConnDataAllRet(this,  pUSHead->reqserailid, pUSHead->funcid, EC_DATAILLEGAL, szResultBuffer, strlen(szResultBuffer));
			}
			return -1;
		}
	}

	// 业务成功的话则获取业务数据信息
	int datalen = 0;
	if(pUSHead->len > sizeof(USDataHeader))
	{
		memcpy(&datalen, data+sizeof(USDataHeader), 4);
	}
  char sztmpretmsg[128] = {0};

	DataRet(pUSHead->reqserailid, pUSHead->funcid);
	if(pUSHead->funcid==150001 && pUSHead->result==0)
	{
		// 如果是登录应答的话做特殊处理，需要解包出userid
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

// 应答返回更新请求map
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

// 监测超时的请求
int CYTCom::CheckTimeOut()
{
	m_reqmaplock.Lock();

	int currtime = (int)time(NULL);
	map<int, tagReq>::iterator iter = m_reqmap.begin();
	while(iter != m_reqmap.end())
	{
		if(currtime-iter->second.sendtime > m_transtimeout)
		{
			// 超时处理
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

// 关闭所有请求信息，目前提示客户端超时
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

// 接收线程
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
		// 首先判断socket状态，如果连接断开则通过回调通知外围
		if(pYTCom->GetStatus() == CS_CLOSE)
		{
			if(pYTCom->m_socket >= 0)
			{
				GlobalLogout(LT_RUN, LOG_WARNING, "[0x%.8x]线程检测到连接断开\r\n", pParam);
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

		// 检测是否可读状态
		selectret = pYTCom->CYTSocket::Select(SOCKSEL_READ, 500);
		if((selectret & SOCKSEL_READ) == SOCKSEL_READ)
		{
			// 检测连接是否已经关闭，如果连接关闭则通过回调通知外围
			sockclose = false;
			pYTCom->m_sockrecvlock.Lock();
			sockclose = pYTCom->CYTSocket::IsClose();
			pYTCom->m_sockrecvlock.UnLock();
			if(sockclose)
			{
				GlobalLogout(LT_RUN, LOG_WARNING, "[0x%.8x]线程检测到连接关闭\r\n", pParam);
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

			// 接收数据并处理
			pYTCom->m_sockrecvlock.Lock();
			if(currlen < sizeof(USDataHeader))
			{
				// 先收包头
				recvlen = pYTCom->CYTSocket::Recv(recvbuf+currlen, sizeof(USDataHeader)-currlen, pYTCom->m_sendandrecvtimeout);
			}
			else
			{
				// 收包体
				recvlen = pYTCom->CYTSocket::Recv(recvbuf+currlen, pUSHead->len-currlen, pYTCom->m_sendandrecvtimeout);
			}
			pYTCom->m_sockrecvlock.UnLock();
			if(recvlen <= 0)
			{
				GlobalLogout(LT_RUN, LOG_WARNING, "[0x%.8x]应答接收失败[%d][%d][%d]\r\n", pParam, totallen, currlen, recvlen);
			}
			else 
			{
				currlen += recvlen;

				// 如果够了一个包头的话则进行业务处理
				if(currlen >= sizeof(USDataHeader))
				{
					// 判断数据长度是否足够，如果不够的话则追加缓存
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
							GlobalLogout(LT_RUN, LOG_WARNING, "[0x%.8x]内存分配失败[%d][%d][%d]\r\n", pParam, totallen, currlen, recvlen);
						}
						continue;
					}
					else if((int)pUSHead->len > currlen)
					{
						// 应收长度未收全
						continue;
					}

					// 进行业务处理
					pYTCom->ProvData(recvbuf, currlen);
					currlen = 0;
				}
			}
		}
		else
		{
			// 如果检测10次都没有消息的话则对已有请求进行超时检测
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
