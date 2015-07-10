// 这是主 DLL 文件。

#include "stdafx.h"
//#include <map>
#include <list>
#include "YTData.h"
#include "YTConn.h"
//#include "../Include/YTConn_Interface.h"
//#include "../Include/YTData_Interface.h"
#include "TradeDll_Interface.h"

//#pragma comment(lib, "../Lib/YTConnDll.lib")
//#pragma comment(lib, "../Lib/BYDataDll.lib")

enum FUNC_ID
{
	LOGIN				= 150001,
	QURELATED			= 100013,
	QUMONEY				= 100100,
	QUAMOUNT			,
	QUUNFINISHENTRUST	,
	QULOG				,
	QUPOSITION			,
	QUBARGAIN			,
	QUCOLLECTBARGAIN	,
	ENTRUST				,
	DISENTRUST			,
	DISLASTENTRUST		,
	DISALLENTRUST		
};
//std::map<unsigned long,void*> m_mapFuncList;
tagTradeCallBackFunc* pFuncList = NULL;
void *  ptrConfig   = NULL;     //配置文件句柄
int iUserID = 0;

/********************连接池，管理socket连接******************************/
std::list<void*>connLists;//连接池的容器队列
int iCurConnects = 0;//当前已建立的数据库连接数量  
int imaxConnects = 200;//连接池中定义的最大数据库连接数 
CRITICAL_SECTION secLock;//线程锁
/**********************************************************/

//请求数据全部返回后，调用该函数
void __stdcall CallBackDataAllRet(void * pConn, int reqno, unsigned int funcid, int errcode, const char * retbuf, int len);
//请求数据部分返回后，调用该函数
void __stdcall CallBackDataPartRet(void * pConn, int reqno, unsigned int funcid, int errcode, const char * retbuf, int len);
//请求超时后，调用该函数
void __stdcall CallBackTimeOut(void * pConn, int reqno, unsigned int funcid);
//连接断开后，调用该函数
void __stdcall CallBackClose(void * pConn);

const char * Trade_GetVer(void)
{
	return YTConn_GetVer();//YTConn_GetVer();
}

void GetParentPath(const char* file,char* path){
  strcpy(path,file);
  for(int i = strlen(path) - 1; i >= 0; --i){
    if(path[i] == '\\'){
      path[i] = '\0';
      break;
    }
  }
}

//初始化连接对象
int Trade_Init(const char *cfgfile, tagTradeCallBackFunc * callbackflist, const char *logpath, char *errmsg, int len)
{
	if (ptrConfig != NULL)
	{
		YTConn_DelConfig(ptrConfig);
	}
	ptrConfig = YTConn_NewConfig();
	if (YTConn_LoadConfig(ptrConfig, cfgfile) < 0)
	{
		YTConn_DelConfig(ptrConfig);
		sprintf_s(errmsg,len,"加载配置文件:%s失败！",cfgfile);
		return -1;
	}

  static tagYTConnCallBackFunc callbackflist1 = 
    {CallBackDataAllRet,CallBackDataPartRet,CallBackTimeOut,CallBackClose};

  char path[1021];
  memset(path,0,1021);
  GetParentPath(cfgfile,path);
	if (YTConn_Init(path,&callbackflist1,logpath) < 0)
	{
		YTConn_Release();
		return -4;
	}
	
	if (callbackflist == NULL)
	{
		strcpy_s(errmsg,len,"没有注册回调函数！");
		return -2;
	}
	//记录客户注册的回调函数
	pFuncList = callbackflist;
	/*m_mapFuncList.insert(std::make_pair(0,callbackflist->Func_TradeCallBackStatus));
	m_mapFuncList.insert(std::make_pair(LOGIN,callbackflist->Func_TradeAnsLogin));
	m_mapFuncList.insert(std::make_pair(QURELATED,callbackflist->Func_TradeAnsQuRelated));
	m_mapFuncList.insert(std::make_pair(QUAMOUNT,callbackflist->Func_TradeAnsQuAmount));
	m_mapFuncList.insert(std::make_pair(QUUNFINISHENTRUST, callbackflist->Func_TradeAnsQuUnFinishEntrust));
	m_mapFuncList.insert(std::make_pair(QULOG,callbackflist->Func_TradeAnsQuLog));
	m_mapFuncList.insert(std::make_pair(QUPOSITION,callbackflist->Func_TradeAnsQuPosition));
	m_mapFuncList.insert(std::make_pair(QUBARGAIN,callbackflist->Func_TradeAnsQuBargain));
	m_mapFuncList.insert(std::make_pair(QUCOLLECTBARGAIN,callbackflist->Func_TradeAnsQuCollectBargain));
	m_mapFuncList.insert(std::make_pair(ENTRUST,callbackflist->Func_TradeAnsEntrust));
	m_mapFuncList.insert(std::make_pair(DISENTRUST,callbackflist->Func_TradeAnsDisEntrust));
	m_mapFuncList.insert(std::make_pair(DISLASTENTRUST,callbackflist->Func_TradeAnsDisLastEntrust));
	m_mapFuncList.insert(std::make_pair(DISALLENTRUST,callbackflist->Func_TradeAnsDisAllEntrust));*/

	//YTConn_CallBackDataAllRet(CallBackDataAllRet);
	//YTConn_CallBackDataPartRet(CallBackDataPartRet);
	//YTConn_CallBackTimeOut(CallBackTimeOut);
	//YTConn_CallBackClose(CallBackClose);

	InitializeCriticalSection(&secLock );
	return 0;
}

