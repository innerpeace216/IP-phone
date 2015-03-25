IpPhoneDlg.cpp
#include "stdafx.h"
#include "IpPhone.h"
#include "IpPhoneDlg.h"
#include <WS2tcpip.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
////////////////////////////////////////////////////////////////////////////
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    

protected:

	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////
CIpPhoneDlg::CIpPhoneDlg(CWnd* pParent /*=NULL*/)
: CDialog(CIpPhoneDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIpPhoneDlg)
	m_showtext = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_issend=false;
	for (int i = 0; i < BUFNUM; i++)         //***********
	{
		m_pWaveInBuf[i] = NULL;
		m_pWaveOutBuf[i] = NULL;
	}
	is_accept=false;
	m_hWavein= 0x0;
	m_hWaveout = 0x0;                  //**************//
}

void CIpPhoneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIpPhoneDlg)
	DDX_Text(pDX, IDC_EDIT1, m_showtext);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_LIST1, m_listwords);
}

BEGIN_MESSAGE_MAP(CIpPhoneDlg, CDialog)
	//{{AFX_MSG_MAP(CIpPhoneDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnButton7)
	ON_BN_CLICKED(IDC_BUTTON8, OnButton8)
	ON_BN_CLICKED(IDC_BUTTON9, OnButton9)
	ON_BN_CLICKED(IDC_BUTTON10, OnButton10)
	ON_BN_CLICKED(IDC_BUTTON11, OnButton11)
	ON_BN_CLICKED(IDC_BUTTON12, OnButton12)
	ON_BN_CLICKED(IDC_BUTTON13, OnCall)
	ON_BN_CLICKED(IDC_BUTTON14, OnHangUp)

	ON_MESSAGE(MM_WIM_OPEN,OnMM_WIM_OPEN)      
	ON_MESSAGE(MM_WIM_DATA,OnMM_WIM_DATA)
	ON_MESSAGE(MM_WOM_OPEN,OnMM_WOM_OPEN)
	ON_MESSAGE(MM_WOM_CLOSE,OnMM_WOM_CLOSE)
	ON_MESSAGE(MM_WIM_CLOSE,OnMM_WIM_CLOSE)

	ON_MESSAGE(WM_DATA_RECEIVED,OnWM_DATA_RECEIVED)        

	//}}AFX_MSG_MAP
//	ON_EN_CHANGE(IDC_EDIT1, &CIpPhoneDlg::OnEnChangeEdit1)
ON_BN_CLICKED(IDC_BUTTON2, &CIpPhoneDlg::OnBnClickedButton2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
BOOL CIpPhoneDlg::OnInitDialog()[初始化]
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_showtext="";
	UpdateData(FALSE);

	m_hWaveoutReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hWaveinReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hWaveinBufReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hWaveoutBufReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	CreateThread(NULL, 0, PointListenThreadFunc, (LPVOID)this, 0, NULL);	//创建单呼接听线程
	CreateThread(NULL, 0, GroupListenThreadFunc, (LPVOID)this, 0, NULL);	//创建组呼接听线程
	CreateThread(NULL, 0, BroadcastListenThreadFunc, (LPVOID)this, 0, NULL);	//创建群呼接听线程

	FILE *fp;
	fp=fopen("config.txt","r"); 
	int i=0;
	while (!feof(fp))
	{
		fscanf(fp,"%s %s %s %s",&ipnum.name,&ipnum.ip,&ipnum.telenum,&ipnum.groupnum);
		m_listwords.Addstring(ipnum.name);//显示联系人列表
		i++;
	}
	fclose(fp);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIpPhoneDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout; 
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIpPhoneDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIpPhoneDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CIpPhoneDlg::OnButton1()	//数字键1按键响应函数
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_showtext+="1";
	UpdateData(FALSE);
}

void CIpPhoneDlg::OnButton2()	//数字键2按键响应函数
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_showtext+="2";
	UpdateData(FALSE);
}

void CIpPhoneDlg::OnButton3()	//数字键3按键响应函数
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_showtext+="3";
	UpdateData(FALSE);
}

void CIpPhoneDlg::OnButton4()	//数字键4按键响应函数
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_showtext+="4";
	UpdateData(FALSE);
}

void CIpPhoneDlg::OnButton5()	//数字键5按键响应函数 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_showtext+="5";
	UpdateData(FALSE);
}

void CIpPhoneDlg::OnButton6()	//数字键6按键响应函数 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_showtext+="6";
	UpdateData(FALSE);
}

void CIpPhoneDlg::OnButton7()	//数字键7按键响应函数
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_showtext+="7";
	UpdateData(FALSE);
}

