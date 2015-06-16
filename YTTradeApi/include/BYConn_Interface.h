/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYConn_Interface.h
* �ļ�ժҪ������ͨѶ��������������Э��ṹ�μ�cpp�ļ�
*
* �򵥵Ĳ�������˵����
* �� �������õ�һ�����
*	 1��ͨ��BYConn_NewConfig����һ������
*	 2��ͨ��BYConn_LoadConfig�������ļ������������
*	 3��ͨ������һϵ�еĺ�����1�еĶ���������ݲ���
*	 4�������Ҫ���浽һ���ļ��Ļ���ͨ��BYConn_SaveConfig����ʵ��
*	 5��ͨ��BYConn_DelConfig����ɾ������
* �� ���ӹ����һ�����
*	 1��ͨ��BYConn_NewConn����һ������
*	 2��ͨ��BYConn_Connect������������
*	 3��ͨ��BYConn_Send��������
*	 4��ͨ��BYConn_Close������������
*	 5��ͨ��BYConn_DelConn����ɾ������
*
* ���ע��˵����
*	 1�����ӵĵ�һ����������ͨѶ��֤����ֻ����֤ͨ���˲��ܷ��ͺ��������
*	 2��ͨѶ��֤�ɹ�����Ҫ�����������ص���Կ��Ϊ���������Կ
*	 3��ͨѶ��Ӧ����Ϣȫ��ͨ���ص�����ʽ����
*
* Dll�޸ģ� 2014-05-22 saimen���� V1.0.0.3
*			2014-xx-xx saimen�޸� V1.0.0.4	�����Ż���
*/
#ifndef __BYCONN_INTERFACE_H__
#define __BYCONN_INTERFACE_H__

#include <windows.h>

//#ifdef _WIN32
//	#ifdef BYCONN_EXPORT
//		#define BYCONN_CALL	extern "C" __declspec(dllexport)
//	#else
//		#define BYCONN_CALL extern "C" __declspec(dllimport)
//	#endif
//#else
//	#define BYCONN_CALL extern "C"
//#endif

//-------------------------------------�ṹ�嶨��-------------------------------------
enum ENum_ConnStatus	// ����״̬����
{
	CS_CLOSE		= 0,	// δ���ӻ����ӶϿ�
	CS_CONNING		= 1,	// ������
	CS_CONNED		= 2,	// ������
};

enum ENum_ErrCode		// ͨѶ������
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
// retbuf		Ӧ�����ݼ����棬��������ػ��棬��Χ�����Զ�ͨ��BYData_Interface.h�е����ݽṹ����ת��
// len			Ӧ�����ݼ����泤��
// �ر�ע�⣺	�����ͨѶ�����Ļ�������������Ƿ��ص�retbuf���棬ҵ���Ĵ�����Ҫ��retbuf���ݽ��н�����õ�
typedef void (__stdcall * pFunc_DataAllRet)(void * pConn, long reqno, unsigned long funcid, long errcode, const char * retbuf, long len);

// ���ݲ��ַ��ػص����壨ע�⣺���ﲻ��ȫ���ؿ����Ƕ���������
// �ص����ص�����retbuf��Ҫ���̴������˻ص����������ݻ��Զ���DLL�ڲ�ɾ��
// pConn		���Ӷ���
// reqno		������
// funcid		���ܺţ��μ�Э���ĵ���
// errno		�����ţ�ע�⣺����Ĵ����Ž�����ͨѶ��Ĵ���ҵ���Ĵ�����retbuf���Լ�ȥ����
// retbuf		Ӧ�����ݼ����棬��������ػ��棬��Χ�����Զ�ͨ��BYData_Interface.h�е����ݽṹ����ת��
// len			Ӧ�����ݼ����泤��
// �ر�ע�⣺	�����ͨѶ�����Ļ�������������Ƿ��ص�retbuf���棬ҵ���Ĵ�����Ҫ��retbuf���ݽ��н�����õ�
typedef void (__stdcall * pFunc_DataPartRet)(void * pConn, long reqno, unsigned long funcid, long errcode, const char * retbuf, long len);

