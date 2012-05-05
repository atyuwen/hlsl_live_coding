#include "common.hpp"
#include "post_process.hpp"
#include "d3d_app.hpp"

#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// constructor / destructor
//////////////////////////////////////////////////////////////////////////
PostProcess::PostProcess(bool blend_enable /* = false */)
	: m_vertex_buffer(NULL)
	, m_index_buffer(NULL)
	, m_vertex_shader(NULL)
	, m_pixel_shader(NULL)
	, m_vertex_layout(NULL)
	, m_raster_state(NULL)
	, m_blend_state(NULL)
	, m_sampler_state(NULL)
{
	InitVertexBuffer();
	InitIndexBuffer();
	InitVertexShader();

	InitRasterState();
	InitBlendState(blend_enable);
	InitSamplerState();

	LoadPixelShaderFromFile(TEXT("pp_common.hlsl"), TEXT("ps_main"));
}

PostProcess::~PostProcess()
{
	SAFE_RELEASE(m_vertex_buffer);
	SAFE_RELEASE(m_index_buffer);
	SAFE_RELEASE(m_vertex_shader);
	SAFE_RELEASE(m_pixel_shader);
	SAFE_RELEASE(m_vertex_layout);
	SAFE_RELEASE(m_raster_state);
	SAFE_RELEASE(m_blend_state);
	SAFE_RELEASE(m_sampler_state);
}

//////////////////////////////////////////////////////////////////////////
// public interfaces
//////////////////////////////////////////////////////////////////////////
void PostProcess::Apply() const
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	D3DApp::GetD3D11DeviceContext()->IASetVertexBuffers(0, 1, &m_vertex_buffer, &stride, &offset);
	D3DApp::GetD3D11DeviceContext()->IASetIndexBuffer(m_index_buffer, DXGI_FORMAT_R32_UINT, 0);

	D3DApp::GetD3D11DeviceContext()->IASetInputLayout(m_vertex_layout);
	D3DApp::GetD3D11DeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11ShaderResourceView* srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {NULL};
	for each(auto &it in m_input_pins)
	{
		srvs[it.first] = it.second;
		D3DApp::GetD3D11DeviceContext()->PSSetSamplers(0, 1, &m_sampler_state);
	}
	D3DApp::GetD3D11DeviceContext()->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

	ID3D11RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {NULL};
	for each(auto &it in m_output_pins) {rtvs[it.first] = it.second;}
	D3DApp::GetD3D11DeviceContext()->OMSetRenderTargets(ARRAYSIZE(rtvs), rtvs, NULL);

	D3DApp::GetD3D11DeviceContext()->RSSetState(m_raster_state);
	D3DApp::GetD3D11DeviceContext()->OMSetBlendState(m_blend_state, NULL, 0xFFFFFFFF);

	D3DApp::GetD3D11DeviceContext()->VSSetShader(m_vertex_shader, NULL, 0);
	D3DApp::GetD3D11DeviceContext()->PSSetShader(m_pixel_shader, NULL, 0);
	D3DApp::GetD3D11DeviceContext()->DrawIndexed(6, 0, 0);
}

void PostProcess::InputPin(int slot, ID3D11ShaderResourceView* srv)
{
	m_input_pins[slot] = srv;
}

void PostProcess::OutputPin(int slot, ID3D11RenderTargetView* rtv)
{
	m_output_pins[slot] = rtv;
}

void PostProcess::SetParameters(int slot, ID3D11Buffer* cbuffer)
{
	D3DApp::GetD3D11DeviceContext()->PSSetConstantBuffers(slot, 1, &cbuffer);
}

//////////////////////////////////////////////////////////////////////////
// private subroutines
//////////////////////////////////////////////////////////////////////////
void PostProcess::InitVertexBuffer()
{
	Vertex vertices[] = {
		{float3(-1.0f, -1.0f, 0.0f), float2(0.0f, 1.0f)},
		{float3(-1.0f,  1.0f, 0.0f), float2(0.0f, 0.0f)},
		{float3( 1.0f,  1.0f, 0.0f), float2(1.0f, 0.0f)},
		{float3( 1.0f, -1.0f, 0.0f), float2(1.0f, 1.0f)},
	};

	D3D11_BUFFER_DESC vertex_buffer_desc;
	ZeroMemory(&vertex_buffer_desc, sizeof(vertex_buffer_desc));
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = sizeof(vertices);
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = 0;
	vertex_buffer_desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertices_data; 
	ZeroMemory(&vertices_data, sizeof(vertices_data));
	vertices_data.pSysMem = vertices;
	D3DApp::GetD3D11Device()->CreateBuffer(&vertex_buffer_desc, &vertices_data, &m_vertex_buffer);
}

void PostProcess::InitIndexBuffer()
{
	unsigned int indices[] = {
		0, 1, 2,
		0, 2, 3,
	};

	D3D11_BUFFER_DESC index_buffer_desc;
	ZeroMemory(&index_buffer_desc, sizeof(index_buffer_desc));
	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = sizeof(indices);
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer_desc.CPUAccessFlags = 0;
	index_buffer_desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA indices_data;
	ZeroMemory(&indices_data, sizeof(indices_data));
	indices_data.pSysMem = indices;
	D3DApp::GetD3D11Device()->CreateBuffer(&index_buffer_desc, &indices_data, &m_index_buffer);
}

