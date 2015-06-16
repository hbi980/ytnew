/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�YTConn.h
* �ļ�ժҪ��ͨѶ�Խӽӿ�
*
* Dll�޸ģ� 2015-05-29 saimen�޸� V1.0
*
*/
#pragma once

#include <windows.h>

#if defined(WIN32)
	#ifdef LIB_YTCONN_API_EXPORT
		#define YTCONN_API_EXPORT extern "C" __declspec(dllexport)
	#else
		#define YTCONN_API_EXPORT extern "C" __declspec(dllimport)
	#endif
#else
	#define YTCONN_API_EXPORT extern "C" 
#endif

//---------------------------------------��ض���---------------------------------------
enum ENum_ConnStatus		// ����״̬����
{
	CS_CLOSE		= 0,	// δ���ӻ����ӶϿ�
	CS_CONNING		= 1,	// ������
	CS_CONNED		= 2,	// ������
};

enum ENum_ErrCode			// ͨѶ������
{
	EC_SUCCESS		= 0,	// ��ȷ
	EC_CONNERR		= -1,	// ͨѶ����
	EC_CONNCLOSE	= -2,	// �����쳣��Ͽ�
	EC_DATAILLEGAL	= -3,	// ���ݷǷ�
};

// ������ȫ���ػص�����
// �ص����ص�����retbuf��Ҫ���̴������˻ص����������ݻ��Զ���DLL�ڲ�ɾ��
// pConn		���Ӷ���
// reqno		������
// funcid		���ܺţ��μ�Э���ĵ���
// errcode		�����ţ�ע�⣺����Ĵ����Ž�����ͨѶ��Ĵ���ҵ���Ĵ�����retbuf���Լ�ȥ����
// retbuf		Ӧ�����ݼ����棬��������ػ��棬��Χ�����Զ�ͨ��YTData.h�е����ݽṹ����ת��
// len			Ӧ�����ݼ����泤��
// �ر�ע�⣺�����ͨѶ�����Ļ�������������Ƿ��ص�retbuf��ҵ���Ĵ�����Ҫ��retbuf���ݽ��н�����õ�
typedef void (__stdcall * pFunc_YTConnDataAllRet)(void * pConn, int reqno, unsigned int funcid, int errcode, const char * retbuf, int len);

// ���ݲ��ַ��ػص����壨ע�⣺���ﲻ��ȫ���ؿ����Ƕ���������
// �ص����ص�����retbuf��Ҫ���̴������˻ص����������ݻ��Զ���DLL�ڲ�ɾ��
// pConn		���Ӷ���
// reqno		������
// funcid		���ܺţ��μ�Э���ĵ���
// errcode		�����ţ�ע�⣺����Ĵ����Ž�����ͨѶ��Ĵ���ҵ���Ĵ�����retbuf���Լ�ȥ����
// retbuf		Ӧ�����ݼ����棬��������ػ��棬��Χ�����Զ�ͨ��YTData.h�е����ݽṹ����ת��
// len			Ӧ�����ݼ����泤��
// �ر�ע�⣺�����ͨѶ�����Ļ�������������Ƿ��ص�retbuf��ҵ���Ĵ�����Ҫ��retbuf���ݽ��н�����õ�
typedef void (__stdcall * pFunc_YTConnDataPartRet)(void * pConn, int reqno, unsigned int funcid, int errcode, const char * retbuf, int len);

// ���ݳ�ʱ�ص����壨��������ͺ�ʱ��ò���Ӧ�����ݵ������
// pConn		���Ӷ���
// reqno		������
// funcid		���ܺţ��μ�Э���ĵ���
typedef void (__stdcall * pFunc_YTConnTimeOut)(void * pConn, int reqno, unsigned int funcid);

// ���ӶϿ��ص�����
// pConn		���Ӷ���
typedef void (__stdcall * pFunc_YTConnClose)(void * pConn);

// �ص��ṹ���о����лص�����ָ��
struct tagYTConnCallBackFunc
{
	pFunc_YTConnDataAllRet	Func_YTConnDataAllRet;
	pFunc_YTConnDataPartRet	Func_YTConnDataPartRet;
	pFunc_YTConnTimeOut		Func_YTConnTimeOut;
	pFunc_YTConnClose		Func_YTConnClose;
};

//--------------------------------------DLL��������-------------------------------------
// ��ȡ�汾����V1.0����ʽ
YTCONN_API_EXPORT const char * YTConn_GetVer(void);

// DLL��ʼ��
// processpath		����·���������ļ�����·����λ��
// callbackflist	�ص�����ָ���б�
// logpath			��־·�������ΪNULL�Ļ���Ĭ��ȡ��ǰ·����logĿ¼
// ����ֵ��			>=0�ɹ� <0ʧ��
YTCONN_API_EXPORT int YTConn_Init(const char * processpath, tagYTConnCallBackFunc * callbackflist, const char * logpath = NULL);

// DLL�ر�
YTCONN_API_EXPORT void YTConn_Release(void);

//---------------------------------���������ļ���������---------------------------------
// �������
// 1���þ����־����������Ϣ���������к�������صĺ�����������ж���Ҫ����þ����Ϣ
// 2�����صľ����Χ���õĻ�����������DelConfig�����ͷţ���Χ��Ҫ��delete����
// ����ֵ��			NULLʧ�ܣ������ɹ�
YTCONN_API_EXPORT void * YTConn_NewConfig(void);

