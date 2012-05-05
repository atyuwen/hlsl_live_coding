#include "common.hpp"

#include <algorithm>
#include <numeric>
#include <boost/bind.hpp>
#include "ayw/vector.hpp"
#include "ayw/constant.hpp"
#include "d3d_app.hpp"
using namespace Ayw;

const tstring g_app_title = TEXT("LIVE CODING");

struct ShaderParameters
{
	float4 time;		// time related parameters
	float4 view;		// viewport size: (width, height, 1 / width, 1 / height)
	float4 fft;			// the short time FFT of the background music
};

//////////////////////////////////////////////////////////////////////////
// static accessors
//////////////////////////////////////////////////////////////////////////
D3DApp g_app;

D3DApp* D3DApp::GetApp()
{
	return &g_app;
}

ID3D11Device* D3DApp::GetD3D11Device()
{
	return g_app.m_d3d11_device;
}

ID3D11DeviceContext* D3DApp::GetD3D11DeviceContext()
{
	return g_app.m_d3d11_device_context;
}

IDWriteFactory* D3DApp::GetDWriteFactory()
{
	return g_app.m_dwrite_factory;
}

HRTimer* D3DApp::GetTimer()
{
	return &g_app.m_timer;
}

PostProcessPtr D3DApp::GetPostProcess()
{
	return g_app.m_custom_pp;
}

SoundPlayerPtr D3DApp::GetSoundPlayer()
{
	return g_app.m_sound_player;
}

//////////////////////////////////////////////////////////////////////////
// constructor / destructor
//////////////////////////////////////////////////////////////////////////
D3DApp::D3DApp()
	: m_hinstance(NULL)
	, m_hwnd(NULL)
	, m_swap_chain(NULL)
	, m_d3d11_device(NULL)
	, m_d3d11_device_context(NULL)
	, m_depthstencil_buffer(NULL)
	, m_rendertarget_view(NULL)
	, m_depthstencil_view(NULL)
	, m_back_buffer(NULL)
	, m_d3d10_device(NULL)
	, m_dwrite_factory(NULL)
	, m_d2d_rendertarget(NULL)
	, m_d2d_texture(NULL)
	, m_shared_texture(NULL)
	, m_keyed_mutex11(NULL)
	, m_keyed_mutex10(NULL)
	, m_parameter_buffer(NULL)
{}

D3DApp::~D3DApp()
{
	SAFE_RELEASE(m_parameter_buffer);

	SAFE_RELEASE(m_depthstencil_buffer);
	SAFE_RELEASE(m_rendertarget_view);
	SAFE_RELEASE(m_depthstencil_view);
	SAFE_RELEASE(m_back_buffer);

	SAFE_RELEASE(m_dwrite_factory);
	SAFE_RELEASE(m_d2d_rendertarget);
	SAFE_RELEASE(m_d2d_texture);

	SAFE_RELEASE(m_shared_texture);
	SAFE_RELEASE(m_keyed_mutex11);
	SAFE_RELEASE(m_keyed_mutex10);

	SAFE_RELEASE(m_d3d11_device_context);
	SAFE_RELEASE(m_d3d10_device);
	SAFE_RELEASE(m_d3d11_device);
	SAFE_RELEASE(m_swap_chain);
}

