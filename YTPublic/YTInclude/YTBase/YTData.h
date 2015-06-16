/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�YTData.h
* �ļ�ժҪ�����ݶԽӽӿ�
*
* Dll�޸ģ� 2015-05-29 saimen�޸� V1.0
*
*/
#pragma once

#include <windows.h>

#if defined(WIN32)
	#ifdef LIB_YTDATA_API_EXPORT
		#define YTDATA_API_EXPORT extern "C" __declspec(dllexport)
	#else
		#define YTDATA_API_EXPORT extern "C" __declspec(dllimport)
	#endif
#else
	#define YTDATA_API_EXPORT extern "C" 
#endif

//---------------------------------------��ض���---------------------------------------
enum ENum_FieldType
{
	FT_STRING	= 0,	// �ַ�����
	FT_CHAR		= 1,	// �ַ���
	FT_INTEGER	= 2,	// ����
	FT_DOUBLE	= 3,	// ������
	FT_PASSWORD	= 4,	// �������ݣ�����blowfish���ܺ�洢��
	FT_BINARY	= 5,	// ���������ݣ����������ݲ���Base64ת����洢��
};

//--------------------------------------DLL��������-------------------------------------
// ��ȡ�汾����V1.0����ʽ
YTDATA_API_EXPORT const char * YTData_GetVer(void);

// DLL��ʼ��
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_Init(void);

// DLL�ر�
YTDATA_API_EXPORT void YTData_Release(void);

// ASCIת��UNICODE
// YTDATA����ȡ�������ݿ���ͨ��������ת���ɿ��ַ�����Ҫ���fieldname����string��char��ʽ��fieldvalue
// ����ֵ��>=0ת����ĳ��� <0���Ȳ���������-100��ʾ���Ȳ�����Ҫ100��
YTDATA_API_EXPORT int YTData_MByteToWChar(LPCSTR cszstr, LPWSTR wszstr, int wsize); 

// UNICODEת��ASCI
// ���Դ����δUNICODE������ͨ��������ת����ASCI��ͨ��YTData��������Ҫ���fieldname����string��char��ʽ��fieldvalue
// ����ֵ��>=0ת����ĳ��� <0���Ȳ���������-100��ʾ���Ȳ�����Ҫ100��
YTDATA_API_EXPORT int YTData_WCharToMByte(LPWSTR wszstr, LPSTR cszstr, int csize);

//--------------------------------------���ݰ�����--------------------------------------
// �������
// 1���þ����־������Ϣ���������еĺ�������صĺ�����������ж���Ҫ����þ����Ϣ
// 2�������Ҫ�����ݰ�������ȫ���������ʼ�������ResetData��������
// 3�����صľ����Χ���õĻ�����������DeleteData�����ͷţ���Χ��Ҫ��delete����
// ����ֵ��			NULLʧ�ܣ������ɹ�
YTDATA_API_EXPORT void * YTData_NewData(void);

// ��������
YTDATA_API_EXPORT void YTData_ResetData(void * handle);

// ɾ�����
YTDATA_API_EXPORT void YTData_DeleteData(void * handle);

// ���ݰ�copy�����sourcehandle��ʾԴ���ݰ����
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_CopyData(void * handle, void * sourcehandle, int startindex = 1, int endindex = 0);

// ���ݺϲ������sourcehandle��ʾԴ���ݰ����
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_AppendData(void * handle, void * sourcehandle);

//----------------------------------������ͨѶ���滥ת----------------------------------
// ת�����ݻ��棺�ú�����Ҫ�����뻺������ת���ɱ�׼���ݽṹ
// ͨѶ���ݴ����õ��Ļ���ͨ��������ת����ָ�������ݽṹ
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_ConvertData(void * handle, const char * data, int len);

// �������ݻ��棺�ú�����Ҫ�����ݽṹת���ɻ�����Ϣ�Ա㷢��
// ע�⣺���ﷵ�ص����ݻ���DLL�ڲ�������Χ��Ҫ��delete����
// ����ֵ��			���ػ���ָ�룬ΪNULL��ʾʧ�ܣ�len���ػ��泤��
YTDATA_API_EXPORT const char * YTData_MakeSendData(void * handle, int * len);