void CIpPhoneDlg::OnButton8()	//数字键8按键响应函数 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_showtext+="8";
	UpdateData(FALSE);
}

void CIpPhoneDlg::OnButton9()	//数字键9按键响应函数
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_showtext+="9";
	UpdateData(FALSE);
}

void CIpPhoneDlg::OnButton10()	//数字键*按键响应函数
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_showtext+="*";
	UpdateData(FALSE);
}

void CIpPhoneDlg::OnButton11()	//数字键0按键响应函数 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_showtext+="0";
	UpdateData(FALSE);
}

void CIpPhoneDlg::OnButton12()	//数字键#按键响应函数
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_showtext+="#";
	UpdateData(FALSE);
}

void CIpPhoneDlg::OnCall()	//拨号键&接听键的按键响应函数
{  
	CString str;
	GetDlgItem(IDC_BUTTON13)->GetWindowText(str);	//获取拨号键的标题文本
	UpdateData();
	if ("呼叫"==str)	//按键为拨号键	
	{
		m_Role=CALLER;	//拨号者设置为身份为CALLER
	    if (m_showtext=="000")	//验证组呼号
		{
			Call_Mode=GroupCallmode;	//设置通话模式为组呼
			GroupCall();	//进行组呼拨号操作
		}
		else
		{
			Call_Mode=PointCallmode;	//设置通话模式为单呼
			PointCall();	//进行单呼拨号操作
		}
	}
	else	//按键为接听键
	{
		GetDlgItem(IDC_EDIT1)->SetWindowText("通话中");	//显示通话状态为“通话中”
		GetDlgItem(IDC_BUTTON14)->SetWindowText("挂断");	//将拒绝接听键改为挂机键
		int ret;
		CWaveFormat wf;
		PlaySound(NULL, NULL,SND_FILENAME | SND_ASYNC|SND_LOOP);	//停止振铃
		//CreateThread(NULL, 0, RecvPointThreadFunc, (LPVOID)this, 0, NULL);
		ret = waveInOpen(&(this->m_hWavein), WAVE_MAPPER, &wf.m_wfx, (DWORD)this->m_hWnd, NULL, CALLBACK_WINDOW);	//打开录音设备
		SetEvent(this->m_hWaveinReady);	//设置事件m_hWaveinReady，表明录音设备准备好
		if (ret != 0)
		{
			AfxMessageBox("ListenThreadFunc waveInOpen Error");
			return ;
		}
		ret = waveOutOpen( &(this->m_hWaveout), WAVE_MAPPER, &wf.m_wfx, (DWORD)this->m_hWnd, NULL, CALLBACK_WINDOW);	//打开播音设备
		SetEvent( this->m_hWaveoutReady);	//设置事件m_hWaveOutReady，表明播音设备准备好
		if (ret != 0)
		{
			AfxMessageBox("ListenThreadFunc waveOutOpen Error");
			return ;
		}
	}
}
void CIpPhoneDlg::OnHangUp()	//挂机键响应函数
{
	// TODO: Add your control notification handler code here
	PlaySound(NULL, NULL,SND_FILENAME | SND_ASYNC|SND_LOOP);	//挂断后不再播放语音，拒绝接听后停止振铃
	m_State = OFFLINE;	//状态设置为离线
	GetDlgItem(IDC_BUTTON13)->SetWindowText("呼叫");
	GetDlgItem(IDC_BUTTON14)->SetWindowText("挂断");
	this->GetDlgItem(IDC_EDIT1)->SetWindowText("通话结束");	//显示通话结束
	this->GetDlgItem(IDC_BUTTON13)->SetWindowText("呼叫");	//接听键变为拨号键，用于下一次的拨号
	//根据不同的通话模式关闭做出不同的响应
		if (Call_Mode==GroupCallmode)
	{
		if(m_Role==CALLER)
		{
			int b=sendgroupoffmess();	//组呼发起者结束通话，发送通话结束消息
		}
		m_GroupSocket_Calling.Close();	//关闭组呼中用于发送数据的套接字
	}
	if (Call_Mode==PointCallmode)
	{
		m_Point_Called.Close();		//关闭单呼中接收数据的套接字
		m_Point_Calling.Close();	//关闭单呼中发送数据的套接字
	}

}
监听线程
DWORD CIpPhoneDlg::GroupListenThreadFunc(LPVOID pParam)		//组呼来电检测线程
{
	int ret;
CWaveFormat wf;
	sockaddr_in addr ;
	int addrlen=sizeof(sockaddr);
	CString NUM;
	CGroupSocket &sock =((CIpPhoneDlg*)pParam)->m_GroupSocket_Listen; //无此行
	((CIpPhoneDlg*)pParam)->m_GroupSocket_Listen.Create();	//创建用于检测来电的套接字
	ret = ((CIpPhoneDlg*)pParam)->m_GroupSocket_Listen.Bind();	//绑定在本地地址上
	if (((CIpPhoneDlg*)pParam)->m_GroupSocket_Listen.m_socket == SOCKET_ERROR)
	{
		AfxMessageBox("Listen Socket Create Error.");
		exit(0);
	}
	if (ret == SOCKET_ERROR)
	{
		AfxMessageBox("Listen Socket Bind Error.");
		exit(0);
	}
	((CIpPhoneDlg*)pParam)->m_Role = CALLEE;	//设置为被叫方
	((CIpPhoneDlg*)pParam)->m_State = ONLINE;	//状态设置为在线
	fd_set fdread;
	FD_ZERO (&fdread);
	FD_SET(((CIpPhoneDlg*)pParam)->m_GroupSocket_Listen.m_socket,&fdread);
	select(0,&fdread,NULL,NULL,NULL);	//检测是否有数据可读，程序阻塞在此处，直到有数据发送过来
	if(((CIpPhoneDlg*)pParam)->m_Role==CALLEE)
	{
		PlaySound("Windows 7 电话拨入声", NULL,SND_FILENAME | SND_ASYNC|SND_LOOP);	//振铃
		((CIpPhoneDlg*)pParam)->GetDlgItem(IDC_EDIT1)->SetWindowText("组来电");		//组来电显示
		((CIpPhoneDlg*)pParam)->is_accept=true;										//标识此时拨号键为拒绝接听键
		((CIpPhoneDlg*)pParam)->GetDlgItem(IDC_BUTTON13)->SetWindowText("接听");	//拨号键改为接听键
		((CIpPhoneDlg*)pParam)->GetDlgItem(IDC_BUTTON14)->SetWindowText("拒绝");	//挂机键改为拒绝接听键
		CreateThread(NULL,0,RecvGroupoffFunc,(LPVOID)pParam, 0, NULL);				//创建接收线程，用于接收组呼发起方发送的结束通话的消息
	}
	CreateThread(NULL, 0, RecvGroupThreadFunc, (LPVOID)pParam, 0, NULL);	//创建线程，用于接收组呼语音数据
	return 1;
}
DWORD CIpPhoneDlg::PointListenThreadFunc(LPVOID pParam)	//单呼来电监测线程
{
	int ret;
	CWaveFormat wf;
	sockaddr_in addr ;
	int addrlen=sizeof(sockaddr);
	((CIpPhoneDlg*)pParam)->m_Point_Listen.Create();	//创建用于监听单呼来电请求的套接字
	if (((CIpPhoneDlg*)pParam)->m_Point_Listen.m_socket == SOCKET_ERROR)
	{
		AfxMessageBox("Listen Socket Create Error.");
		exit(0);
	}
	ret = ((CIpPhoneDlg*)pParam)->m_Point_Listen.Bind();	//绑定在单呼通话端口7000
	if (ret == SOCKET_ERROR)
	{
		AfxMessageBox("Listen Socket Bind Error.");
		exit(0);
	}
	ret = ((CIpPhoneDlg*)pParam)->m_Point_Listen.Listen();	//监听连接被叫方连接请求
	if (ret == SOCKET_ERROR)
	{
		AfxMessageBox("Listen Socket Listen Error.");
		exit(0);
	}
	//CreateThread(NULL, 0, RecvPointThreadFunc, (LPVOID)pParam, 0, NULL);
	while (1)
	{
		ret = ((CIpPhoneDlg*)pParam)->m_Point_Listen.Accept((sockaddr*)&addr,&addrlen);		//程序会阻塞在这里，直到连接建立
		if (ret == SOCKET_ERROR)
		{
			AfxMessageBox("Listen Socket Accept Error.");
			exit(0);
		}
		CString addres=inet_ntoa(addr.sin_addr);	//根据建立连接的对方的IP地址，从配置文件得到对方的公务号
		FILE *fp;
		fp=fopen("config.txt","r"); 
		int i=0;
		while (!feof(fp))
		{
			fscanf(fp,"%s %s %s",&((CIpPhoneDlg*)pParam)->ipnum.ip,&((CIpPhoneDlg*)pParam)->ipnum.telenum,&((CIpPhoneDlg*)pParam)->ipnum.groupnum);
			if (((CIpPhoneDlg*)pParam)->ipnum.ip==addres)
			{
				break;
			}
			i++;
		}
		fclose(fp);	
		
		((CIpPhoneDlg*)pParam)->m_showtext=((CIpPhoneDlg*)pParam)->ipnum.telenum;
		((CIpPhoneDlg*)pParam)->m_showtext+="来电";
	((CIpPhoneDlg*)pParam)->GetDlgItem(IDC_EDIT1)->SetWindowText(((CIpPhoneDlg*)pParam)->m_showtext);//来电显示
		((CIpPhoneDlg*)pParam)->Call_Mode=PointCallmode;	//通话模式设置为单呼
		((CIpPhoneDlg*)pParam)->m_Point_Called = CPointSocket(ret);	//用m_Point_Called做为被叫方发送数据的套接字
		PlaySound( "Windows 7 电话拨入声.wav", NULL,SND_FILENAME | SND_ASYNC | SND_LOOP);	//来电振铃
		((CIpPhoneDlg*)pParam)->GetDlgItem(IDC_BUTTON13)->SetWindowText("接听");	//拨号键改为接听键
		((CIpPhoneDlg*)pParam)->GetDlgItem(IDC_BUTTON14)->SetWindowText("拒绝");	//挂机键改为拒绝接听键
		((CIpPhoneDlg*)pParam)->m_Role = CALLEE;	//状态设置为被叫
		((CIpPhoneDlg*)pParam)->m_State = ONLINE;	//状态设置为在线
		CreateThread(NULL, 0, RecvPointThreadFunc, (LPVOID)pParam, 0, NULL);	//创建被叫方接收语音数据的线程
	}
	return 1;
}