void PostProcess::InitVertexShader()
{
	ID3DBlob *vertex_shader_buffer = NULL;
	D3DX11CompileFromFile(
		TEXT("fx/pp_common.hlsl"),
		NULL,
		NULL,
		TEXT("vs_main"),
		TEXT("vs_4_0"),
		0,
		0,
		NULL,
		&vertex_shader_buffer,
		NULL,
		NULL);

	D3DApp::GetD3D11Device()->CreateVertexShader(
		vertex_shader_buffer->GetBufferPointer(),
		vertex_shader_buffer->GetBufferSize(),
		NULL,
		&m_vertex_shader);

	// create vertex layout
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},  
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},  
	};
	UINT num_elements = ARRAYSIZE(layout);
	D3DApp::GetD3D11Device()->CreateInputLayout(
		layout,
		num_elements,
		vertex_shader_buffer->GetBufferPointer(),
		vertex_shader_buffer->GetBufferSize(),
		&m_vertex_layout);

	SAFE_RELEASE(vertex_shader_buffer);
}

void PostProcess::InitRasterState()
{
	CD3D11_RASTERIZER_DESC raster_state_desc(D3D11_DEFAULT);
	raster_state_desc.CullMode = D3D11_CULL_NONE;
	raster_state_desc.DepthClipEnable = false;
	D3DApp::GetD3D11Device()->CreateRasterizerState(&raster_state_desc, &m_raster_state);
}

void PostProcess::InitBlendState(bool blend_enable)
{
	D3D11_BLEND_DESC blend_desc;
	D3D11_RENDER_TARGET_BLEND_DESC target_blend_desc;
	target_blend_desc.BlendEnable = blend_enable;
	target_blend_desc.SrcBlend = D3D11_BLEND_SRC_COLOR;
	target_blend_desc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	target_blend_desc.BlendOp = D3D11_BLEND_OP_ADD;
	target_blend_desc.SrcBlendAlpha = D3D11_BLEND_ONE;
	target_blend_desc.DestBlendAlpha = D3D11_BLEND_ZERO;
	target_blend_desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	target_blend_desc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	blend_desc.AlphaToCoverageEnable = false;
	blend_desc.IndependentBlendEnable = false;
	blend_desc.RenderTarget[0] = target_blend_desc;
	D3DApp::GetD3D11Device()->CreateBlendState(&blend_desc, &m_blend_state);
}

void PostProcess::InitSamplerState()
{
	CD3D11_SAMPLER_DESC sampler_desc(D3D11_DEFAULT);
	D3DApp::GetD3D11Device()->CreateSamplerState(&sampler_desc, &m_sampler_state);
}

bool PostProcess::LoadPixelShaderFromFile(const tstring& file_name, const tstring& entry_point)
{
	tstring file_path = TEXT("fx/") + file_name;
	ID3DBlob* error_buffer = NULL;
	ID3DBlob* pixel_shader_buffer = NULL;
	ID3D11PixelShader* pixel_shader = NULL;

	HRESULT hr = D3DX11CompileFromFile(
					file_path.c_str(),
					NULL,
					NULL,
					entry_point.c_str(),
					TEXT("ps_4_0"),
					0,
					0,
					NULL,
					&pixel_shader_buffer,
					&error_buffer,
					NULL);

	if (FAILED(hr))
	{
		if (error_buffer != NULL)
		{
			m_error_message = tstring(static_cast<tchar*>(error_buffer->GetBufferPointer()));
		}
		SAFE_RELEASE(pixel_shader_buffer);
		SAFE_RELEASE(error_buffer);
		return false;
	}

	hr = D3DApp::GetD3D11Device()->CreatePixelShader(
			pixel_shader_buffer->GetBufferPointer(),
			pixel_shader_buffer->GetBufferSize(),
			NULL,
			&pixel_shader);

	if (FAILED(hr))
	{
		m_error_message = TEXT("create pixel shader failed.");
		SAFE_RELEASE(pixel_shader_buffer);
		SAFE_RELEASE(error_buffer);
		return false;
	}

	SAFE_RELEASE(m_pixel_shader);
	m_pixel_shader = pixel_shader;
	SAFE_RELEASE(pixel_shader_buffer);
	SAFE_RELEASE(error_buffer);
	return true;
}

bool PostProcess::LoadPixelShaderFromMemory(const tstring& shader_content, const tstring& entry_point)
{
	ID3DBlob* error_buffer = NULL;
	ID3DBlob* pixel_shader_buffer = NULL;
	ID3D11PixelShader* pixel_shader = NULL;

	HRESULT hr = D3DX11CompileFromMemory(
		shader_content.c_str(),
		shader_content.length(),
		NULL,
		NULL,
		NULL,
		entry_point.c_str(),
		TEXT("ps_4_0"),
		0,
		0,
		NULL,
		&pixel_shader_buffer,
		&error_buffer,
		NULL);

	if (FAILED(hr))
	{
		if (error_buffer != NULL)
		{
			m_error_message = tstring(static_cast<tchar*>(error_buffer->GetBufferPointer()));
		}
		SAFE_RELEASE(pixel_shader_buffer);
		SAFE_RELEASE(error_buffer);
		return false;
	}

	hr = D3DApp::GetD3D11Device()->CreatePixelShader(
		pixel_shader_buffer->GetBufferPointer(),
		pixel_shader_buffer->GetBufferSize(),
		NULL,
		&pixel_shader);

	if (FAILED(hr))
	{
		m_error_message = TEXT("create pixel shader failed.");
		SAFE_RELEASE(pixel_shader_buffer);
		SAFE_RELEASE(error_buffer);
		return false;
	}

	SAFE_RELEASE(m_pixel_shader);
	m_pixel_shader = pixel_shader;
	SAFE_RELEASE(pixel_shader_buffer);
	SAFE_RELEASE(error_buffer);
	return true;
}

tstring PostProcess::GetErrorMessage() const
{
	return m_error_message;
}
