#include "common.hpp"
#include "text_editor.hpp"
#include "d3d_app.hpp"
#include "shader_header.hpp"

#include <commdlg.h>
#include <sstream>
#include <fstream>

//////////////////////////////////////////////////////////////////////////
// default shader content
//////////////////////////////////////////////////////////////////////////
const wchar_t default_shader_content[] =
	L"cbuffer Parameters\n"
	L"{\n"
	L"  float4 time;\n"
	L"  float4 view;\n"
	L"  float4 fft;\n"
	L"}\n"
	L"\n"
	L"float4 ps_main(in float2 in_tex : TEXCOORD) : SV_TARGET\n"
	L"{\n"
	L"  return float4(0.3, 0.3, 0.3, 1);\n"
	L"}\n"
	;

const size_t MAX_NUM_LINES = 25;

//////////////////////////////////////////////////////////////////////////
// constructor / destructor
//////////////////////////////////////////////////////////////////////////
TextEditor::TextEditor()
	: m_text_format(NULL)
	, m_text_format_small(NULL)
	, m_text_layout(NULL)
	, m_default_brush(NULL)
	, m_line_offset(0)
{

}

TextEditor::~TextEditor()
{
	SAFE_RELEASE(m_text_format);
	SAFE_RELEASE(m_text_format_small);
	SAFE_RELEASE(m_text_layout);
	SAFE_RELEASE(m_default_brush);
}

TextEditor::CompileError::CompileError()
{
	Clear();
}

void TextEditor::CompileError::Clear()
{
	row = -1;
	column = -1;
	message.clear();

	remain_time = 0;
	is_located = false;
	location = float2(0, 0);
	alpha = 0;
}

//////////////////////////////////////////////////////////////////////////
// public interfaces
//////////////////////////////////////////////////////////////////////////
bool TextEditor::Initialize(ID2D1RenderTarget* d2d_rt)
{
	HRESULT hr = S_OK;

	// create text format
	hr = D3DApp::GetDWriteFactory()->CreateTextFormat(
		L"Consolas",
		NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		16.0f,
		L"en-us",
		&m_text_format);

	if (FAILED(hr))
	{
		return false;
	}

	m_text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	m_text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	m_text_format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

	// create text format (small)
	hr = D3DApp::GetDWriteFactory()->CreateTextFormat(
		L"Consolas",
		NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		13.0f,
		L"en-us",
		&m_text_format_small);

	if (FAILED(hr))
	{
		return false;
	}

	m_text_format_small->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	m_text_format_small->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	m_text_format_small->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

	// init editable text
	m_editable_text.SetText(default_shader_content);

	// create text layout
	const int text_box_width = D3DApp::GetApp()->GetWidth() - 300;
	const int text_box_height = D3DApp::GetApp()->GetHeight() - 300;
	hr = D3DApp::GetDWriteFactory()->CreateTextLayout(
		m_editable_text.GetText().c_str(),
		m_editable_text.GetText().length(),
		m_text_format,
		static_cast<float>(text_box_width),
		static_cast<float>(text_box_height),
		&m_text_layout);

	if (FAILED(hr))
	{
		return false;
	}

	// create default brush
	hr = d2d_rt->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &m_default_brush);
	if (FAILED(hr))
	{
		return false;
	}

	// init syntax highlighter
	m_syntax_hightlighter.Intialize(d2d_rt);
	RefreshTextLayout();

	// init common headers for shader
	ShaderHeader::InitShaderHeader();

	return true;
}

void TextEditor::Update(float delta_time)
{
	m_caret_idle_time += delta_time;

	if (m_compile_error.remain_time > 0)
	{
		m_compile_error.remain_time -= delta_time;

		if (m_compile_error.remain_time > 1.0f)
		{
			m_compile_error.alpha = 1.0f;
		}
		else
		{
			m_compile_error.alpha = m_compile_error.remain_time;
		}
	}
}