接收线程
DWORD CIpPhoneDlg::RecvGroupThreadFunc(LPVOID pParam )	//组呼语音数据接收线程
{
	WaitForSingleObject( ((CIpPhoneDlg*)pParam)->m_hWaveoutBufReady,INFINITE);
	int devnum;
	int ret = 0;
	char *pBuf = NULL;
	CWaveFormat wf;
	CGroupSocket &sock =((CIpPhoneDlg*)pParam)->m_GroupSocket_Listen; 
	int flag=0;
	while ( ((CIpPhoneDlg*)pParam)->m_State == ONLINE) //状态为在线时，则一直接收数据
	{
		pBuf = new char[CWaveBuffer::BUFSIZE];
		while (ret < CWaveBuffer::BUFSIZE && ret != SOCKET_ERROR ) 
		{
			ret += sock.Recv(pBuf + ret, CWaveBuffer::BUFSIZE - ret);	//接收语音数据
			((CIpPhoneDlg*)pParam)->Call_Mode=GroupCallmode;	//状态设置为组呼
			if(sock.m_address!=((CIpPhoneDlg*)pParam)->getmyaddress())	//接收到本机以外其他主机发送的数据，则表示有人接听主叫方的群呼来电
			{
				
				if(((CIpPhoneDlg*)pParam)->m_Role==CALLER)
				{
					PlaySound(NULL, NULL,SND_FILENAME | SND_ASYNC|SND_LOOP);
					((CIpPhoneDlg*)pParam)->GetDlgItem(IDC_EDIT1)->SetWindowText("通话中");
				}
			}

			if (sock.m_address==((CIpPhoneDlg*)pParam)->getmyaddress())	//屏蔽本机的声音
			{
				ret=0;
			}

		}
		if (ret == SOCKET_ERROR)
			break;

		((CIpPhoneDlg*)pParam)->m_pWaveOutBuf[((CIpPhoneDlg*)pParam)->m_WaveOutIndex]->ResetWaveOutBuffer();	//设置接收语音数据的缓冲区的格式
		((CIpPhoneDlg*)pParam)->m_pWaveOutBuf[((CIpPhoneDlg*)pParam)->m_WaveOutIndex]->UseWaveOutBuffer(pBuf, ret);	//复制数据到缓冲区队列中

		ret = ((CIpPhoneDlg*)pParam)->m_pWaveOutBuf[((CIpPhoneDlg*)pParam)->m_WaveOutIndex]->Prepare();	//将接收到的数据送至语音播放设备
		if (ret != 0)
		{
			AfxMessageBox("Prepare Error in RecvThreadFunc");
		}	

		::PostMessage( ((CIpPhoneDlg*)pParam)->m_hWnd, WM_DATA_RECEIVED, ((CIpPhoneDlg*)pParam)->m_WaveOutIndex, 0);	//发送消息WM_DATA_RECEIVED，其响应函数播放语音
		((CIpPhoneDlg*)pParam)->m_WaveOutIndex = (((CIpPhoneDlg*)pParam)->m_WaveOutIndex + 1) % BUFNUM;	//指针指向下一缓冲区，继续接收语音数据

		delete []pBuf;
		pBuf = NULL;
	}
	((CIpPhoneDlg*)pParam)->GetDlgItem(IDC_EDIT1)->SetWindowText("通话结束");	//当状态为离线或者连接断掉时则通话结束
	((CIpPhoneDlg*)pParam)->m_State = OFFLINE;	//设置为离线状态
	waveOutClose( ((CIpPhoneDlg*)pParam)->m_hWaveout);	//关闭语音播放设备
	waveInClose( ((CIpPhoneDlg*)pParam)->m_hWavein);	//关闭语音采集设备
	return 1;
}