void ReleaseConns()
{
	EnterCriticalSection(&secLock);
	std::list<void *>::iterator iter = connLists.begin(); 
	for (;iter != connLists.end();iter++)
	{
		YTConn_Close(*iter);
		YTConn_DelConn(*iter);
	}
	connLists.clear();
	iCurConnects = 0;
	LeaveCriticalSection(&secLock);
}
void Trade_Release(void)
 {
	ReleaseConns();
	YTConn_Release();
 }

int SendData(void* data,int iFuncID, void* conn)
{
	int sendlen = 0;
	const char * sendbuf = YTData_MakeSendData(data, &sendlen);
	long reqno = YTConn_Send(conn, iFuncID, sendbuf, sendlen);
	YTData_DeleteData(data);
	if (reqno >0)
	{
		return reqno;
	}
	else
		return -3;
}

void* GetConnect()
{
	void* con;  
	EnterCriticalSection(&secLock); 
	if(connLists.size()>0)//连接池容器中还有连接  
	{  
		con=connLists.front();//得到第一个连接  
		connLists.pop_front();//移除第一个连接  
		if(YTConn_GetStatus(con) < 0)//如果连接已经被关闭，删除后重新建立一个  
		{  
			YTConn_Close(con);
			YTConn_Connect(con);
		}  
		
		LeaveCriticalSection(&secLock);  
		return con;  
	}  
	else
	{
		if(iCurConnects< imaxConnects)
		{//还可以创建新的连接  
			con= YTConn_NewConn(ptrConfig);
			if (YTConn_Connect(con) < 0)
			{
				YTConn_DelConn(con);
				con = NULL;
			}
			LeaveCriticalSection(&secLock);
			return con;
		}  
		else
		{//建立的连接数已经达到maxSize  
			LeaveCriticalSection(&secLock);
			return NULL; 
		}  
	}  
}

void PutConnect(void* conn)
{
	EnterCriticalSection(&secLock);
	connLists.push_back(conn);
	LeaveCriticalSection(&secLock);
}

int Trade_Login(tagTradeReqLogin *pReqLogin, char *errmsg, int len)
{
	if (pReqLogin == NULL)
	{
		return -1;
	}
	if (iCurConnects > 0)  //已经有用户登录了，本次应该是切换账户
	{
		ReleaseConns();
	}	
  

  for(int k=0;k<imaxConnects/2;k++)  
  { 
    void * datahandle = YTData_NewData();
    void* conn_login = YTConn_NewConn(ptrConfig);
    if (YTConn_Connect(conn_login) < 0)
    {
      strcpy_s(errmsg,len,"与服务端建立连接失败！");
      return -2;
    } 

    if(datahandle == NULL)
    {
      return -4;
    }
    YTData_AppendRow(datahandle);
    YTData_AddFieldString(datahandle, "user", pReqLogin->User);
    YTData_AddFieldPwd(datahandle, "pwd", pReqLogin->Password);
    SendData(datahandle,150001,conn_login);
  }  

  return 0;  
}

int Trade_QuRelated(tagTradeReqQuRelated *pReqQuRelated, char *errmsg, int len)
{
	if (pReqQuRelated == NULL)
	{
		return -1;
	}

	void * datahandle = YTData_NewData();
	if(datahandle == NULL)
	{
		return -4;
	}
	YTData_AppendRow(datahandle);
	YTData_AddFieldInt(datahandle, "userid", pReqQuRelated->UserID);

	void* conn = GetConnect();
	if (conn == NULL)
	{
		return -3;
	}
	else
	{
		int irtn = SendData(datahandle,100013,conn);
		PutConnect(conn);
		return irtn;
	}
}

int Trade_QuMoney(tagTradeReqQuMoney *pReqQuMoney, char *errmsg, int len)
{
	if (pReqQuMoney == NULL)
	{
		return -1;
	}

	void * datahandle = YTData_NewData();
	if(datahandle == NULL)
	{
		return -4;
	}
	YTData_AppendRow(datahandle);
	YTData_AddFieldInt(datahandle, "userid", pReqQuMoney->UserID);
	YTData_AddFieldInt(datahandle, "accountid", pReqQuMoney->AccID);

	void* conn = GetConnect();
	if (conn == NULL)
	{
		return -3;
	}
	else
	{
		int irtn = SendData(datahandle,100100,conn);
		PutConnect(conn);
		return irtn;
	}
	
}