void TextEditor::Render(ID2D1RenderTarget* d2d_rt) const
{
	D2D1_RECT_F rect = D2D1::RectF(100, 100, 200 + m_text_layout->GetMaxWidth(), 200 + m_text_layout->GetMaxHeight());

	// draw dynamic boarder
	d2d_rt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	m_default_brush->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.6f));
	float margin = cos(D3DApp::GetTimer()->GetTime() * 5) * 2 + 4;
	D2D1_RECT_F rect_dyn = D2D1::RectF(rect.left - margin, rect.top - margin, rect.right + margin, rect.bottom + margin);
	d2d_rt->DrawRectangle(rect_dyn, m_default_brush);

	// draw rect and border
	d2d_rt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	m_default_brush->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.3f));
	d2d_rt->FillRectangle(rect, m_default_brush);
	m_default_brush->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.6f));
	d2d_rt->DrawRectangle(rect, m_default_brush);

	// draw highlighted line's background
	m_default_brush->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.5f));
	D2D1_RECT_F rect_line = D2D1::RectF(rect.left, m_caret_loc_hight.y + 150, rect.right, m_caret_loc_hight.y + m_caret_loc_hight.z + 150);
	d2d_rt->FillRectangle(rect_line, m_default_brush);

	// draw selected text's background
	m_default_brush->SetColor(D2D1::ColorF(0.0f, 0.8f, 0.8f, 0.5f));
	for each(auto &it in m_selection_fields)
	{
		D2D1_RECT_F field = D2D1::RectF(it.x + 150, it.y + 150, it.x + it.z + 150, it.y + it.w + 150);
		d2d_rt->FillRectangle(field, m_default_brush);
	}

	// draw text
	m_default_brush->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f));
	d2d_rt->DrawTextLayout(D2D1::Point2F(150, 150), m_text_layout, m_default_brush);

	// draw caret
	m_default_brush->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f, abs(cos(m_caret_idle_time * 3.0f))));
	D2D1_POINT_2F pt1 = D2D1::Point2F(m_caret_loc_hight.x + 150, m_caret_loc_hight.y + 150);
	D2D1_POINT_2F pt2 = D2D1::Point2F(m_caret_loc_hight.x + 150, m_caret_loc_hight.y + m_caret_loc_hight.z + 150);
	d2d_rt->DrawLine(pt1, pt2, m_default_brush, 2.0f);

	// draw compile error tip
	if (m_compile_error.alpha > 0)
	{
		m_default_brush->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.9f * m_compile_error.alpha));
		d2d_rt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		float2 error_loc = m_compile_error.location + float2(150, 150);
		D2D1_RECT_F err_rect = D2D1::RectF(rect.left, error_loc.y - 16.0f, rect.right, error_loc.y);
		D2D1_ROUNDED_RECT tip_rect = D2D1::RoundedRect(err_rect, 3.0f, 3.0f);
		d2d_rt->FillRoundedRectangle(tip_rect, m_default_brush);

		m_default_brush->SetColor(D2D1::ColorF(0.7f, 0.0f, 0.0f, m_compile_error.alpha));
		if (!m_compile_error.is_located)
		{
			m_text_format_small->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		}
		else if (m_compile_error.location.x > m_text_layout->GetMaxWidth() / 2.0f)
		{
			m_text_format_small->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		}
		else
		{
			m_text_format_small->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
		}
		
		D2D1_RECT_F msg_rect =D2D1::RectF(err_rect.left + 10.0f, err_rect.top, err_rect.right - 10.0f, err_rect.bottom);
		d2d_rt->DrawTextA(m_compile_error.message.c_str(), m_compile_error.message.length(), m_text_format_small, msg_rect, m_default_brush);
		
		if (m_compile_error.is_located)
		{
			float2 arrow_loc = error_loc + float2(0, cos(D3DApp::GetTimer()->GetTime() * 5.0f) * 3.0f - 2.0f);
			d2d_rt->DrawLine(D2D1::Point2F(arrow_loc.x, arrow_loc.y),  D2D1::Point2F(arrow_loc.x, arrow_loc.y - 12.0f), m_default_brush);
			d2d_rt->DrawLine(D2D1::Point2F(arrow_loc.x, arrow_loc.y),  D2D1::Point2F(arrow_loc.x - 4.0f, arrow_loc.y - 5.0f), m_default_brush);
			d2d_rt->DrawLine(D2D1::Point2F(arrow_loc.x, arrow_loc.y),  D2D1::Point2F(arrow_loc.x + 4.0f, arrow_loc.y - 5.0f), m_default_brush);
		}
	}
}