DWORD CIpPhoneDlg::RecvPointThreadFunc( LPVOID pParam )		//单呼时用于接收语音数据的线程
{
	WaitForSingleObject( ((CIpPhoneDlg*)pParam)->m_hWaveoutBufReady,INFINITE);	//等待事件m_hWaveoutBufReady，表明语音播放设备已经打开，且缓冲区已经准备好	
	int ret = 0;
	char *pBuf = NULL;
	CWaveFormat wf;
	CPointSocket &sock = ((CIpPhoneDlg*)pParam)->m_Point_Calling;
	if (CALLEE==((CIpPhoneDlg*)pParam)->m_Role)
	{
		sock = ((CIpPhoneDlg*)pParam)->m_Point_Called;
	}
	while ( ((CIpPhoneDlg*)pParam)->m_State == ONLINE)         //状态为在线，则循环接收数据
	{
		pBuf = new char[CWaveBuffer::BUFSIZE];
		while (ret < CWaveBuffer::BUFSIZE && ret != SOCKET_ERROR )	//数据要存满一个缓冲区才向下执行
		{
			ret += sock.Recv(pBuf + ret, CWaveBuffer::BUFSIZE - ret);	//接收语音数据
			int  b=GetLastError();
			((CIpPhoneDlg*)pParam)->Call_Mode=PointCallmode;
			PlaySound(NULL, NULL,SND_FILENAME | SND_ASYNC|SND_LOOP);
			((CIpPhoneDlg*)pParam)->GetDlgItem(IDC_EDIT1)->SetWindowText("通话中"); //主叫方接收到数据则表明被叫方接听了来电
		}
		if (ret == SOCKET_ERROR)
		{
			break;
		}
		((CIpPhoneDlg*)pParam)->m_pWaveOutBuf[((CIpPhoneDlg*)pParam)->m_WaveOutIndex]->ResetWaveOutBuffer();	//设置接收语音数据的缓冲区的格式
		((CIpPhoneDlg*)pParam)->m_pWaveOutBuf[((CIpPhoneDlg*)pParam)->m_WaveOutIndex]->UseWaveOutBuffer(pBuf, ret);	//复制数据到缓冲区队列中				
		ret = ((CIpPhoneDlg*)pParam)->m_pWaveOutBuf[((CIpPhoneDlg*)pParam)->m_WaveOutIndex]->Prepare();	//将接收到的数据送至语音播放设备
		if (ret != 0)
		{
			AfxMessageBox("Prepare Error in RecvThreadFunc");			
		}					
		::PostMessage( ((CIpPhoneDlg*)pParam)->m_hWnd, WM_DATA_RECEIVED, ((CIpPhoneDlg*)pParam)->m_WaveOutIndex, 0);	//发送消息WM_DATA_RECEIVED，其响应函数播放语音
		((CIpPhoneDlg*)pParam)->m_WaveOutIndex = (((CIpPhoneDlg*)pParam)->m_WaveOutIndex + 1) % BUFNUM;	//指针指向下一缓冲区，继续接收语音数据
		delete []pBuf;
		pBuf = NULL;
	}
	((CIpPhoneDlg*)pParam)->GetDlgItem(IDC_EDIT1)->SetWindowText("通话结束");	//离线状态或连接断掉都表示通话结束
	((CIpPhoneDlg*)pParam)->m_State = OFFLINE;	//设置为离线状态
	waveOutClose( ((CIpPhoneDlg*)pParam)->m_hWaveout);	//关闭语音播放设备
	waveInClose( ((CIpPhoneDlg*)pParam)->m_hWavein);	//关闭语音采集设备
	return 1;
}


