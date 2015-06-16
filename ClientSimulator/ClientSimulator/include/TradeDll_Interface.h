
/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�TradeDll_Interface.h
* �ļ�ժҪ�����׶Խӽӿ�
*
*
* Dll�޸ģ� 2014-09-21 saimen�޸� V1.0.0.1
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

//---------------------------------------��ض���---------------------------------------
/*���������롾ExchangeID�����壺
1���Ϻ��ڻ���������XSGE
2�������ڻ���������XDCE
3��֣���ڻ���������XZCE
4���н�����CCFX
5���Ͻ�����XSHG
6�������XSHE

֤ȯ���ProductType�����壨��Ʊ1-99���ڻ�100-199����
1��ָ��
2��A��
3��B��
4��LOF
5��ETF
6����ջ���
7����ծ�ع�
8��ծȯ
9��Ȩ֤
10����Ȩ
11����ҵ��
12������ͶƱ
100���ڻ�
101����Ʒ��Ȩ
102����ָ��Ȩ
103��ETF��Ȩ
104����Ʊ��Ȩ

Ʒ�ֶ��塾ProductID����Ŀǰ��Ȩ����Ȩ�У�ֱ��ȡ�������ķ��࣬�����¹���
1�����Ʒ��ֻ����Ȩ��ֱ����Ʒ����Ϣ�������ָ��ȨIO��������Ʒ�ڻ�IF��
2�����Ʒ�����ڻ�Ҳ����Ȩ������ͭ���ڻ�Ʒ�־���cu,��ȨƷ�־���cu_o��_o�ͱ�ʾ��Ȩ

ί��״̬��EntrustStatus����
'0' δ��
'1' ����
'2' �ѱ�
'3' ����
'4' ���ɴ���
'5' ����
'6' �ѳ�
'7' ����
'8' �ѳ�
'9' �ϵ�*/

//-------------------------------------���ݽṹ����-------------------------------------
struct tagTradeCallBackStatus				// ������״̬�仯
{
	int		status;							// ��ǰ״̬��0-�쳣 1-����
	char	statusname[64];					// ״̬������Ϣ
};

struct tagTradeErrorInfo					// ���ô�����Ϣ
{
	int		ErrorID;						// ������룬>=0�ɹ� <0ʧ��
	char	ErrorMsg[512];					// ������Ϣ
};

struct tagTradeReqLogin						// ��¼����
{
	char	User[20];						// ����Ա���ƣ��������룩
	char	Password[20];					// ���루�������룩
};
struct tagTradeAnsLogin						// ��¼Ӧ��
{
	int		UserID;							// ����Ա���
	int		UserType;						// ����Ա���ͣ�0-����Ա 1-����Ա
};

struct tagTradeReqQuRelated					// ����Ա������̨�ʽ��˺Ų�ѯ���󡾹���Ա���Բ�ѯ���н���Ա�ģ�������Աֻ�ܲ�ѯ�Լ��ġ�
{
	int		UserID;							// ����Ա��ţ��������룬����ǹ���Ա����д0��
};
struct tagTradeAnsQuRelated					// ����Ա������̨�ʽ��˺Ų�ѯӦ��
{
	int		UserID;							// ����Ա���
	char	User[20];						// ����Ա����
	double	StopMoney;						// ͣ��λ
	int		CounterID;						// ��̨���
	int		AccID;							// ��̨�ʽ��˺ű��
	char	AccountName[20];				// ��̨�ʽ��˺ű���
	int		LockFlag;						// ��ȯ��־��0-����ȯ�����ڹ�̨ʹ�á� 1-��ȯ�����ڹ�̨ʹ�á� 2-��ͨ���׹�̨
	double	CommissionRate;					// Ӷ�����
	double	UsableMoney;					// ����Ա�������̨�ʽ��˺��Ͽ��õ��ʽ�ʣ����
};

struct tagTradeReqQuMoney					// ����Ա��Ӧ��̨�ʽ��˺��ʽ��ѯ����
{
	int		UserID;							// ����Ա��ţ��������룩
	int		AccID;							// ��̨�ʽ��˺ű�ţ��������룩
};
struct tagTradeAnsQuMoney					// ����Ա��Ӧ��̨�ʽ��˺��ʽ��ѯӦ��
{
	double	UsableMoney;					// ����Ա�������̨�ʽ��˺��Ͽ��õ��ʽ�ʣ����
};

