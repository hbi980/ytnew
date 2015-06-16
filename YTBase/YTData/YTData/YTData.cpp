/*
* ���ݽṹ��������
*			�ֶθ���<SOH>��¼����<SOH><SOA>
*			�ֶ�1=�ֶ�����<SOH>�ֶ�2=�ֶ�����<SOH>...�ֶ�n=�ֶ�����<SOH><SOA>
*			��ֵ1<SOH>��ֵ2<SOH>...��ֵn<SOH><SOA>
*			��ֵ1<SOH>��ֵ2<SOH>...��ֵn<SOH><SOA>
*			.....................................
*			��ֵ1<SOH>��ֵ2<SOH>...��ֵn<SOH><SOA>
*			
*			�����ֶ����Ͷ������£�
*			0-�ַ�����
*			1-�ַ���
*			2-���ͣ�BYTE������Ŀǰ�������ͣ�
*			3-������
*			4-�������ݣ�����blowfish����Ȼ��base64ת����洢��
*			5-���������ݣ����������ݲ���base64ת����洢��
*/
#include "YTStep.h"

// ÿ���޸Ķ���汾��Ϣ���汾��¼�޸���Ϣ��YTData.h�ļ�ͷ��д��
#define MAIN_VER	"1.0"

// ��̬ȫ�ֱ��������ڷ�ֹ�ظ���ʼ��������
static bool g_IsExist = false;

// ��ȡ�汾����V1.0����ʽ
const char * YTData_GetVer(void)
{
	return MAIN_VER;
}

// DLL��ʼ��
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_Init(void)
{
	if(g_IsExist)
	{
		return 0;
	}
	else
	{
		g_IsExist = true;
		return 0;
	}
}

// DLL�ر�
void YTData_Release(void)
{
	if(!g_IsExist)
	{
		return;
	}
	else
	{
		g_IsExist = false;
	}
}

// ASCIת��UNICODE
// YTDATA����ȡ�������ݿ���ͨ��������ת���ɿ��ַ�����Ҫ���fieldname����string��char��ʽ��fieldvalue
// ����ֵ��>=0ת����ĳ��� <0���Ȳ���������-100��ʾ���Ȳ�����Ҫ100��
int YTData_MByteToWChar(LPCSTR cszstr, LPWSTR wszstr, int wsize)
{
	int minsize = MultiByteToWideChar(CP_ACP, 0, cszstr, -1, NULL, 0);
    if(minsize < wsize)
    {
		return 0-minsize;
    }
	else
	{
		MultiByteToWideChar(CP_ACP, 0, cszstr, -1, wszstr, minsize);
		return minsize;
	}
} 

// UNICODEת��ASCI
// ���Դ����δUNICODE������ͨ��������ת����ASCI��ͨ��YTData��������Ҫ���fieldname����string��char��ʽ��fieldvalue
// ����ֵ��>=0ת����ĳ��� <0���Ȳ���������-100��ʾ���Ȳ�����Ҫ100��
int YTData_WCharToMByte(LPWSTR wszstr, LPSTR cszstr, int csize)
{
	int minsize = WideCharToMultiByte(CP_OEMCP, 0, wszstr, -1, NULL, 0, NULL, FALSE);
	if(minsize < csize)
	{
		return 0-minsize;
	}
	else
	{
		WideCharToMultiByte(CP_OEMCP, 0, wszstr, -1, cszstr, csize, NULL, FALSE);
		return minsize;
	}
}

// �������
// 1���þ����־������Ϣ���������еĺ�������صĺ�����������ж���Ҫ����þ����Ϣ
// 2�������Ҫ�����ݰ�������ȫ���������ʼ�������ResetData��������
// 3�����صľ����Χ���õĻ�����������DeleteData�����ͷţ���Χ��Ҫ��delete����
// ����ֵ��			NULLʧ�ܣ������ɹ�
void * YTData_NewData(void)
{
	CYTStep * ret = new CYTStep;
	return ret;
}

