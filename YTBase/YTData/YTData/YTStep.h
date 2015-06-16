#pragma once

#include "../../../YTPublic/YTInclude/YTBase/YTData.h"
#include "YTStepSec.h"
#include <hash_map>

// 字符串比较函数
struct CharLess : public binary_function<const char *, const char *, bool>
{
public:
	result_type operator()(const first_argument_type& _Left, const second_argument_type& _Right) const
	{
		return(stricmp(_Left, _Right) < 0 ? true : false);
	}
};

// 字段信息
typedef struct
{
	BYTE	FieldType;	// 字段类型，参见YTData.h中ENum_FieldType字段类型定义
	BYTE	count;		// 字段名长度
	char	data[];		// 字段名信息
} DataColumn;

// 整体数据信息
class CYTStep
{
public:
	CYTStep(void);
	~CYTStep(void);

// 公共导出函数
public:
	void ResetData(void);													// 重置数据
	int CopyData(CYTStep * sourcestep, int startindex, int endindex);		// 数据包copy
	int AppendData(CYTStep * sourcestep);									// 数据包合并
	
	int ConvertData(const char * data, int len);							// 转换数据缓存：该函数主要将传入缓存数据结构转换成标准数据结构
	const char * MakeSendData(int * len);									// 生成数据缓存：该函数主要将数据结构转换成缓存信息以便发送

	void SetRetCode(int retcode);											// 设置retcode返回值，用于标志业务是否成功
	int GetRetCode(void);													// 获取retcode返回值
	void SetRetMsg(const char* pMsg);										// 设置comment返回值，当业务失败时有效，为失败信息
	int GetRetMsg(char * outbuf, int len);									// 获取comment返回值

	int GetRowCount(void);													// 获取记录条数
	int InsertRow(int rowindex);											// 在指定位置插入一条记录
	int DeleteRow(int rowindex);											// 删除一条指定记录
	int AppendRow(void);													// 在最后增加一条记录

	int GotoBeginRow(void);													// 转到第一条记录
	int GotoEndRow(void);													// 转到最后一条记录
	int GotoPreRow(void);													// 转到上一条记录
	int GotoNextRow(void);													// 转到下一条记录
	int GotoFixRow(int rowindex);											// 转到指定的某条记录

	int GetFieldCount(void);												// 获取字段个数
	int GetFieldType(int fieldindex);										// 获取字段类型
	const char * GetFieldName(int fieldindex);								// 获取字段名
	int AddField(const char * fieldname, int fieldtype);					// 添加一个字段
	int DeleteField(const char * fieldname);								// 删除一个字段

	bool IsNullValue(const char * fieldname);								// 判断字段值是否为空

	int GetFieldString(const char * fieldname, char * outbuf, int len);		// 获取字段串型数据
	char GetFieldChar(const char * fieldname);								// 获取字符型数据
	int GetFieldInt(const char * fieldname);								// 获取整型数据
	double GetFieldDouble(const char * fieldname);							// 获取浮点型数据
	int GetFieldPwd(const char * fieldname, char * outpwd, int pwdlen);		// 获取密码数据
	int GetFieldBinary(const char * fieldname, char * outbuf, int * outlen);// 获取二进制数据

	int AddFieldString(const char * fieldname, const char * value);			// 添加字段串型数据
	int AddFieldChar(const char * fieldname, const char value);				// 添加字符型数据
	int AddFieldInt(const char * fieldname, int value);						// 添加整型数据
	int AddFieldDouble(const char * fieldname, double value);				// 添加浮点型数据
	int AddFieldPwd(const char * fieldname, const char * value);			// 添加密码数据
	int AddFieldBinary(const char * fieldname, const char * value, int len);// 添加二进制数据

	int SearchByField(const char * fieldname, const char * value);			// 根据字段值查找某条指定记录
	
protected:
	int GetColumnInfoLen(void);												// 获取字段信息长度
	int	GetRowInfoLen(void);												// 获取数值信息长度
	void GetDataBytesCount(int &totallen, int &columnlen, int &rowlen);		// 获取数值信息长度
	
// 受保护访问变量
protected:
	vector<DataColumn *>		m_vectfield;		// 字段信息

	hash_map<const char *, int, hash_compare<const char *, CharLess> >	m_mapfield;		// 字段索引位置
	vector<CYTStepSec *> 		m_vectfieldvalue;	// 每个字段对应的一系列数据
	
	int							m_totalrow;			// 总记录数
	CYTLock						m_section;			// 记录控制锁

// 私有成员变量
private:
	int							m_retcode;			// 标志业务是否成功，>=0成功 <0失败
	string						m_retmsg;			// 业务失败时有效，为失败信息

	int							m_currrow;			// 当前记录行
	BYTE						m_crypekey[9];		// 密码加密密钥，由服务器动态生成返回
	char *						m_pbuffer;			// 生成数据缓存：生成发送数据缓存时有用
};