//////////////////////////////////////////////////////////////////////////
// public interfaces
//////////////////////////////////////////////////////////////////////////
bool D3DApp::Initialize(HINSTANCE hinstance, int width, int height)
{
	m_hinstance = hinstance;
	m_width = width;
	m_height = height;

	if (!InitializeWindow())
	{
		MessageBox(NULL, TEXT("Error creating window"), TEXT("Error"), MB_OK | MB_ICONERROR);
		return false;
	}
	if (!InitializeD3D())
	{
		MessageBox(NULL, TEXT("Error initializing D3D"), TEXT("Error"), MB_OK | MB_ICONERROR);
		return false;
	}
	if (!InitializeScene())
	{
		MessageBox(NULL, TEXT("Error initializing scene"), TEXT("Error"), MB_OK | MB_ICONERROR);
		return false;
	}
	if (!InitializeShaders())
	{
		MessageBox(NULL, TEXT("Error initializing shaders"), TEXT("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	m_text_editor = TextEditorPtr(new TextEditor);
	if (!m_text_editor->Initialize(m_d2d_rendertarget))
	{
		MessageBox(NULL, TEXT("Error initializing text editor"), TEXT("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	m_sound_player = SoundPlayerPtr(new SoundPlayer);
	if (!m_sound_player->Initialize())
	{
		MessageBox(NULL, TEXT("Error initializing sound player"), TEXT("Error"), MB_OK | MB_ICONERROR);
		return false;
	}
	m_sound_player->PlaySound(TEXT("media/bgm.mp3"), true);

	RegisterTimerEvents();
	return true;
}

int D3DApp::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_timer.Tick();
			UpdateScene(m_timer.GetDeltaTime());
			RenderScene();
		}
	}
	return msg.wParam;
}

void D3DApp::Destroy()
{

}

int D3DApp::GetWidth() const
{
	return m_width;
}

int D3DApp::GetHeight() const
{
	return m_height;
}

//////////////////////////////////////////////////////////////////////////
// window procedure
//////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK D3DApp::WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	D3DApp* app = D3DApp::GetApp();
	TextEditorPtr editor = app->m_text_editor;
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	if (editor && editor->HandleWindowMessage(hwnd, message, wparam, lparam))
	{
		return 0;
	}

	return DefWindowProc(hwnd, message, wparam, lparam);
}

//////////////////////////////////////////////////////////////////////////
// timer events procedure
//////////////////////////////////////////////////////////////////////////
void D3DApp::RegisterTimerEvents()
{
	m_timer.AddEvent(1.0f, boost::bind(&D3DApp::TimerEventsProc, this, _1, _2), TEXT("update_fps"));
}

void D3DApp::TimerEventsProc(int cnt, const tstring& tag)
{
	if (tag == TEXT("update_fps"))
	{
		float delta_time = m_timer.GetDeltaTime();
		int fps = static_cast<int>(1.0f / std::max(delta_time, Ayw::c_eps));
		tstring title = g_app_title + TEXT("  FPS: ") + to_string(fps);
		SetWindowText(m_hwnd, title.c_str());
	}
}

//////////////////////////////////////////////////////////////////////////
// private subroutines
//////////////////////////////////////////////////////////////////////////
bool D3DApp::InitializeWindow()
{
	WNDCLASSEX wc;
	tstring wnd_class = TEXT("aywd3dapp");
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = D3DApp::WindowProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = wnd_class.c_str();
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	RECT rect = {0, 0, m_width, m_height};
	DWORD wnd_style = WS_OVERLAPPEDWINDOW & (~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX));
	AdjustWindowRect(&rect, wnd_style, FALSE);
	m_hwnd = CreateWindowEx(
		NULL,
		wnd_class.c_str(),
		g_app_title.c_str(),
		wnd_style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		m_hinstance,
		this);

	if (m_hwnd == NULL)
	{
		return false;
	}

	ShowWindow(m_hwnd, SW_SHOWNORMAL);
	UpdateWindow(m_hwnd);

	GetClientRect(m_hwnd, &rect);
	m_width = rect.right - rect.left;
	m_height = rect.bottom - rect.top;
	return true;
}

bool D3DApp::InitializeD3D()
{
	// describe our SwapChain Buffer
	DXGI_MODE_DESC mode_desc;
	ZeroMemory(&mode_desc, sizeof(DXGI_MODE_DESC));
	mode_desc.Width = m_width;
	mode_desc.Height = m_height;
	mode_desc.RefreshRate.Numerator = 60;
	mode_desc.RefreshRate.Denominator = 1;
	mode_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	mode_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	mode_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapchain_desc; 
	ZeroMemory(&swapchain_desc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapchain_desc.BufferDesc = mode_desc;
	swapchain_desc.SampleDesc.Count = 1;
	swapchain_desc.SampleDesc.Quality = 0;
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc.BufferCount = 1;
	swapchain_desc.OutputWindow = m_hwnd; 
	swapchain_desc.Windowed = TRUE;
	swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// create DXGI factory to enumerate adapters
	IDXGIFactory1 *dxgi_factory;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgi_factory);	

	// use the first adapter	
	IDXGIAdapter1 *adapter;
	hr = dxgi_factory->EnumAdapters1(0, &adapter);
	dxgi_factory->Release();

	// create our Direct3D 11 Device and SwapChain
	hr = D3D11CreateDeviceAndSwapChain(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		NULL, NULL,	D3D11_SDK_VERSION, &swapchain_desc, &m_swap_chain, &m_d3d11_device, NULL, &m_d3d11_device_context);

	// initialize Direct2D, Direct3D 10.1, DirectWrite
	InitializeDWrite(adapter);

	// release the Adapter interface
	adapter->Release();

	// create our BackBuffer and Render Target
	hr = m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_back_buffer);
	hr = m_d3d11_device->CreateRenderTargetView(m_back_buffer, NULL, &m_rendertarget_view);

	// describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthstencil_desc;
	depthstencil_desc.Width     = m_width;
	depthstencil_desc.Height    = m_height;
	depthstencil_desc.MipLevels = 1;
	depthstencil_desc.ArraySize = 1;
	depthstencil_desc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthstencil_desc.SampleDesc.Count   = 1;
	depthstencil_desc.SampleDesc.Quality = 0;
	depthstencil_desc.Usage          = D3D11_USAGE_DEFAULT;
	depthstencil_desc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
	depthstencil_desc.CPUAccessFlags = 0; 
	depthstencil_desc.MiscFlags      = 0;

	// create the Depth/Stencil View
	m_d3d11_device->CreateTexture2D(&depthstencil_desc, NULL, &m_depthstencil_buffer);
	m_d3d11_device->CreateDepthStencilView(m_depthstencil_buffer, NULL, &m_depthstencil_view);
	
	// set render target views and depth stencil view
	m_d3d11_device_context->OMSetRenderTargets(1, &m_rendertarget_view, m_depthstencil_view);

	// set view port
	D3D11_VIEWPORT view_port;
	view_port.Width = static_cast<float>(m_width);
	view_port.Height = static_cast<float>(m_height);
	view_port.MinDepth = 0.0f;
	view_port.MaxDepth = 1.0f;
	view_port.TopLeftX = 0;
	view_port.TopLeftY = 0;
	m_d3d11_device_context->RSSetViewports(1, &view_port);

	// create a shader resource review from the texture D2D will render to
	hr = m_d3d11_device->CreateShaderResourceView(m_shared_texture, NULL, &m_d2d_texture);

	// create a constant buffer
	D3D11_BUFFER_DESC buffer_desc;
	ZeroMemory(&buffer_desc, sizeof(D3D11_BUFFER_DESC));
	buffer_desc.Usage	  = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = sizeof(ShaderParameters);
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	hr = m_d3d11_device->CreateBuffer(&buffer_desc, NULL, &m_parameter_buffer);

	return true;
}

