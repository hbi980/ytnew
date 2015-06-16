/*
* 数据结构定义如下
*			字段个数<SOH>记录个数<SOH><SOA>
*			字段1=字段类型<SOH>字段2=字段类型<SOH>...字段n=字段类型<SOH><SOA>
*			数值1<SOH>数值2<SOH>...数值n<SOH><SOA>
*			数值1<SOH>数值2<SOH>...数值n<SOH><SOA>
*			.....................................
*			数值1<SOH>数值2<SOH>...数值n<SOH><SOA>
*			
*			其中字段类型定义如下：
*			0-字符串型
*			1-字符型
*			2-整型（BYTE型数据目前归于整型）
*			3-浮点型
*			4-密码数据（采用blowfish加密然后经base64转换后存储）
*			5-二进制数据（二进制数据采用base64转换后存储）
*/
#include "YTStep.h"

// 每次修改定义版本信息，版本记录修改信息在YTData.h文件头部写明
#define MAIN_VER	"1.0"

// 静态全局变量，用于防止重复初始化和析构
static bool g_IsExist = false;

// 获取版本：“V1.0”格式
const char * YTData_GetVer(void)
{
	return MAIN_VER;
}

// DLL初始化
// 返回值：			>=0成功 <0失败
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

// DLL关闭
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

// ASCI转成UNICODE
// YTDATA操作取出的数据可以通过本函数转换成宽字符，主要针对fieldname还有string、char方式的fieldvalue
// 返回值：>=0转换后的长度 <0长度不够（返回-100表示长度不够需要100）
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

// UNICODE转成ASCI
// 如果源数据未UNICODE，可以通过本函数转换成ASCI后通过YTData操作，主要针对fieldname还有string、char方式的fieldvalue
// 返回值：>=0转换后的长度 <0长度不够（返回-100表示长度不够需要100）
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

// 创建句柄
// 1、该句柄标志数据信息，下面所有的和数据相关的函数请求参数中都需要传入该句柄信息
// 2、如果需要将数据包的数据全部清空做初始化则调用ResetData函数即可
// 3、返回的句柄外围不用的话则调用下面的DeleteData函数释放，外围不要做delete操作
// 返回值：			NULL失败，其他成功
void * YTData_NewData(void)
{
	CYTStep * ret = new CYTStep;
	return ret;
}

// 重置数据
void YTData_ResetData(void * handle)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		pYTStep->ResetData();
	}
}

// 删除句柄
void YTData_DeleteData(void * handle)
{
	CYTStep * pYTStep = (CYTStep *)handle;
	if(pYTStep != NULL)
	{
		delete pYTStep;
		pYTStep = NULL;
	}
}

// 数据包copy，入参sourcehandle表示源数据包句柄
// 返回值：			>=0成功 <0失败
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

// 数据合并，入参sourcehandle表示源数据包句柄
// 返回值：			>=0成功 <0失败
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

// 转换数据缓存：该函数主要将传入缓存数据转换成标准数据结构
// 通讯数据传输后得到的缓存通过本函数转换成指定的数据结构
// 返回值：			>=0成功 <0失败
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

// 生成数据缓存：该函数主要将数据结构转换成缓存信息以便发送
// 注意：这里返回的数据缓存DLL内部管理，外围不要做delete操作
// 返回值：			返回缓存指针，为NULL表示失败，len返回缓存长度
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

// 设置返回值
// 返回值：			>=0成功 <0失败
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

// 设置返回信息
// 返回值：			>=0成功 <0失败
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

// 获取返回值
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

// 获取返回信息
// 返回值：			>=0成功 <0失败
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

// 获取记录条数
// 返回值：			>=0记录个数 <0失败
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

// 在指定位置插入一条记录
// 返回值：			>=0成功 <0失败
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

// 删除一条指定记录
// 返回值：			>=0成功 <0失败
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

// 在最后增加一条记录
// 返回值：			>=0成功 <0失败
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

// 转到第一条记录
// 返回值：			>=0成功 <0失败
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

// 转到最后一条记录
// 返回值：			>=0成功 <0失败
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

// 转到上一条记录
// 返回值：			>=0成功 <0失败
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

// 转到下一条记录
// 返回值：			>=0成功 <0失败
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

// 转到指定的某条记录
// 返回值：			>=0成功 <0失败
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

// 获取字段个数
// 返回值：			>=0字段个数 <0失败
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

// 获取字段类型
// 返回值：			>=0字段类型（参见上面ENum_FieldType定义） <0失败
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

// 获取字段名
// 返回值：			NULL失败，其他标示字段名
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

// 添加一个字段
// fieldtype为字段类型，具体参见上面ENum_FieldType定义
// 返回值：			>=0成功 <0失败
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

// 删除一个字段
// 返回值：			>=0成功 <0失败
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

// 判断字段值是否为空
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

// 获取字符串型数据
// 返回值：			>=0成功 <0失败
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

// 获取字符型数据
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

// 获取整型数据
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

// 获取浮点型数据
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

// 获取密码数据
// 返回值：			>=0成功 <0失败
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

// 获取二进制数据
// 返回值：			>=0成功 <0失败
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

// 添加字符串型数据
// 返回值：			>=0成功 <0失败
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

// 添加字符型数据
// 返回值：			>=0成功 <0失败
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

// 添加整型数据
// 返回值：			>=0成功 <0失败
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

// 添加浮点型数据
// 返回值：			>=0成功 <0失败
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

// 添加密码数据
// 返回值：			>=0成功 <0失败
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

// 添加二进制数据
// 返回值：			>=0成功 <0失败
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

// 根据字段值查找指定数据
// 返回值：			>=0表示指定记录位置 <0未找到对应的记录
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
