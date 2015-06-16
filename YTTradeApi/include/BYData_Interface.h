/*
* Copyright (c) 2014, WinTech
* All rights reserved.
*
* �ļ����ƣ�BYData_Interface.h
* �ļ�ժҪ�����ݽ�����������������Э��ṹ�μ�cpp�ļ�
*
* �򵥵Ĳ�������˵����
* �� ���յ����ݵ�һ�����
*	 1��ͨ��BYData_NewData����һ������
*	 2��ͨ��BYData_ConvertData���������յ������ݸ�1�еĶ���������ݽṹת��
*	 3��ͨ������һϵ�еĺ�����1�еĶ���������ݲ���
*	 4��ͨ��BYData_DeleteData����ɾ������
* �� ��Ҫ������������
*	 1��ͨ��BYData_NewData����һ������
*	 2��ͨ��һϵ�е����ݲ�����1�е����ݶ���ֵ
*	 3��ͨ��BYData_MakeSendData�����������ݷ��ͻ���������ݷ���
*	 4��ͨ��BYData_DeleteData����ɾ������ע�⣺һ��ɾ������3�����ɵ����ݻ�����Զ���ʧ�ˣ�
* �� �򵥵�����ѭ����ȡ
*	 1��ͨ��BYData_GetFieldString��BYData_GetFieldChar�Ⱥ�����ȡ�ֶ�ֵ
*	 2��ͨ��BYData_GotoNextRowת����һ����¼��Ȼ��ص�1�Ĳ�����ֱ��BYData_GotoNextRowʧ�����ʾ�����Ѿ������
* �� �򵥵�����ѭ�����
*	 1��ͨ��BYData_AppendRow���һ��
*	 2��ͨ��BYData_AddFieldString��BYData_AddFieldChar�Ⱥ��������ֶζ�Ӧ�����ݣ����ж������ݼ�����1
*
* Dll�޸ģ� 2014-05-22 saimen���� V1.0.0.3
*			2014-xx-xx saimen�޸� V1.0.0.4	�����Ż��棬����ASCI��UNICODE��ת�ĺ���
*/
#ifndef __BYDATA_INTERFACE_H__
#define __BYDATA_INTERFACE_H__

#include <windows.h>

#ifdef _WIN32
	#ifdef BYDATA_EXPORT
		#define BYDATA_CALL	extern "C" __declspec(dllexport)
	#else
		#define BYDATA_CALL extern "C" __declspec(dllimport)
	#endif
#else
	#define BYDATA_CALL extern "C"
#endif

//-------------------------------------�ṹ�嶨��-------------------------------------
enum ENum_FieldType		// �ֶ����Ͷ���
{
	FT_STRING	= 0,	// �ַ�����
	FT_CHAR		= 1,	// �ַ���
	FT_INTEGER	= 2,	// ����
	FT_DOUBLE	= 3,	// ������
	FT_PASSWORD	= 4,	// �������ݣ�����blowfish���ܺ�洢��
	FT_BINARY	= 5,	// ���������ݣ����������ݲ���Base64ת����洢��
};

enum ENum_SortType		// ����ʽ����
{
	ST_NULL		= 0,	// ����
	ST_DOWN		= 1,	// ����
	ST_UP		= 2,	// ����
};

//------------------------------------������������------------------------------------

//------------------------------------------------------------------------------------
// DLL��������

// ��ȡ�汾����V1.0 B001����ʽ
const char * BYData_GetVer();

// DLL��ʼ��
// errmsg	������Ϣ����
// len		���泤��
// ����ֵ��	>=0�ɹ� <0ʧ��
long BYData_Init(char * errmsg, long len);

// DLL�ر�
void BYData_Release();

// ASCIת��UNICODE
// BYDATA����ȡ�������ݿ�ͨ��������ת���ɿ��ַ�����Ҫ���fieldname����string��char��ʽ��fieldvalue
// ����ֵ��>=0ת����ĳ��� <0���Ȳ���������-100�ͱ�ʾ���Ȳ�������Ҫ100�� 
long BYData_MByteToWChar(LPCSTR cszstr, LPWSTR wszstr, long wsize); 

// UNICODEת��ASCI
// ���Դ����ΪUNICODE����ͨ��������ת����ASCI�����BYDATA��������Ҫ���fieldname����string��char��ʽ��fieldvalue
// ����ֵ��>=0ת����ĳ��� <0���Ȳ���������-100�ͱ�ʾ���Ȳ�������Ҫ100�� 
long BYData_WCharToMByte(LPWSTR wszstr, LPCSTR cszstr, long csize);


//------------------------------------------------------------------------------------
// ���ݰ���������

// �������
// 1���þ����־������Ϣ���������к�������صĺ�����������ж���Ҫ����þ����Ϣ
// 2�������Ҫ�����ݰ�������ȫ���������ʼ�������ResetData��������
// 3�����صľ����Χ���õĻ�����������DeleteData�����ͷţ���Χ��Ҫ��delete����
// ����ֵ��NULLʧ�ܣ������ɹ�
void * BYData_NewData();