struct tagTradeReqQuAmount					// ��ȯ���ù�����ѯ����
{
	int		UserID;							// ����Ա��ţ��������룩
	int		AccID;							// ��̨�ʽ��˺ű�ţ��ͿյĻ����ѯ�ý���Ա���������й�̨�ʽ��˺�
	char	InstrumentID[32];				// ��Լ���룬�ͿյĻ����ѯ�ý���Ա��Ӧ��̨�ʽ��˺ŵ�����ȯ������
	char	ExchangeID[12];					// ����������
};
struct tagTradeAnsQuAmount					// ��ȯ���ù�����ѯӦ��
{
	int		AccID;							// ��̨�ʽ��˺ű��
	char	AccountName[20];				// ��̨�ʽ��˺ű���
	char	InstrumentID[32];				// ��Լ����
	char	InstrumentName[24];				// ��Լ����
	char	ExchangeID[12];					// ����������
	int		Amount;							// ���ù���
};

struct tagTradeReqQuUnFinishEntrust			// δ���ί�в�ѯ���󡾱�����ʾ��
{
	int		UserID;							// ����Ա��ţ��������룩
	int		LastUpdateTime;					// �ϴ�������ʱ�䣬HHMMSSxxx��ʽ���������룬����ֻ�����б䶯��ί����Ϣ�Ա�ί�����ݶ�ʱ���ٽ���������
};
struct tagTradeAnsQuUnFinishEntrust			// δ���ί�в�ѯӦ��
{
	int		UserID;							// ����Ա���
	char	User[20];						// ����Ա����
	int		CounterID;						// ��̨���
	int		AccID;							// ��̨�ʽ��˺ű��
	char	AccountName[20];				// ��̨�ʽ��˺ű���
	char	InstrumentID[32];				// ��Լ����
	char	InstrumentName[24];				// ��Լ����
	char	ExchangeID[12];					// ����������
	int		EntrustDate;					// ί�����ڣ�YYYYMMDD��ʽ
	int		EntrustTime;					// ί��ʱ�䣬HHMMSS��ʽ
	char	EntrustType;					// ί�����'1'-�ֽ����� '2'-��ȯ���� '3'-��ͣ��ȯ����
	char	EntrustStatus;					// ί��״̬���鿴����ί��״̬����
	char	EntrustNo[32];					// ί�б��
	double	EntrustPrice;					// ί�м۸�
	int		EntrustAmount;					// ί������
	int		BargAmount;						// �ɽ�����
	int		LastUpdateTime;					// �ϴ�������ʱ�䣬HHMMSSxxx��ʽ
};

struct tagTradeReqQuLog						// ������־��ѯ���󡾱�����ʾ��
{
	int		UserID;							// ����Ա��ţ��������룩
	int		Index;							// ��־��ţ��������룬����������Ų���������ѯģʽ����ֹһ�������ݹ���������
};
struct tagTradeAnsQuLog						// ������־��ѯӦ��
{
	int		UserID;							// ����Ա���
	char	User[20];						// ����Ա����
	int		AccID;							// ��̨�ʽ��˺ű��
	char	AccountName[20];				// ��̨�ʽ��˺ű���
	char	InstrumentID[32];				// ��Լ����
	char	InstrumentName[24];				// ��Լ����
	int		EntrustDate;					// ί�����ڣ�YYYYMMDD��ʽ
	int		EntrustTime;					// ί��ʱ�䣬HHMMSS��ʽ
	char	EntrustStatus;					// ί��״̬���鿴����ί��״̬����
	char	EntrustNo[32];					// ί�б��
	char	LogDetail[256];					// ��־��Ϣ����
	int		Index;							// ��־���
};

struct tagTradeReqQuPosition				// ��ǰ��λ��ѯ���󡾱�����ʾ��
{
	int		UserID;							// ����Ա��ţ��������룩
	int		AccID;							// ��̨�ʽ��˺ű�ţ��ͿյĻ����ѯ�ý���Ա���������й�̨�ʽ��˺�
};
struct tagTradeAnsQuPosition				// ��ǰ��λ��ѯӦ��ע����һ��Ӧ���¼����ǰ��3���ֶ�ͳ����Ϣ������Ĳ�λ���ݴӵڶ�����¼��ʼ��
{
	double	TotalMarketValue;				// ����ֵ����һ����¼���أ�
	double	FloatProfitorLoss;				// �ܸ���ӯ������һ����¼���أ�
	double	AchieveFpl;						// ��ʵ��ӯ������һ����¼���أ�
	char	User[20];						// ����Ա����
	char	AccountName[20];				// ��̨�ʽ��˺ű���
	char	InstrumentID[32];				// ��Լ����
	char	InstrumentName[24];				// ��Լ����
	char	ExchangeID[12];					// ����������
	int		Amount;							// ��ǰ����
	int		Side;							// 1-��� -1-�ղ�
	double	CurrentFloatProfit;				// ��ǰ��ӯ
	double	AveragePrice;					// ƽ���۸�
};