bool D3DApp::InitializeDWrite(IDXGIAdapter1* adapter)
{
	// create our Direc3D 10.1 Device
	HRESULT hr = D3D10CreateDevice1(adapter, D3D10_DRIVER_TYPE_HARDWARE, NULL,D3D10_CREATE_DEVICE_BGRA_SUPPORT,
		D3D10_FEATURE_LEVEL_9_3, D3D10_1_SDK_VERSION, &m_d3d10_device);	

	// create Shared Texture that Direct3D 10.1 will render on
	D3D11_TEXTURE2D_DESC shared_tex_desc;	
	ZeroMemory(&shared_tex_desc, sizeof(shared_tex_desc));
	shared_tex_desc.Width = m_width;
	shared_tex_desc.Height = m_height;	
	shared_tex_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	shared_tex_desc.MipLevels = 1;	
	shared_tex_desc.ArraySize = 1;
	shared_tex_desc.SampleDesc.Count = 1;
	shared_tex_desc.Usage = D3D11_USAGE_DEFAULT;
	shared_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;	
	shared_tex_desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
	hr = m_d3d11_device->CreateTexture2D(&shared_tex_desc, NULL, &m_shared_texture);

	// get the keyed mutex for the shared texture (for D3D11)
	hr = m_shared_texture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&m_keyed_mutex11);	

	// get the shared handle needed to open the shared texture in D3D10.1
	IDXGIResource *shared_resource10;
	HANDLE shared_handle10;
	hr = m_shared_texture->QueryInterface(__uuidof(IDXGIResource), (void**)&shared_resource10);
	hr = shared_resource10->GetSharedHandle(&shared_handle10);	
	shared_resource10->Release();

	// open the surface for the shared texture in D3D10.1
	IDXGISurface1 *shared_surface10;
	hr = m_d3d10_device->OpenSharedResource(shared_handle10, __uuidof(IDXGISurface1), (void**)(&shared_surface10));
	hr = shared_surface10->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&m_keyed_mutex10);	

	// create D2D factory
	ID2D1Factory *d2d_factory;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), (void**)&d2d_factory);
	D2D1_RENDER_TARGET_PROPERTIES rendertaget_properties;
	ZeroMemory(&rendertaget_properties, sizeof(rendertaget_properties));
	rendertaget_properties.type = D2D1_RENDER_TARGET_TYPE_HARDWARE;
	rendertaget_properties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED);	
	hr = d2d_factory->CreateDxgiSurfaceRenderTarget(shared_surface10, &rendertaget_properties, &m_d2d_rendertarget);
	shared_surface10->Release();
	d2d_factory->Release();	

	// DirectWrite
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&m_dwrite_factory));

	m_d3d10_device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);	
	return true;
}

