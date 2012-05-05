#ifndef _SYNTAX_HIGHLIGHTER_INCLUDED_HPP_
#define _SYNTAX_HIGHLIGHTER_INCLUDED_HPP_

#include <vector>
#include <set>

class SyntaxHighlighter
{
public:
	enum TokenEnv
	{
		TE_Normal,
		TE_Dot,
		TE_Colon,
		TE_LineComment,
		TE_BlockComment,
	};

	struct TokenContext
	{
		TokenEnv env;
		size_t depth;
		size_t intent;
	};

	enum TokenType
	{
		TT_Word,
		TT_Keyword,
		TT_Constant,
		TT_Function,
		TT_Member,
		TT_Semantic,
		TT_Comment,
		TT_Separator,
		TT_Illegal,
		Num_TokenTypes,
	};

	struct Token
	{
		size_t start_pos;
		size_t end_pos;
		std::wstring word;

		TokenType type;
		size_t depth;
		size_t indent;

		Token(const std::wstring& text, size_t start, size_t end,
			  TokenType type = TT_Word, size_t depth = 0, size_t indent = 0);
	};

	struct DrawStyle
	{
		float3 color;
		bool bold;
		bool underlined;
		ID2D1SolidColorBrush* brush;

		DrawStyle(const float3& color = float3(), bool bold = false, bool underlined = false);
		~DrawStyle();
	};

public:
	SyntaxHighlighter();
	virtual ~SyntaxHighlighter();

public:
	void Intialize(ID2D1RenderTarget* d2d_rt);
	void Hightlight(const std::wstring& text, size_t start_pos, size_t end_pos, size_t caret_pos, IDWriteTextLayout* layout);

	size_t GetNumberTokens() const;
	const Token& GetToken(size_t idx) const;

	int FetchTokenForward(size_t pos) const;
	int FetchTokenBackward(size_t pos) const;

	size_t FetchIndent(size_t pos) const;
	size_t FetchDepth(size_t pos) const;

private:
	void InitDrawStyles(ID2D1RenderTarget* d2d_rt);

	void Parse(const std::wstring& text);
	void ParseToken(size_t &pos, TokenContext& context, const std::wstring& text);

	void ResolveToken(TokenContext& context, Token& tok);

private:
	std::set<std::wstring> m_keywords_set;
	std::set<std::wstring> m_semantics_set;
	std::set<std::wstring> m_global_funcs_set;
	std::set<std::wstring> m_member_funcs_set;

	std::vector<Token> m_tokens;
	std::vector<DrawStyle> m_draw_styles;
};

#endif  // _SYNTAX_HIGHLIGHTER_INCLUDED_HPP_