const std::wstring& TextEditor::GetText() const
{
	return m_editable_text.GetText();
}

bool TextEditor::HandleWindowMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		OnKeyPress(static_cast<UINT>(wparam));
		return true;

	case WM_CHAR:
		OnKeyCharacter(static_cast<UINT>(wparam));
		return true;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		SetFocus(hwnd);
		SetCapture(hwnd);
		OnMousePress(message, static_cast<float>(GET_X_LPARAM(lparam)), static_cast<float>(GET_Y_LPARAM(lparam)));
		return true;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		ReleaseCapture();
		OnMouseRelease(message, static_cast<float>(GET_X_LPARAM(lparam)), static_cast<float>(GET_Y_LPARAM(lparam)));
		return true;

	case WM_MOUSEMOVE:
		OnMouseMove(static_cast<float>(GET_X_LPARAM(lparam)), static_cast<float>(GET_Y_LPARAM(lparam)));
		return true;

	case WM_MOUSEWHEEL:
		OnMouseScroll(0, GET_WHEEL_DELTA_WPARAM(wparam) / static_cast<float>(WHEEL_DELTA));
		return true;
	case WM_MOUSEHWHEEL:
		OnMouseScroll(GET_WHEEL_DELTA_WPARAM(lparam) / static_cast<float>(WHEEL_DELTA), 0);
		return true;

	case WM_MOUSELEAVE:
	case WM_CAPTURECHANGED:
		OnMouseExit();
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// private subroutines
//////////////////////////////////////////////////////////////////////////
void TextEditor::RefreshTextLayout()
{
	size_t cur_line = m_editable_text.GetCaretLine();
	if (m_line_offset > cur_line)
	{
		m_line_offset = cur_line;
	}
	else if (m_line_offset + MAX_NUM_LINES - 1 < cur_line)
	{
		m_line_offset = cur_line - MAX_NUM_LINES + 1;
	}

	size_t subtext_begin = m_editable_text.GetTextPos(m_line_offset, 0);
	size_t subtext_end = m_editable_text.GetTextPos(m_line_offset + MAX_NUM_LINES, 0);
	std::wstring subtext = m_editable_text.GetText().substr(subtext_begin, subtext_end - subtext_begin);
	IDWriteTextLayout* new_layout = NULL;
	HRESULT hr = D3DApp::GetDWriteFactory()->CreateTextLayout(
		subtext.c_str(),
		subtext.length(),
		m_text_format,
		static_cast<float>(m_text_layout->GetMaxWidth()),
		static_cast<float>(m_text_layout->GetMaxHeight()),
		&new_layout);

	if (SUCCEEDED(hr))
	{
		SAFE_RELEASE(m_text_layout);
		m_text_layout = new_layout;
		m_syntax_hightlighter.Hightlight(
			m_editable_text.GetText(),
			subtext_begin, subtext_end,
			m_editable_text.GetCaretPos(),
			m_text_layout);

		// get the caret location
		{
			DWRITE_HIT_TEST_METRICS hit_test_metrics;
			m_text_layout->HitTestTextPosition(
				m_editable_text.GetCaretPos() - subtext_begin,
				false,
				&m_caret_loc_hight.x,
				&m_caret_loc_hight.y,
				&hit_test_metrics);

			m_caret_loc_hight.z = hit_test_metrics.height;
		}

		// get the selection ranges
		m_selection_fields.clear();
		EditableText::Selection selection = m_editable_text.GetSelection();
		if (selection.IsValid())
		{
			size_t left = std::min(selection.start_pos, selection.end_pos);
			size_t right = std::max(selection.start_pos, selection.end_pos);
			left = left > subtext_begin ? left - subtext_begin : 0;
			right = right < subtext_end ? right - subtext_begin : subtext_end - subtext_begin;

			UINT32 num_sections;
			m_text_layout->HitTestTextRange(
				left,
				right - left,
				0,
				0,
				NULL,
				0,
				&num_sections);

			std::vector<DWRITE_HIT_TEST_METRICS> hit_test_metrics(num_sections);
			m_text_layout->HitTestTextRange(
				left,
				right - left,
				0,
				0,
				&hit_test_metrics[0],
				static_cast<UINT32>(hit_test_metrics.size()),
				&num_sections);

			m_selection_fields.resize(num_sections);
			for (int i = 0; i != m_selection_fields.size(); ++i)
			{
				m_selection_fields[i].x = hit_test_metrics[i].left;
				m_selection_fields[i].y = hit_test_metrics[i].top;
				m_selection_fields[i].z = hit_test_metrics[i].width;
				m_selection_fields[i].w = hit_test_metrics[i].height;
			}
		}
	}

	m_caret_idle_time = 0;
}

