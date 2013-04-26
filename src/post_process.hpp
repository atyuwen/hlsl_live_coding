#ifndef _POST_PROCESS_INCLUDED_HPP_
#define _POST_PROCESS_INCLUDED_HPP_

#include <map>
#include <boost/shared_ptr.hpp>

class PostProcess;
typedef boost::shared_ptr<PostProcess> PostProcessPtr;

class PostProcess
{
	struct Vertex {
		float3 pos;
		float2 tex;
	};

public:
	PostProcess(bool blend_enable = false);
	virtual ~PostProcess();

public:
	void Apply() const;

	bool LoadPixelShaderFromFile(const tstring& file_name, const tstring& entry_point);
	bool LoadPixelShaderFromMemory(const tstring& shader_content, const tstring& entry_point);
	tstring GetErrorMessage() const;

	void InputPin(int slot, ID3D11ShaderResourceView* srv);
	void OutputPin(int slot, ID3D11RenderTargetView* rtv);
	void SetParameters(int slot, ID3D11Buffer* cbuffer);

private:
	void InitVertexBuffer();
	void InitIndexBuffer();
	void InitVertexShader();

	void InitRasterState();
	void InitBlendState(bool blend_enable);
	void InitSamplerState();
	void InitDepthStencilState();

private:
	ID3D11Buffer* m_vertex_buffer;
	ID3D11Buffer* m_index_buffer;

	ID3D11VertexShader* m_vertex_shader;
	ID3D11PixelShader* m_pixel_shader;
	ID3D11InputLayout* m_vertex_layout;

	ID3D11RasterizerState* m_raster_state;
	ID3D11BlendState* m_blend_state;
	ID3D11SamplerState* m_sampler_state;
	ID3D11DepthStencilState* m_depth_stencil_state;
	tstring m_error_message;

	std::map<int, ID3D11ShaderResourceView*> m_input_pins;
	std::map<int, ID3D11RenderTargetView*> m_output_pins;
};

#endif  // _POST_PROCESS_INCLUDED_HPP_