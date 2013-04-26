#include "common.hpp"

#include <algorithm>
#include <numeric>
#include <boost/bind.hpp>
#include "ayw/vector.hpp"
#include "ayw/constant.hpp"
#include "d3d_app.hpp"
using namespace Ayw;

const tstring g_app_title = TEXT("LIVE CODING");

const int g_aa_sample = 8;
const float g_aa_waiting_time = 0.5f;

struct ShaderParameters
{
	float4 time;		// time related parameters
	float4 view;		// (view width, view height, 1 / width, 1 / height)
	float4 freq;		// the short time FFT of the background music
	float4 mpos;        // (mouse.x, mouse.y, mouse.wheel, 0)
};

ShaderParameters g_shader_param;

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
	, m_back_buffer_rtv(NULL)
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
	, m_jitter_buffer(NULL)
	, m_custom_texture(NULL)
	, m_hide_editor(false)
	, m_mouse_wheel(0)
	, m_antialiasing(false)
	, m_aa_control_time(0)
	, m_aa_frame(false)
{
	ZeroMemory(m_offscreen_textures, sizeof(m_offscreen_textures));
	ZeroMemory(m_offscreen_srvs, sizeof(m_offscreen_srvs));
	ZeroMemory(m_offscreen_rtvs, sizeof(m_offscreen_rtvs));
}

