#include "common.hpp"
#include "syntax_highlighter.hpp"

#include "keywords.hpp"
#include <cctype>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// constructor / destructor
//////////////////////////////////////////////////////////////////////////
SyntaxHighlighter::SyntaxHighlighter()
{

}

SyntaxHighlighter::~SyntaxHighlighter()
{

}

SyntaxHighlighter::
Token::Token(const std::wstring& text, size_t start, size_t end,
			 TokenType type, size_t depth, size_t indent)
	: start_pos(start)
	, end_pos(end)
	, type(type)
	, depth(depth)
	, indent(indent)
{
	word = text.substr(start_pos, end_pos - start_pos);
}

SyntaxHighlighter::
DrawStyle::DrawStyle(const float3& color, bool bold, bool underlined)
	: color(color)
	, bold(bold)
	, underlined(underlined)
	, brush(NULL)
{

}

SyntaxHighlighter::
DrawStyle::~DrawStyle()
{
	SAFE_RELEASE(brush);
}

//////////////////////////////////////////////////////////////////////////
// public interfaces
//////////////////////////////////////////////////////////////////////////
void SyntaxHighlighter::Intialize(ID2D1RenderTarget* d2d_rt)
{
	m_keywords_set.clear();
	m_keywords_set.insert(kewords, kewords + ARRAYSIZE(kewords));

	m_semantics_set.clear();
	m_semantics_set.insert(semantics, semantics + ARRAYSIZE(semantics));

	m_global_funcs_set.clear();
	m_global_funcs_set.insert(global_funcs, global_funcs + ARRAYSIZE(global_funcs));

	m_member_funcs_set.clear();
	m_member_funcs_set.insert(member_funcs, member_funcs + ARRAYSIZE(member_funcs));

	InitDrawStyles(d2d_rt);
}

void SyntaxHighlighter::Hightlight(const std::wstring& text, size_t start_pos, size_t end_pos, size_t caret_pos, IDWriteTextLayout* layout)
{
	Parse(text);

	for (int i = 0; i != m_tokens.size(); ++i)
	{
		Token& tok = m_tokens[i];
		DrawStyle& style = m_draw_styles[tok.type];

		if (tok.end_pos < start_pos) continue;
		if (tok.start_pos >= end_pos) break;

		size_t range_start = tok.start_pos > start_pos ? tok.start_pos - start_pos : 0;
		size_t range_end = tok.end_pos < end_pos ? tok.end_pos - start_pos : end_pos - start_pos;
		DWRITE_TEXT_RANGE range = {range_start, range_end - range_start};

		layout->SetDrawingEffect(style.brush, range);

		if (style.bold)
		{
			layout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
		}
		if (style.underlined)
		{
			layout->SetUnderline(true, range);
		}
	}

	// highlight the active scope.
	size_t caret_depth = FetchDepth(caret_pos);
	for (int i = FetchTokenBackward(caret_pos); i >= 0; --i)
	{
		Token& tok = m_tokens[i];
		if (tok.end_pos < start_pos) break;

		if (tok.depth < caret_depth)
		{
			size_t range_start = tok.start_pos > start_pos ? tok.start_pos - start_pos : 0;
			size_t range_end = tok.end_pos < end_pos ? tok.end_pos - start_pos : end_pos - start_pos;
			DWRITE_TEXT_RANGE range = {range_start, range_end - range_start};

			layout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
			break;
		}
	}

	for (int i = FetchTokenForward(caret_pos); i != m_tokens.size(); ++i)
	{
		Token& tok = m_tokens[i];
		if (tok.start_pos >= end_pos) break;
	
		if (tok.depth < caret_depth)
		{
			size_t range_start = tok.start_pos > start_pos ? tok.start_pos - start_pos : 0;
			size_t range_end = tok.end_pos < end_pos ? tok.end_pos - start_pos : end_pos - start_pos;
			DWRITE_TEXT_RANGE range = {range_start, range_end - range_start};

			layout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
			break;
		}
	}

	return;
}

size_t SyntaxHighlighter::GetNumberTokens() const
{
	return m_tokens.size();
}

const SyntaxHighlighter::Token& SyntaxHighlighter::GetToken(size_t idx) const
{
	return m_tokens[idx];
}