int Trade_QuAmount(tagTradeReqQuAmount *pReqQuAmount, char *errmsg, int len)
{
	if (pReqQuAmount == NULL)
	{
		return -1;
	}

	void * datahandle = YTData_NewData();
	if(datahandle == NULL)
	{
		return -4;
	}
	YTData_AppendRow(datahandle);
	YTData_AddFieldInt(datahandle, "userid", pReqQuAmount->UserID);
	YTData_AddFieldInt(datahandle, "accountid", pReqQuAmount->AccID);
	YTData_AddFieldString(datahandle, "symbol", pReqQuAmount->InstrumentID);
	YTData_AddFieldString(datahandle, "exchange", pReqQuAmount->ExchangeID);

	void* conn = GetConnect();
	if (conn == NULL)
	{
		return -3;
	}
	else
	{
		int irtn = SendData(datahandle,100101,conn);
		PutConnect(conn);
		return irtn;
	}
}

int Trade_QuUnFinishEntrust(tagTradeReqQuUnFinishEntrust *pReqQuUnFinishEntrust, char *errmsg, int len)
{
	if (pReqQuUnFinishEntrust == NULL)
	{
		return -1;
	}	

	void * datahandle = YTData_NewData();
	if(datahandle == NULL)
	{
		return -4;
	}
	YTData_AppendRow(datahandle);
	YTData_AddFieldInt(datahandle, "userid", pReqQuUnFinishEntrust->UserID);
	YTData_AddFieldInt(datahandle, "lasttime", pReqQuUnFinishEntrust->LastUpdateTime);

	void* conn = GetConnect();
	if (conn == NULL)
	{
		return -3;
	}
	else
	{
		int irtn = SendData(datahandle,100102,conn);
		PutConnect(conn);
		return irtn;
	}
}

int Trade_QuLog(tagTradeReqQuLog *pReqQuLog, char *errmsg, int len)
{
	if (pReqQuLog == NULL)
	{
		return -1;
	}	

	void * datahandle = YTData_NewData();
	if(datahandle == NULL)
	{
		return -4;
	}
	YTData_AppendRow(datahandle);
	YTData_AddFieldInt(datahandle, "userid", pReqQuLog->UserID);
	YTData_AddFieldInt(datahandle, "qryindex", pReqQuLog->Index);

	void* conn = GetConnect();
	if (conn == NULL)
	{
		return -3;
	}
	else
	{
		int irtn = SendData(datahandle,100103,conn);
		PutConnect(conn);
		return irtn;
	}
}

int Trade_QuPosition(tagTradeReqQuPosition *pReqQuPosition, char *errmsg, int len)
{
	if (pReqQuPosition == NULL)
	{
		return -1;
	}

	void * datahandle = YTData_NewData();
	if(datahandle == NULL)
	{
		return -4;
	}
	YTData_AppendRow(datahandle);
	YTData_AddFieldInt(datahandle, "userid", pReqQuPosition->UserID);
	YTData_AddFieldInt(datahandle, "accountid", pReqQuPosition->AccID);

	void* conn = GetConnect();
	if (conn == NULL)
	{
		return -3;
	}
	else
	{
		int irtn = SendData(datahandle,100104,conn);
		PutConnect(conn);
		return irtn;
	}
}

int Trade_QuBargain(tagTradeReqQuBargain *pReqQuBargain, char *errmsg, int len)
{
	if (pReqQuBargain == NULL)
	{
		return -1;
	}	

	void * datahandle = YTData_NewData();
	if(datahandle == NULL)
	{
		return -4;
	}
	YTData_AppendRow(datahandle);
	YTData_AddFieldInt(datahandle, "userid", pReqQuBargain->UserID);
	YTData_AddFieldInt(datahandle, "accountid", pReqQuBargain->AccID);

	void* conn = GetConnect();
	if (conn == NULL)
	{
		return -3;
	}
	else
	{
		int irtn = SendData(datahandle,100105,conn);
		PutConnect(conn);
		return irtn;
	}
}