struct tagTradeReqQuBargain					// ���˳ɽ���¼��ϸ��ѯ���󡾱�����ʾ��
{
	int		UserID;							// ����Ա��ţ��������룩
	int		AccID;							// ��̨�ʽ��˺ű�ţ��ͿյĻ����ѯ�ý���Ա���������й�̨�ʽ��˺�
};
struct tagTradeAnsQuBargain					// ���˳ɽ���¼��ϸ��ѯӦ��
{
	int		UserID;							// ����Ա���
	char	User[20];						// ����Ա����
	int		AccID;							// ��̨�ʽ��˺ű��
	char	AccountName[20];				// ��̨�ʽ��˺ű���
	char	InstrumentID[32];				// ��Լ����
	char	InstrumentName[24];				// ��Լ����
	char	EntrustType;					// ί�����'1'-�ֽ����� '2'-��ȯ���� '3'-��ͣ��ȯ����
	int		EntrustTime;					// ί��ʱ�䣬HHMMSS��ʽ
	char	EntrustNo[32];					// ί�б��
	int		BargAmount;						// �ɽ�����
	double	BargPrice;						// �ɽ��۸�
	double	EntrustFee;						// ���׷���
};

struct tagTradeReqQuCollectBargain			// ���˳ɽ���¼ͳ�Ʋ�ѯ���󡾱�����ʾ��
{
	int		UserID;							// ����Ա��ţ��������룩
	int		BeginDate;						// ��ʼ���ڣ�YYYYMMDD��ʽ���������룩
	int		EndDate;						// �������ڣ�YYYYMMDD��ʽ���������룩
};
struct tagTradeAnsQuCollectBargain			// ���˳ɽ���¼ͳ�Ʋ�ѯӦ��ע����һ��Ӧ���¼����ǰ��2���ֶ�ͳ����Ϣ������ĳɽ���¼ͳ�����ݴӵڶ�����¼��ʼ��
{
	double	AchieveFpl;						// ��ʵ��ӯ������һ����¼���أ�
	double	TotalTradeMoney;				// �ܽ��׶��һ����¼���أ�
	int		UserID;							// ����Ա���
	char	AccountName[20];				// ��̨�ʽ��˺ű���
	char	InstrumentID[32];				// ��Լ����
	char	InstrumentName[24];				// ��Լ����
	int		BargAmount;						// �ɽ�����
	int		EntrustCount;					// ���ױ���
	double	BargMoney;						// ���׶�
	double	Profits;						// ��ʵ������
	double	EntrustFee;						// ���׷���
};

struct tagTradeReqEntrust					// ί���µ�����
{
	int		UserID;							// ����Ա��ţ��������룩
	int		AccID;							// ��̨�ʽ��˺ű�ţ��������룩
	char	InstrumentID[32];				// ��Լ���루�������룩
	char	InstrumentName[24];				// ��Լ���ƣ��������룩
	char	ExchangeID[12];					// ���������루�������룩
	char	EntrustType;					// ί�����'1'-�ֽ����� '2'-��ȯ���� '3'-��ͣ��ȯ�������������룩
	double	EntrustPrice;					// ί�м۸񣨱������룩
	int		EntrustAmount;					// ί���������������룩
};

struct tagTradeReqDisEntrust				// ί�г�������
{
	int		UserID;							// ����Ա��ţ��������룩
	int		AccID;							// ��̨�ʽ��˺ű�ţ��������룩
	int		EntrustDate;					// ί�����ڣ�YYYYMMDD��ʽ���������룩
	char	EntrustNo[32];					// ί�б�ţ��������룩
	char	ExchangeID[12];					// ���������루�������룩
};

struct tagTradeReqDisLastEntrust			// ����ǰ��̨�ʽ��˺ŵĵ�ǰ����������һ��ί������
{
	int		UserID;							// ����Ա��ţ��������룩
	int		AccID;							// ��̨�ʽ��˺ű�ţ��������룩
	char	InstrumentID[32];				// ��Լ���루�������룩
	char	ExchangeID[12];					// ���������루�������룩
};

