#ifndef DATA_CLIENT
#define DATA_CLIENT

#include <vector>
#include <map>
#include <cstring>
struct FuncInfo
{
	FuncInfo()
	{
		iFuncID = 0;
		iTimeSpan = 1;
		iTimes = 0;
		iTotalTimes =0;
		sParaNameArry.RemoveAll();
	}
	FuncInfo(const FuncInfo &rf )
	{
		if ( this == &rf )
		{
			return;
		}

		iFuncID = rf.iFuncID;
		iTimeSpan = rf.iTimeSpan;
		iTimes = rf.iTimes;
		iTotalTimes = rf.iTotalTimes;
		sParaNameArry.RemoveAll();
		sParaValuesArry.RemoveAll();
		for (int i=0; i< rf.sParaNameArry.GetSize();i++)
		{
			sParaNameArry.Add(rf.sParaNameArry[i]);
		}
		for (int i=0; i< rf.sParaValuesArry.GetSize();i++)
		{
			sParaValuesArry.Add(rf.sParaValuesArry[i]);
		}
	}
	FuncInfo &operator=( const FuncInfo &rf )
	{
		if ( this == &rf )
		{
			return *this;
		}

		iFuncID = rf.iFuncID;
		iTimeSpan = rf.iTimeSpan;
		iTimes = rf.iTimes;
		iTotalTimes = rf.iTotalTimes;
		sParaNameArry.RemoveAll();
		sParaValuesArry.RemoveAll();
		for (int i=0; i< rf.sParaNameArry.GetSize();i++)
		{
			sParaNameArry.Add(rf.sParaNameArry[i]);
		}
		for (int i=0; i< rf.sParaValuesArry.GetSize();i++)
		{
			sParaValuesArry.Add(rf.sParaValuesArry[i]);
		}
		return *this;
	}

	int iFuncID;
	int iTimeSpan;
	int iTimes;   //iTimeSpan时间内几次
	int iTotalTimes;  //总的执行次数
	int iReqTimes;    //已发请求数
	int iAnsTimes;    //已收请求数
	int iErrTimes;    //失败请求数，即未收到正常应答请求数
	int iMaxReqTime;
	int iAvgReqTime;
	CStringArray sParaNameArry;
	CStringArray sParaValuesArry;
};


struct FuncReqInfo
{
	int iFuncID;   //功能号
	int iTimeSpan; 
	int iTimes;   //iTimeSpan时间内几次
	int iTotalTimes;  //总的执行次数
	int iReqTimes;    //已发请求数
	std::map<int,double> mapReqNO2ReqTm;
	CStringArray sParaNameArry;
	CStringArray sParaValuesArry;
};

struct FuncAnsInfo
{
	int iFuncID;   //功能号
	int iAnsTimes;    //已收请求数
	int iErrTimes;    //失败请求数，即未收到正常应答请求数
	std::map<int,double> mapReqNO2AndTm;
};

struct ClientInfo
{
	CString sUser;
	CString sPwd;
	int iUserID;
	bool isLogon;
	bool isConnected;
};

enum FUNC_ID
{
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
#endif