int Trade_QuCollectBargain(tagTradeReqQuCollectBargain *pReqQuCollectBargain, char *errmsg, int len)
{
	if (pReqQuCollectBargain == NULL)
	{
		return -1;
	}	

	void * datahandle = YTData_NewData();
	if(datahandle == NULL)
	{
		return -4;
	}
	YTData_AppendRow(datahandle);
	YTData_AddFieldInt(datahandle, "userid", pReqQuCollectBargain->UserID);
	YTData_AddFieldInt(datahandle, "begindate", pReqQuCollectBargain->BeginDate);
	YTData_AddFieldInt(datahandle, "enddate", pReqQuCollectBargain->EndDate);

	void* conn = GetConnect();
	if (conn == NULL)
	{
		return -3;
	}
	else
	{
		int irtn = SendData(datahandle,100106,conn);
		PutConnect(conn);
		return irtn;
	}
}

int Trade_Entrust(tagTradeReqEntrust *pReqEntrust, char *errmsg, int len)
{
	if (pReqEntrust == NULL)
	{
		return -1;
	}

	void * datahandle = YTData_NewData();
	if(datahandle == NULL)
	{
		return -4;
	}
	YTData_AppendRow(datahandle);
	YTData_AddFieldInt(datahandle, "userid", pReqEntrust->UserID);
	YTData_AddFieldInt(datahandle, "accountid", pReqEntrust->AccID);
	YTData_AddFieldString(datahandle, "side", &(pReqEntrust->EntrustType));
	YTData_AddFieldString(datahandle, "symbol", pReqEntrust->InstrumentID);
	YTData_AddFieldString(datahandle, "cname", pReqEntrust->InstrumentName);
	YTData_AddFieldString(datahandle, "exchange", pReqEntrust->ExchangeID);
	YTData_AddFieldDouble(datahandle, "price", pReqEntrust->EntrustPrice);
	YTData_AddFieldInt(datahandle, "entrustamount", pReqEntrust->EntrustAmount);
	//YTData_AddFieldString(datahandle, "uplimitflag", pReqEntrust->);

	void* conn = GetConnect();
	if (conn == NULL)
	{
		return -3;
	}
	else
	{
		int irtn = SendData(datahandle,100107,conn);
		PutConnect(conn);
		return irtn;
	}
}

int Trade_DisEntrust(tagTradeReqDisEntrust *pReqDisEntrust, char *errmsg, int len)
{
	if (pReqDisEntrust == NULL)
	{
		return -1;
	}	

	void * datahandle = YTData_NewData();
	if(datahandle == NULL)
	{
		return -4;
	}
	YTData_AppendRow(datahandle);
	YTData_AddFieldInt(datahandle, "userid", pReqDisEntrust->UserID);
	YTData_AddFieldInt(datahandle, "accountid", pReqDisEntrust->AccID);
	YTData_AddFieldInt(datahandle, "entrustdate", pReqDisEntrust->EntrustDate);
	YTData_AddFieldString(datahandle, "entrustno", pReqDisEntrust->EntrustNo);
	YTData_AddFieldString(datahandle, "exchange", pReqDisEntrust->ExchangeID);
	//YTData_AddFieldString(datahandle, "uplimitflag", pReqEntrust->);

	void* conn = GetConnect();
	if (conn == NULL)
	{
		return -3;
	}
	else
	{
		int irtn = SendData(datahandle,100108,conn);
		PutConnect(conn);
		return irtn;
	}
}

int Trade_DisLastEntrust(tagTradeReqDisLastEntrust *pReqDisLastEntrust, char *errmsg, int len)
{
	if (pReqDisLastEntrust == NULL)
	{
		return -1;
	}

	void * datahandle = YTData_NewData();
	if(datahandle == NULL)
	{
		return -4;
	}
	YTData_AppendRow(datahandle);
	YTData_AddFieldInt(datahandle, "userid", pReqDisLastEntrust->UserID);
	YTData_AddFieldInt(datahandle, "accountid", pReqDisLastEntrust->AccID);
	YTData_AddFieldString(datahandle, "symbol", pReqDisLastEntrust->InstrumentID);
	YTData_AddFieldString(datahandle, "exchange", pReqDisLastEntrust->ExchangeID);

	void* conn = GetConnect();
	if (conn == NULL)
	{
		return -3;
	}
	else
	{
		int irtn = SendData(datahandle,100109,conn);
		PutConnect(conn);
		return irtn;
	}
}

int Trade_DisAllEntrust(tagTradeReqDisAllEntrust *pReqDisAllEntrust, char *errmsg, int len)
{
	if (pReqDisAllEntrust == NULL)
	{
		return -1;
	}	

	void * datahandle = YTData_NewData();
	if(datahandle == NULL)
	{
		return -4;
	}
	YTData_AppendRow(datahandle);
	YTData_AddFieldInt(datahandle, "userid", pReqDisAllEntrust->UserID);
	YTData_AddFieldInt(datahandle, "accountid", pReqDisAllEntrust->AccID);
	YTData_AddFieldString(datahandle, "symbol", pReqDisAllEntrust->InstrumentID);
	YTData_AddFieldString(datahandle, "exchange", pReqDisAllEntrust->ExchangeID);

	void* conn = GetConnect();
	if (conn == NULL)
	{
		return -3;
	}
	else
	{
		int irtn = SendData(datahandle,100110,conn);
		PutConnect(conn);
		return irtn;
	}
}