int SyntaxHighlighter::FetchTokenForward(size_t pos) const
{
	auto it = std::upper_bound(m_tokens.begin(), m_tokens.end(), pos,
		[](size_t lhs, const Token& rhs) {return lhs < rhs.end_pos;});
	return it - m_tokens.begin();
}

int SyntaxHighlighter::FetchTokenBackward(size_t pos) const
{
	auto it = std::upper_bound(m_tokens.begin(), m_tokens.end(), pos,
		[](size_t lhs, const Token& rhs) {return lhs < rhs.start_pos + 1;});

	if (it == m_tokens.begin()) return -1;
	else return it - 1 - m_tokens.begin();
}

size_t SyntaxHighlighter::FetchIndent(size_t pos) const
{
	int idx = FetchTokenBackward(pos);
	if (idx == -1) return 0;

	const Token& tok = m_tokens[idx];
	if (tok.type == TT_Separator && (tok.word == L"{" || tok.word == L"("))
	{
		return tok.indent + 1;
	}
	return tok.indent;
}

size_t SyntaxHighlighter::FetchDepth(size_t pos) const
{
	int idx = FetchTokenBackward(pos);
	if (idx == -1) return 0;

	const Token& tok = m_tokens[idx];
	if (tok.type == TT_Separator && (tok.word == L"{" || tok.word == L"("))
	{
		return tok.depth + 1;
	}
	return tok.depth;
}

//////////////////////////////////////////////////////////////////////////
// private subroutines
//////////////////////////////////////////////////////////////////////////
void SyntaxHighlighter::InitDrawStyles(ID2D1RenderTarget* d2d_rt)
{
	m_draw_styles.resize(Num_TokenTypes);

	m_draw_styles[TT_Word		] = DrawStyle(float3(1.00f, 1.00f, 1.00f), false, false);
	m_draw_styles[TT_Keyword	] = DrawStyle(float3(0.54f, 0.68f, 0.94f), true,  false);
	m_draw_styles[TT_Constant	] = DrawStyle(float3(0.83f, 0.73f, 0.91f), false, false);
	m_draw_styles[TT_Function	] = DrawStyle(float3(0.96f, 0.68f, 0.41f), true , false);
	m_draw_styles[TT_Member		] = DrawStyle(float3(0.96f, 0.68f, 0.41f), true , false);
	m_draw_styles[TT_Semantic	] = DrawStyle(float3(0.98f, 0.69f, 0.81f), false, false);
	m_draw_styles[TT_Comment	] = DrawStyle(float3(0.54f, 0.94f, 0.85f), false, false);
	m_draw_styles[TT_Separator	] = DrawStyle(float3(1.00f, 1.00f, 1.00f), false, false);
	m_draw_styles[TT_Illegal	] = DrawStyle(float3(1.00f, 1.00f, 1.00f), false, true );

	// create brushes
	for (int i = 0; i != Num_TokenTypes; ++i)
	{
		DrawStyle& style = m_draw_styles[i];
		D2D1_COLOR_F d2d_color = D2D1::ColorF(style.color.x, style.color.y, style.color.z);
		d2d_rt->CreateSolidColorBrush(d2d_color, &style.brush);
	}
}

void SyntaxHighlighter::Parse(const std::wstring& text)
{
	m_tokens.clear();

	size_t pos = 0;
	TokenContext context = {TE_Normal, 0, 0};

	while(pos < text.length())
	{
		ParseToken(pos, context, text);
	}
}