//--------------------------------------��Ч�Կ���--------------------------------------
// ���÷���ֵ
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_SetRetCode(void * handle, int retcode);

// ���÷�����Ϣ
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_SetRetMsg(void * handle, const char* retmsg);

// ��ȡ����ֵ
YTDATA_API_EXPORT int YTData_GetRetCode(void * handle);

// ��ȡ������Ϣ
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_GetRetMsg(void * handle, char * outbuf, int len);

//-------------------------------------��¼��������-------------------------------------
// ��ȡ��¼����
// ����ֵ��			>=0��¼���� <0ʧ��
YTDATA_API_EXPORT int YTData_GetRowCount(void * handle);

// ��ָ��λ�ò���һ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_InsertRow(void * handle, int rowindex);

// ɾ��һ��ָ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_DeleteRow(void * handle, int rowindex);

// ���������һ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_AppendRow(void * handle);

// ת����һ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_GotoBeginRow(void * handle);

// ת�����һ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_GotoEndRow(void * handle);

// ת����һ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_GotoPreRow(void * handle);

// ת����һ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_GotoNextRow(void * handle);

// ת��ָ����ĳ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_GotoFixRow(void * handle, int rowindex);

//---------------------------------------�ֶο���---------------------------------------
// ��ȡ�ֶθ���
// ����ֵ��			>=0�ֶθ��� <0ʧ��
YTDATA_API_EXPORT int YTData_GetFieldCount(void * handle);

// ��ȡ�ֶ�����
// ����ֵ��			>=0�ֶ����ͣ��μ�����ENum_FieldType���壩 <0ʧ��
YTDATA_API_EXPORT int YTData_GetFieldType(void * handle, int fieldindex);

// ��ȡ�ֶ���
// ����ֵ��			NULLʧ�ܣ�������ʾ�ֶ���
YTDATA_API_EXPORT const char * YTData_GetFieldName(void * handle, int fieldindex);

// ���һ���ֶ�
// fieldtypeΪ�ֶ����ͣ�����μ�����ENum_FieldType����
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_AddField(void * handle, const char * fieldname, int fieldtype = FT_STRING);

// ɾ��һ���ֶ�
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_DeleteField(void * handle, const char * fieldname);

//--------------------------------------�ֶ�ֵ����--------------------------------------
// �ж��ֶ�ֵ�Ƿ�Ϊ��
YTDATA_API_EXPORT bool YTData_IsNullValue(void * handle, const char * fieldname);

// ��ȡ�ַ���������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_GetFieldString(void * handle, const char * fieldname, char * outbuf, int len);

// ��ȡ�ַ�������
YTDATA_API_EXPORT char YTData_GetFieldChar(void * handle, const char * fieldname);

// ��ȡ��������
YTDATA_API_EXPORT int YTData_GetFieldInt(void * handle, const char * fieldname);

// ��ȡ����������
YTDATA_API_EXPORT double YTData_GetFieldDouble(void * handle, const char * fieldname);

// ��ȡ��������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_GetFieldPwd(void * handle, const char * fieldname, char * outpwd, int pwdlen);

// ��ȡ����������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_GetFieldBinary(void * handle, const char * fieldname, char * outbuf, int * outlen);

// ����ַ���������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_AddFieldString(void * handle, const char * fieldname, const char * value);

// ����ַ�������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_AddFieldChar(void * handle, const char * fieldname, const char value);

// �����������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_AddFieldInt(void * handle, const char * fieldname, int value);

// ��Ӹ���������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_AddFieldDouble(void * handle, const char * fieldname, double value);

// �����������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_AddFieldPwd(void * handle, const char * fieldname, const char * value);

// ��Ӷ���������
// ����ֵ��			>=0�ɹ� <0ʧ��
YTDATA_API_EXPORT int YTData_AddFieldBinary(void * handle, const char * fieldname, const char * value, int len);

//-------------------------------------��ظ�������-------------------------------------
// �����ֶ�ֵ����ָ������
// ����ֵ��			>=0��ʾָ����¼λ�� <0δ�ҵ���Ӧ�ļ�¼
YTDATA_API_EXPORT int YTData_SearchByField(void * handle, const char * fieldname, const char * value);