// ��������
void BYData_ResetData(void * handle);

// ɾ�����
void BYData_DeleteData(void * handle);

// ���ݰ�copy�����sourcehandle��ʾԴ���ݰ����
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_CopyData(void * handle, void * sourcehandle, long index = 1);

// ���ݺϲ������sourcehandle��ʾԴ���ݰ����
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_AppendData(void * handle, void * sourcehandle);

//------------------------------------------------------------------------------------
// ���ݰ���ͨѶ���滥ת

// ת�����ݻ��棺�ú�����Ҫ�����뻺������ת���ɱ�׼���ݽṹ
// ͨѶ���ݴ����õ��Ļ���ͨ��������ת����ָ�������ݽṹ
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_ConvertData(void * handle, const char * data, long len);

// �������ݻ��棺�ú�����Ҫ�����ݽṹת���ɻ�����Ϣ�Ա㷢��
// ע�⣺���ﷵ�ص����ݻ���DLL�ڲ�������Χ��Ҫ��delete����
// ����ֵ�����ػ���ָ�룬ΪNULL��ʾʧ�ܣ�len���ػ��泤��
const char * BYData_MakeSendData(void * handle, long * len);


//------------------------------------------------------------------------------------
// ��¼��������

// ��ȡ��¼����
// ����ֵ��>=0��¼���� <0ʧ��
long BYData_GetRowCount(void * handle);

// ��ָ��λ�ò���һ����¼
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_InsertRow(void * handle, long rowindex);

// ɾ��һ��ָ����¼
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_DeleteRow(void * handle, long rowindex);

// ���������һ����¼
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_AppendRow(void * handle);

// ת����һ����¼
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_GotoBeginRow(void * handle);

// ת�����һ����¼
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_GotoEndRow(void * handle);

// ת����һ����¼
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_GotoPreRow(void * handle);

// ת����һ����¼
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_GotoNextRow(void * handle);

// ת��ָ����ĳ����¼
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_GotoFixRow(void * handle, long rowindex);


//------------------------------------------------------------------------------------
// �ֶο���

// ��ȡ�ֶθ���
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_GetFieldCount(void * handle);

// ��ȡ�ֶ�����
// ����ֵ��>=0�ֶ����ͣ��μ�����ENum_FieldType���壩 <0ʧ��
long BYData_GetFieldType(void * handle, long fieldindex);

// ��ȡ�ֶ���
// ����ֵ��NULLʧ�ܣ�������ʾ�ֶ���
const char * BYData_GetFieldName(void * handle, long fieldindex);

// ���һ���ֶ�
// fieldtypeΪ�ֶ����ͣ�����μ�����ENum_FieldType����
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_AddField(void * handle, const char * fieldname, long fieldtype = FT_STRING);

// ɾ��һ���ֶ�
long BYData_DeleteField(void * handle, const char * fieldname);


//------------------------------------------------------------------------------------
// �ֶ�ֵ����

// �ж��ֶ�ֵ�Ƿ�Ϊ��
bool BYData_IsNullValue(void * handle, const char * fieldname);

// ��ȡ�ַ���������
long BYData_GetFieldString(void * handle, const char * fieldname, char * outbuf, long len);

// ��ȡ�ַ�������
char BYData_GetFieldChar(void * handle, const char * fieldname);

// ��ȡ��������
long BYData_GetFieldInt(void * handle, const char * fieldname);

// ��ȡ����������
double BYData_GetFieldDouble(void * handle, const char * fieldname);

// ��ȡ��������
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_GetFieldPwd(void * handle, const char * fieldname, char * outpwd, long pwdlen);

// ��ȡ����������
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_GetFieldBinary(void * handle, const char * fieldname, char * outbuf, long * outlen);

// ����ַ���������
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_AddFieldString(void * handle, const char * fieldname, const char * value);

// ����ַ�������
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_AddFieldChar(void * handle, const char * fieldname, const char value);

// �����������
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_AddFieldInt(void * handle, const char * fieldname, long value);

// ��Ӹ���������
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_AddFieldDouble(void * handle, const char * fieldname, double value);

// �����������
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_AddFieldPwd(void * handle, const char * fieldname, const char * value);

// ��Ӷ���������
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_AddFieldBinary(void * handle, const char * fieldname, const char * value, long len);


//------------------------------------------------------------------------------------
// ��������

// �����ֶ�ֵ����ָ����¼
// �����ã�>=0��ʾָ����¼λ�� <0δ�ҵ���¼
long BYData_SearchByField(void * handle, const char * fieldname, const char * value);

// ����ĳ���ֶ�������δʵ�֣�
// sorttype��ʾ����ʽ������μ������ENum_SortType
// ����ֵ��>=0�ɹ� <0ʧ��
long BYData_SortByField(void * handle, const char * fieldname, long sorttype = ST_DOWN);

#endif