//打开语音设备之后的消息响应函数
LRESULT CIpPhoneDlg::OnMM_WIM_OPEN(UINT wParam,LONG lParam)	
{
	int ret;
	m_WaveInIndex = 0;

	WaitForSingleObject(m_hWaveinReady, INFINITE);

	for (int i = 0; i < BUFNUM; i++)
	{
		m_pWaveInBuf[i] = new CWaveBuffer(m_hWavein);	//为语音采集定义缓冲区

		ret = m_pWaveInBuf[i]->Prepare();	//初始化缓冲区的格式
		if (ret != MMSYSERR_NOERROR)
			AfxMessageBox("WAVEIN Prepare Error.");

		ret = m_pWaveInBuf[i]->AddToWaveInBuffer();	//将缓冲区提供给语音采集使用
		if (ret != MMSYSERR_NOERROR)
			AfxMessageBox("WAVEIN Add Error.");  	
	}

	SetEvent(m_hWaveinBufReady);

	ret = waveInStart (m_hWavein);	//开始录音，录满一个Buffer会发送消息MM_WIM_DATA

	if (ret != MMSYSERR_NOERROR)
		AfxMessageBox("waveInStart Error.");
	return 1;
}

//采集满一个缓冲区的消息响应函数
LRESULT CIpPhoneDlg::OnMM_WIM_DATA(UINT wParam,LONG lParam)///由参数传递来数据和数据的长度
{
	UINT ret;
	CWaveFormat wf;

	if (m_State == OFFLINE)
		return 0;
	
	if (Call_Mode==PointCallmode)
	{
		CPointSocket &sock = m_Point_Calling;
		if (m_Role == CALLEE)				
		{
			sock = m_Point_Called;			
		}
		ret = sock.Send(((WAVEHDR*)lParam)->lpData, ((WAVEHDR*)lParam)->dwBytesRecorded);//通过该单播套接字向对方发送数据
	}
	if (Call_Mode==GroupCallmode)
	{
		m_GroupSocket_Calling.Create();  //如果呼叫模式选为组呼，则创建一个组呼套接字

	    CGroupSocket &sock = m_GroupSocket_Calling;

	    ret = sock.Send(((WAVEHDR*)lParam)->lpData, ((WAVEHDR*)lParam)->dwBytesRecorded); //通过该套接字向组播组发送数据
	}
	    m_pWaveInBuf[m_WaveInIndex]->ResetWaveInBuffer();

	    ret = m_pWaveInBuf[m_WaveInIndex]->Prepare();   //初始化缓冲区的格式
	if (ret != 0)
	{
		AfxMessageBox("Prepare Error in OnMM_WIM_DATA");
		return 0;
	}

	ret = m_pWaveInBuf[m_WaveInIndex]->AddToWaveInBuffer();  //将缓冲区提供给语音采集使用
	if (ret != 0)
	{
		AfxMessageBox("add buffer Error in OnMM_WIM_DATA");
		return 0;
	}

	m_WaveInIndex = (m_WaveInIndex+1) % BUFNUM;
	return 1;
}
//在打开播放录音设备时产生的消息响应函数
LRESULT CIpPhoneDlg::OnMM_WOM_OPEN(UINT wParam,LONG lParam)
{
	m_WaveOutIndex = 0;

	WaitForSingleObject(m_hWaveoutReady, INFINITE);

	for (int i = 0; i < BUFNUM; i++)
	{
		m_pWaveOutBuf[i] = new CWaveBuffer(m_hWaveout);  //为播放语音数据定义缓冲区
	}

	SetEvent(m_hWaveoutBufReady);
	return 1;
}
//接收音频数据缓冲满时产生的消息处理函数
LRESULT CIpPhoneDlg::OnWM_DATA_RECEIVED(UINT wParam,LONG lParam)
{
	if (m_State == OFFLINE)
		return 0;

	int ret;

	ret = waveOutReset(m_hWaveout);
	if (ret != 0)
	{
		AfxMessageBox("waveOutReset Error in OnWM_DATA_RECEIVED.");
		return 0;
	}

	ret = m_pWaveOutBuf[wParam]->Play();  //播放接收到的语音数据
	if (ret != 0)
	{
		AfxMessageBox("Play Error");
		return 0;
	}
	return  1;
}