// ���ݳ�ʱ�ص����壨��������ͺ�ʱ��ò���Ӧ�����ݵ������
// pConn		���Ӷ���
// reqno		������
// funcid		���ܺţ��μ�Э���ĵ���
typedef void (__stdcall * pFunc_TimeOut)(void * pConn, long reqno, unsigned long funcid);

// ���ӶϿ��ص�����
// pConn		���Ӷ���
typedef void (__stdcall * pFunc_Close)(void * pConn);

//------------------------------------������������------------------------------------

//------------------------------------------------------------------------------------
// DLL��������

// ��ȡ�汾����V1.0 B001����ʽ
 const char * BYConn_GetVer();

// DLL��ʼ��
// logpath	��־·��
// errmsg	������Ϣ����
// len		���泤��
// ����ֵ��	>=0�ɹ� <0ʧ��
 long BYConn_Init(const char * logpath, char * errmsg, long len);

// DLL�ر�
 void BYConn_Release();


//------------------------------------------------------------------------------------
// ���������ļ���������

// �������
// 1���þ����־����������Ϣ���������к�������صĺ�����������ж���Ҫ����þ����Ϣ
// 2�����صľ����Χ���õĻ�����������DelConfig�����ͷţ���Χ��Ҫ��delete����
// ����ֵ��		NULLʧ�ܣ������ɹ�
 void * BYConn_NewConfig();

// ɾ�����
 void BYConn_DelConfig(void * cfghandle);

// ���ļ��м�����������
// ����ֵ��		>=0�ɹ� <0ʧ��
 long BYConn_LoadConfig(void * cfghandle, const char * filename);

// �����������õ��ļ���
// ���filenameΪNULL���ʾ���浽֮ǰ����ʱ������ļ�
 long BYConn_SaveConfig(void * cfghandle, const char * filename = NULL);

// ��ȡ�ַ���������
// ����ֵ��		NULLʧ�ܣ������ɹ�
 const char * BYConn_ReadStrConfig(void * cfghandle, const char * section, const char * key, const char * defaultvalue);

// ��ȡ��������
 long BYConn_ReadIntConfig(void * cfghandle, const char * section, const char * key, long defaultvalue);

// д���ַ���������
// ����ֵ��		>=0�ɹ� <0ʧ��
 long BYConn_WriteStrConfig(void * cfghandle, const char * section, const char * key, const char * value);

// д����������
// ����ֵ��		>=0�ɹ� <0ʧ��
long BYConn_WriteIntConfig(void * cfghandle, const char * section, const char * key, long value);


//------------------------------------------------------------------------------------
// ͨѶ�ص��������ã�ע�⣺���¸����ص������������ظ����ã�

// ����������ȫ���ػص�����
void BYConn_CallBackDataAllRet(pFunc_DataAllRet callbackfun);

// �������ݲ��ַ��ػص�����
void BYConn_CallBackDataPartRet(pFunc_DataPartRet callbackfun);

// �������ݳ�ʱ�ص�����
void BYConn_CallBackTimeOut(pFunc_TimeOut callbackfun);

// �������ӶϿ��ص�����
void BYConn_CallBackClose(pFunc_Close callbackfun);


//------------------------------------------------------------------------------------
// ���ӹ�����غ���

// �������Ӿ��
// 1���þ����־���Ӷ�����Ϣ���������к�������صĺ�����������ж���Ҫ����þ����Ϣ
// 2�����صľ����Χ���õĻ�����������DelConn�����ͷţ���Χ��Ҫ��delete����
// confighandle	�������þ������BYConn_NewConfig������������һһ��Ӧ��־�����д���IP��ַ����Ϣ��
// userid       �û���ţ�û�е�¼ʱ��0����¼�󽨶���������û����
// ����ֵ��		NULLʧ�ܣ������ɹ�
void * BYConn_NewConn(void * cfghandle, long userid);

// ɾ������
void BYConn_DelConn(void * connhandle);