/**********************************以下实现回调函数*********************************/


void MakeRtnData(void * pConn,long reqno, unsigned long funcid, long errcode,const char * errInfo , const char * retbuf, long len,bool islast)
{
	tagTradeErrorInfo *pErrInfo = new tagTradeErrorInfo();
	pErrInfo->ErrorID = 0;
	void * datahandle = NULL;
	int iRowCount = 0;
	if (errcode >= 0)
	{
		datahandle = YTData_NewData();
		YTData_ConvertData(datahandle, retbuf, len);
		iRowCount = YTData_GetRowCount(datahandle);
		if (iRowCount > 0)
		{
			YTData_GotoBeginRow(datahandle);
			pErrInfo->ErrorID = YTData_GetFieldInt(datahandle,"retcode");
			if (pErrInfo->ErrorID < 0)
			{
				YTData_GetFieldString(datahandle,"comment",pErrInfo->ErrorMsg,sizeof(pErrInfo->ErrorMsg));
			}
		}
	}
	else
	{
		pErrInfo->ErrorID = errcode;
		strcpy_s(pErrInfo->ErrorMsg, sizeof(pErrInfo->ErrorMsg), errInfo);
	}

	int i = 0;
	switch(funcid)
	{
	case LOGIN:
		if (errcode < 0 || pErrInfo->ErrorID < 0)  
		{
			pFuncList->Func_TradeAnsLogin(NULL,pErrInfo);
			
		}
		else  //正常返回
		{
			tagTradeAnsLogin* pPara = new tagTradeAnsLogin();
			pPara->UserID = YTData_GetFieldInt(datahandle,"userid");
			pPara->UserType = YTData_GetFieldInt(datahandle,"usertype");
			iUserID = pPara->UserID;

			//登陆成功之后，初始化连接池。连接池开始有maxconn/2 个连接
      void* conn = pConn;  
      EnterCriticalSection(&secLock); 
      //for(int k=0;k<imaxConnects/2;k++)  
      //{ 
      //  conn = YTConn_NewConn(ptrConfig);
      //  if (YTConn_Connect(conn) >=0)
      //  {
      //    connLists.push_back(conn);
      //    iCurConnects++;
      //  }
      //}  
      connLists.push_back(conn);
      iCurConnects++;
      LeaveCriticalSection(&secLock);  
      if(iCurConnects >= imaxConnects/2){ 
			  pFuncList->Func_TradeAnsLogin(pPara,pErrInfo);
      }
			delete pPara;			
		}
		break;
	case QURELATED:
		if (errcode < 0 || pErrInfo->ErrorID < 0)  
		{
			pFuncList->Func_TradeAnsQuRelated(NULL,0,pErrInfo,reqno,islast);

		}
		else  //正常返回
		{
			iRowCount = YTData_GetRowCount(datahandle); 
			tagTradeAnsQuRelated* pPara = new tagTradeAnsQuRelated[iRowCount];
			for (i=0;i<iRowCount;i++)
			{
				pPara[i].UserID = YTData_GetFieldInt(datahandle,"userid");
				YTData_GetFieldString(datahandle,"user",pPara[i].User,sizeof(pPara[i].User));
				pPara[i].AccID = YTData_GetFieldInt(datahandle,"accountid");
				YTData_GetFieldString(datahandle,"accountalias",pPara[i].AccountName,sizeof(pPara[i].AccountName));
				pPara[i].CounterID = YTData_GetFieldInt(datahandle,"counterid");
				pPara[i].UsableMoney = YTData_GetFieldDouble(datahandle,"usablemoney");
			}
			pFuncList->Func_TradeAnsQuRelated(pPara,iRowCount,pErrInfo,reqno,islast);
			delete []pPara;
		}
		break;
	case QUMONEY:
		if (errcode < 0 || pErrInfo->ErrorID < 0)  
		{
			pFuncList->Func_TradeAnsQuMoney(NULL,0,pErrInfo,reqno,islast);

		}
		else  //正常返回
		{
			iRowCount = YTData_GetRowCount(datahandle); 
			tagTradeAnsQuMoney* pPara = new tagTradeAnsQuMoney[iRowCount];
			for (i=0;i<iRowCount;i++)
			{
				pPara[i].UsableMoney = YTData_GetFieldDouble(datahandle,"usablemoney");
			}
			pFuncList->Func_TradeAnsQuMoney(pPara,iRowCount,pErrInfo,reqno,islast);
			delete []pPara;
		}
		break;
	case QUAMOUNT:
		if (errcode < 0 || pErrInfo->ErrorID < 0)  
		{
			pFuncList->Func_TradeAnsQuAmount(NULL,0,pErrInfo,reqno,islast);

		}
		else  //正常返回
		{
			iRowCount = YTData_GetRowCount(datahandle); 
			tagTradeAnsQuAmount* pPara = new tagTradeAnsQuAmount[iRowCount];
			for (i=0;i<iRowCount;i++)
			{

				pPara[i].AccID = YTData_GetFieldInt(datahandle,"accountid");
				YTData_GetFieldString(datahandle,"accountalias",pPara[i].AccountName,sizeof(pPara[i].AccountName));
				pPara[i].Amount = YTData_GetFieldInt(datahandle,"stockamount");
				YTData_GetFieldString(datahandle,"symbol",pPara[i].InstrumentID,sizeof(pPara[i].InstrumentID));
				YTData_GetFieldString(datahandle,"exchange",pPara[i].ExchangeID,sizeof(pPara[i].ExchangeID));
				YTData_GetFieldString(datahandle,"cname",pPara[i].InstrumentName,sizeof(pPara[i].InstrumentName));
			}
			pFuncList->Func_TradeAnsQuAmount(pPara,iRowCount,pErrInfo,reqno,islast);
			delete []pPara;
		}
		break;
	case QUUNFINISHENTRUST:
		if (errcode < 0 || pErrInfo->ErrorID < 0)  
		{
			pFuncList->Func_TradeAnsQuUnFinishEntrust(NULL,0,pErrInfo,reqno,islast);

		}
		else  //正常返回
		{
			iRowCount = YTData_GetRowCount(datahandle); 
			tagTradeAnsQuUnFinishEntrust* pPara = new tagTradeAnsQuUnFinishEntrust[iRowCount];
			for (i=0;i<iRowCount;i++)
			{
				pPara[i].UserID = YTData_GetFieldInt(datahandle,"userid");
				YTData_GetFieldString(datahandle,"user",pPara[i].User,sizeof(pPara[i].User));
				pPara[i].AccID = YTData_GetFieldInt(datahandle,"accountid");
				YTData_GetFieldString(datahandle,"accountalias",pPara[i].AccountName,sizeof(pPara[i].AccountName));
				pPara[i].CounterID = YTData_GetFieldInt(datahandle,"counterid");
				YTData_GetFieldString(datahandle,"symbol",pPara[i].InstrumentID,sizeof(pPara[i].InstrumentID));
				YTData_GetFieldString(datahandle,"exchange",pPara[i].ExchangeID,sizeof(pPara[i].ExchangeID));
				YTData_GetFieldString(datahandle,"cname",pPara[i].InstrumentName,sizeof(pPara[i].InstrumentName));
				pPara[i].EntrustDate = YTData_GetFieldInt(datahandle,"entrustdate");
				pPara[i].EntrustTime = YTData_GetFieldInt(datahandle,"entrusttime");
				pPara[i].EntrustType = YTData_GetFieldChar(datahandle,"entrusttype");
				pPara[i].EntrustStatus = YTData_GetFieldChar(datahandle,"entruststatus");
				YTData_GetFieldString(datahandle,"entrustno",pPara[i].EntrustNo,sizeof(pPara[i].EntrustNo));
				pPara[i].EntrustPrice = YTData_GetFieldDouble(datahandle,"entrustprice");
				pPara[i].EntrustAmount = YTData_GetFieldInt(datahandle,"entrustamount");
				pPara[i].BargAmount = YTData_GetFieldInt(datahandle,"stockturnover");
				pPara[i].LastUpdateTime = YTData_GetFieldInt(datahandle,"lasttime");
			}
			pFuncList->Func_TradeAnsQuUnFinishEntrust(pPara,iRowCount,pErrInfo,reqno,islast);
			delete []pPara;
		}
		break;
	case QULOG:
		if (errcode < 0 || pErrInfo->ErrorID < 0)  
		{
			pFuncList->Func_TradeAnsQuLog(NULL,0,pErrInfo,reqno,islast);

		}
		else  //正常返回
		{
			iRowCount = YTData_GetRowCount(datahandle); 
			tagTradeAnsQuLog* pPara = new tagTradeAnsQuLog[iRowCount];
			for (i=0;i<iRowCount;i++)
			{
				pPara[i].UserID = YTData_GetFieldInt(datahandle,"userid");
				YTData_GetFieldString(datahandle,"user",pPara[i].User,sizeof(pPara[i].User));
				pPara[i].AccID = YTData_GetFieldInt(datahandle,"accountid");
				YTData_GetFieldString(datahandle,"accountalias",pPara[i].AccountName,sizeof(pPara[i].AccountName));
				YTData_GetFieldString(datahandle,"symbol",pPara[i].InstrumentID,sizeof(pPara[i].InstrumentID));
				YTData_GetFieldString(datahandle,"cname",pPara[i].InstrumentName,sizeof(pPara[i].InstrumentName));
				pPara[i].EntrustDate = YTData_GetFieldInt(datahandle,"entrustdate");
				pPara[i].EntrustTime = YTData_GetFieldInt(datahandle,"entrusttime");
				pPara[i].EntrustStatus = YTData_GetFieldChar(datahandle,"entruststatus");
				YTData_GetFieldString(datahandle,"entrustno",pPara[i].EntrustNo,sizeof(pPara[i].EntrustNo));
				YTData_GetFieldString(datahandle,"event",pPara[i].LogDetail,sizeof(pPara[i].LogDetail));
				pPara[i].Index = YTData_GetFieldInt(datahandle,"qryindex");
			}
			pFuncList->Func_TradeAnsQuLog(pPara,iRowCount,pErrInfo,reqno,islast);
			delete []pPara;
		}
		break;
	case QUPOSITION:
		if (errcode < 0 || pErrInfo->ErrorID < 0)  
		{
			pFuncList->Func_TradeAnsQuPosition(NULL,0,pErrInfo,reqno,islast);

		}
		else  //正常返回
		{
			iRowCount = YTData_GetRowCount(datahandle); 
			tagTradeAnsQuPosition* pPara = new tagTradeAnsQuPosition[iRowCount];
			for (i=0;i<iRowCount;i++)
			{
				if (i==0)
				{
					pPara[i].TotalMarketValue = YTData_GetFieldDouble(datahandle,"totalmarketvalue");
					pPara[i].FloatProfitorLoss = YTData_GetFieldDouble(datahandle,"floatprofitorloss");
					pPara[i].AchieveFpl = YTData_GetFieldDouble(datahandle,"achievefpl");
				}
				YTData_GetFieldString(datahandle,"user",pPara[i].User,sizeof(pPara[i].User));
				YTData_GetFieldString(datahandle,"accountalias",pPara[i].AccountName,sizeof(pPara[i].AccountName));
				YTData_GetFieldString(datahandle,"symbol",pPara[i].InstrumentID,sizeof(pPara[i].InstrumentID));
				YTData_GetFieldString(datahandle,"cname",pPara[i].InstrumentName,sizeof(pPara[i].InstrumentName));
				YTData_GetFieldString(datahandle,"exchange",pPara[i].ExchangeID,sizeof(pPara[i].ExchangeID));
				pPara[i].Amount = YTData_GetFieldInt(datahandle,"stockamount");
				pPara[i].Side = YTData_GetFieldInt(datahandle,"side");
				pPara[i].CurrentFloatProfit = YTData_GetFieldDouble(datahandle,"currentfloatprofit");
				pPara[i].AveragePrice = YTData_GetFieldDouble(datahandle,"averageprice");
			}
			pFuncList->Func_TradeAnsQuPosition(pPara,iRowCount,pErrInfo,reqno,islast);
			delete []pPara;
		}
		break;
	case QUBARGAIN:
		if (errcode < 0 || pErrInfo->ErrorID < 0)  
		{
			pFuncList->Func_TradeAnsQuBargain(NULL,0,pErrInfo,reqno,islast);

		}
		else  //正常返回
		{
			iRowCount = YTData_GetRowCount(datahandle); 
			tagTradeAnsQuBargain* pPara = new tagTradeAnsQuBargain[iRowCount];
			for (i=0;i<iRowCount;i++)
			{
				pPara[i].UserID = YTData_GetFieldInt(datahandle,"userid");
				YTData_GetFieldString(datahandle,"user",pPara[i].User,sizeof(pPara[i].User));
				pPara[i].AccID = YTData_GetFieldInt(datahandle,"accountid");
				YTData_GetFieldString(datahandle,"accountalias",pPara[i].AccountName,sizeof(pPara[i].AccountName));
				YTData_GetFieldString(datahandle,"symbol",pPara[i].InstrumentID,sizeof(pPara[i].InstrumentID));
				YTData_GetFieldString(datahandle,"cname",pPara[i].InstrumentName,sizeof(pPara[i].InstrumentName));
				pPara[i].EntrustType = YTData_GetFieldChar(datahandle,"entrusttype");
				pPara[i].EntrustTime = YTData_GetFieldInt(datahandle,"entrusttime");
				YTData_GetFieldString(datahandle,"entrustno",pPara[i].EntrustNo,sizeof(pPara[i].EntrustNo));
				pPara[i].BargAmount = YTData_GetFieldInt(datahandle,"stockturnover");
				pPara[i].BargPrice = YTData_GetFieldDouble(datahandle,"bargainprice");
				pPara[i].EntrustFee = YTData_GetFieldChar(datahandle,"entrustfee");
			}
			pFuncList->Func_TradeAnsQuBargain(pPara,iRowCount,pErrInfo,reqno,islast);
			delete []pPara;
		}
		break;
	case QUCOLLECTBARGAIN:
		if (errcode < 0 || pErrInfo->ErrorID < 0)  
		{
			pFuncList->Func_TradeAnsQuCollectBargain(NULL,0,pErrInfo,reqno,islast);

		}
		else  //正常返回
		{
			iRowCount = YTData_GetRowCount(datahandle); 
			tagTradeAnsQuCollectBargain* pPara = new tagTradeAnsQuCollectBargain[iRowCount];
			for (i=0;i<iRowCount;i++)
			{
				if (i == 0)
				{
					pPara[i].AchieveFpl = YTData_GetFieldDouble(datahandle,"achievefpl");
					pPara[i].TotalTradeMoney = YTData_GetFieldDouble(datahandle,"totaltrademoney");
				}
				pPara[i].UserID = YTData_GetFieldInt(datahandle,"userid");
				YTData_GetFieldString(datahandle,"accountalias",pPara[i].AccountName,sizeof(pPara[i].AccountName));
				YTData_GetFieldString(datahandle,"symbol",pPara[i].InstrumentID,sizeof(pPara[i].InstrumentID));
				YTData_GetFieldString(datahandle,"cname",pPara[i].InstrumentName,sizeof(pPara[i].InstrumentName));
				pPara[i].BargAmount = YTData_GetFieldInt(datahandle,"stockturnover");
				pPara[i].EntrustCount = YTData_GetFieldInt(datahandle,"entrustcount");
				pPara[i].BargMoney = YTData_GetFieldDouble(datahandle,"priceturnover");
				pPara[i].Profits = YTData_GetFieldDouble(datahandle,"profits");
				pPara[i].EntrustFee = YTData_GetFieldChar(datahandle,"entrustfee");
			}
			pFuncList->Func_TradeAnsQuCollectBargain(pPara,iRowCount,pErrInfo,reqno,islast);
			delete []pPara;
		}
		break;
	case ENTRUST:
		pFuncList->Func_TradeAnsEntrust(pErrInfo,reqno);
		break;
	case DISENTRUST:
		pFuncList->Func_TradeAnsDisEntrust(pErrInfo,reqno);
		break;
	case  DISLASTENTRUST:
		pFuncList->Func_TradeAnsDisLastEntrust(pErrInfo,reqno);
		break;
	case DISALLENTRUST:
		pFuncList->Func_TradeAnsDisAllEntrust(pErrInfo,reqno);
		break;
	default:
		break;
	}

	YTData_DeleteData(datahandle);
	delete pErrInfo;
}

