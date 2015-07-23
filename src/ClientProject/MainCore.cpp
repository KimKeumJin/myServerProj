#include "stdafx.h"
#include "MainCore.h"
#include "StageManager.h"


CMainCore::CMainCore()
	:m_hWnd(NULL),
	m_bWindowed(false),
	m_pDevice(NULL),
	m_pSprite(NULL),
	m_pFont(NULL),
	m_pStageMgr(NULL)

{
}


CMainCore::~CMainCore()
{
	SAFE_DELETE(m_pStageMgr);

	SAFE_RELEASE(m_pFont);
	SAFE_RELEASE(m_pSprite);
	SAFE_RELEASE(m_pDevice);
}

bool CMainCore::Init(HWND hWnd)
{
#ifdef _DEBUG
	//_CrtSetBreakAlloc(0);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	if (CreateDevice(hWnd, 400, 300, true) == false)
		return false;

	m_pStageMgr = new CStageManager;
	if (m_pStageMgr->Init(STAGE_LOGO) == false)
		return false;

	return true;
}

bool CMainCore::CreateDevice(HWND hWnd, UINT width, UINT height, bool bWindowed)
{
	m_hWnd = hWnd;
	m_bWindowed = bWindowed;
	D3DDEVTYPE deviceType = D3DDEVTYPE_HAL;

	SAFE_RELEASE(m_pDevice);

	IDirect3D9* d3d9 = 0;
	d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d9)
	{
		::MessageBox(0, _T("Direct3DCreate9() - FAILED"), 0, 0);
		return false;
	}

	// Step 2: Check for hardware vp.

	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, deviceType, &caps);

	int vp = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// Step 3: Fill out the D3DPRESENT_PARAMETERS structure.

	D3DPRESENT_PARAMETERS d3dpp;
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.Windowed = bWindowed;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	// Step 4: Create the device.
	DWORD hr;
	hr = d3d9->CreateDevice(
		D3DADAPTER_DEFAULT, // primary adapter
		deviceType,         // device type
		hWnd,               // window associated with device
		vp,                 // vertex processing
		&d3dpp,             // present parameters
		&m_pDevice);            // return created device

	if (FAILED(hr))
	{
		// try again using a 16-bit depth buffer
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

		hr = d3d9->CreateDevice(
			D3DADAPTER_DEFAULT,
			deviceType,
			hWnd,
			vp,
			&d3dpp,
			&m_pDevice);

		if (FAILED(hr))
		{
			d3d9->Release(); // done with d3d9 object
			::MessageBox(0, _T("CreateDevice() - FAILED"), 0, 0);
			return false;
		}
	}	

	// 추가 설정
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	// 스프라이트를 생성한다.
	D3DXCreateSprite(m_pDevice, &m_pSprite);

	if (InitFont(10, 20, FW_MEDIUM, false, L"맑은 고딕"/*L"Arial"*/) == false)
	{
		d3d9->Release();
		return false;
	}

	d3d9->Release(); // done with d3d9 object

	return true;
}


bool CMainCore::InitFont(UINT Width, UINT Height, UINT Weight, BOOL Italic, TCHAR* pName)
{
	SAFE_RELEASE(m_pFont);

	D3DXFONT_DESC		df;
	ZeroMemory(&df, sizeof(df));
	df.Width = Width;
	df.Height = Height;
	df.Weight = Weight;
	df.Italic = Italic;
	df.CharSet = DEFAULT_CHARSET;
	df.MipLevels = 1;
	lstrcpyW(df.FaceName, pName);

	if (FAILED(D3DXCreateFontIndirect(m_pDevice, &df, &m_pFont)))
	{
		::MessageBox(0, _T("CreateFont() - FAILED"), 0, 0);
		return false;
	}

	return true;
}


int CMainCore::UpdateFrame()
{
	m_pStageMgr->UpdateFrame();

	return 0;
}

int CMainCore::Render()
{
	static LARGE_INTEGER prevCounter;
	static float fpsTime = 0.f;
	static int fpsCount = 0;
	static float fps = 0.f;

	m_pDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);
	if(SUCCEEDED(m_pDevice->BeginScene()))
	{
		m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);

		// frame per seconds
		LARGE_INTEGER counter, frequency;
		QueryPerformanceCounter(&counter);
		QueryPerformanceFrequency(&frequency);

		float elapsed = (counter.QuadPart - prevCounter.QuadPart) / (float)frequency.QuadPart;
		prevCounter = counter;


		fpsTime += elapsed;
		++fpsCount;

		if (fpsCount > 60)
		{
			fps = 60 / fpsTime;
			fpsTime = 0.f;
			fpsCount = 0;
		}

		RECT rc = {0, 0, 100, 20};
		wchar_t szFPS[256] = { 0, };
		swprintf_s(szFPS, 256, L"frame per second : %0.4f", fps);
		m_pFont->DrawTextW(m_pSprite, szFPS, wcslen(szFPS), &rc, DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 255, 255));

		m_pSprite->End();

		m_pDevice->EndScene();
	}

	m_pDevice->Present(NULL, NULL, NULL, NULL);

	//m_pStageMgr->Render();

	return 0;
}