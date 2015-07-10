// ClientSimulatorDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "data.h"
#include <vector>
#include <map>
#include "TraderAPI.h"

// CClientSimulatorDlg �Ի���
class CClientSimulatorDlg : public CDialog
{
// ����
public:
	CClientSimulatorDlg(CWnd* pParent = NULL);	// ��׼���캯��

	void ReadXmlInfo();

// �Ի�������
	enum { IDD = IDD_CLIENTSIMULATOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	static void thdSendmsg(void *par);
	void RefreshStatistic(int iType);
	void InitStatisticData();
	//static void CallAnsProcess(int iFuncNO, int iReqNO, int iErrID, char* sErrMsg);

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_sEndTime;
	CString m_sBeginTime;
	CListCtrl m_lst_Rpt;
	CString m_sTmSpan;
	

private:
	ClientInfo m_Client;
	std::vector<FuncReqInfo*> m_vctReqInfo;

	TraderAPI* m_pTrade;
	bool m_isInitSucess;
	static int m_ithds;
	static CRITICAL_SECTION m_mutexCS_Thd;

	static bool m_isStop;  //�Ƿ�ֹͣ����
	static HANDLE m_stopevent;
	
public:
	afx_msg void OnBnClickedbtnstart();
public:
	afx_msg void OnBnClickedBtnLogon();
	afx_msg void OnClose();
	CEdit m_editTimespan;
public:
	afx_msg void OnBnClickedbtnstop();
  afx_msg void OnEnSetfocusCount();
};