void __stdcall CallBackDataAllRet(void * pConn, int reqno, unsigned int funcid, int errcode, const char * retbuf, int len)
{
	MakeRtnData(pConn,reqno, funcid, errcode,"", retbuf, len,true);
	EnterCriticalSection(&secLock);
	connLists.push_back(pConn);
	LeaveCriticalSection(&secLock);
}

void __stdcall CallBackDataPartRet(void * pConn, int reqno, unsigned int funcid, int errcode, const char * retbuf, int len)
{
	MakeRtnData(pConn,reqno, funcid, errcode,"", retbuf, len,false);
	EnterCriticalSection(&secLock);
	connLists.push_back(pConn);
	LeaveCriticalSection(&secLock);
}

void __stdcall CallBackTimeOut(void * pConn, int reqno, unsigned int funcid)
{
	MakeRtnData(pConn,reqno, funcid, -99,"通讯超时！", "", 0,true);
	if (YTConn_Connect(pConn) >= 0)
	{
		EnterCriticalSection(&secLock);
		connLists.push_back(pConn);
		LeaveCriticalSection(&secLock);
	}
	else
	{
		tagTradeCallBackStatus *pCallBackStatus = new tagTradeCallBackStatus();
		pCallBackStatus->status = 0;
		strcpy(pCallBackStatus->statusname ,"连接被关闭了！");
		pFuncList->Func_TradeCallBackStatus(pCallBackStatus);
		delete pCallBackStatus;

	}
}

void __stdcall CallBackClose(void * pConn)
{
	if (YTConn_Connect(pConn) >= 0)
	{
		EnterCriticalSection(&secLock);
		connLists.push_back(pConn);
		LeaveCriticalSection(&secLock);
	}
	else
	{
		tagTradeCallBackStatus *pCallBackStatus = new tagTradeCallBackStatus();
		pCallBackStatus->status = 0;
		strcpy(pCallBackStatus->statusname ,"连接被关闭了！");
		pFuncList->Func_TradeCallBackStatus(pCallBackStatus);
		delete pCallBackStatus;
		
	}
}