#pragma once

#include <map>
#include "include/TradeDll_Interface.h"

//typedef void (*pFunc_AnsProcess)(int , int , int , char* );
class TraderAPI
{
public:
	TraderAPI(void);
	TraderAPI(std::map<int,int>* mapReq2ReqTM,std::map<int,int>* mapReq2AnsTM);

	//初始化交易模块
	bool InitTrade(const char *cfgfile, const char *logpath, char *errmsg, int len);

public:
	~TraderAPI(void);

public:
	tagTradeCallBackFunc m_oFuncList;
};