// ��������
void YTData_ResetData(void * handle)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		pYTStep->ResetData();
	}
}

// ɾ�����
void YTData_DeleteData(void * handle)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		delete pYTStep;
		pYTStep = NULL;
	}
}

// ���ݰ�copy�����sourcehandle��ʾԴ���ݰ����
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_CopyData(void * handle, void * sourcehandle, int startindex, int endindex)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->CopyData((CYTStep *)sourcehandle, startindex, endindex);
	}
	else
	{
		return -1;
	}
}

// ���ݺϲ������sourcehandle��ʾԴ���ݰ����
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_AppendData(void * handle, void * sourcehandle)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->AppendData((CYTStep *)sourcehandle);
	}
	else
	{
		return -1;
	}
}

// ת�����ݻ��棺�ú�����Ҫ�����뻺������ת���ɱ�׼���ݽṹ
// ͨѶ���ݴ����õ��Ļ���ͨ��������ת����ָ�������ݽṹ
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_ConvertData(void * handle, const char * data, int len)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->ConvertData(data, len);
	}
	else
	{
		return -1;
	}
}

// �������ݻ��棺�ú�����Ҫ�����ݽṹת���ɻ�����Ϣ�Ա㷢��
// ע�⣺���ﷵ�ص����ݻ���DLL�ڲ�������Χ��Ҫ��delete����
// ����ֵ��			���ػ���ָ�룬ΪNULL��ʾʧ�ܣ�len���ػ��泤��
const char * YTData_MakeSendData(void * handle, int * len)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->MakeSendData(len);
	}
	else
	{
		return NULL;
	}
}

// ���÷���ֵ
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_SetRetCode(void * handle, int retcode)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		pYTStep->SetRetCode(retcode);
		return 0;
	}
	else
	{
		return -1;
	}
}

// ���÷�����Ϣ
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_SetRetMsg(void * handle, const char* retmsg)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		pYTStep->SetRetMsg(retmsg);
		return 0;
	}
	else
	{
		return -1;
	}
}

// ��ȡ����ֵ
int YTData_GetRetCode(void * handle)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GetRetCode();
	}
	else
	{
		return -1;
	}
}

// ��ȡ������Ϣ
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_GetRetMsg(void * handle, char * outbuf, int len)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GetRetMsg(outbuf, len);
	}
	else
	{
		return -1;
	}
}

// ��ȡ��¼����
// ����ֵ��			>=0��¼���� <0ʧ��
int YTData_GetRowCount(void * handle)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GetRowCount();
	}
	else
	{
		return -1;
	}
}

// ��ָ��λ�ò���һ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_InsertRow(void * handle, int rowindex)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->InsertRow(rowindex);
	}
	else
	{
		return -1;
	}
}

// ɾ��һ��ָ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_DeleteRow(void * handle, int rowindex)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->DeleteRow(rowindex);
	}
	else
	{
		return -1;
	}
}

// ���������һ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_AppendRow(void * handle)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->AppendRow();
	}
	else
	{
		return -1;
	}
}

// ת����һ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_GotoBeginRow(void * handle)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GotoBeginRow();
	}
	else
	{
		return -1;
	}
}

// ת�����һ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_GotoEndRow(void * handle)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GotoEndRow();
	}
	else
	{
		return -1;
	}
}

// ת����һ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_GotoPreRow(void * handle)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GotoPreRow();
	}
	else
	{
		return -1;
	}
}

// ת����һ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_GotoNextRow(void * handle)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GotoNextRow();
	}
	else
	{
		return -1;
	}
}

// ת��ָ����ĳ����¼
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_GotoFixRow(void * handle, int rowindex)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GotoFixRow(rowindex);
	}
	else
	{
		return -1;
	}
}

// ��ȡ�ֶθ���
// ����ֵ��			>=0�ֶθ��� <0ʧ��
int YTData_GetFieldCount(void * handle)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GetFieldCount();
	}
	else
	{
		return -1;
	}
}