bool D3DApp::InitializeScene()
{
	return true;
}


bool D3DApp::InitializeShaders()
{
	m_custom_pp = PostProcessPtr(new PostProcess);

	m_copy_pp = PostProcessPtr(new PostProcess(true));
	if (!m_copy_pp->LoadPixelShaderFromFile(TEXT("pp_copy.hlsl"), TEXT("ps_main")))
	{
		return false;
	}
	return true;
}

void D3DApp::UpdateScene(float delta_time)
{
	m_text_editor->Update(delta_time);
	m_sound_player->Update(delta_time);

	// get sound spectrum
	std::vector<float> spectrum;
	m_sound_player->GetSpectrum(256, spectrum);
	float low_freq		= *std::max_element(spectrum.begin(),	   spectrum.begin() + 1 );
	float mid_low_freq	= *std::max_element(spectrum.begin() + 1,  spectrum.begin() + 5 );
	float mid_high_freq = *std::max_element(spectrum.begin() + 5,  spectrum.begin() + 21);
	float high_freq		= *std::max_element(spectrum.begin() + 21, spectrum.begin() + 85);

	// update shader parameters
	ShaderParameters param;
	param.time = float4(m_timer.GetTime(), 0, 0, 0);
	param.view = float4(static_cast<float>(m_width), static_cast<float>(m_height), 1.0f / m_width, 1.0f / m_height);
	param.fft = float4(low_freq, mid_low_freq, mid_high_freq, high_freq);
	void *tmp = m_parameter_buffer;
	m_d3d11_device_context->UpdateSubresource(m_parameter_buffer, 0, NULL, &param, 0, 0);
	m_copy_pp->SetParameters(0, m_parameter_buffer);
}

void D3DApp::RenderScene()
{
	m_d3d11_device_context->ClearRenderTargetView(m_rendertarget_view, float4(0, 0, 0, 1).ptr());
	m_d3d11_device_context->ClearDepthStencilView(m_depthstencil_view, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 255);
	
	m_custom_pp->OutputPin(0, m_rendertarget_view);
	m_custom_pp->Apply();

	RenderOverlay();
	m_copy_pp->InputPin(0, m_d2d_texture);
	m_copy_pp->OutputPin(0, m_rendertarget_view);
	m_copy_pp->Apply();

	m_swap_chain->Present(0, 0);
}

void D3DApp::RenderOverlay()
{
	// release the d3d11 device
	m_keyed_mutex11->ReleaseSync(0);

	// switch to the d3d10.1 device
	m_keyed_mutex10->AcquireSync(0, 5);

	// draw d2d content
	m_d2d_rendertarget->BeginDraw();

	// clear d2d background
	m_d2d_rendertarget->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));

	// draw text editor
	m_text_editor->Render(m_d2d_rendertarget);

	// end draw
	m_d2d_rendertarget->EndDraw();

	// release the d3d10.1 device
	m_keyed_mutex10->ReleaseSync(1);

	// switch back to the d3d11 device
	m_keyed_mutex11->AcquireSync(1, 5);
}