struct tagTradeReqDisAllEntrust				// ����ǰ��̨�ʽ��˺ŵĵ�ǰ���������ί������
{
	int		UserID;							// ����Ա��ţ��������룩
	int		AccID;							// ��̨�ʽ��˺ű�ţ��������룩
	char	InstrumentID[32];				// ��Լ���루�������룩
	char	ExchangeID[12];					// ���������루�������룩
};

//-------------------------------------�ص���������-------------------------------------
/* ������ʽ�̶���
   ����һ����׼Ӧ��ṹ����
   ��������Ӧ�����ݸ���
   ��������������Ϣ������ͨ������Ϣ�ж�ҵ���Ƿ�ɹ�����ɹ���ȡ����һӦ����Ϣ
   �����ģ������ţ���Χͨ�������Ŷ�Ӧ�����Ӧ����Ϣ
   �����壺�Ƿ����Ӧ�����ݣ�����Լ�����һ�������Ӧ������������ֶ�λر����أ����һ���ر���ʶΪtrue

   �ر�˵����
   1������״̬�䶯�ص�ֻҪ�ж�Ӧ��ṹ����Ϣ
   2������ÿ�ζ���ָ������ĳ���ض��˻������Ƿ�ɹ�
   3���ر��е�error�ṹ��ErrorID>=0���һر���count=0��ʾû�н����¼
   4��ί���µ����߳����ȵ�������صĲ��ṩbLast��־
*/

// ����״̬�䶯�ص�
typedef void (_stdcall * pFunc_TradeCallBackStatus)(tagTradeCallBackStatus *pCallBackStatus);

// �˻���¼Ӧ��ص�
typedef void (_stdcall * pFunc_TradeAnsLogin)(tagTradeAnsLogin *pAnsLogin, tagTradeErrorInfo *pErrInfo);

