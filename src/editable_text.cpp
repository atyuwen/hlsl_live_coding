#include "common.hpp"
#include "editable_text.hpp"

//////////////////////////////////////////////////////////////////////////
// constructor / destructor
//////////////////////////////////////////////////////////////////////////
EditableText::EditableText()
{
	SetCaretPos(0);
	SetSelection(0, 0);
	UpdateHorizenPos();
}

EditableText::~EditableText()
{

}

bool EditableText::Selection::IsValid(int text_length /* = -1 */) const
{
	if (start_pos == end_pos) return false;

	if (text_length >= 0)
	{
		if (start_pos > static_cast<size_t>(text_length)) return false;
		if (end_pos > static_cast<size_t>(text_length)) return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// public interfaces
//////////////////////////////////////////////////////////////////////////
void EditableText::SetText(const std::wstring& text)
{
	m_text = text;
	SetCaretPos(0);
	UpdateHorizenPos();

	m_undo_records.clear();
	m_redo_records.clear();
}

void EditableText::SetCaretPos(size_t pos, bool extend_selection /*= false*/)
{
	SetCaretPosInner(pos, extend_selection);
	UpdateHorizenPos();
}

void EditableText::MoveCharLeft(bool extend_selection /*= false*/)
{
	if (m_caret_pos > 0)
	{
		SetCaretPos(m_caret_pos - 1, extend_selection);
	}
}

void EditableText::MoveCharRight(bool extend_selection /*= false*/)
{
	size_t pos = m_caret_pos;
	if (pos + 1 < m_text.length() && m_text[pos] == '\r' && m_text[pos + 1] == '\n')
	{
		SetCaretPos(pos + 2, extend_selection);
	}
	else
	{
		SetCaretPos(pos + 1, extend_selection);
	}
}

void EditableText::MoveWordLeft(bool extend_selection /*= false*/)
{
	int pos = m_caret_pos - 1;
	for (; pos >= 0; --pos)
	{
		if (!isspace(m_text[pos])) break;
		if (pos != m_caret_pos - 1 && m_text[pos] == '\n') break;
	}

	if (pos < 0 || m_text[pos] == '\n')
	{
		SetCaretPos(pos + 1, extend_selection);
	}
	else if (!isalnum(m_text[pos]) && m_text[pos] != '_')
	{
		SetCaretPos(pos, extend_selection);
	}
	else
	{
		for (; pos >= 0; --pos)
		{
			if (!isalnum(m_text[pos]) && m_text[pos] != '_') break;
		}
		SetCaretPos(pos + 1, extend_selection);
	}
}

void EditableText::MoveWordRight(bool extend_selection /*= false*/)
{
	size_t pos = m_caret_pos;
	for (; pos < m_text.length(); ++pos)
	{
		if (m_text[pos] != '\r' && m_text[pos] != '\n') break;
	}

	if (pos == m_caret_pos)
	{
		for (; pos < m_text.length(); ++pos)
		{
			if (!isalnum(m_text[pos]) && m_text[pos] != '_') break;
		}
		if (pos == m_caret_pos && pos < m_text.length()) pos += 1;
	}

	for (; pos < m_text.length(); ++pos)
	{
		if (!isspace(m_text[pos]) || m_text[pos] == '\n') break;
	}

	SetCaretPos(pos, extend_selection);
}

void EditableText::MoveLineUp(bool extend_selection /*= false*/)
{
	size_t pos = GetLineBeginPos(m_caret_pos);
	if (pos > 0)
	{
		pos = GetLineBeginPos(pos - 1);
	}

	size_t end_pos = GetLineEndPos(pos);
	pos = std::min(pos + m_horizen_pos, end_pos);
	SetCaretPosInner(pos, extend_selection);
}

void EditableText::MoveLineDown(bool extend_selection /*= false*/)
{
	size_t pos = GetLineEndPos(m_caret_pos);
	if (pos < m_text.length())
	{
		pos = GetLineEndPos(pos + 1);
	}

	size_t start_pos = GetLineBeginPos(pos);
	pos = std::min(start_pos + m_horizen_pos, pos);
	SetCaretPosInner(pos, extend_selection);
}

void EditableText::MoveLineBegin(bool extend_selection /*= false*/)
{
	size_t pos = GetLineBeginPos(m_caret_pos);
	SetCaretPos(pos, extend_selection);
}

void EditableText::MoveLineHome(bool extend_selection /*= false*/)
{
	size_t pos = GetLineBeginPos(m_caret_pos);
	for (; pos < m_text.length(); ++pos)
	{
		if (!isspace(m_text[pos]) || m_text[pos] == '\n') break;
	}
	SetCaretPos(pos, extend_selection);
}

void EditableText::MoveLineEnd(bool extend_selection /*= false*/)
{
	size_t pos = GetLineEndPos(m_caret_pos);
	SetCaretPos(pos, extend_selection);
}

void EditableText::MoveTextBegin(bool extend_selection /*= false*/)
{
	SetCaretPos(0, extend_selection);
}

void EditableText::MoveTextEnd(bool extend_selection /*= false*/)
{
	SetCaretPos(m_text.length(), extend_selection);
}

void EditableText::MoveToLine(size_t line, bool extend_selection /*= false*/)
{
	size_t pos = GetTextPos(line, 0);
	SetCaretPos(pos, extend_selection);
}

void EditableText::InsertChar(wchar_t c)
{
	std::wstring str(1, c);
	InsertText(str);	
}

void EditableText::InsertText(const std::wstring& text)
{
	DeleteSelection();
	m_text.insert(m_text.begin() + m_caret_pos, text.begin(), text.end());
	SetCaretPos(m_caret_pos + text.length());

	EditOperation op = {EO_Insert, m_caret_pos - text.length(), text};
	m_undo_records.push_back(op);
	m_redo_records.clear();
}

void EditableText::DeleteSelection()
{
	if (!m_selection.IsValid()) return;

	size_t left = std::min(m_selection.start_pos, m_selection.end_pos);
	size_t right = std::max(m_selection.start_pos, m_selection.end_pos);
	std::wstring text_to_del(m_text.begin() + left, m_text.begin() + right);
	m_text.erase(m_text.begin() + left, m_text.begin() + right);
	SetCaretPos(left);

	EditOperation op = {EO_Delete, left, text_to_del};
	m_undo_records.push_back(op);
	m_redo_records.clear();
}

void EditableText::CopyToClipboard() const
{
	if (!m_selection.IsValid()) return;

	if (OpenClipboard(NULL))
	{
		if (EmptyClipboard())
		{
			size_t left = std::min(m_selection.start_pos, m_selection.end_pos);
			size_t right = std::max(m_selection.start_pos, m_selection.end_pos);
			std::wstring selected_text = m_text.substr(left, right - left);

			size_t num_bytes = sizeof(wchar_t) * (selected_text.length() + 1);
			HGLOBAL clipboard_data = GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT, num_bytes);

			if (clipboard_data != NULL)
			{
				void* memory = GlobalLock(clipboard_data);
				if (memory != NULL)
				{
					memcpy(memory, selected_text.c_str(), num_bytes);
					GlobalUnlock(clipboard_data);
					if (SetClipboardData(CF_UNICODETEXT, clipboard_data) != NULL)
					{
						clipboard_data = NULL;
					}
				}
				GlobalFree(clipboard_data);
			}
		}
		CloseClipboard();
	}
}

void EditableText::PasteFromClipboard()
{
	DeleteSelection();

	if (OpenClipboard(NULL))
	{
		HGLOBAL clipboard_data = GetClipboardData(CF_UNICODETEXT);
		if (clipboard_data != NULL)
		{
			void* memory = GlobalLock(clipboard_data);
			wchar_t* text = reinterpret_cast<wchar_t*>(memory);
			InsertText(text);
		}
	}
}

void EditableText::Undo()
{
	if (m_undo_records.empty()) return;

	EditOperation op = m_undo_records.back();
	m_undo_records.pop_back();
	m_redo_records.push_back(op);

	if (op.type == EO_Delete)
	{
		m_text.insert(m_text.begin() + op.pos, op.text.begin(), op.text.end());
		SetCaretPos(op.pos + op.text.length());
	}
	else if (op.type == EO_Insert)
	{
		m_text.erase(m_text.begin() + op.pos, m_text.begin() + op.pos + op.text.length());
		SetCaretPos(op.pos);
	}
}

void EditableText::Redo()
{
	if (m_redo_records.empty()) return;

	EditOperation op = m_redo_records.back();
	m_redo_records.pop_back();
	m_undo_records.push_back(op);

	if (op.type == EO_Insert)
	{
		m_text.insert(m_text.begin() + op.pos, op.text.begin(), op.text.end());
		SetCaretPos(op.pos + op.text.length());
	}
	else if (op.type == EO_Delete)
	{
		m_text.erase(m_text.begin() + op.pos, m_text.begin() + op.pos + op.text.length());
		SetCaretPos(op.pos);
	}
}

const std::wstring& EditableText::GetText() const
{
	return m_text;
}

size_t EditableText::GetTextPos(size_t line, size_t column) const
{
	size_t cur_line = 0;
	size_t pos = 0;
	for (; pos < m_text.length(); ++pos)
	{
		if (cur_line == line) break;
		if (m_text[pos] == '\n') ++cur_line;
	}

	size_t line_end = GetLineEndPos(pos);
	if (line_end - pos < column) return line_end;
	return pos + column;
}

size_t EditableText::GetCaretPos() const
{
	return m_caret_pos;
}

size_t EditableText::GetCaretLine() const
{
	return std::count_if(m_text.begin(), m_text.begin() + m_caret_pos,
		[](wchar_t c) {return c == '\n';});
}

EditableText::Selection EditableText::GetSelection() const
{
	return m_selection;
}

//////////////////////////////////////////////////////////////////////////
// private subroutines
//////////////////////////////////////////////////////////////////////////
void EditableText::SetSelection(size_t start, size_t end)
{
	m_selection.start_pos = start;
	m_selection.end_pos = end;
}

void EditableText::SetSelectionStart(size_t start)
{
	m_selection.start_pos = start;
}

void EditableText::SetSelectionEnd(size_t end)
{
	m_selection.end_pos = end;
}

size_t EditableText::GetLineBeginPos(size_t current_pos) const
{
	int pos = current_pos - 1;
	for (; pos >= 0; --pos)
	{
		if (m_text[pos] == '\n') break;
	}
	return pos + 1;
}

size_t EditableText::GetLineEndPos(size_t current_pos) const
{
	size_t pos = current_pos;
	for (; pos < m_text.length(); ++pos)
	{
		if (m_text[pos] == '\n') break;
	}
	return pos;
}

void EditableText::SetCaretPosInner(size_t pos, bool extend_selection /*= false*/)
{
	pos = std::min(pos, m_text.length());

	if (pos > 0 && pos < m_text.length() && m_text[pos - 1] == '\r' && m_text[pos] == '\n')
	{
		pos = pos - 1;
	}
	m_caret_pos = pos;

	if (extend_selection)
	{
		SetSelectionEnd(m_caret_pos);
	}
	else
	{
		SetSelection(m_caret_pos, m_caret_pos);
	}
}

void EditableText::UpdateHorizenPos()
{
	size_t pos = GetLineBeginPos(m_caret_pos);
	m_horizen_pos = m_caret_pos - pos;
}
