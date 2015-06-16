
/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* 文件名称：TradeDll_Interface.h
* 文件摘要：交易对接接口
*
*
* Dll修改： 2014-09-21 saimen修改 V1.0.0.1
*	
*/
#pragma once

#include <windows.h>

#if defined(WIN32)
	#ifdef LIB_TRADEDLL_API_EXPORT
		#define TRADEDLL_API_EXPORT extern "C" __declspec(dllexport)
	#else
		#define TRADEDLL_API_EXPORT extern "C" __declspec(dllimport)
	#endif
#else
	#define TRADEDLL_API_EXPORT extern "C" 
#endif

//---------------------------------------相关定义---------------------------------------
/*交易所代码【ExchangeID】定义：
1、上海期货交易所：XSGE
2、大连期货交易所：XDCE
3、郑州期货交易所：XZCE
4、中金所：CCFX
5、上交所：XSHG
6、深交所：XSHE

证券类别【ProductType】定义（股票1-99，期货100-199）：
1、指数
2、A股
3、B股
4、LOF
5、ETF
6、封闭基金
7、国债回购
8、债券
9、权证
10、行权
11、创业板
12、网络投票
100、期货
101、商品期权
102、股指期权
103、ETF期权
104、股票期权

品种定义【ProductID】：目前期权和期权有，直接取服务器的分类，按以下规则
1、如果品种只有期权则直接用品种信息，比如股指期权IO、或者商品期货IF等
2、如果品种有期货也有期权，比如铜，期货品种就是cu,期权品种就是cu_o，_o就表示期权

委托状态【EntrustStatus】：
'0' 未报
'1' 正报
'2' 已报
'3' 待撤
'4' 部成待撤
'5' 部撤
'6' 已撤
'7' 部成
'8' 已成
'9' 废单*/

//-------------------------------------数据结构定义-------------------------------------
struct tagTradeCallBackStatus				// 服务器状态变化
{
	int		status;							// 当前状态，0-异常 1-正常
	char	statusname[64];					// 状态描述信息
};

struct tagTradeErrorInfo					// 常用错误信息
{
	int		ErrorID;						// 错误代码，>=0成功 <0失败
	char	ErrorMsg[512];					// 错误信息
};

struct tagTradeReqLogin						// 登录请求
{
	char	User[20];						// 交易员名称（必须输入）
	char	Password[20];					// 密码（必须输入）
};
struct tagTradeAnsLogin						// 登录应答
{
	int		UserID;							// 交易员编号
	int		UserType;						// 交易员类型，0-管理员 1-交易员
};

struct tagTradeReqQuRelated					// 交易员关联柜台资金账号查询请求【管理员可以查询所有交易员的，而交易员只能查询自己的】
{
	int		UserID;							// 交易员编号（必须输入，如果是管理员则填写0）
};
struct tagTradeAnsQuRelated					// 交易员关联柜台资金账号查询应答
{
	int		UserID;							// 交易员编号
	char	User[20];						// 交易员名称
	double	StopMoney;						// 停机位
	int		CounterID;						// 柜台编号
	int		AccID;							// 柜台资金账号编号
	char	AccountName[20];				// 柜台资金账号别名
	int		LockFlag;						// 锁券标志：0-非锁券【两融柜台使用】 1-锁券【两融柜台使用】 2-普通交易柜台
	double	CommissionRate;					// 佣金费率
	double	UsableMoney;					// 交易员在这个柜台资金账号上可用的资金剩余数
};

struct tagTradeReqQuMoney					// 交易员对应柜台资金账号资金查询请求
{
	int		UserID;							// 交易员编号（必须输入）
	int		AccID;							// 柜台资金账号编号（必须输入）
};
struct tagTradeAnsQuMoney					// 交易员对应柜台资金账号资金查询应答
{
	double	UsableMoney;					// 交易员在这个柜台资金账号上可用的资金剩余数
};

