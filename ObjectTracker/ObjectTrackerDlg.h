
// ObjectTrackerDlg.h : header file
//

#pragma once


// CObjectTrackerDlg dialog
class CObjectTrackerDlg : public CDialogEx
{
// Construction
public:
	CObjectTrackerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_OBJECTTRACKER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	static DWORD Thread(LPVOID *lpvParam);
	void ThreadProcess();
	void StopThread();
	bool started;
	CWinThread* thread;
	CvCapture* capture;
	InterfaceBGS* bgs;
	CFont font;
	//Flag para resetear el contador
	bool m_bRestart;
	bool m_bStop;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	CStatic m_ctlCountAB;
	CStatic m_ctlCountBA;
	afx_msg void OnBnClickedStartCapture();
	afx_msg void OnBnClickedResetCounter();
	afx_msg void OnBnClickedStopCapture();
};
