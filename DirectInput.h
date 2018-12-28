#ifndef _DIRECTINPUT_H_
#define _DIRECTINPUT_H_

#define DIRECTINPUT_VERSION 0x800
#include <dinput.h>
#include "Global.h"

#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

#define KEYS 256

class DirectInput
{
private:
	static DirectInput* pInstance;

	HWND            m_hwnd;		//	The Main Window Handle.
	HINSTANCE		m_hInstance;

	LPDIRECTINPUT8			m_diObj;
	LPDIRECTINPUTDEVICE8	m_diKeyboard;
	LPDIRECTINPUTDEVICE8	m_diMouse;

	char					m_keyboard[KEYS];
	DIMOUSESTATE2			m_diMouseState;

	bool	MouseButtonDown[3];
	bool	MouseButtonUp[3];
	bool	MouseButtonIdle[3];

	int m_nX, m_nY;	// 마우스 위치

	DirectInput()	// 모든 변수 초기화
	{
		m_diObj = 0;
		m_diKeyboard =0;
		m_diMouse =0;
		memset(&m_keyboard, 0, sizeof(m_keyboard));
		m_nX = 0;
		m_nY = 0;
		m_hwnd = NULL;
		m_hInstance = NULL;
	}

	virtual ~DirectInput(){}
	DirectInput(const DirectInput &di){}
	DirectInput & operator=(const DirectInput &di){ return *this;}

public:

	static DirectInput* GetInstance()
	{
		if(pInstance == 0)
		{
			pInstance = new DirectInput;
		}
		
		return pInstance;
	}

	static void DeleteInstance(void)
	{
		if(pInstance)
		{
			delete pInstance;
			pInstance = NULL;
		}
	}

	void InitDirectInput(HINSTANCE inst, HWND hwnd);
	void ShutdownDirectInput(void);
	void CreateMouseDevice(DWORD dwFlags);
	void CreateKeyboardDevice(DWORD dwFlags);
	bool DIKeyboardHandler(DWORD dkcode);
	D3DXVECTOR3 DIMouseHandler(void);
	bool DIMouseButtonHandler(int button);
	bool OnMouseButtonDown(int button);
	bool OnMouseButtonUp(int button);
};

#endif 