#ifndef _TEXT_EDITOR_INCLUDED_HPP_
#define _TEXT_EDITOR_INCLUDED_HPP_

#include <boost/shared_ptr.hpp>
#include <map>
#include "syntax_highlighter.hpp"
#include "editable_text.hpp"

class TextEditor;
typedef boost::shared_ptr<TextEditor> TextEditorPtr;

class TextEditor
{
	struct CompileError
	{
		int row;
		int column;
		std::wstring message;

		bool is_located;
		float2 location;
		float remain_time;
		float alpha;

		CompileError();
		void Clear();
	};

public:
	TextEditor();
	virtual ~TextEditor();

public:
	bool Initialize(ID2D1RenderTarget* d2d_rt);
	bool HandleWindowMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	void Update(float delta_time);
	void Render(ID2D1RenderTarget* d2d_rt) const;

	const std::wstring& GetText() const;

	float2 GetMousePos(int from_event = 0) const;
	float GetMouseWheel() const;

private:
	void RefreshTextLayout();

	void AutoIndent();
	void AutoJumpOver();
	void AutoJumpInto();
	void AutoJumpOut();

	void ReloadPixelShader();
	void ParseCompileError(const tstring& fxc_error);

	void OpenFile();
	void SaveFile(bool save_as_new = false);

	void OnMousePress(UINT message, float x, float y);
	void OnMouseRelease(UINT message, float x, float y);
	void OnMouseMove(float x, float y);
	void OnMouseScroll(float x_scroll, float y_scroll);
	void OnMouseExit();
	void OnKeyPress(UINT32 key_code);
	void OnKeyCharacter(UINT32 char_code);

private:
	EditableText m_editable_text;
	std::wstring m_file_path;

	SyntaxHighlighter m_syntax_hightlighter;
	CompileError m_compile_error;

	size_t m_line_offset;
	float3 m_caret_loc_hight;
	float m_caret_idle_time;
	std::vector<float4> m_selection_fields;

	std::map<int, float2> m_recorded_mouse_pos;
	float2 m_mouse_pos;
	float m_mouse_wheel;

	IDWriteTextFormat* m_text_format;
	IDWriteTextFormat* m_text_format_small;
	IDWriteTextLayout* m_text_layout;
	ID2D1SolidColorBrush* m_default_brush;
};

#endif  // _TEXT_EDITOR_INCLUDED_HPP_
