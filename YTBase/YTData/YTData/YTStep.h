#pragma once

#include "../../../YTPublic/YTInclude/YTBase/YTData.h"
#include "YTStepSec.h"
#include <hash_map>

// �ַ����ȽϺ���
struct CharLess : public binary_function<const char *, const char *, bool>
{
public:
	result_type operator()(const first_argument_type& _Left, const second_argument_type& _Right) const
	{
		return(stricmp(_Left, _Right) < 0 ? true : false);
	}
};

// �ֶ���Ϣ
typedef struct
{
	BYTE	FieldType;	// �ֶ����ͣ��μ�YTData.h��ENum_FieldType�ֶ����Ͷ���
	BYTE	count;		// �ֶ�������
	char	data[];		// �ֶ�����Ϣ
} DataColumn;

// ����������Ϣ
class CYTStep
{
public:
	CYTStep(void);
	~CYTStep(void);

// ������������
public:
	void ResetData(void);													// ��������
	int CopyData(CYTStep * sourcestep, int startindex, int endindex);		// ���ݰ�copy
	int AppendData(CYTStep * sourcestep);									// ���ݰ��ϲ�
	
	int ConvertData(const char * data, int len);							// ת�����ݻ��棺�ú�����Ҫ�����뻺�����ݽṹת���ɱ�׼���ݽṹ
	const char * MakeSendData(int * len);									// �������ݻ��棺�ú�����Ҫ�����ݽṹת���ɻ�����Ϣ�Ա㷢��

	void SetRetCode(int retcode);											// ����retcode����ֵ�����ڱ�־ҵ���Ƿ�ɹ�
	int GetRetCode(void);													// ��ȡretcode����ֵ
	void SetRetMsg(const char* pMsg);										// ����comment����ֵ����ҵ��ʧ��ʱ��Ч��Ϊʧ����Ϣ
	int GetRetMsg(char * outbuf, int len);									// ��ȡcomment����ֵ

	int GetRowCount(void);													// ��ȡ��¼����
	int InsertRow(int rowindex);											// ��ָ��λ�ò���һ����¼
	int DeleteRow(int rowindex);											// ɾ��һ��ָ����¼
	int AppendRow(void);													// ���������һ����¼

	int GotoBeginRow(void);													// ת����һ����¼
	int GotoEndRow(void);													// ת�����һ����¼
	int GotoPreRow(void);													// ת����һ����¼
	int GotoNextRow(void);													// ת����һ����¼
	int GotoFixRow(int rowindex);											// ת��ָ����ĳ����¼

	int GetFieldCount(void);												// ��ȡ�ֶθ���
	int GetFieldType(int fieldindex);										// ��ȡ�ֶ�����
	const char * GetFieldName(int fieldindex);								// ��ȡ�ֶ���
	int AddField(const char * fieldname, int fieldtype);					// ���һ���ֶ�
	int DeleteField(const char * fieldname);								// ɾ��һ���ֶ�

	bool IsNullValue(const char * fieldname);								// �ж��ֶ�ֵ�Ƿ�Ϊ��

	int GetFieldString(const char * fieldname, char * outbuf, int len);		// ��ȡ�ֶδ�������
	char GetFieldChar(const char * fieldname);								// ��ȡ�ַ�������
	int GetFieldInt(const char * fieldname);								// ��ȡ��������
	double GetFieldDouble(const char * fieldname);							// ��ȡ����������
	int GetFieldPwd(const char * fieldname, char * outpwd, int pwdlen);		// ��ȡ��������
	int GetFieldBinary(const char * fieldname, char * outbuf, int * outlen);// ��ȡ����������

	int AddFieldString(const char * fieldname, const char * value);			// ����ֶδ�������
	int AddFieldChar(const char * fieldname, const char value);				// ����ַ�������
	int AddFieldInt(const char * fieldname, int value);						// �����������
	int AddFieldDouble(const char * fieldname, double value);				// ��Ӹ���������
	int AddFieldPwd(const char * fieldname, const char * value);			// �����������
	int AddFieldBinary(const char * fieldname, const char * value, int len);// ��Ӷ���������

	int SearchByField(const char * fieldname, const char * value);			// �����ֶ�ֵ����ĳ��ָ����¼
	
protected:
	int GetColumnInfoLen(void);												// ��ȡ�ֶ���Ϣ����
	int	GetRowInfoLen(void);												// ��ȡ��ֵ��Ϣ����
	void GetDataBytesCount(int &totallen, int &columnlen, int &rowlen);		// ��ȡ��ֵ��Ϣ����
	
// �ܱ������ʱ���
protected:
	vector<DataColumn *>		m_vectfield;		// �ֶ���Ϣ

	hash_map<const char *, int, hash_compare<const char *, CharLess> >	m_mapfield;		// �ֶ�����λ��
	vector<CYTStepSec *> 		m_vectfieldvalue;	// ÿ���ֶζ�Ӧ��һϵ������
	
	int							m_totalrow;			// �ܼ�¼��
	CYTLock						m_section;			// ��¼������

// ˽�г�Ա����
private:
	int							m_retcode;			// ��־ҵ���Ƿ�ɹ���>=0�ɹ� <0ʧ��
	string						m_retmsg;			// ҵ��ʧ��ʱ��Ч��Ϊʧ����Ϣ

	int							m_currrow;			// ��ǰ��¼��
	BYTE						m_crypekey[9];		// ���������Կ���ɷ�������̬���ɷ���
	char *						m_pbuffer;			// �������ݻ��棺���ɷ������ݻ���ʱ����
};