void TextEditor::OnMousePress(UINT message, float x, float y)
{

}

void TextEditor::OnMouseRelease(UINT message, float x, float y)
{

}

void TextEditor::OnMouseMove(float x, float y)
{

}

void TextEditor::OnMouseScroll(float x_scroll, float y_scroll)
{

}

void TextEditor::OnMouseExit()
{

}

void TextEditor::OnKeyPress(UINT32 key_code)
{
	bool held_shift   = (GetKeyState(VK_SHIFT)   & 0x80) != 0;
	bool held_control = (GetKeyState(VK_CONTROL) & 0x80) != 0;

	switch (key_code)
	{
	case VK_RETURN:
		if (held_control && held_shift)
		{
			m_editable_text.MoveLineEnd();
			m_editable_text.InsertChar('\n');
		}
		else if (held_control)
		{
			m_editable_text.MoveLineBegin();
			m_editable_text.InsertChar('\n');
			m_editable_text.MoveCharLeft();
		}
		else
		{
			m_editable_text.InsertChar('\n');
		}
		AutoIndent();
		RefreshTextLayout();
		break;

	case VK_BACK:
		if (!m_editable_text.GetSelection().IsValid())
		{
			if (held_control) m_editable_text.MoveWordLeft(true);
			else m_editable_text.MoveCharLeft(true);
		}
		m_editable_text.DeleteSelection();
		RefreshTextLayout();
		break;

	case VK_DELETE:
		if (!m_editable_text.GetSelection().IsValid())
		{
			if (held_control) m_editable_text.MoveWordRight(true);
			else m_editable_text.MoveCharRight(true);
		}
		m_editable_text.DeleteSelection();
		RefreshTextLayout();
		break;

	case VK_LEFT:
		if (held_control)
		{
			m_editable_text.MoveWordLeft(held_shift);
		}
		else
		{
			m_editable_text.MoveCharLeft(held_shift);
		}
		RefreshTextLayout();
		break;

	case VK_RIGHT:
		if (held_control)
		{
			m_editable_text.MoveWordRight(held_shift);
		}
		else
		{
			m_editable_text.MoveCharRight(held_shift);
		}
		RefreshTextLayout();
		break;

	case VK_UP:
		m_editable_text.MoveLineUp(held_shift);
		RefreshTextLayout();
		break;

	case VK_DOWN:
		m_editable_text.MoveLineDown(held_shift);
		RefreshTextLayout();
		break;

	case VK_HOME:
		if (held_control)
		{
			m_editable_text.MoveTextBegin(held_shift);
		}
		else
		{
			m_editable_text.MoveLineHome(held_shift);
		}
		RefreshTextLayout();
		break;

	case VK_END:
		if (held_control)
		{
			m_editable_text.MoveTextEnd(held_shift);
		}
		else
		{
			m_editable_text.MoveLineEnd(held_shift);
		}
		RefreshTextLayout();
		break;

	case 'L':
		if (held_control)
		{
			// cut current line (include trailing '\n').
			m_editable_text.MoveLineBegin();
			m_editable_text.MoveLineEnd(true);
			m_editable_text.MoveCharRight(true);
			m_editable_text.CopyToClipboard();
			m_editable_text.DeleteSelection();
			RefreshTextLayout();
		}
		break;

	case 'A':
		if (held_control)
		{
			m_editable_text.MoveTextBegin();
			m_editable_text.MoveTextEnd(true);
			RefreshTextLayout();
		}
		break;

	case 'C':
		if (held_control)
		{
			// it nothing is selected, copy current line (include trailing '\n').
			if (!m_editable_text.GetSelection().IsValid())
			{
				size_t pos = m_editable_text.GetCaretPos();
				m_editable_text.MoveLineBegin();
				m_editable_text.MoveLineEnd(true);
				m_editable_text.MoveCharRight(true);
				m_editable_text.CopyToClipboard();
				m_editable_text.SetCaretPos(pos);
			}
			else
			{
				m_editable_text.CopyToClipboard();
			}
		}
		break;

	case 'X':
		if (held_control)
		{
			m_editable_text.CopyToClipboard();
			m_editable_text.DeleteSelection();
			RefreshTextLayout();
		}
		break;

	case 'V':
		if (held_control)
		{
			m_editable_text.PasteFromClipboard();
			RefreshTextLayout();
		}
		break;

	case 'Z':
		if (held_control)
		{
			m_editable_text.Undo();
			RefreshTextLayout();
		}
		break;

	case 'Y':
		if (held_control)
		{
			m_editable_text.Redo();
			RefreshTextLayout();
		}
		break;

	case VK_TAB:
		AutoJumpOver();
		RefreshTextLayout();
		break;

	case VK_OEM_COMMA:
		if (held_control)
		{
			AutoJumpInto();
			RefreshTextLayout();
		}
		break;

	case VK_OEM_PERIOD:
		if (held_control)
		{
			AutoJumpOut();
			RefreshTextLayout();
		}
		break;
	
	case 'N':
		if (held_control)
		{
			m_editable_text.MoveTextBegin();
			m_editable_text.MoveTextEnd(true);
			m_editable_text.InsertText(default_shader_content);
			m_editable_text.SetCaretPos(0);
			RefreshTextLayout();
			ReloadPixelShader();
		}
		break;

	case 'O':
		if (held_control)
		{
			OpenFile();
			RefreshTextLayout();
			ReloadPixelShader();
		}
		break;

	case 'S':
		if (held_control)
		{
			SaveFile(held_shift);
			ReloadPixelShader();
		}
		break;

	case VK_F7:
		ReloadPixelShader();
		break;

	case 'M':
		if (held_control)
		{
			D3DApp::GetSoundPlayer()->ToggleMute();
		}
		break;

	case VK_OEM_PLUS:
		if (held_control)
		{
			D3DApp::GetSoundPlayer()->ChangeVolume(0.1f);
		}
		break;

	case VK_OEM_MINUS:
		if (held_control)
		{
			D3DApp::GetSoundPlayer()->ChangeVolume(-0.1f);
		}
		break;

	default:
		break;
	}
}