// ����Ա������̨�ʽ��˺Ų�ѯӦ��ص�
typedef void (_stdcall * pFunc_TradeAnsQuRelated)(tagTradeAnsQuRelated *pAnsQuRelated, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// ����Ա��Ӧ��̨�ʽ��˺��ʽ��ѯӦ��ص�
typedef void (_stdcall * pFunc_TradeAnsQuMoney)(tagTradeAnsQuMoney *pAnsQuMoney, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// ��ȯ���ù�����ѯӦ��ص�
typedef void (_stdcall * pFunc_TradeAnsQuAmount)(tagTradeAnsQuAmount *pAnsQuAmount, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// δ���ί�в�ѯӦ��ص�
typedef void (_stdcall * pFunc_TradeAnsQuUnFinishEntrust)(tagTradeAnsQuUnFinishEntrust *pAnsQuUnFinishEntrust, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// ������־��ѯӦ��ص�
typedef void (_stdcall * pFunc_TradeAnsQuLog)(tagTradeAnsQuLog *pAnsQuLog, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// ��ǰ��λ��ѯӦ��ص�
typedef void (_stdcall * pFunc_TradeAnsQuPosition)(tagTradeAnsQuPosition *pAnsQuPosition, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// ���˳ɽ���¼��ϸ��ѯӦ��ص�
typedef void (_stdcall * pFunc_TradeAnsQuBargain)(tagTradeAnsQuBargain *pAnsQuBargain, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// ���˳ɽ���¼ͳ�Ʋ�ѯӦ��ص�
typedef void (_stdcall * pFunc_TradeAnsQuCollectBargain)(tagTradeAnsQuCollectBargain *pAnsQuCollectBargain, int count, tagTradeErrorInfo *pErrInfo, int nRequestID, bool bLast);

// ί���µ�Ӧ��ص�
typedef void (_stdcall * pFunc_TradeAnsEntrust)(tagTradeErrorInfo *pErrInfo, int nRequestID);

// ί�г���Ӧ��ص�
typedef void (_stdcall * pFunc_TradeAnsDisEntrust)(tagTradeErrorInfo *pErrInfo, int nRequestID);

// ����ǰ��̨�ʽ��˺ŵĵ�ǰ����������һ��ί��Ӧ��ص�
typedef void (_stdcall * pFunc_TradeAnsDisLastEntrust)(tagTradeErrorInfo *pErrInfo, int nRequestID);

// ����ǰ��̨�ʽ��˺ŵĵ�ǰ���������ί��Ӧ��ص�
typedef void (_stdcall * pFunc_TradeAnsDisAllEntrust)(tagTradeErrorInfo *pErrInfo, int nRequestID);

// �ص��ṹ���о����лص�����ָ��
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

//-------------------------------------������������-------------------------------------
// ��ȡ�汾����V1.0.0.1����ʽ
TRADEDLL_API_EXPORT const char * Trade_GetVer(void);

// DLL��ʼ��
// cfgfile			�����ļ�
// callbackflist	�ص�����ָ���б�
// logpath			��־·��������Ϊ�յĻ���Ĭ��ΪlogĿ¼
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0�ɹ� <0ʧ��
TRADEDLL_API_EXPORT int Trade_Init(const char *cfgfile, tagTradeCallBackFunc * callbackflist, const char *logpath, char *errmsg, int len);

// DLL�ر�
TRADEDLL_API_EXPORT void Trade_Release(void);

// ����Ա��¼
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0������ <0ʧ��
TRADEDLL_API_EXPORT int Trade_Login(tagTradeReqLogin *pReqLogin, char *errmsg, int len);

// ����Ա������̨�ʽ��˺Ų�ѯ
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0�����ţ���Ӧ��ص��еı������Ӧ�� <0ʧ��
TRADEDLL_API_EXPORT int Trade_QuRelated(tagTradeReqQuRelated *pReqQuRelated, char *errmsg, int len);

// ����Ա��Ӧ��̨�ʽ��˺��ʽ��ѯ
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0�����ţ���Ӧ��ص��еı������Ӧ�� <0ʧ��
TRADEDLL_API_EXPORT int Trade_QuMoney(tagTradeReqQuMoney *pReqQuMoney, char *errmsg, int len);

// ��ȯ���ù�����ѯ
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0�����ţ���Ӧ��ص��еı������Ӧ�� <0ʧ��
TRADEDLL_API_EXPORT int Trade_QuAmount(tagTradeReqQuAmount *pReqQuAmount, char *errmsg, int len);

// δ���ί�в�ѯ��������ʾ��
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0�����ţ���Ӧ��ص��еı������Ӧ�� <0ʧ��
TRADEDLL_API_EXPORT int Trade_QuUnFinishEntrust(tagTradeReqQuUnFinishEntrust *pReqQuUnFinishEntrust, char *errmsg, int len);

// ������־��ѯ��������ʾ��
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0�����ţ���Ӧ��ص��еı������Ӧ�� <0ʧ��
TRADEDLL_API_EXPORT int Trade_QuLog(tagTradeReqQuLog *pReqQuLog, char *errmsg, int len);

// ��ǰ��λ��ѯ��������ʾ��
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0�����ţ���Ӧ��ص��еı������Ӧ�� <0ʧ��
TRADEDLL_API_EXPORT int Trade_QuPosition(tagTradeReqQuPosition *pReqQuPosition, char *errmsg, int len);

// ���˳ɽ���¼��ϸ��ѯ��������ʾ��
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0�����ţ���Ӧ��ص��еı������Ӧ�� <0ʧ��
TRADEDLL_API_EXPORT int Trade_QuBargain(tagTradeReqQuBargain *pReqQuBargain, char *errmsg, int len);

// ���˳ɽ���¼ͳ�Ʋ�ѯ��������ʾ��
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0�����ţ���Ӧ��ص��еı������Ӧ�� <0ʧ��
TRADEDLL_API_EXPORT int Trade_QuCollectBargain(tagTradeReqQuCollectBargain *pReqQuCollectBargain, char *errmsg, int len);

// ί���µ�
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0�����ţ���Ӧ��ص��еı������Ӧ�� <0ʧ��
TRADEDLL_API_EXPORT int Trade_Entrust(tagTradeReqEntrust *pReqEntrust, char *errmsg, int len);

// ί�г���
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0�����ţ���Ӧ��ص��еı������Ӧ�� <0ʧ��
TRADEDLL_API_EXPORT int Trade_DisEntrust(tagTradeReqDisEntrust *pReqDisEntrust, char *errmsg, int len);

// ����ǰ��̨�ʽ��˺ŵĵ�ǰ����������һ��ί��
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0�����ţ���Ӧ��ص��еı������Ӧ�� <0ʧ��
TRADEDLL_API_EXPORT int Trade_DisLastEntrust(tagTradeReqDisLastEntrust *pReqDisLastEntrust, char *errmsg, int len);

// ����ǰ��̨�ʽ��˺ŵĵ�ǰ���������ί��
// errmsg			������Ϣ����
// len				���泤��
// ����ֵ��			>=0�����ţ���Ӧ��ص��еı������Ӧ�� <0ʧ��
TRADEDLL_API_EXPORT int Trade_DisAllEntrust(tagTradeReqDisAllEntrust *pReqDisAllEntrust, char *errmsg, int len);