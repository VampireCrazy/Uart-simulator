
// JH-Transducer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CJHTransducerApp:
// �йش����ʵ�֣������ JH-Transducer.cpp
//

class CJHTransducerApp : public CWinApp
{
public:
	CJHTransducerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CJHTransducerApp theApp;