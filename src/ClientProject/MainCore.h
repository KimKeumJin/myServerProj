#pragma once

class CStageManager;
class CMainCore
{
private:
	HWND					m_hWnd;
	bool					m_bWindowed;
	LPDIRECT3DDEVICE9		m_pDevice;
	LPD3DXSPRITE			m_pSprite;
	LPD3DXFONT				m_pFont;

	CStageManager*			m_pStageMgr;

public:
	bool Init(HWND hWnd);
	int UpdateFrame();
	int Render();

private:
	bool CreateDevice(HWND hWnd, UINT width, UINT height, bool bWindowed);
	bool InitFont(UINT Width, UINT Height, UINT Weight, BOOL Italic, TCHAR* pName);

public:
	CMainCore();
	~CMainCore();
};

