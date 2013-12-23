
// ObjectTrackerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ObjectTracker.h"
#include "ObjectTrackerDlg.h"
#include "afxdialogex.h"

#include "package_tracking/ObjectTracking.h"
#include "package_analysis/ObjectCounter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CObjectTrackerDlg dialog




CObjectTrackerDlg::CObjectTrackerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CObjectTrackerDlg::IDD, pParent)
	, started(false)
	, m_bRestart(false)
	, m_bStop(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CObjectTrackerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COUNTAB, m_ctlCountAB);
	DDX_Control(pDX, IDC_COUNTBA, m_ctlCountBA);
}

BEGIN_MESSAGE_MAP(CObjectTrackerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START_CAPTURE, &CObjectTrackerDlg::OnBnClickedStartCapture)
	ON_BN_CLICKED(IDC_RESET_COUNTER, &CObjectTrackerDlg::OnBnClickedResetCounter)
	ON_BN_CLICKED(IDC_STOP_CAPTURE, &CObjectTrackerDlg::OnBnClickedStopCapture)
END_MESSAGE_MAP()


// CObjectTrackerDlg message handlers

BOOL CObjectTrackerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
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
	if(started == false)
	{
		cvNamedWindow("INPUT",CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
		HWND hWnd = (HWND) cvGetWindowHandle("INPUT");
		HWND hParent = ::GetParent(hWnd);
		::SetParent(hWnd, GetDlgItem(IDC_FRAME_INPUT)->m_hWnd);
		::ShowWindow(hParent, SW_HIDE);
		CRect rect;
		GetDlgItem(IDC_FRAME_INPUT)->GetWindowRect(&rect);
		cvResizeWindow("INPUT", rect.Width(), rect.Height());
	}


	if(font.CreatePointFont(600,"Tahoma"))
	{
		m_ctlCountAB.SetFont(&font);
		m_ctlCountBA.SetFont(&font);
	}


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CObjectTrackerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CObjectTrackerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CObjectTrackerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CObjectTrackerDlg::OnBnClickedStartCapture()
{
	if(started == false)
	{
		thread = AfxBeginThread((AFX_THREADPROC) CObjectTrackerDlg::Thread, (LPVOID)this);
	}
	else
	{
		MessageBox(_T("Thread is already initialized!"));
		return;
	}
}

DWORD CObjectTrackerDlg::Thread(LPVOID *lpvParam)
{
  CObjectTrackerDlg *thr = (CObjectTrackerDlg*) lpvParam;
  thr->ThreadProcess();
  return NULL;
}

void CObjectTrackerDlg::ThreadProcess()
{
	int iFrameRate = 0;
	IplImage* frame = NULL;
	IplImage *frame_aux = NULL;
	char c;
	iFrameRate = 25;

	//0 Index of the camera
	capture = cvCreateCameraCapture(0);
	
    if( !capture )
    {
      MessageBox("ERROR: Cannot initialize video!");
      return;
	}

	frame_aux = cvQueryFrame(capture);
	frame = cvCreateImage(cvSize((int)frame_aux->width , (int)frame_aux->height), frame_aux->depth, frame_aux->nChannels);

	//There are plenty of different algorithms, we use codebook in this case
	bgs = new CodeBoock;

	started = true;

	/* People Counting Algorithm */
	ObjectCounter *objectCounter;
	objectCounter = new ObjectCounter();

	/* Blob Tracking */
	cv::Mat img_blob;
	ObjectTracking* objTracking;
	objTracking = new ObjectTracking;

	objTracking->SetFrameRate(iFrameRate);

	for(;;)
	{
		frame_aux = cvQueryFrame(capture);
		if(!frame_aux) break;

	   cvResize(frame_aux, frame);

		cv::Mat img_input(frame);

		cv::Mat img_mask;
		
		//Backgroudn Sustraction process (get bw image)
		bgs->process(img_input, img_mask, img_blob);
    
		if(!img_mask.empty())
		{
			// Perform the blob tracking on the image
			objTracking->process(img_input, img_mask, img_blob);

			// Perform people counting
			objectCounter->setInput(img_blob);

			//Count the objects by frame
			objectCounter->setTracks(objTracking->getTracks());
			objectCounter->processTracks();
	
			//Put the ab count to the statics
			CString sCant;

			sCant.Format("%i",objectCounter->getAbCount());
			m_ctlCountAB.SetWindowText(sCant);

			sCant.Format("%i",objectCounter->getBaCount());
			m_ctlCountBA.SetWindowText(sCant);

			if(m_bRestart)
			{
				objectCounter->reStartCounter();
				m_bRestart = false;
			}

			if(m_bStop)
				break;
		}

		c = cvWaitKey(iFrameRate);
	}
	objTracking->WriteObjToFile();
	m_bStop = false;

}

void CObjectTrackerDlg::OnBnClickedResetCounter()
{
	m_bRestart = true;
}


void CObjectTrackerDlg::OnBnClickedStopCapture()
{
	if(started)
	{
		m_bStop = true;

		StopThread();

		if(capture)
			cvReleaseCapture(&capture);

		delete bgs;
		bgs = NULL;

		started = false;
	}
}

void CObjectTrackerDlg::StopThread()
{
	DWORD exit_code = NULL;

	if(thread != NULL)
	{
		GetExitCodeThread(thread->m_hThread, &exit_code);
    
		if(exit_code == STILL_ACTIVE)
		{
			::TerminateThread(thread->m_hThread, 0);
			CloseHandle(thread->m_hThread);
		}

		thread->m_hThread = NULL;
		thread = NULL;
	}
}
