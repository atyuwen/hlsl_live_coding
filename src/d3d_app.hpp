#ifndef _D3D_APP_HPP_INCLUDED_
#define _D3D_APP_HPP_INCLUDED_

#include "hr_timer.hpp"
#include "post_process.hpp"
#include "text_editor.hpp"
#include "sound_player.hpp"

class D3DApp
{
public:
	D3DApp();
	virtual ~D3DApp();

public:
	bool Initialize(HINSTANCE hinstance, int width, int height);
	int Run();
	void Destroy();

	int GetWidth() const;
	int GetHeight() const;

public:
	static D3DApp* GetApp();
	static ID3D11Device* GetD3D11Device();
	static ID3D11DeviceContext* GetD3D11DeviceContext();
	static IDWriteFactory* GetDWriteFactory();
	static HRTimer* GetTimer();
	static PostProcessPtr GetPostProcess();
	static SoundPlayerPtr GetSoundPlayer();

public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	void RegisterTimerEvents();
	void TimerEventsProc(int cnt, const tstring& tag);

private:
	bool InitializeWindow();
	bool InitializeD3D();
	bool InitializeScene();
	bool InitializeShaders();
	
	void UpdateScene(float delta_time);
	void RenderScene();

	bool InitializeDWrite(IDXGIAdapter1* adapter);
	void RenderOverlay();

private:
	HINSTANCE m_hinstance;
	int m_width;
	int m_height;
	HWND m_hwnd;
	HRTimer m_timer;

	IDXGISwapChain* m_swap_chain;
	ID3D11Device* m_d3d11_device;
	ID3D11DeviceContext* m_d3d11_device_context;
	ID3D11Texture2D* m_depthstencil_buffer;
	ID3D11RenderTargetView* m_rendertarget_view;
	ID3D11DepthStencilView* m_depthstencil_view;
	ID3D11Texture2D* m_back_buffer;

	ID3D10Device1* m_d3d10_device;
	IDWriteFactory* m_dwrite_factory;
	ID2D1RenderTarget* m_d2d_rendertarget;
	ID3D11ShaderResourceView* m_d2d_texture;

	ID3D11Texture2D* m_shared_texture;
	IDXGIKeyedMutex* m_keyed_mutex11;
	IDXGIKeyedMutex* m_keyed_mutex10;

	PostProcessPtr m_copy_pp;
	PostProcessPtr m_custom_pp;
	ID3D11Buffer* m_parameter_buffer;
	ID3D11ShaderResourceView* m_custom_texture;

	TextEditorPtr m_text_editor;
	bool m_hide_editor;
	SoundPlayerPtr m_sound_player;
};

#endif  // _D3D_APP_HPP_INCLUDED_