// �������� 
// cryptkey		ͨѶ��Կ
// len			ͨѶ��Կ����
// ����ֵ��		>=0�ɹ� <0ʧ��
long BYConn_Connect(void * connhandle, const char * cryptkey, long len);

// �ر�����
long BYConn_Close(void * connhandle);

// ��ȡ����״̬
// ����ֵ��		�μ������ENum_ConnStatus�ṹ����
long BYConn_GetStatus(void * connhandle);

// ��ȡ��ǰ���ӵķ�����
// ����ֵ��		NULLû�����ӣ���NULL��ʾ���ӷ�������ַ
const char * BYConn_GetServer(void * connhandle, long * port);

// �����������ݽ��������첽��ʽ�����ݽ��յ��Ժ��ͨ������ĸ����ص��������أ�
// funcid		���ܺţ��μ�Э���ĵ���
// data			Ҫ���͵����ݣ�ע�⣺���ﴫ������������壬��ͷ��DLL�Լ��ڲ��������ƣ�
// len			���ݳ���
// ����ֵ��		���>0��ʾ�����ţ�<=0��ʾ����ʧ��
long BYConn_Send(void * connhandle, unsigned long funcid, const char * data, long len);


//------------------------------------------------------------------------------------
// ���������ݹ�����غ���

// ����������ͷ��������
#pragma pack(1)

typedef struct
{
	unsigned long	ConnID;			// �ỰID
	long			UserID;			// �û���ţ����ڱ�ʶ����Ա���
	unsigned long	FuncID;			// ���ܺţ��μ�Э���ĵ���
	long			ReqNo;			// �����ţ�ע�⣺0--999999Ϊ�����š�1000000--1999999Ϊ���ͱ�ţ�����������
	long			ErrNo;			// �����ţ��μ������ֵ䣩
} tagPackHead;

#pragma pack()

// �������ݴ�������δ���������һ�������Ӧ���������Ӧ�����ݻ��������
// phead		����������ͷ��������
// data			Ҫ���͵����ݣ�ע�⣺���ﴫ������������壩
// len			���ݳ���
// key			���������Կ
// keylen		���������Կ����
// outbuf		�������ݣ������ķ�������
// outlen		�������ݣ����泤�ȣ�������Ȳ����Ļ����ֵ��Ϊ����������Ҫ�ĳ���
// ����ֵ��		>0 ���������ݳ���
//				=0 �������
//				<0 ����Ĵ���󻺴泤�Ȳ��㣬��ʱoutlen������Ҫ��ҵ�����ݳ���
long BYConn_Pack(tagPackHead * phead, const char * data, long len, const char * key, long keylen,const char * outbuf, long * outlen);

// Ӧ�����ݽ����ÿ�ν����һ��ҵ�������
// data			���յ������ݣ�ע�⣺����������а�ͷ�Ͱ��壩
// len			���ݳ���
// key			���������Կ
// keylen		���������Կ����
// phead		����������ͷ��������
// outbuf		�������ݣ�������ҵ�����ݣ�ע�⣺���������ҵ������û�а�ͷ��
// outlen		�������ݣ�ʵ�ʵ�ҵ�����ݵĳ��ȣ����Ϊ0���ʾû�н��������
// ����ֵ��		>0 �Ѿ�����Ľ������ݳ��ȣ���ʱ����outlen�������ҵ�����ݳ���
//				=0 ����Ҫ������������
//				<0 �����ҵ�����ݳ��Ȳ��㣬��ʱoutlen������Ҫ��ҵ�����ݳ���
long BYConn_UnPack(const char * data, long len, const char * key, long keylen, tagPackHead * phead, const char * outbuf, long * outlen);

// ���ɼ�����Կ
// key			��Կ
// len			��Կ����
// ����ֵ��		>0	 ��Կ����
//				=0  �������Կ���泤�Ȳ��㣬��ʱlen������Ҫ����Կ����
//				<0  ʧ��
long BYConn_GenerateKey(const char * key, long * len);

#endif