void TextEditor::OnKeyCharacter(UINT32 char_code)
{
	// only handle normal characters
	if (char_code >= 0x20 && char_code < 0x7F)
	{
		wchar_t c = static_cast<wchar_t>(char_code);
		if (c == '{')
		{
			m_editable_text.InsertText(L"{\n}");
			m_editable_text.MoveCharLeft();
			AutoIndent();
			m_editable_text.MoveLineBegin();
			m_editable_text.MoveCharLeft();
			m_editable_text.InsertChar('\n');
			AutoIndent();
		}
		else if (c == '}')
		{
			m_editable_text.InsertChar(c);
			AutoIndent();
		}
		else if (c == '(')
		{
			EditableText::Selection selection = m_editable_text.GetSelection();
			if (selection.IsValid())
			{
				size_t left = std::min(selection.start_pos, selection.end_pos);
				size_t right = std::max(selection.start_pos, selection.end_pos);
				m_editable_text.SetCaretPos(left);
				m_editable_text.InsertChar('(');
				m_editable_text.SetCaretPos(right + 1);
				m_editable_text.InsertChar(')');
			}
			else
			{
				m_editable_text.InsertText(L"()");
				m_editable_text.MoveCharLeft(false);
			}
		}
		else
		{
			m_editable_text.InsertChar(static_cast<wchar_t>(char_code));			
		}
		RefreshTextLayout();
	}
}

