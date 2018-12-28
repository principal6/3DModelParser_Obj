#include "DirectInput.h"

DirectInput* DirectInput::pInstance = 0;

void DirectInput::InitDirectInput(HINSTANCE inst, HWND hwnd)
{
	m_hInstance = inst;
	m_hwnd = hwnd;

	memset(MouseButtonDown, false, sizeof(MouseButtonDown));
	memset(MouseButtonUp, false, sizeof(MouseButtonUp));
	memset(MouseButtonIdle, true, sizeof(MouseButtonIdle));

	if(FAILED(DirectInput8Create(m_hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **) &m_diObj, NULL)))
		return;
}

void DirectInput::CreateMouseDevice(DWORD dwFlags)
{
	if(FAILED(m_diObj->CreateDevice(GUID_SysMouse, &m_diMouse, NULL)))
		return;

	if(FAILED(m_diMouse->SetDataFormat(&c_dfDIMouse2)))
		return;

	if(FAILED(m_diMouse->SetCooperativeLevel(m_hwnd, dwFlags)))
		return;

	if(FAILED(m_diMouse->Acquire()))
		return;
}

void DirectInput::CreateKeyboardDevice(DWORD dwFlags)
{
	if(FAILED(m_diObj->CreateDevice(GUID_SysKeyboard, &m_diKeyboard, NULL)))
		return;

	if(FAILED(m_diKeyboard->SetDataFormat(&c_dfDIKeyboard)))
		return;

	if(FAILED(m_diKeyboard->SetCooperativeLevel(m_hwnd, dwFlags)))
		return;

	if(FAILED(m_diKeyboard->Acquire()))
		return;
}

void DirectInput::ShutdownDirectInput(void)
{
	if(m_diMouse)
	{
		m_diMouse->Unacquire();
		SAFE_RELEASE(m_diMouse);
	}

	if(m_diKeyboard)
	{
		m_diKeyboard->Unacquire();
		SAFE_RELEASE(m_diKeyboard);
	}
	
	SAFE_RELEASE(m_diObj);
}

bool DirectInput::DIKeyboardHandler(DWORD dkcode)
{
	HRESULT hr;

	memset(&m_keyboard, 0, sizeof(m_keyboard));

	if(FAILED(hr = m_diKeyboard->GetDeviceState(sizeof(m_keyboard),(LPVOID) &m_keyboard)))
	{
		if(hr == DIERR_INPUTLOST)
			m_diKeyboard->Acquire();
	}

	if(m_keyboard[dkcode] & 0x80)	// 키가 눌리면 true를 반환
		return true;

	return false;
}

D3DXVECTOR3 DirectInput::DIMouseHandler(void)
{
	D3DXVECTOR3 p;
	HRESULT hr;

	memset(&m_diMouseState, 0, sizeof(m_diMouseState));
	if(FAILED(hr = m_diMouse->GetDeviceState(sizeof(m_diMouseState),(LPVOID) &m_diMouseState)))
	{
		if(hr == DIERR_INPUTLOST)
			m_diMouse->Acquire();
	}

	p.x = (float)m_diMouseState.lX;
	p.y = (float)m_diMouseState.lY;
	p.z = (float)m_diMouseState.lZ;

	return p;
}

bool DirectInput::DIMouseButtonHandler(int button)
{
	if(m_diMouseState.rgbButtons[button] & 0x80)
	{
		if ( (MouseButtonDown[button] == false) && (MouseButtonIdle[button] == true) )
		{
			MouseButtonUp[button] = false;
			MouseButtonDown[button] = true;
		}
		return true;
	}
	else if(!m_diMouseState.rgbButtons[button])
	{
		if ( (MouseButtonDown[button] == false) && (MouseButtonIdle[button] == false) )
		{
			MouseButtonUp[button] = true;
			MouseButtonIdle[button] = true;
			MouseButtonDown[button] = false;
		}
		else if ( (MouseButtonDown[button] == true) && (MouseButtonIdle[button] == true) )
		{
			MouseButtonUp[button] = true;
			MouseButtonDown[button] = false;
		}
	}

	return false;
}

bool DirectInput::OnMouseButtonDown(int button)
{
	if (MouseButtonDown[button] == true)
	{
		MouseButtonDown[button] = false;
		MouseButtonIdle[button] = false;
		return true;
	}

	return false;
}

bool DirectInput::OnMouseButtonUp(int button)
{
	if (MouseButtonUp[button] == true)
	{
		MouseButtonUp[button] = false;
		MouseButtonIdle[button] = true;
		return true;
	}

	return false;
}