//关闭波形录音设备时产生的消息响应函数
LRESULT CIpPhoneDlg::OnMM_WIM_CLOSE(UINT wParam,LONG lParam)
{
	for (int i = 0; i < BUFNUM; i++)
	{
		if (m_pWaveInBuf[i] != NULL)
			delete m_pWaveInBuf[i];

		m_pWaveInBuf[i] = NULL; 
	}
	return 1;
}
//关闭播放语音设备时产生的消息响应函数
LRESULT CIpPhoneDlg::OnMM_WOM_CLOSE(UINT wParam,LONG lParam)
{
	for (int i = 0; i < BUFNUM; i++)
	{
		if (m_pWaveOutBuf[i] != NULL)
			delete m_pWaveOutBuf[i];

		m_pWaveOutBuf[i] = NULL; 
	}
	return 1;
}

DWORD CIpPhoneDlg::getmyaddress()  //得到本机IP地址的线程
{
	char name[50];
	ZeroMemory(name,sizeof(name));
	gethostname(name,50);
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	struct addrinfo *res;
	getaddrinfo(name, NULL, &hints, &res);
	struct sockaddr_in* pSockaddr=(sockaddr_in*)res->ai_addr;
	char *pIP=inet_ntoa(pSockaddr->sin_addr);
	DWORD Localhost_IP=inet_addr(pIP);
	return Localhost_IP;
}


