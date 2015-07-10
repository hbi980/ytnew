// ClientSimulatorDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "include/Tinyxml.h"
#include <process.h>
#include <afxmt.h>
#include <Windows.h>
#include <time.h>
#include "basefuncs.h"
#include "ClientSimulator.h"
#include "ClientSimulatorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClientSimulatorDlg �Ի���

#pragma comment(lib,"lib/tinyxmld.lib")

extern std::vector<FuncAnsInfo*> g_vctAnsInfo;
extern HANDLE g_event;
extern int g_iUserID;
extern std::map<int,int> g_mapFuncID2RowID;


int CClientSimulatorDlg::m_ithds = 0;
bool CClientSimulatorDlg::m_isStop = false; 
HANDLE CClientSimulatorDlg::m_stopevent = CreateEvent(NULL, false, true, NULL);

CRITICAL_SECTION CClientSimulatorDlg::m_mutexCS_Thd;
CClientSimulatorDlg::CClientSimulatorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClientSimulatorDlg::IDD, pParent)
	, m_sBeginTime(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	g_mapFuncID2RowID.clear();
	m_pTrade = NULL;
	m_isInitSucess = false;
	m_ithds = 0;
	InitializeCriticalSection(&m_mutexCS_Thd);
}

void CClientSimulatorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TM_END, m_sEndTime);
	DDX_Text(pDX, IDC_TM_BEGIN, m_sBeginTime);
	DDX_Control(pDX, IDC_list_rpt, m_lst_Rpt);
	DDX_Text(pDX, IDC_timespan, m_sTmSpan);
	DDX_Control(pDX, IDC_timespan, m_editTimespan);
  DDX_Text(pDX, IDC_Count, iCurConnects);
}

BEGIN_MESSAGE_MAP(CClientSimulatorDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_btn_start, &CClientSimulatorDlg::OnBnClickedbtnstart)
	ON_BN_CLICKED(IDC_BTN_LOGON, &CClientSimulatorDlg::OnBnClickedBtnLogon)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_btn_stop, &CClientSimulatorDlg::OnBnClickedbtnstop)
  ON_EN_SETFOCUS(IDC_Count, &CClientSimulatorDlg::OnEnSetfocusCount)
END_MESSAGE_MAP()


// CClientSimulatorDlg ��Ϣ�������

BOOL CClientSimulatorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	DWORD dwStyle = m_lst_Rpt.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
	dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��
	//dwStyle |= LVS_EX_CHECKBOXES;//itemǰ����checkbox�ؼ�
	m_lst_Rpt.SetExtendedStyle(dwStyle); //������չ���

	m_lst_Rpt.InsertColumn(0,"FuncID",LVCFMT_LEFT, 60 );
	m_lst_Rpt.InsertColumn(1,"����Ƶ��",LVCFMT_LEFT, 80 );
	m_lst_Rpt.InsertColumn(2,"�ѷ�����",LVCFMT_LEFT, 80 );
	m_lst_Rpt.InsertColumn(3,"��������",LVCFMT_LEFT, 80 );
	m_lst_Rpt.InsertColumn(4,"���Ӧ��ʱ��(΢��)",LVCFMT_LEFT, 120 );
	m_lst_Rpt.InsertColumn(5,"ƽ��Ӧ��ʱ��(΢��)",LVCFMT_LEFT, 120 );
	
	m_isStop = false;
	m_Client.isLogon = false;
	m_Client.isConnected = false;
	ReadXmlInfo();
	char sbuf[512];
	for (int i=0;i<m_vctReqInfo.size();i++)
	{
		m_lst_Rpt.InsertItem(i,"");
		_itoa(m_vctReqInfo[i]->iFuncID,sbuf,10);
		m_lst_Rpt.SetItemText(i,0,sbuf);
		g_mapFuncID2RowID.insert(std::make_pair(m_vctReqInfo[i]->iFuncID,i));
		
		_itoa(m_vctReqInfo[i]->iTimes,sbuf,10);
		m_lst_Rpt.SetItemText(i,1,sbuf);
	}
	
	g_event = CreateEvent(NULL, false, false, NULL);
	if (m_pTrade == NULL)
	{
		m_pTrade = new TraderAPI();
	}
	char SerrMsg[1024] = {0};
	char filenameBuf[512] = {0};
	GetCurrentDirectory(sizeof(sbuf),sbuf);
	sprintf_s(filenameBuf,sizeof(filenameBuf), "%s\\server.ini",sbuf);
	strcat(sbuf,"\\log");

	if (m_pTrade->InitTrade(filenameBuf,sbuf,SerrMsg, 1024))
	{
		m_Client.isConnected = true;
		m_isInitSucess = true;
	}
	else
	{
		m_isInitSucess = false;
		MessageBox("���׽ӿڳ�ʼ��ʧ�ܣ�");
	}
	
	return TRUE; 
}