D3DApp::~D3DApp()
{
	SAFE_RELEASE(m_jitter_buffer);
	SAFE_RELEASE(m_parameter_buffer);
	SAFE_RELEASE(m_custom_texture);

	for (int i = 0; i != ARRAYSIZE(m_offscreen_textures); ++i)
	{
		SAFE_RELEASE(m_offscreen_rtvs[i]);
		SAFE_RELEASE(m_offscreen_srvs[i]);
		SAFE_RELEASE(m_offscreen_textures[i]);
	}

	SAFE_RELEASE(m_depthstencil_buffer);
	SAFE_RELEASE(m_back_buffer_rtv);
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
			if (m_timer.SyncTick(1.0f / 60.0f))
			{
				UpdateScene(m_timer.GetDeltaTime());
				RenderScene();
			}
			else
			{
				timeBeginPeriod(1);
				Sleep(1);
				timeEndPeriod(1);
			}
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

float2 D3DApp::GetMousePos(int from_event /*= 0*/) const
{
	if (from_event == 0) return m_mouse_pos;

	if (m_recorded_mouse_pos.find(from_event) != m_recorded_mouse_pos.end())
	{
		return m_mouse_pos - m_recorded_mouse_pos.at(from_event);
	}
	return float2(0, 0);
}

float D3DApp::GetMouseWheel() const
{
	return m_mouse_wheel;
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
	case WM_KEYDOWN:
		{
			bool held_control = (GetKeyState(VK_CONTROL) & 0x80) != 0;
			bool held_shift   = (GetKeyState(VK_SHIFT)   & 0x80) != 0;
			UINT key_code = static_cast<UINT>(wparam);

			if (key_code == VK_F1)
			{
				app->m_hide_editor = !app->m_hide_editor;
				return 0;
			}
			else if (key_code == VK_F2)
			{
				app->m_antialiasing = !app->m_antialiasing;
				app->m_aa_control_time = 0;
				return 0;
			}
			else if (key_code == 'M' && held_control)
			{
				if (app->m_sound_player) app->m_sound_player->ToggleMute();
				return 0;
			}
			else if (key_code == VK_OEM_PLUS && held_control)
			{
				if (app->m_sound_player) app->m_sound_player->ChangeVolume(0.1f);
				return 0;
			}
			else if (key_code == VK_OEM_MINUS && held_control)
			{
				if (app->m_sound_player) app->m_sound_player->ChangeVolume(-0.1f);
				return 0;
			}
			else if (key_code == 'N' && held_control)
			{
				if (editor) editor->NewFile();
				app->m_recorded_mouse_pos.clear();
				app->m_mouse_wheel = 0;
				app->m_hide_editor = false;
				g_shader_param.mpos = float4(0, 0, 0, 0);
				app->m_aa_control_time = 0;
				return 0;
			}
			else if (key_code == 'O' && held_control)
			{
				if (editor) editor->OpenFile();
				app->m_recorded_mouse_pos.clear();
				app->m_mouse_wheel = 0;
				app->m_hide_editor = false;
				g_shader_param.mpos = float4(0, 0, 0, 0);
				app->m_aa_control_time = 0;
				return 0;
			}
			else if (key_code == 'S' && held_control)
			{
				if (editor) editor->SaveFile(held_shift);
				app->m_aa_control_time = 0;
				return 0;
			}
			break;
		}
	
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		app->m_recorded_mouse_pos[message] = float2(
			static_cast<float>(GET_X_LPARAM(lparam)), static_cast<float>(GET_Y_LPARAM(lparam)));
		break;

	case WM_LBUTTONUP:
		app->m_recorded_mouse_pos.erase(WM_LBUTTONDOWN);
		break;
	case WM_RBUTTONUP:
		app->m_recorded_mouse_pos.erase(WM_RBUTTONDOWN);
		break;
	case WM_MBUTTONUP:
		app->m_recorded_mouse_pos.erase(WM_MBUTTONDOWN);
		app->m_mouse_wheel = 0;
		break;

	case WM_MOUSEMOVE:
		app->m_mouse_pos = float2(
			static_cast<float>(GET_X_LPARAM(lparam)), static_cast<float>(GET_Y_LPARAM(lparam)));
		break;

	case WM_MOUSEWHEEL:
		app->m_mouse_wheel += GET_WHEEL_DELTA_WPARAM(wparam) / static_cast<float>(WHEEL_DELTA);
		break;;
	}

	if (editor && !app->m_hide_editor)
	{
		if (editor->HandleWindowMessage(hwnd, message, wparam, lparam))
		{
			return 0;
		}
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
	hr = m_d3d11_device->CreateRenderTargetView(m_back_buffer, NULL, &m_back_buffer_rtv);

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
	m_d3d11_device_context->OMSetRenderTargets(1, &m_back_buffer_rtv, m_depthstencil_view);

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

	// create off-screen textures
	CD3D11_TEXTURE2D_DESC offscreen_tex_desc(DXGI_FORMAT_R16G16B16A16_FLOAT, m_width, m_height);
	offscreen_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	offscreen_tex_desc.MipLevels = 1;
	for (int i = 0; i != ARRAYSIZE(m_offscreen_textures); ++i)
	{
		m_d3d11_device->CreateTexture2D(&offscreen_tex_desc, NULL, &m_offscreen_textures[i]);
		m_d3d11_device->CreateShaderResourceView(m_offscreen_textures[i], NULL, &m_offscreen_srvs[i]);
		m_d3d11_device->CreateRenderTargetView(m_offscreen_textures[i], NULL, &m_offscreen_rtvs[i]);
	}

	// create a constant buffer
	D3D11_BUFFER_DESC buffer_desc;
	ZeroMemory(&buffer_desc, sizeof(D3D11_BUFFER_DESC));
	buffer_desc.Usage	  = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = sizeof(ShaderParameters);
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	hr = m_d3d11_device->CreateBuffer(&buffer_desc, NULL, &m_parameter_buffer);

	buffer_desc.ByteWidth = sizeof(float4);
	hr = m_d3d11_device->CreateBuffer(&buffer_desc, NULL, &m_jitter_buffer);

	// create a texture from file
	D3DX11CreateShaderResourceViewFromFile(m_d3d11_device, TEXT("media/tex.bmp"), NULL, NULL, &m_custom_texture, &hr);

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

	m_copy_pp = PostProcessPtr(new PostProcess);
	if (!m_copy_pp->LoadPixelShaderFromFile(TEXT("pp_copy.hlsl"), TEXT("ps_main")))
	{
		return false;
	}

	m_resolve_pp = PostProcessPtr(new PostProcess(true));
	if (!m_resolve_pp->LoadPixelShaderFromFile(TEXT("pp_resolve.hlsl"), TEXT("ps_main")))
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
	m_sound_player->GetSpectrum(1024, spectrum);
	float low_freq		= *std::max_element(spectrum.begin(),	    spectrum.begin() + 16 );
	float mid_low_freq	= *std::max_element(spectrum.begin() + 16,  spectrum.begin() + 64 );
	float mid_high_freq = *std::max_element(spectrum.begin() + 64,  spectrum.begin() + 128);
	float high_freq		= *std::max_element(spectrum.begin() + 128, spectrum.begin() + 256);
	float4 freq(low_freq, mid_low_freq, mid_high_freq, high_freq);

	// mouse interaction
	float2 mouse_pos = GetMousePos(WM_RBUTTONDOWN);
	float mouse_wheel = GetMouseWheel();
	float4 mpos(mouse_pos.x, mouse_pos.y, mouse_wheel, 0);
	float4 delta = mpos - g_shader_param.mpos;
	if (m_antialiasing)
	{
		if (delta.length_sqr() > 1e-3) m_aa_control_time = 0;
		else m_aa_control_time += std::min(delta_time, g_aa_waiting_time * 0.5f);
	}

	// update shader parameters
	const float smooth_factor = 0.8f;
	g_shader_param.time = float4(m_timer.GetTime(), 0, 0, 0);
	g_shader_param.view = float4(static_cast<float>(m_width), static_cast<float>(m_height), 1.0f / m_width, 1.0f / m_height);
	g_shader_param.freq = g_shader_param.freq * smooth_factor + freq * (1 - smooth_factor);
	g_shader_param.mpos = g_shader_param.mpos * smooth_factor + mpos * (1 - smooth_factor);
	m_d3d11_device_context->UpdateSubresource(m_parameter_buffer, 0, NULL, &g_shader_param, 0, 0);
	m_copy_pp->SetParameters(0, m_parameter_buffer);
}

void D3DApp::RenderScene()
{
	m_d3d11_device_context->ClearRenderTargetView(m_back_buffer_rtv, float4(0, 0, 0, 1).ptr());
	m_d3d11_device_context->ClearDepthStencilView(m_depthstencil_view, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 255);

	bool enable_aa = false;
	if (m_antialiasing && m_aa_control_time > g_aa_waiting_time)
	{
		enable_aa = true;
		m_aa_frame = (m_aa_frame + 1) % (g_aa_sample * g_aa_sample);

		float2 jitter(static_cast<float>(m_aa_frame % g_aa_sample), static_cast<float>(m_aa_frame / g_aa_sample));
		float offset = g_aa_sample / 2.0 + 0.5f;
		jitter = (jitter - float2(offset, offset)) / g_aa_sample;
		jitter /= float2(static_cast<float>(m_width), static_cast<float>(m_height));

		float4 jitter_param(jitter.x, jitter.y, 0, 0);
		m_d3d11_device_context->UpdateSubresource(m_jitter_buffer, 0, NULL, &jitter_param, 0, 0);
		m_d3d11_device_context->VSSetConstantBuffers(0, 1, &m_jitter_buffer);
	}
	else
	{
		m_aa_frame = 0;
	}

	if (m_custom_texture != NULL)
	{
		m_custom_pp->InputPin(0, m_custom_texture);
	}
	m_custom_pp->OutputPin(0, m_offscreen_rtvs[enable_aa]);
	m_custom_pp->Apply();

	if (enable_aa)
	{
		float4 jitter_param(0, 0, 0, 0);
		m_d3d11_device_context->UpdateSubresource(m_jitter_buffer, 0, NULL, &jitter_param, 0, 0);
		m_d3d11_device_context->VSSetConstantBuffers(0, 1, &m_jitter_buffer);

		m_resolve_pp->InputPin(0, m_offscreen_srvs[1]);
		m_resolve_pp->OutputPin(0, m_offscreen_rtvs[0]);
		m_resolve_pp->Apply();
	}

	RenderOverlay();
	m_copy_pp->InputPin(0, m_offscreen_srvs[0]);
	m_copy_pp->InputPin(1, m_d2d_texture);
	m_copy_pp->OutputPin(0, m_back_buffer_rtv);
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
	if (!m_hide_editor)
	{
		m_text_editor->Render(m_d2d_rendertarget);
	}

	// end draw
	m_d2d_rendertarget->EndDraw();

	// release the d3d10.1 device
	m_keyed_mutex10->ReleaseSync(1);

	// switch back to the d3d11 device
	m_keyed_mutex11->AcquireSync(1, 5);
}