struct tagTradeReqQuAmount					// 融券可用股数查询请求
{
	int		UserID;							// 交易员编号（必须输入）
	int		AccID;							// 柜台资金账号编号，送空的话则查询该交易员关联的所有柜台资金账号
	char	InstrumentID[32];				// 合约代码，送空的话则查询该交易员对应柜台资金账号的所有券可用数
	char	ExchangeID[12];					// 交易所代码
};
struct tagTradeAnsQuAmount					// 融券可用股数查询应答
{
	int		AccID;							// 柜台资金账号编号
	char	AccountName[20];				// 柜台资金账号别名
	char	InstrumentID[32];				// 合约代码
	char	InstrumentName[24];				// 合约名称
	char	ExchangeID[12];					// 交易所代码
	int		Amount;							// 可用股数
};

struct tagTradeReqQuUnFinishEntrust			// 未完成委托查询请求【报表显示】
{
	int		UserID;							// 交易员编号（必须输入）
	int		LastUpdateTime;					// 上次最后更新时间，HHMMSSxxx格式（必须输入，这里只更新有变动的委托信息以便委托数据多时减少交互流量）
};
struct tagTradeAnsQuUnFinishEntrust			// 未完成委托查询应答
{
	int		UserID;							// 交易员编号
	char	User[20];						// 交易员名称
	int		CounterID;						// 柜台编号
	int		AccID;							// 柜台资金账号编号
	char	AccountName[20];				// 柜台资金账号别名
	char	InstrumentID[32];				// 合约代码
	char	InstrumentName[24];				// 合约名称
	char	ExchangeID[12];					// 交易所代码
	int		EntrustDate;					// 委托日期，YYYYMMDD格式
	int		EntrustTime;					// 委托时间，HHMMSS格式
	char	EntrustType;					// 委托类别，'1'-现金买入 '2'-融券卖出 '3'-涨停融券卖出
	char	EntrustStatus;					// 委托状态，查看顶部委托状态定义
	char	EntrustNo[32];					// 委托编号
	double	EntrustPrice;					// 委托价格
	int		EntrustAmount;					// 委托数量
	int		BargAmount;						// 成交数量
	int		LastUpdateTime;					// 上次最后更新时间，HHMMSSxxx格式
};

struct tagTradeReqQuLog						// 交易日志查询请求【报表显示】
{
	int		UserID;							// 交易员编号（必须输入）
	int		Index;							// 日志序号（必须输入，这里增加序号采用增量查询模式，防止一次性数据过大的情况）
};
struct tagTradeAnsQuLog						// 交易日志查询应答
{
	int		UserID;							// 交易员编号
	char	User[20];						// 交易员名称
	int		AccID;							// 柜台资金账号编号
	char	AccountName[20];				// 柜台资金账号别名
	char	InstrumentID[32];				// 合约代码
	char	InstrumentName[24];				// 合约名称
	int		EntrustDate;					// 委托日期，YYYYMMDD格式
	int		EntrustTime;					// 委托时间，HHMMSS格式
	char	EntrustStatus;					// 委托状态，查看顶部委托状态定义
	char	EntrustNo[32];					// 委托编号
	char	LogDetail[256];					// 日志信息描述
	int		Index;							// 日志序号
};

struct tagTradeReqQuPosition				// 当前仓位查询请求【报表显示】
{
	int		UserID;							// 交易员编号（必须输入）
	int		AccID;							// 柜台资金账号编号，送空的话则查询该交易员关联的所有柜台资金账号
};
struct tagTradeAnsQuPosition				// 当前仓位查询应答（注：第一条应答记录返回前面3个字段统计信息，具体的仓位数据从第二条记录开始）
{
	double	TotalMarketValue;				// 总市值（第一条记录返回）
	double	FloatProfitorLoss;				// 总浮动盈亏（第一条记录返回）
	double	AchieveFpl;						// 总实现盈亏（第一条记录返回）
	char	User[20];						// 交易员名称
	char	AccountName[20];				// 柜台资金账号别名
	char	InstrumentID[32];				// 合约代码
	char	InstrumentName[24];				// 合约名称
	char	ExchangeID[12];					// 交易所代码
	int		Amount;							// 当前数量
	int		Side;							// 1-多仓 -1-空仓
	double	CurrentFloatProfit;				// 当前浮盈
	double	AveragePrice;					// 平均价格
};