void SyntaxHighlighter::ParseToken(size_t &pos, TokenContext& context, const std::wstring& text)
{
	wchar_t current_char = text[pos];
	wchar_t next_char = pos + 1 < text.length() ? text[pos + 1] : 0;

	// space
	if (isspace(current_char))
	{
		if (context.env == TE_LineComment && current_char == '\n')
		{
			context.env = TE_Normal; 
		}
		pos += 1;
		return;
	}

	// block comment begin
	if (current_char == '/' && next_char == '*')
	{
		if (context.env != TE_LineComment) context.env = TE_BlockComment;
		Token tok(text, pos, pos + 2, TT_Comment);
		m_tokens.push_back(tok);
		ResolveToken(context, tok);
		pos += 2;
		return;
	}

	// block comment end
	if (current_char == '*' && next_char == '/')
	{
		Token tok(text, pos, pos + 2, TT_Comment);
		if (context.env != TE_BlockComment && context.env != TE_LineComment) tok.type = TT_Illegal;
		if (context.env == TE_BlockComment) context.env = TE_Normal;
		m_tokens.push_back(tok);
		ResolveToken(context, tok);
		pos += 2;
		return;
	}

	// line comment
	if (current_char == '/' && next_char == '/')
	{
		if (context.env != TE_BlockComment) context.env = TE_LineComment;
		Token tok(text, pos, pos + 2, TT_Comment);
		m_tokens.push_back(tok);
		ResolveToken(context, tok);
		pos += 2;
		return;
	}

	// word
	if (isalpha(current_char) || current_char == '_')
	{
		int start_pos = pos;
		for (pos = pos + 1; pos < text.length(); ++pos)
		{
			if (!isalnum(text[pos]) && text[pos] != '_') break;
		}

		Token tok(text, start_pos, pos, TT_Word);
		ResolveToken(context, tok);
		m_tokens.push_back(tok);
		return;
	}

	// literal constant
	if (isdigit(current_char) || current_char == '.' && isdigit(next_char))
	{
		int start_pos = pos;
		bool meet_digit = false;
		bool meet_dot = false;
		bool meet_suffix = false;
		bool illegal = false;
		for (; pos < text.length() && (isalnum(text[pos]) || text[pos] == '.'); ++pos)
		{
			if (illegal) continue;
			if (meet_suffix) {illegal = true; continue;}

			if (isdigit(text[pos]))
			{
				meet_digit = true;
			}
			else if (text[pos] == '.')
			{
				if (!meet_dot) meet_dot = true; else illegal = true;
			}
			else if (isalpha(text[pos]))
			{
				if (!meet_digit || meet_suffix) illegal = true;
				else meet_suffix = true;
			}
		}
		Token tok(text, start_pos, pos);
		tok.type = illegal ? TT_Illegal : TT_Constant;
		ResolveToken(context, tok);
		m_tokens.push_back(tok);
	}

	// other separators
	{
		Token tok(text, pos, pos + 1, TT_Separator);
		ResolveToken(context, tok);
		m_tokens.push_back(tok);
		pos += 1;
		return;
	}
}

void SyntaxHighlighter::ResolveToken(TokenContext& context, Token &tok)
{
	tok.depth = context.depth;
	tok.indent = context.intent;

	const std::wstring& word = tok.word; 
	if (context.env == TE_LineComment || context.env == TE_BlockComment)
	{
		tok.type = TT_Comment;
	}
	else if (context.env == TE_Dot)
	{
		context.env = TE_Normal;
		if (tok.type == TT_Word)
		{
			if (m_member_funcs_set.find(word) != m_member_funcs_set.end()) tok.type = TT_Member;
		}
		else tok.type = TT_Illegal;
	}
	else if (context.env == TE_Colon)
	{
		context.env = TE_Normal;
		if (tok.type == TT_Word)
		{
			std::wstring word_lower_case = word;
			std::transform(word.begin(), word.end(), word_lower_case.begin(), ::tolower);
			
			if (m_semantics_set.find(word_lower_case) != m_semantics_set.end()) tok.type = TT_Semantic;
		}
		else tok.type = TT_Illegal;
	}
	else if (context.env == TE_Normal)
	{
		if (tok.type == TT_Separator)
		{
			if (word == L".") context.env = TE_Dot;
			else if (word == L":") context.env = TE_Colon;
			else if (word == L"{" || word == L"(")
			{
				context.depth += 1;
				if (word == L"{") context.intent += 1;
			}
			else if (word == L"}" || word == L")")
			{
				if (context.depth > 0) context.depth -= 1;
				if (word == L"}" && context.intent > 0) context.intent -= 1;
				tok.depth = context.depth;
				tok.indent = context.intent;
			}
		}

		if (tok.type == TT_Word)
		{
			if (m_keywords_set.find(word) != m_keywords_set.end()) tok.type = TT_Keyword;
			if (m_global_funcs_set.find(word) != m_global_funcs_set.end()) tok.type = TT_Function;
		}
	}
}
