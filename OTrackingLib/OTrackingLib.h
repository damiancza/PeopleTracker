// OTrackingLib.h : main header file for the OTrackingLib DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COTrackingLibApp
// See OTrackingLib.cpp for the implementation of this class
//

class COTrackingLibApp : public CWinApp
{
public:
	COTrackingLibApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