struct tagTradeReqQuBargain					// 个人成交记录明细查询请求【报表显示】
{
	int		UserID;							// 交易员编号（必须输入）
	int		AccID;							// 柜台资金账号编号，送空的话则查询该交易员关联的所有柜台资金账号
};
struct tagTradeAnsQuBargain					// 个人成交记录明细查询应答
{
	int		UserID;							// 交易员编号
	char	User[20];						// 交易员名称
	int		AccID;							// 柜台资金账号编号
	char	AccountName[20];				// 柜台资金账号别名
	char	InstrumentID[32];				// 合约代码
	char	InstrumentName[24];				// 合约名称
	char	EntrustType;					// 委托类别，'1'-现金买入 '2'-融券卖出 '3'-涨停融券卖出
	int		EntrustTime;					// 委托时间，HHMMSS格式
	char	EntrustNo[32];					// 委托编号
	int		BargAmount;						// 成交数量
	double	BargPrice;						// 成交价格
	double	EntrustFee;						// 交易费用
};

struct tagTradeReqQuCollectBargain			// 个人成交记录统计查询请求【报表显示】
{
	int		UserID;							// 交易员编号（必须输入）
	int		BeginDate;						// 开始日期，YYYYMMDD格式（必须输入）
	int		EndDate;						// 结束日期，YYYYMMDD格式（必须输入）
};
struct tagTradeAnsQuCollectBargain			// 个人成交记录统计查询应答（注：第一条应答记录返回前面2个字段统计信息，具体的成交记录统计数据从第二条记录开始）
{
	double	AchieveFpl;						// 总实现盈亏（第一条记录返回）
	double	TotalTradeMoney;				// 总交易额（第一条记录返回）
	int		UserID;							// 交易员编号
	char	AccountName[20];				// 柜台资金账号别名
	char	InstrumentID[32];				// 合约代码
	char	InstrumentName[24];				// 合约名称
	int		BargAmount;						// 成交数量
	int		EntrustCount;					// 交易笔数
	double	BargMoney;						// 交易额
	double	Profits;						// 已实现利润
	double	EntrustFee;						// 交易费用
};

struct tagTradeReqEntrust					// 委托下单请求
{
	int		UserID;							// 交易员编号（必须输入）
	int		AccID;							// 柜台资金账号编号（必须输入）
	char	InstrumentID[32];				// 合约代码（必须输入）
	char	InstrumentName[24];				// 合约名称（必须输入）
	char	ExchangeID[12];					// 交易所代码（必须输入）
	char	EntrustType;					// 委托类别，'1'-现金买入 '2'-融券卖出 '3'-涨停融券卖出（必须输入）
	double	EntrustPrice;					// 委托价格（必须输入）
	int		EntrustAmount;					// 委托数量（必须输入）
};

struct tagTradeReqDisEntrust				// 委托撤单请求
{
	int		UserID;							// 交易员编号（必须输入）
	int		AccID;							// 柜台资金账号编号（必须输入）
	int		EntrustDate;					// 委托日期，YYYYMMDD格式（必须输入）
	char	EntrustNo[32];					// 委托编号（必须输入）
	char	ExchangeID[12];					// 交易所代码（必须输入）
};

struct tagTradeReqDisLastEntrust			// 撤当前柜台资金账号的当前代码的最近的一笔委托请求
{
	int		UserID;							// 交易员编号（必须输入）
	int		AccID;							// 柜台资金账号编号（必须输入）
	char	InstrumentID[32];				// 合约代码（必须输入）
	char	ExchangeID[12];					// 交易所代码（必须输入）
};

struct tagTradeReqDisAllEntrust				// 撤当前柜台资金账号的当前代码的所有委托请求
{
	int		UserID;							// 交易员编号（必须输入）
	int		AccID;							// 柜台资金账号编号（必须输入）
	char	InstrumentID[32];				// 合约代码（必须输入）
	char	ExchangeID[12];					// 交易所代码（必须输入）
};

