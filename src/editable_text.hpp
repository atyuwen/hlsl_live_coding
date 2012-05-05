#ifndef _EDITABLE_TEXT_HPP_INCLUDED_
#define _EDITABLE_TEXT_HPP_INCLUDED_

#include <string>
#include <vector>

class EditableText
{
public:
	enum EditType
	{
		EO_Insert,
		EO_Delete,
	};

	struct EditOperation
	{
		EditType type;
		size_t pos;
		std::wstring text;
	};

	struct Selection
	{
		size_t start_pos;
		size_t end_pos;
		bool IsValid(int text_length = -1) const;
	};

public:
	EditableText();
	virtual ~EditableText();

public:
	void SetText(const std::wstring& text);
	void SetCaretPos(size_t pos, bool extend_selection = false);

	void MoveCharLeft(bool extend_selection = false);
	void MoveCharRight(bool extend_selection = false);
	void MoveWordLeft(bool extend_selection = false);
	void MoveWordRight(bool extend_selection = false);

	void MoveLineUp(bool extend_selection = false);
	void MoveLineDown(bool extend_selection = false);
	void MoveLineBegin(bool extend_selection = false);
	void MoveLineHome(bool extend_selection = false);
	void MoveLineEnd(bool extend_selection = false);
	void MoveTextBegin(bool extend_selection = false);
	void MoveTextEnd(bool extend_selection = false);
	void MoveToLine(size_t line, bool extend_selection = false);

	void InsertChar(wchar_t c);
	void InsertText(const std::wstring& text);
	void DeleteSelection();

	void CopyToClipboard() const;
	void PasteFromClipboard();

	void Undo();
	void Redo();

	const std::wstring& GetText() const;
	size_t GetTextPos(size_t line, size_t column) const;

	size_t GetCaretPos() const;
	size_t GetCaretLine() const;
	Selection GetSelection() const;

private:
	void SetSelection(size_t start, size_t end);
	void SetSelectionStart(size_t start);
	void SetSelectionEnd(size_t end);

	size_t GetLineBeginPos(size_t current_pos) const;
	size_t GetLineEndPos(size_t current_pos) const;

	void SetCaretPosInner(size_t pos, bool extend_selection = false);
	void UpdateHorizenPos();

private:
	std::wstring m_text;
	size_t m_caret_pos;
	size_t m_horizen_pos;
	Selection m_selection;

	std::vector<EditOperation> m_undo_records;
	std::vector<EditOperation> m_redo_records;
};

#endif  // _EDITABLE_TEXT_HPP_INCLUDED_