// ɾ�����
YTCONN_API_EXPORT void YTConn_DelConfig(void * cfghandle);

// ���ļ��м�����������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTCONN_API_EXPORT int YTConn_LoadConfig(void * cfghandle, const char * filename);

// �����������õ��ļ���
// ���filenameΪNULL���ʾ���浽֮ǰ����ʱ������ļ�
// ����ֵ��			>=0�ɹ� <0ʧ��
YTCONN_API_EXPORT int YTConn_SaveConfig(void * cfghandle, const char * filename = NULL);

// ��ȡ�ַ���������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTCONN_API_EXPORT const char * YTConn_ReadStrConfig(void * cfghandle, const char * section, const char * key, const char * defaultvalue);

// ��ȡ��������
YTCONN_API_EXPORT int YTConn_ReadIntConfig(void * cfghandle, const char * section, const char * key, int defaultvalue);

// д���ַ���������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTCONN_API_EXPORT int YTConn_WriteStrConfig(void * cfghandle, const char * section, const char * key, const char * value);

// д����������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTCONN_API_EXPORT int YTConn_WriteIntConfig(void * cfghandle, const char * section, const char * key, int value);

//-----------------------------------���ӹ�����غ���-----------------------------------
// �������Ӿ��
// 1���þ����־���Ӷ�����Ϣ���������к�������صĺ�����������ж���Ҫ����þ����Ϣ
// 2�����صľ����Χ���õĻ�����������DelConn�����ͷţ���Χ��Ҫ��delete����
// confighandle		�������þ������YTConn_NewConfig������������һһ��Ӧ�����д���IP��ַ����Ϣ��
// ����ֵ��			NULLʧ�ܣ������ɹ�
YTCONN_API_EXPORT void * YTConn_NewConn(void * cfghandle);

// ɾ������
YTCONN_API_EXPORT void YTConn_DelConn(void * connhandle);

// ��������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTCONN_API_EXPORT int YTConn_Connect(void * connhandle);

// �ر�����
YTCONN_API_EXPORT int YTConn_Close(void * connhandle);

// ��ȡ����״̬
// ����ֵ��			�μ������ENum_ConnStatus�ṹ����
YTCONN_API_EXPORT int YTConn_GetStatus(void * connhandle);

// ��ȡ��ǰ���ӵķ�����
// ����ֵ��			NULLû�����ӣ�������ʾ���ӷ�������ַ
YTCONN_API_EXPORT const char * YTConn_GetServer(void * connhandle, int * port);

// �����������ݽ��������첽��ʽ�������յ����ͨ������ĸ����ص��������أ�
// funcid			���ܺţ��μ�Э���ĵ���
// data				Ҫ���͵����ݣ�ע�⣺���ﴫ������������壬��ͷ��DLL�Լ��ڲ��������ƣ�
// len				���ݳ���
// ����ֵ��			>=0������ <0ʧ��
YTCONN_API_EXPORT int YTConn_Send(void * connhandle, unsigned int funcid, const char * data, int len);

//----------------------------------������������غ���----------------------------------
#pragma pack(1)
typedef struct 						// ����������ͷ��������
{
	unsigned int	chunnelid;		// ͨ����
	unsigned int	userid;			// �û����
	unsigned int	funcid;			// ���ܺ�
	unsigned int	reqserailid;	// ������Ϣ���
	unsigned int	result;			// ϵͳ������
} tagYTConnPackHead;
#pragma pack()

// �������ݴ�������δ���������һ�������Ӧ����
// phead			����������ͷ����
// data				Ҫ���͵����ݣ�ע�⣺���ﴫ������������壩
// len				���ݳ���
// outbuf			�������ݣ������ķ�������
// outlen			�������ݣ����泤�ȣ�������Ȳ����Ļ����ֵ��Ϊ����������Ҫ�ĳ���
// ����ֵ:			>0 ���������ݳ���
//					=0 �������
//					<0 ����Ĵ���󻺴泤�Ȳ��㣬��ʱoutlen������Ҫ��ҵ�����ݳ���
YTCONN_API_EXPORT int YTConn_Pack(tagYTConnPackHead * phead, const char * data, int len, const char * outbuf, int * outlen);

// Ӧ�����ݽ��
// data				���յ������ݣ�ע�⣺����������а�ͷ�Ͱ��壩
// len				���ݳ���
// phead			����������ͷ��������
// outbuf			�������ݣ�������ҵ�����ݣ�ע�⣺���������ҵ������û�а�ͷ��
// outlen			�������ݣ�ʵ�ʵ�ҵ�����ݵĳ��ȣ����Ϊ0���ʾû�н��������
// ����ֵ:			>0 �Ѿ�����Ľ������ݳ��ȣ���ʱ����outlen���ؽ������ҵ�����ݳ���
//					=0 ����Ҫ������������
//					<0 �����ҵ�����ݳ��Ȳ��㣬��ʱoutlen������Ҫ��ҵ�����ݳ���
YTCONN_API_EXPORT int YTConn_UnPack(const char * data, int len, tagYTConnPackHead * phead, const char * outbuf, int * outlen);