void TextEditor::AutoIndent()
{
	RefreshTextLayout();

	size_t pos = m_editable_text.GetCaretPos();
	m_editable_text.MoveLineBegin();
	size_t line_begin = m_editable_text.GetCaretPos();
	m_editable_text.MoveLineHome();
	size_t line_home = m_editable_text.GetCaretPos();

	size_t indent = m_syntax_hightlighter.FetchIndent(line_home + 1);
	int num_space_should = indent * 2;
	int name_space_have = line_home - line_begin;

	int diff = num_space_should - name_space_have;
	if (diff > 0)
	{
		std::wstring str(diff, ' ');
		m_editable_text.InsertText(str);
	}
	else
	{
		m_editable_text.SetCaretPos(line_home + diff, true);
		m_editable_text.DeleteSelection();
	}
}

void TextEditor::AutoJumpOver()
{
	RefreshTextLayout();

	if (m_editable_text.GetSelection().IsValid())
	{
		m_editable_text.InsertText(L"  ");
		return;
	}

	size_t pos = m_editable_text.GetCaretPos();
	m_editable_text.MoveLineHome();
	size_t line_home = m_editable_text.GetCaretPos();
	if (pos <= line_home)
	{
		m_editable_text.InsertText(L"  ");
		return;
	}

	m_editable_text.SetCaretPos(pos);
	m_editable_text.MoveWordRight();
	size_t jump_pos = m_editable_text.GetCaretPos();
	int idx = m_syntax_hightlighter.FetchTokenBackward(jump_pos);
	if (idx != -1)
	{
		const SyntaxHighlighter::Token& tok = m_syntax_hightlighter.GetToken(idx);
		const std::wstring &word = tok.word;
		if (word == L"}" || word == L")") jump_pos = tok.end_pos;
		else jump_pos = pos;
	}

	if (pos == jump_pos)
	{
		m_editable_text.SetCaretPos(pos);
		m_editable_text.InsertText(L"  ");
	}	
	else m_editable_text.SetCaretPos(jump_pos);
}

void TextEditor::AutoJumpInto()
{
	RefreshTextLayout();

	size_t pos = m_editable_text.GetCaretPos();
	size_t depth = m_syntax_hightlighter.FetchDepth(pos);

	int idx = m_syntax_hightlighter.FetchTokenBackward(pos);
	for (size_t i = idx + 1; i < m_syntax_hightlighter.GetNumberTokens(); ++i)
	{
		if (m_syntax_hightlighter.GetToken(i).depth < depth) break;

		if (m_syntax_hightlighter.GetToken(i).depth > depth)
		{
			pos = m_syntax_hightlighter.GetToken(i).start_pos;
			m_editable_text.SetCaretPos(pos);
			break;
		}
	}
}

void TextEditor::AutoJumpOut()
{
	RefreshTextLayout();

	size_t pos = m_editable_text.GetCaretPos();
	size_t depth = m_syntax_hightlighter.FetchDepth(pos);

	int idx = m_syntax_hightlighter.FetchTokenBackward(pos);
	for (size_t i = idx + 1; i < m_syntax_hightlighter.GetNumberTokens(); ++i)
	{
		if (m_syntax_hightlighter.GetToken(i).depth < depth)
		{
			pos = m_syntax_hightlighter.GetToken(i).end_pos;
			m_editable_text.SetCaretPos(pos);
			break;
		}
	}
}

void TextEditor::ReloadPixelShader()
{
	const std::wstring &text = m_editable_text.GetText();
	tstring shader_content = ShaderHeader::GetHeaderText();
	shader_content.append(text.begin(), text.end());

	bool compiled_ok = D3DApp::GetPostProcess()->LoadPixelShaderFromMemory(shader_content, TEXT("ps_main"));
	if (compiled_ok)
	{
		m_compile_error.Clear();
	}
	else
	{
		ParseCompileError(D3DApp::GetPostProcess()->GetErrorMessage());
	}
}