// ��ȡ�ֶ�����
// ����ֵ��			>=0�ֶ����ͣ��μ�����ENum_FieldType���壩 <0ʧ��
int YTData_GetFieldType(void * handle, int fieldindex)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GetFieldType(fieldindex);
	}
	else
	{
		return -1;
	}
}

// ��ȡ�ֶ���
// ����ֵ��			NULLʧ�ܣ�������ʾ�ֶ���
const char * YTData_GetFieldName(void * handle, int fieldindex)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GetFieldName(fieldindex);
	}
	else
	{
		return NULL;
	}
}

// ���һ���ֶ�
// fieldtypeΪ�ֶ����ͣ�����μ�����ENum_FieldType����
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_AddField(void * handle, const char * fieldname, int fieldtype)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->AddField(fieldname, fieldtype);
	}
	else
	{
		return -1;
	}
}

// ɾ��һ���ֶ�
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_DeleteField(void * handle, const char * fieldname)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->DeleteField(fieldname);
	}
	else
	{
		return -1;
	}
}

// �ж��ֶ�ֵ�Ƿ�Ϊ��
bool YTData_IsNullValue(void * handle, const char * fieldname)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->IsNullValue(fieldname);
	}
	else
	{
		return true;
	}
}

// ��ȡ�ַ���������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_GetFieldString(void * handle, const char * fieldname, char * outbuf, int len)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GetFieldString(fieldname, outbuf, len);
	}
	else
	{
		memset(outbuf, 0, len);
		return -1;
	}
}

// ��ȡ�ַ�������
char YTData_GetFieldChar(void * handle, const char * fieldname)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GetFieldChar(fieldname);
	}
	else
	{
		return '\0';
	}
}

// ��ȡ��������
int YTData_GetFieldInt(void * handle, const char * fieldname)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GetFieldInt(fieldname);
	}
	else
	{
		return 0;
	}
}

// ��ȡ����������
double YTData_GetFieldDouble(void * handle, const char * fieldname)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GetFieldDouble(fieldname);
	}
	else
	{
		return 0;
	}
}

// ��ȡ��������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_GetFieldPwd(void * handle, const char * fieldname, char * outpwd, int pwdlen)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GetFieldPwd(fieldname, outpwd, pwdlen);
	}
	else
	{
		return -1;
	}
}

// ��ȡ����������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_GetFieldBinary(void * handle, const char * fieldname, char * outbuf, int * outlen)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->GetFieldBinary(fieldname, outbuf, outlen);
	}
	else
	{
		return -1;
	}
}

// ����ַ���������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_AddFieldString(void * handle, const char * fieldname, const char * value)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->AddFieldString(fieldname, value);
	}
	else
	{
		return -1;
	}
}

// ����ַ�������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_AddFieldChar(void * handle, const char * fieldname, const char value)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->AddFieldChar(fieldname, value);
	}
	else
	{
		return -1;
	}
}

// �����������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_AddFieldInt(void * handle, const char * fieldname, int value)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->AddFieldInt(fieldname, value);
	}
	else
	{
		return -1;
	}
}

// ��Ӹ���������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_AddFieldDouble(void * handle, const char * fieldname, double value)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->AddFieldDouble(fieldname, value);
	}
	else
	{
		return -1;
	}
}

// �����������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_AddFieldPwd(void * handle, const char * fieldname, const char * value)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->AddFieldPwd(fieldname, value);
	}
	else
	{
		return -1;
	}
}

// ��Ӷ���������
// ����ֵ��			>=0�ɹ� <0ʧ��
int YTData_AddFieldBinary(void * handle, const char * fieldname, const char * value, int len)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->AddFieldBinary(fieldname, value, len);
	}
	else
	{
		return -1;
	}
}

// �����ֶ�ֵ����ָ������
// ����ֵ��			>=0��ʾָ����¼λ�� <0δ�ҵ���Ӧ�ļ�¼
int YTData_SearchByField(void * handle, const char * fieldname, const char * value)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		return pYTStep->SearchByField(fieldname, value);
	}
	else
	{
		return -1;
	}
}