int CIpPhoneDlg::sendgroupoffmess() //组播呼叫方发送结束通话命令
{
	SOCKET sendmess;    //定义一个用于发送数据的套接字
	SOCKET m_socketM;   //定义一个多播套接字
	sendmess=WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0,WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF|WSA_FLAG_OVERLAPPED);//创建一个用于多播发送的套接字
	BOOL bFlag=TRUE;
	::setsockopt(sendmess,SOL_SOCKET,SO_REUSEADDR,(char*)&bFlag,sizeof(bFlag)); //允许套接字绑定到一个已在使用的地址上
	SOCKADDR_IN remote;         //加入一个组播组
	remote.sin_family=AF_INET;

	CString groupaddr="224.0.0.";
	int intnum=get_groupnum();   //获得本机所属的组播组的组号
	CString num;
	num.Format("%d", intnum);
	groupaddr+=num;              //得到组播组的组地址
	remote.sin_addr.s_addr=inet_addr(groupaddr);
	remote.sin_port=::htons(6789);
	char pBuf[8]="offline";
	int len=8;

	if(m_socketM=WSAJoinLeaf(sendmess,(SOCKADDR*)&remote,sizeof(remote),NULL,NULL,NULL,NULL,JL_BOTH)==INVALID_SOCKET) //将套接字加入到该组播组，并设置为接收者和发送者；调用成功后返回一个组播套接字。
	int zu= ::sendto(sendmess, pBuf, len, 0, (sockaddr*)&remote, sizeof(remote));   //向该组播组发送数据
	return zu;
}
int CIpPhoneDlg::get_groupnum()  // 得到所属组播组组号的函数
{
	netunit ipnum;   
	FILE *fp;
	fp=fopen("config.txt","r");   //打开配置文件
	DWORD mynum=getmyaddress();
	while (!feof(fp))
	{
		fscanf(fp,"%s %s %s",&ipnum.ip,&ipnum.telenum,&ipnum.groupnum);
		if (inet_addr(ipnum.ip)==mynum)    //得到所属组播组的组号
			break;


	}
	int num=atoi(ipnum.groupnum);

	fclose(fp);
	return num; 
}

DWORD CIpPhoneDlg::RecvGroupoffFunc(LPVOID pParam)    //从组播组接收结束通话命令的线程
{
	SOCKADDR_IN m_local;
	m_local.sin_addr.s_addr = INADDR_ANY;
	m_local.sin_family = AF_INET;
	m_local.sin_port = ::ntohs(6789);
	SOCKET m_socket=WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0,WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF|WSA_FLAG_OVERLAPPED);;
	SOCKET m_socketM;  //创建一个组播组套接字
	int b=::bind(m_socket, (struct sockaddr*)&m_local, sizeof(m_local));
	BOOL bFlag=TRUE;
	::setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&bFlag,sizeof(bFlag));//允许套接字绑定到一个已在使用的地址上

	SOCKADDR_IN remote;        //加入一个组播组
	remote.sin_family=AF_INET;
	CString groupaddr="224.0.0.";
	int intnum=((CIpPhoneDlg*)pParam)->get_groupnum();  //获得本机所属的组播组的组号
	CString num;
	num.Format("%d", intnum);
	groupaddr+=num;
	remote.sin_addr.s_addr=inet_addr(groupaddr);
	remote.sin_port=::htons(8000);
	m_socketM=WSAJoinLeaf(m_socket,(SOCKADDR*)&remote,sizeof(remote),NULL,NULL,NULL,NULL,JL_BOTH); 
	SOCKADDR_IN from;
	int ret;
	int lena=sizeof(from);
	char pbuf[8];
	int len=8;
	ret=::recvfrom(m_socket,pbuf,len,0,(SOCKADDR*)&from,&lena);
	char *k=inet_ntoa(from.sin_addr);
	DWORD  m_address;
	m_address=from.sin_addr.S_un.S_addr;
	((CIpPhoneDlg*)pParam)->GetDlgItem(IDC_EDIT1)->SetWindowText("通话结束");
	return m_address;
}