void CClientSimulatorDlg::ReadXmlInfo()
{
	TiXmlDocument* xmldoc = new TiXmlDocument();
	if (xmldoc == NULL )
	{
		return;
	}
	if (xmldoc->LoadFile("pressure.xml") == false)
	{
		MessageBox(xmldoc->ErrorDesc());
		delete xmldoc;
		xmldoc = NULL;
		return;
	}
	TiXmlElement* xeroot = xmldoc->RootElement();  //Class
	TiXmlNode* xnClient = xeroot->FirstChild("Client");
	m_Client.sUser = xnClient->FirstChildElement("user")->Attribute("ID");
	m_Client.sPwd = xnClient->FirstChildElement("user")->Attribute("pwd");
	TiXmlNode* xnFunc = xnClient->FirstChild("Func");
	while(xnFunc != NULL)
	{
		FuncReqInfo* fcinfo = new FuncReqInfo();
		fcinfo->iFuncID = atoi(xnFunc->ToElement()->Attribute("funcid"));
		fcinfo->iTimeSpan = atoi(xnFunc->FirstChildElement("Base")->Attribute("Timespan"));
		fcinfo->iTimes = atoi(xnFunc->FirstChildElement("Base")->Attribute("Times"));
		TiXmlNode* xndata = xnFunc->FirstChildElement("data");
		if (!xndata->NoChildren())  //���������
		{
			TiXmlElement* xePara = xndata->FirstChildElement("para");
			while(xePara != NULL)
			{
				fcinfo->sParaNameArry.Add(xePara->FirstAttribute()->Name());
				fcinfo->sParaValuesArry.Add(xePara->FirstAttribute()->Value());
				xePara = xePara->NextSiblingElement();
			}
		}
		m_vctReqInfo.push_back(fcinfo);

		FuncAnsInfo* ansInfo = new FuncAnsInfo();
		ansInfo->iFuncID = fcinfo->iFuncID;
		g_vctAnsInfo.push_back(ansInfo);
		xnFunc = xnFunc->NextSibling();
	}

	delete xmldoc;
	xnFunc = NULL;
	xmldoc = NULL;
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CClientSimulatorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
//
HCURSOR CClientSimulatorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//unsigned _stdcall CClientSimulatorDlg::thdSendmsg(void *par)
void CClientSimulatorDlg::thdSendmsg(void *par)
{
	
	if (par == NULL)
	{
		EnterCriticalSection(&m_mutexCS_Thd);
		m_ithds--;
		LeaveCriticalSection(&m_mutexCS_Thd);
		return;
	}

	FuncReqInfo* pReqInfo = (FuncReqInfo*)par;

	void * pPara = NULL;
	switch(pReqInfo->iFuncID)
	{
	case QURELATED:
		pPara = new tagTradeReqQuRelated();
		((tagTradeReqQuRelated*)pPara)->UserID = g_iUserID;
		break;
	case QUMONEY:
		pPara = new tagTradeReqQuMoney();
		((tagTradeReqQuMoney*)pPara)->UserID = g_iUserID;
		((tagTradeReqQuMoney*)pPara)->AccID = atoi(pReqInfo->sParaValuesArry[0]);
		break;
	case QUAMOUNT:
		pPara = new tagTradeReqQuAmount();
		((tagTradeReqQuAmount*)pPara)->UserID = g_iUserID;
		((tagTradeReqQuAmount*)pPara)->AccID = atoi(pReqInfo->sParaValuesArry[0]);
		strcpy_s(((tagTradeReqQuAmount*)pPara)->InstrumentID,pReqInfo->sParaValuesArry[1].GetBuffer(0));
		strcpy_s(((tagTradeReqQuAmount*)pPara)->ExchangeID,pReqInfo->sParaValuesArry[2].GetBuffer(0));
		break;
	case QUUNFINISHENTRUST:
		pPara = new tagTradeReqQuUnFinishEntrust();
		((tagTradeReqQuUnFinishEntrust*)pPara)->UserID = g_iUserID;
		((tagTradeReqQuUnFinishEntrust*)pPara)->LastUpdateTime = atoi(pReqInfo->sParaValuesArry[0]);    //������ע��
		break;
	case QULOG:
		pPara = new tagTradeReqQuLog();
		((tagTradeReqQuLog*)pPara)->UserID = g_iUserID;
		((tagTradeReqQuLog*)pPara)->Index = atoi(pReqInfo->sParaValuesArry[0]);    //������ע��
		break;
	case QUPOSITION:
		pPara = new tagTradeReqQuPosition();
		((tagTradeReqQuPosition*)pPara)->UserID = g_iUserID;
		((tagTradeReqQuPosition*)pPara)->AccID = atoi(pReqInfo->sParaValuesArry[0]);
		break;
	case QUBARGAIN:
		pPara = new tagTradeReqQuBargain();
		((tagTradeReqQuBargain*)pPara)->UserID = g_iUserID;
		((tagTradeReqQuBargain*)pPara)->AccID = atoi(pReqInfo->sParaValuesArry[0]);
		break;
	case QUCOLLECTBARGAIN:
		pPara = new tagTradeReqQuCollectBargain();
		((tagTradeReqQuCollectBargain*)pPara)->UserID = g_iUserID;
		((tagTradeReqQuCollectBargain*)pPara)->BeginDate = atoi(pReqInfo->sParaValuesArry[0]);
		((tagTradeReqQuCollectBargain*)pPara)->EndDate = atoi(pReqInfo->sParaValuesArry[1]);
		break;
	case ENTRUST:
		pPara = new tagTradeReqEntrust();
		((tagTradeReqEntrust*)pPara)->UserID = g_iUserID;
		((tagTradeReqEntrust*)pPara)->AccID = atoi(pReqInfo->sParaValuesArry[0]);
		strcpy_s(((tagTradeReqEntrust*)pPara)->InstrumentID,pReqInfo->sParaValuesArry[1].GetBuffer(0));
		strcpy_s(((tagTradeReqEntrust*)pPara)->InstrumentName,pReqInfo->sParaValuesArry[2].GetBuffer(0));
		strcpy_s(((tagTradeReqEntrust*)pPara)->ExchangeID,pReqInfo->sParaValuesArry[3].GetBuffer(0));
		((tagTradeReqEntrust*)pPara)->EntrustType = pReqInfo->sParaValuesArry[4].GetBuffer(0)[0];
		((tagTradeReqEntrust*)pPara)->EntrustPrice = atof(pReqInfo->sParaValuesArry[5]);
		((tagTradeReqEntrust*)pPara)->EntrustAmount = atoi(pReqInfo->sParaValuesArry[6]);
		break;
	//case DISENTRUST:
	//	break;
	case DISLASTENTRUST:
		pPara = new tagTradeReqDisLastEntrust();
		((tagTradeReqDisLastEntrust*)pPara)->UserID = g_iUserID;
		((tagTradeReqDisLastEntrust*)pPara)->AccID = atoi(pReqInfo->sParaValuesArry[0]);
		strcpy_s(((tagTradeReqDisLastEntrust*)pPara)->InstrumentID,pReqInfo->sParaValuesArry[1].GetBuffer(0));
		strcpy_s(((tagTradeReqDisLastEntrust*)pPara)->ExchangeID,pReqInfo->sParaValuesArry[2].GetBuffer(0));
		break;
	case DISALLENTRUST:
		pPara = new tagTradeReqDisAllEntrust();
		((tagTradeReqDisAllEntrust*)pPara)->UserID = g_iUserID;
		((tagTradeReqDisAllEntrust*)pPara)->AccID = atoi(pReqInfo->sParaValuesArry[0]);
		strcpy_s(((tagTradeReqDisAllEntrust*)pPara)->InstrumentID,pReqInfo->sParaValuesArry[1].GetBuffer(0));
		strcpy_s(((tagTradeReqDisAllEntrust*)pPara)->ExchangeID,pReqInfo->sParaValuesArry[2].GetBuffer(0));
		break;
	default:
		break;
	}
	int iSpan = pReqInfo->iTimeSpan*1000/pReqInfo->iTimes;

	char sErrmsg[1024] = {0};
	int i=0;
	int iReqNO;
	double dBeginTime = 0;
	HANDLE hdTimeout = CreateEvent(NULL, false, false, NULL);
	while(i< pReqInfo->iTotalTimes)
	{
		if (m_isStop)
		{
			break;
		}
		dBeginTime = GetTickCountEX();
		switch(pReqInfo->iFuncID)
		{
		case QURELATED:
			iReqNO = Trade_QuRelated((tagTradeReqQuRelated*)pPara,sErrmsg,1024);
			break;
		case QUMONEY:
			iReqNO = Trade_QuMoney((tagTradeReqQuMoney*)pPara,sErrmsg,1024);
			break;
		case QUAMOUNT:
			iReqNO = Trade_QuAmount((tagTradeReqQuAmount*)pPara,sErrmsg,1024);
			break;
		case QUUNFINISHENTRUST:
			iReqNO = Trade_QuUnFinishEntrust((tagTradeReqQuUnFinishEntrust*)pPara,sErrmsg,1024);
			break;
		case QULOG:
			iReqNO = Trade_QuLog((tagTradeReqQuLog*)pPara,sErrmsg,1024);
			break;
		case QUPOSITION:
			iReqNO = Trade_QuPosition((tagTradeReqQuPosition*)pPara,sErrmsg,1024);
			pPara = new tagTradeReqQuPosition();
			break;
		case QUBARGAIN:
			iReqNO = Trade_QuBargain((tagTradeReqQuBargain*)pPara,sErrmsg,1024);
			break;
		case QUCOLLECTBARGAIN:
			iReqNO = Trade_QuCollectBargain((tagTradeReqQuCollectBargain*)pPara,sErrmsg,1024);
			break;
		case ENTRUST:
			iReqNO = Trade_Entrust((tagTradeReqEntrust*)pPara,sErrmsg,1024);
			break;
			//case DISENTRUST:
			//	break;
		case DISLASTENTRUST:
			iReqNO = Trade_DisLastEntrust((tagTradeReqDisLastEntrust*)pPara,sErrmsg,1024);
			break;
		case DISALLENTRUST:
			iReqNO = Trade_DisAllEntrust((tagTradeReqDisAllEntrust*)pPara,sErrmsg,1024);
			break;
		default:
			break;
		}
		pReqInfo->mapReqNO2ReqTm.insert(std::make_pair(iReqNO,dBeginTime));
		pReqInfo->iReqTimes++;
		WaitForSingleObject(hdTimeout, iSpan);
		i++;
	}
	delete pPara;

	EnterCriticalSection(&m_mutexCS_Thd);
	m_ithds--;
	if (m_ithds == 0)  //�Ѿ��˳����в����߳���
	{
		SetEvent(m_stopevent);
	}
	LeaveCriticalSection(&m_mutexCS_Thd);
}

void CClientSimulatorDlg::OnBnClickedbtnstart()
{
	if (m_Client.isConnected == false)
	{
		MessageBox("��δ��������!\n");
		return;
	}
	if (m_Client.isLogon == false)
	{
		MessageBox("����Աδ��½!\n");
		return;
	}

	ResetEvent(m_stopevent);
	InitStatisticData();
	//((CButton*)GetDlgItem(IDC_btn_start))->EnableWindow(FALSE);
	
	m_editTimespan.GetWindowText(m_sTmSpan);
	for (int i=0;i< m_vctReqInfo.size(); i++)
	{
		if (m_vctReqInfo[i]->iTimes >0 && m_vctReqInfo[i]->iTimeSpan > 0)
		{
			int iSpan = atoi(m_sTmSpan.GetBuffer());
			m_vctReqInfo[i]->iTotalTimes = iSpan / m_vctReqInfo[i]->iTimeSpan * m_vctReqInfo[i]->iTimes;
			_beginthread(thdSendmsg,0,m_vctReqInfo[i]);
			EnterCriticalSection(&m_mutexCS_Thd);
			m_ithds++;
			LeaveCriticalSection(&m_mutexCS_Thd);
		}
		
	}

	//���̸߳�������Ӧ�����ݣ��Լ�������־
	HANDLE hdTimeout = CreateEvent(NULL, false, false, NULL);
	while(m_ithds > 0)
	{
		RefreshStatistic(0);
		WaitForSingleObject(hdTimeout, 1000);
	}
	//sleepһ����ʱʱ�䣬��һ����������Ӧ��
	Sleep(10*1000);
	RefreshStatistic(1);

}

void CClientSimulatorDlg::InitStatisticData()
{
	if (m_vctReqInfo.size() <= 0)
	{
		return;
	}
	for (int i=0; i<m_vctReqInfo.size();i++)
	{
		m_vctReqInfo[i]->iReqTimes = 0;
		m_vctReqInfo[i]->iTotalTimes = 0;
		m_vctReqInfo[i]->mapReqNO2ReqTm.clear();

		g_vctAnsInfo[i]->iAnsTimes = 0;
		g_vctAnsInfo[i]->iErrTimes = 0;
		g_vctAnsInfo[i]->mapReqNO2AndTm.clear();
	}
}

//itype=0 -- ֻͳ������/Ӧ�������itype=1 -- ͳ������Ӧ��ʱ�� 
void CClientSimulatorDlg::RefreshStatistic(int iType)
{
	char sbuf[16] = {0};
	for (int i=0;i<m_lst_Rpt.GetItemCount();i++)
	{
		FuncReqInfo* pReqInfo = m_vctReqInfo[i];
		if (pReqInfo->iTotalTimes <= 0)
		{
			break;
		}
		
		FuncAnsInfo* pAnsInfo = g_vctAnsInfo[i];
		sprintf_s(sbuf,"%d",pReqInfo->iReqTimes);
		m_lst_Rpt.SetItemText(i,2,sbuf);
		sprintf_s(sbuf,"%d",pAnsInfo->iAnsTimes);
		m_lst_Rpt.SetItemText(i,3,sbuf);
		
		if (iType == 1)
		{
			int iMaxReqTime = 0;
			int iAvgReqTime = 0;
			int iTotalTime = 0;
			int iUseTime = 0;
			std::map<int,double>::iterator iter = pReqInfo->mapReqNO2ReqTm.begin();
			std::map<int,double>::iterator iter_ans;
			for (;iter != pReqInfo->mapReqNO2ReqTm.end();iter++)
			{
				iter_ans = pAnsInfo->mapReqNO2AndTm.find(iter->first);
				if (iter_ans != pAnsInfo->mapReqNO2AndTm.end())
				{
					iUseTime = iter_ans->second - iter->second;
					if (iMaxReqTime <  iUseTime)
					{
						iMaxReqTime = iUseTime;
					}
					iTotalTime += iUseTime;
				}
			}
			if ( pAnsInfo->iAnsTimes != 0)
			{
				iAvgReqTime = iTotalTime / pAnsInfo->iAnsTimes;
			}
			else
				iAvgReqTime = 0;
			
			sprintf_s(sbuf,"%d",iMaxReqTime);
			m_lst_Rpt.SetItemText(i,4,sbuf);
			sprintf_s(sbuf,"%d",iAvgReqTime);
			m_lst_Rpt.SetItemText(i,5,sbuf);
		}
	}
	m_lst_Rpt.UpdateData(true);

	
}
void CClientSimulatorDlg::OnBnClickedBtnLogon()
{
	this->EnableWindow(FALSE);
	char SerrMsg[1024] = {0};
	if (m_isInitSucess)
	{
		tagTradeReqLogin objLogin;
		strcpy_s(objLogin.User, sizeof(objLogin.User), m_Client.sUser);
		strcpy_s(objLogin.Password, sizeof(objLogin.Password), m_Client.sPwd);
		int rtn = Trade_Login(&objLogin, SerrMsg, 1024);
		if (rtn < 0)
		{
			MessageBox("��½ʧ��\n");
		}
		else
		{
			DWORD dResult = ::WaitForSingleObject(g_event,10*1000);
			if (dResult == WAIT_OBJECT_0)
			{
				m_Client.isLogon = true;
				MessageBox("��½�ɹ�\n");
			}
			else
			{
				MessageBox("��ʱ����½ʧ�ܣ�\n");
			}
      UpdateData(FALSE);
		}
	}
	else
	{
		MessageBox("���׽ӿڳ�ʼ��ʧ�ܣ�");
	}

	this->EnableWindow(TRUE);
}

void CClientSimulatorDlg::OnClose()
{
	delete m_pTrade;
	m_isStop = true;

	WaitForSingleObject(m_stopevent,10*1000);  //����10��

	for (int i =0; i<m_vctReqInfo.size();i++)
	{
		FuncReqInfo * pReq = m_vctReqInfo[i];
		delete pReq;
		FuncAnsInfo* pAns = g_vctAnsInfo[i];
		delete pAns;
	}
	m_vctReqInfo.clear();
	g_vctAnsInfo.clear();
	CDialog::OnClose();
}

void CClientSimulatorDlg::OnBnClickedbtnstop()
{
	m_isStop = true;
}


void CClientSimulatorDlg::OnEnSetfocusCount()
{
  UpdateData(FALSE);
}
