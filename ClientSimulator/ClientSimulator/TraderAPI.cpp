#include "StdAfx.h"
#include "data.h"
#include "basefuncs.h"
#include "TraderAPI.h"

#pragma comment(lib,"YTTradeApi.lib")

std::vector<FuncAnsInfo*> g_vctAnsInfo;
HANDLE g_event;
int g_iUserID=0;
std::map<int,int> g_mapFuncID2RowID;


/******************定义回调函数，用来处理服务端的应答*************************/
void _stdcall RecallTradeAnsLogin(tagTradeAnsLogin *pAnsLogin, tagTradeErrorInfo *pErrInfo)  //编写登陆的回调函数
{
	if (pErrInfo->ErrorID < 0)
	{
		//todo 写日志
	}
	else
	{
		g_iUserID = pAnsLogin->UserID;
		SetEvent(g_event);  //通知应用，登陆成功
	}
}

inline void WriteAnsTime(tagTradeErrorInfo *pErrInfo,int nRequestID, bool bLast,int iFuncNO)
{
	int iNO = g_mapFuncID2RowID[iFuncNO];
	FuncAnsInfo* oAnsInfo = g_vctAnsInfo[iNO];
	if (pErrInfo->ErrorID >= 0 )
	{
		if (bLast == true)
		{
			oAnsInfo->iAnsTimes++;
			oAnsInfo->mapReqNO2AndTm.insert(std::make_pair(nRequestID, GetTickCountEX()) );
		}

	}
	else
	{
		oAnsInfo->iErrTimes++;
	}
	oAnsInfo = NULL;
}

void _stdcall RecallTradeAnsQuAmount(tagTradeAnsQuAmount *pAnsQuAmount, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast)
{
	WriteAnsTime(pErrInfo,nRequestID,bLast,QUAMOUNT);
}

void _stdcall RecallTradeAnsQuBargain(tagTradeAnsQuBargain *pAnsQuBargain, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast)
{
	WriteAnsTime(pErrInfo,nRequestID,bLast,QUBARGAIN);
}

void _stdcall RecallTradeAnsQuCollectBargain(tagTradeAnsQuCollectBargain *pAnsQuCollectBargain, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast)
{
	WriteAnsTime(pErrInfo,nRequestID,bLast,QUCOLLECTBARGAIN);
}

void _stdcall RecallTradeAnsQuLog(tagTradeAnsQuLog *pAnsQuLog, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast)
{
	WriteAnsTime(pErrInfo,nRequestID,bLast,QULOG);
}

void _stdcall RecallTradeAnsQuMoney(tagTradeAnsQuMoney *pAnsQuMoney, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast)
{
	WriteAnsTime(pErrInfo,nRequestID,bLast,QUMONEY);
}

void _stdcall RecallTradeAnsQuPosition(tagTradeAnsQuPosition *pAnsQuPosition, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast)
{
	WriteAnsTime(pErrInfo,nRequestID,bLast,QUPOSITION);
}

void _stdcall RecallTradeAnsQuRelated(tagTradeAnsQuRelated *pAnsQuRelated, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast)
{
	WriteAnsTime(pErrInfo,nRequestID,bLast,QURELATED);
}

void _stdcall RecallTradeAnsQuUnFinishEntrust(tagTradeAnsQuUnFinishEntrust *pAnsQuUnFinishEntrust, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast)
{
	WriteAnsTime(pErrInfo,nRequestID,bLast,QUUNFINISHENTRUST);
}

void _stdcall RecallTradeAnsDisAllEntrust(tagTradeErrorInfo *pErrInfo, int nRequestID)
{
	WriteAnsTime(pErrInfo,nRequestID,true,DISALLENTRUST);
}

void _stdcall RecllTradeAnsDisLastEntrust(tagTradeErrorInfo *pErrInfo, int nRequestID)
{
	WriteAnsTime(pErrInfo,nRequestID,true,DISLASTENTRUST);
}

void _stdcall RecallTradeAnsDisEntrust(tagTradeErrorInfo *pErrInfo, int nRequestID)
{
	WriteAnsTime(pErrInfo,nRequestID,true,DISENTRUST);
}

void _stdcall RecallTradeAnsEntrust(tagTradeErrorInfo *pErrInfo, int nRequestID)
{
	WriteAnsTime(pErrInfo,nRequestID,true,ENTRUST);
}

TraderAPI::TraderAPI(void)
{
}

TraderAPI::TraderAPI(std::map<int,int>* mapReq2ReqTM,std::map<int,int>* mapReq2AnsTM)
{

}

TraderAPI::~TraderAPI(void)
{
	Trade_Release();
}

bool TraderAPI::InitTrade(const char *cfgfile, const char *logpath, char *errmsg, int len)
{
	//注册回调函数
	m_oFuncList.Func_TradeAnsLogin = RecallTradeAnsLogin;
	m_oFuncList.Func_TradeAnsQuAmount = RecallTradeAnsQuAmount;
	m_oFuncList.Func_TradeAnsQuBargain = RecallTradeAnsQuBargain;
	m_oFuncList.Func_TradeAnsQuCollectBargain = RecallTradeAnsQuCollectBargain;
	m_oFuncList.Func_TradeAnsQuLog = RecallTradeAnsQuLog;
	m_oFuncList.Func_TradeAnsQuMoney = RecallTradeAnsQuMoney;
	m_oFuncList.Func_TradeAnsQuPosition = RecallTradeAnsQuPosition;
	m_oFuncList.Func_TradeAnsQuRelated = RecallTradeAnsQuRelated;
	m_oFuncList.Func_TradeAnsQuUnFinishEntrust = RecallTradeAnsQuUnFinishEntrust;
	m_oFuncList.Func_TradeAnsDisAllEntrust = RecallTradeAnsDisAllEntrust;
	m_oFuncList.Func_TradeAnsDisEntrust = RecallTradeAnsDisEntrust;
	m_oFuncList.Func_TradeAnsDisLastEntrust = RecllTradeAnsDisLastEntrust;
	m_oFuncList.Func_TradeAnsEntrust = RecallTradeAnsEntrust;
	

	if (Trade_Init(cfgfile,&m_oFuncList,logpath,errmsg,len) < 0)
	{
		return false;
	}
	return true;
}