bool CIpPhoneDlg::GetIPfromNUM()   //从配置文件中获得公务号码所对应的IP地址
{
	UpdateData(TRUE);
	bool flag=FALSE;  //没有找到对应IP
	FILE *fp;
	fp=fopen("config.txt","r"); 
	int i=0;
	while (!feof(fp))
	{
		fscanf(fp,"%s %s %s",&ipnum.ip,&ipnum.telenum,&ipnum.groupnum);
		if (ipnum.telenum==m_showtext)
		{
			flag=TRUE;  //找到对应IP
			break;
		}
		i++;
	}
	fclose(fp);
	return flag;
}

呼叫部分
void CIpPhoneDlg::GroupCall()  //选择组呼模式进行呼叫的函数
{
	int ret;
	in_addr dwIPaddr;
	CWaveFormat wf;
	

	m_State = ONLINE;
	ret = m_GroupSocket_Calling.Create();   //创建一个组呼套接字用于发送数据
	if (ret == SOCKET_ERROR)
	{
		AfxMessageBox("Create Error in OnButtonCall.");

		return;
	}
	PlaySound("Windows 7 电话拨出声", NULL,SND_FILENAME | SND_ASYNC|SND_LOOP);
	ret = waveInGetNumDevs();
	//打开录制波形音频设备
	ret = waveInOpen(&m_hWavein, WAVE_MAPPER, &wf.m_wfx, (DWORD)this->m_hWnd, NULL, CALLBACK_WINDOW);
	SetEvent(m_hWaveinReady); //设置事件m_hWaveinReady，表明录音设备准备好
	if (ret != 0)
	{
		AfxMessageBox("WAVEIN Open Error in OnButtonCall");
		return;
	}
	//打开播放波形音频设备
	ret = waveOutOpen( &m_hWaveout, WAVE_MAPPER, &wf.m_wfx, (DWORD)this->m_hWnd, NULL, CALLBACK_WINDOW);
	SetEvent(m_hWaveoutReady); //设置事件m_hWaveoutReady，表明播放设备准备好
	if (ret != 0)
	{
		AfxMessageBox("WAVEOUT Open Error in OnButtonCall");
		return ;
	}

}

void CIpPhoneDlg::PointCall()   //选择点对点单呼模式进行呼叫的实现函数
{ 
	if (FALSE==GetIPfromNUM())
	{
		MessageBox("错误号码，请重新输入");
		m_showtext="";
		UpdateData(FALSE);
	}
	else
	{
		int ret;
		in_addr dwIPaddr;
		CWaveFormat wf;
		m_Role = CALLER;
		ret = m_Point_Calling.Create();     //创建一个单呼套接字
		if (ret == SOCKET_ERROR)
		{
			AfxMessageBox("Create Error in OnButtonCall.");
			return;
		}
		dwIPaddr.S_un.S_addr=inet_addr(ipnum.ip);  
		ret = m_Point_Calling.Connect(dwIPaddr.S_un.S_addr);   //与呼叫的对方连接
		if (ret == SOCKET_ERROR)
		{
			AfxMessageBox("Connect Error in OnButtonCall.");
			return;
		}
		PlaySound("Windows 7 电话拨出声", NULL,SND_FILENAME | SND_ASYNC|SND_LOOP);
		ret = waveInGetNumDevs();
		ret = waveInOpen(&m_hWavein, WAVE_MAPPER, &wf.m_wfx, (DWORD)this->m_hWnd, NULL, CALLBACK_WINDOW); //打开录音设备
		SetEvent(m_hWaveinReady); //设置事件m_hWaveinReady，表明录音设备准备好
		if (ret != 0)
		{
			AfxMessageBox("WAVEIN Open Error in OnButtonCall");
			return;
		}
		ret = waveOutOpen( &m_hWaveout, WAVE_MAPPER, &wf.m_wfx, (DWORD)this->m_hWnd, NULL, CALLBACK_WINDOW); //打开播放设备
		SetEvent(m_hWaveoutReady);//设置事件m_hWaveoutReady，表明播放设备准备好
		if (ret != 0)
		{
			AfxMessageBox("WAVEOUT Open Error in OnButtonCall");
			return ;
		}
		m_State = ONLINE;
		CreateThread(NULL, 0, RecvPointThreadFunc, (LPVOID)this, 0, NULL);	
	}
}