//-------------------------------------回调函数定义-------------------------------------
/* 函数格式固定：
   参数一：标准应答结构数据
   参数二：应答数据个数
   参数三：错误信息，首先通过该信息判断业务是否成功，如成功则取参数一应答信息
   参数四：请求编号，外围通过这个编号对应请求和应答信息
   参数五：是否最后应答数据，这里约定如果一个请求对应多结果集，这里分多次回报返回，最后一个回本标识为true

   特别说明：
   1、连接状态变动回调只要判断应答结构中信息
   2、登入每次都是指定返回某个特定账户登入是否成功
   3、回报中的error结构中ErrorID>=0并且回报中count=0表示没有结果纪录
   4、委托下单或者撤单等单结果返回的不提供bLast标志
*/

// 连接状态变动回调
typedef void (_stdcall * pFunc_TradeCallBackStatus)(tagTradeCallBackStatus *pCallBackStatus);

// 账户登录应答回调
typedef void (_stdcall * pFunc_TradeAnsLogin)(tagTradeAnsLogin *pAnsLogin, tagTradeErrorInfo *pErrInfo);

// 交易员关联柜台资金账号查询应答回调
typedef void (_stdcall * pFunc_TradeAnsQuRelated)(tagTradeAnsQuRelated *pAnsQuRelated, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// 交易员对应柜台资金账号资金查询应答回调
typedef void (_stdcall * pFunc_TradeAnsQuMoney)(tagTradeAnsQuMoney *pAnsQuMoney, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// 融券可用股数查询应答回调
typedef void (_stdcall * pFunc_TradeAnsQuAmount)(tagTradeAnsQuAmount *pAnsQuAmount, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// 未完成委托查询应答回调
typedef void (_stdcall * pFunc_TradeAnsQuUnFinishEntrust)(tagTradeAnsQuUnFinishEntrust *pAnsQuUnFinishEntrust, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// 交易日志查询应答回调
typedef void (_stdcall * pFunc_TradeAnsQuLog)(tagTradeAnsQuLog *pAnsQuLog, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// 当前仓位查询应答回调
typedef void (_stdcall * pFunc_TradeAnsQuPosition)(tagTradeAnsQuPosition *pAnsQuPosition, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// 个人成交记录明细查询应答回调
typedef void (_stdcall * pFunc_TradeAnsQuBargain)(tagTradeAnsQuBargain *pAnsQuBargain, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// 个人成交记录统计查询应答回调
typedef void (_stdcall * pFunc_TradeAnsQuCollectBargain)(tagTradeAnsQuCollectBargain *pAnsQuCollectBargain, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// 委托下单应答回调
typedef void (_stdcall * pFunc_TradeAnsEntrust)(tagTradeErrorInfo *pErrInfo, int nRequestID);

// 委托撤单应答回调
typedef void (_stdcall * pFunc_TradeAnsDisEntrust)(tagTradeErrorInfo *pErrInfo, int nRequestID);

// 撤当前柜台资金账号的当前代码的最近的一笔委托应答回调
typedef void (_stdcall * pFunc_TradeAnsDisLastEntrust)(tagTradeErrorInfo *pErrInfo, int nRequestID);

// 撤当前柜台资金账号的当前代码的所有委托应答回调
typedef void (_stdcall * pFunc_TradeAnsDisAllEntrust)(tagTradeErrorInfo *pErrInfo, int nRequestID);

// 回调结构：列举所有回调函数指针
struct tagTradeCallBackFunc
{
	pFunc_TradeCallBackStatus			Func_TradeCallBackStatus;
	pFunc_TradeAnsLogin					Func_TradeAnsLogin;
	pFunc_TradeAnsQuRelated				Func_TradeAnsQuRelated;
	pFunc_TradeAnsQuMoney				Func_TradeAnsQuMoney;
	pFunc_TradeAnsQuAmount				Func_TradeAnsQuAmount;
	pFunc_TradeAnsQuUnFinishEntrust		Func_TradeAnsQuUnFinishEntrust;
	pFunc_TradeAnsQuLog					Func_TradeAnsQuLog;
	pFunc_TradeAnsQuPosition			Func_TradeAnsQuPosition;
	pFunc_TradeAnsQuBargain				Func_TradeAnsQuBargain;
	pFunc_TradeAnsQuCollectBargain		Func_TradeAnsQuCollectBargain;
	pFunc_TradeAnsEntrust				Func_TradeAnsEntrust;
	pFunc_TradeAnsDisEntrust			Func_TradeAnsDisEntrust;
	pFunc_TradeAnsDisLastEntrust		Func_TradeAnsDisLastEntrust;
	pFunc_TradeAnsDisAllEntrust			Func_TradeAnsDisAllEntrust;
};

//-------------------------------------导出函数定义-------------------------------------
// 获取版本：“V1.0.0.1”格式
TRADEDLL_API_EXPORT const char * Trade_GetVer(void);

// DLL初始化
// cfgfile			配置文件
// callbackflist	回调函数指针列表
// logpath			日志路径，设置为空的话则默认为log目录
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0成功 <0失败
TRADEDLL_API_EXPORT int Trade_Init(const char *cfgfile, tagTradeCallBackFunc * callbackflist, const char *logpath, char *errmsg, int len);

// DLL关闭
TRADEDLL_API_EXPORT void Trade_Release(void);

// 交易员登录
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0请求编号 <0失败
TRADEDLL_API_EXPORT int Trade_Login(tagTradeReqLogin *pReqLogin, char *errmsg, int len);

// 交易员关联柜台资金账号查询
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0请求编号（与应答回调中的编号做对应） <0失败
TRADEDLL_API_EXPORT int Trade_QuRelated(tagTradeReqQuRelated *pReqQuRelated, char *errmsg, int len);

// 交易员对应柜台资金账号资金查询
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0请求编号（与应答回调中的编号做对应） <0失败
TRADEDLL_API_EXPORT int Trade_QuMoney(tagTradeReqQuMoney *pReqQuMoney, char *errmsg, int len);

// 融券可用股数查询
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0请求编号（与应答回调中的编号做对应） <0失败
TRADEDLL_API_EXPORT int Trade_QuAmount(tagTradeReqQuAmount *pReqQuAmount, char *errmsg, int len);

// 未完成委托查询【报表显示】
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0请求编号（与应答回调中的编号做对应） <0失败
TRADEDLL_API_EXPORT int Trade_QuUnFinishEntrust(tagTradeReqQuUnFinishEntrust *pReqQuUnFinishEntrust, char *errmsg, int len);

// 交易日志查询【报表显示】
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0请求编号（与应答回调中的编号做对应） <0失败
TRADEDLL_API_EXPORT int Trade_QuLog(tagTradeReqQuLog *pReqQuLog, char *errmsg, int len);

// 当前仓位查询【报表显示】
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0请求编号（与应答回调中的编号做对应） <0失败
TRADEDLL_API_EXPORT int Trade_QuPosition(tagTradeReqQuPosition *pReqQuPosition, char *errmsg, int len);

// 个人成交记录明细查询【报表显示】
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0请求编号（与应答回调中的编号做对应） <0失败
TRADEDLL_API_EXPORT int Trade_QuBargain(tagTradeReqQuBargain *pReqQuBargain, char *errmsg, int len);

// 个人成交记录统计查询【报表显示】
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0请求编号（与应答回调中的编号做对应） <0失败
TRADEDLL_API_EXPORT int Trade_QuCollectBargain(tagTradeReqQuCollectBargain *pReqQuCollectBargain, char *errmsg, int len);

// 委托下单
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0请求编号（与应答回调中的编号做对应） <0失败
TRADEDLL_API_EXPORT int Trade_Entrust(tagTradeReqEntrust *pReqEntrust, char *errmsg, int len);

// 委托撤单
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0请求编号（与应答回调中的编号做对应） <0失败
TRADEDLL_API_EXPORT int Trade_DisEntrust(tagTradeReqDisEntrust *pReqDisEntrust, char *errmsg, int len);

// 撤当前柜台资金账号的当前代码的最近的一笔委托
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0请求编号（与应答回调中的编号做对应） <0失败
TRADEDLL_API_EXPORT int Trade_DisLastEntrust(tagTradeReqDisLastEntrust *pReqDisLastEntrust, char *errmsg, int len);

// 撤当前柜台资金账号的当前代码的所有委托
// errmsg			错误信息缓存
// len				缓存长度
// 返回值：			>=0请求编号（与应答回调中的编号做对应） <0失败
TRADEDLL_API_EXPORT int Trade_DisAllEntrust(tagTradeReqDisAllEntrust *pReqDisAllEntrust, char *errmsg, int len);