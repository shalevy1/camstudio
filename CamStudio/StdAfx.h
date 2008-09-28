// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__EB5B2A10_2312_4984_8371_6DE5181F903F__INCLUDED_)
#define AFX_STDAFX_H__EB5B2A10_2312_4984_8371_6DE5181F903F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
//#define _AFX_SECURE_NO_WARNINGS
// replacing calls to deprecated functions with calls to the new secure versions of those functions.
//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES		1

#include "targetver.h"		// define WINVER

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

//#include <stdio.h>
//#include <memory.h>
//#include <setjmp.h>
//#include <mmsystem.h>
#include <vfw.h>
//#include <windowsx.h>

#define sscanf	sscanf_s

#endif // !defined(AFX_STDAFX_H__EB5B2A10_2312_4984_8371_6DE5181F903F__INCLUDED_)