void TextEditor::ParseCompileError(const tstring& fxc_error)
{
	m_compile_error.Clear();

	if (!fxc_error.empty())
	{
		std::istringstream iss(fxc_error);
		
		if (fxc_error[0] == '(')
		{
			char igored_char;
			iss >> igored_char;
			iss >> m_compile_error.row >> igored_char >> m_compile_error.column;
			iss >> igored_char >> igored_char;

			m_compile_error.row -= 1 + ShaderHeader::GetHeaderLines();
			m_compile_error.column -= 1;
		}

		std::string ignored_word;
		iss >> ignored_word >> ignored_word;

		assert(iss.good());
		std::wstring message(fxc_error.begin() + static_cast<int>(iss.tellg()), fxc_error.end());
		m_compile_error.message = message.substr(1, message.find_first_of('\n'));

		// calculate error location
		if (m_compile_error.row < static_cast<int>(m_line_offset))
		{
			m_compile_error.location = float2(0, -20.0f);
		}
		else if (m_compile_error.row >= static_cast<int>(m_line_offset + MAX_NUM_LINES))
		{
			m_compile_error.location = float2(0, m_text_layout->GetMaxHeight() + 30.0f);
		}
		else
		{
			size_t subtext_begin = m_editable_text.GetTextPos(m_line_offset, 0);
			size_t error_pos = m_editable_text.GetTextPos(m_compile_error.row, m_compile_error.column);

			assert(error_pos >= subtext_begin);
			DWRITE_HIT_TEST_METRICS hit_test_metrics;
			m_text_layout->HitTestTextPosition(
				error_pos - subtext_begin,
				false,
				&m_compile_error.location.x,
				&m_compile_error.location.y,
				&hit_test_metrics);

			m_compile_error.is_located = true;
		}

		m_compile_error.remain_time = 3.0f;
		m_compile_error.alpha = 1.0f;
	}
}

void TextEditor::OpenFile()
{
	OPENFILENAMEW ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	std::vector<wchar_t> file_path(512 + 1);
	ofn.lpstrFile = &file_path[0];
	ofn.nMaxFile = 512;

	std::vector<wchar_t> directory(512 + 1);
	GetCurrentDirectoryW(512,  &directory[0]);
	std::wstring init_dir = std::wstring(&directory[0]) + L"\\save\\";
	ofn.lpstrInitialDir = init_dir.c_str();

	if (GetOpenFileNameW(&ofn))
	{
		m_file_path = std::wstring(&file_path[0]);

		std::wifstream ifs(m_file_path.c_str());
		if (!ifs.is_open()) return;

		ifs.seekg(0, std::ios::end);
		int length = static_cast<int>(ifs.tellg());

		std::vector<wchar_t> buffer(length + 1);
		ifs.seekg(0, std::ios::beg);
		ifs.read(&buffer[0], length);
		ifs.close();

		m_editable_text.SetText(std::wstring(&buffer[0]));
	}
}

void TextEditor::SaveFile(bool save_as_new /*= false*/)
{
	if (m_file_path.empty() || save_as_new)
	{
		OPENFILENAMEW ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		std::vector<wchar_t> file_path(512 + 1);
		ofn.lpstrFile = &file_path[0];
		ofn.nMaxFile = 512;

		std::vector<wchar_t> directory(512 + 1);
		GetCurrentDirectoryW(512,  &directory[0]);
		std::wstring init_dir = std::wstring(&directory[0]) + L"\\save\\";
		ofn.lpstrInitialDir = init_dir.c_str();

		if (GetSaveFileNameW(&ofn))
		{
			m_file_path = std::wstring(&file_path[0]);
		}
	}

	if (m_file_path.empty()) return;

	std::wofstream ofs(m_file_path.c_str());
	if (!ofs.is_open()) return;

	ofs.write(m_editable_text.GetText().c_str(), m_editable_text.GetText().length());
	ofs.close();
}
