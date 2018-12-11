#include "Menu.h"

#include <iostream> // cout
#include <conio.h>
#include <algorithm>
#include <iomanip>
#include <cctype>
#include <cassert>

namespace Menu {

	void MenuItem::SetContext(void* context)
	{
		_assotiatedContext = context;
	}

	MenuNode::MenuNode(const tstring& caption) :MenuItem(caption)
	{
		_alwaysShowMessage = false;
	}
	MenuNode::~MenuNode()
	{
	}

	void MenuItem::UnlockMessage()
	{
		_alwaysShowMessage = true;
	}

	void MenuItem::LockMessage()
	{
		_alwaysShowMessage = false;
	}

	void MenuItem::Select()
	{
		_isSelected = true;
	}

	void MenuItem::Release()
	{
		_isSelected = false;
	}

	void MenuItem::SetVisible()
	{
		_isVisible = true;
	}

	void MenuItem::Hide()
	{
		_isVisible = false;
	}

	void MenuItem::RunCallback()
	{
		if (_callback)
		{
			_showMessage = true;
			_callbackResult = _callback();
		}
	}

	bool MenuItem::IsSelected() const
	{
		return _isSelected;
	}

	bool MenuItem::IsMessageVisible() const
	{
		return _showMessage && _alwaysShowMessage;
	}

	void MenuItem::SetErrorMessage(tstring message)
	{
		_errorMessage = message;
	}

	void MenuItem::SetSuccessMessage(tstring message)
	{
		_successMessage = message;
	}

	const tstring& MenuItem::GetMessage()
	{
		_showMessage = false;
		return  _callbackResult ? _successMessage : _errorMessage;
	}

	const tstring& MenuItem::GetCaption() const
	{
		return _caption;
	}

	size_t MenuItem::GetHotKey() const
	{
		return _hotkey;
	}

	size_t MenuItem::GetCaptionLength() const
	{
		return _caption.length();
	}

	void MenuItem::SetHotkey(size_t code)
	{
		_hotkey = code;
	}

	bool MenuItem::IsVisible() const
	{
		return _isVisible;
	}

	void MenuNode::Add(std::shared_ptr<MenuItem> node)
	{
		// add to vector
		auto ptr = std::dynamic_pointer_cast<MenuNode>(node);
		if (ptr)
		{
			ptr->SetMaxVisibleMenuItems(_maxVisibleItems);
		}

		_menuItems.emplace_back(node);

		// change offset if needed
		auto captionLength = node->GetCaptionLength();
		if (_hotkeyOffset < captionLength)
			_hotkeyOffset = captionLength;

		// assign first as active if menu is empty
		if (_menuItems.size() == 1)
			SetFirtsSelected();

		// assign hotkey
		AssignHotkey(_menuItems.back());
	}
	void MenuNode::Draw()
	{
		Clear();

		// update deleted elements
		for (auto it = _menuItems.begin(); it != _menuItems.end(); ++it)
		{
			if (it->get()->Deleted())
			{
				// set nect selected
				SetNextSelected();

				// if marked to delete remove element
				it = _menuItems.erase(it);

				// on empty
				if (_menuItems.empty())
				{
					OnBack();
					return;
				}

				// check for break condition
				if (it == _menuItems.end())
				{
					break;
				}
			}
		}

		auto fromIt = _menuItems.begin();
		auto toIt = _menuItems.end();

		if (_maxVisibleItems < _menuItems.size())
		{
			fromIt = GetSelectedMenuIterator();
			toIt = std::next(fromIt);

			auto usedCount = 1u;
			auto directionUp = true;
			while (!(usedCount == _maxVisibleItems || usedCount == _menuItems.size()))
			{
				// choose direction
				if (directionUp)
				{
					// up
					if (fromIt != _menuItems.begin())
					{
						fromIt = std::prev(fromIt);
						++usedCount;
					}
				}
				else
				{
					//down
					if (toIt != _menuItems.end())
					{
						toIt = std::next(toIt);
						++usedCount;
					}
				}
				directionUp = !directionUp;
			}
		}
		for (auto it = fromIt; it != toIt; ++it)
		{
			PrintMenuItem(*it);
		}
	}

	void MenuNode::Clear() const
	{
		COORD coord = { 0, 0 };
		// If the function fails, the return value is zero.
		if (SetConsoleCursorPosition(_hOutput, coord) == 0)
		{
			auto ret = GetLastError();
			assert(false);
		}

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

		for (auto i = 0u; i < _maxVisibleItems; ++i)
			_outstream << std::setfill(_T(' ')) << std::setw(csbi.dwSize.X - 1) << _T(" ") << std::endl;

		// If the function fails, the return value is zero.
		if (SetConsoleCursorPosition(_hOutput, coord) == 0)
		{
			auto ret = GetLastError();
			assert(false);
		}
	}

	void MenuNode::ProcessHotKey(int32_t code)
	{
		if (IsHotKeyInUse(code))
		{
			auto activeIter = GetSelectedMenuIterator();
			auto iter = activeIter->get();

			if (iter->IsVisible())
			{
				iter->Release();
				_hotkeys.at(code).lock()->Select();

				Draw();
			}
		}
	}

	void MenuNode::PrintMenuItem(const std::shared_ptr<MenuItem>& item) const
	{
		if (item->IsVisible())
		{
			_outstream << (item->IsSelected() ? _T("->") : _T("  ")) << std::left << std::setw(_hotkeyOffset + 3) << item->GetCaption();
			auto hotkey = item->GetHotKey();
			if (hotkey)
			{
				_outstream << _T("[");
				switch (_hkpolicy)
				{
				case HotkeyPolicy::hp_letters: _outstream << TCHAR(hotkey); break;
				case HotkeyPolicy::hp_fx_keys: _outstream << _T("F");
				case HotkeyPolicy::hp_numbers: _outstream << (hotkey - 1); break;
				default: break;
				}

				_outstream << _T("]  ");
			}
			// show message
			if (item->IsMessageVisible())
				_outstream << item->GetMessage();

			// new ine and synchronization
			_outstream << std::endl;
		}
	}

	void MenuNode::OnBack()
	{
		_isProcessing = false;
	}

	void MenuNode::OnEnter()
	{
		auto it = GetSelectedMenuIterator();
		if (it != _menuItems.end())
		{
			it->get()->RunCallback();

			it = GetSelectedMenuIterator();
			if (it != _menuItems.end())
				it->get()->Execute();
			else
			{
				OnBack();
				return;
			}
		}
		Draw();
	}

	void MenuNode::ProcessKey()
	{
		while (_isProcessing)
		{
			auto ch = GetKey();
			if (ch == 0 || ch == 224)
			{
				// due to guidlines need to call this function twice
				// depends on implementation of getch analogs
				ch = GetKey();
				switch (ch)
				{
				case 75:
					/* left arrow handling */
					OnBack();
					break;
				case 77:
					/* right arrow handling */
					OnEnter();
					break;
				case 72:
					/* up arrow handling */
					SetPreviousSelected();
					Draw();
					break;
				case 80:
					/* down arrow handling */
					SetNextSelected();
					Draw();
					break;
				default:
				{
					if (_hkpolicy == HotkeyPolicy::hp_fx_keys)
					{
						//f1 = f12
						if (ch >= 59 && ch <= 68 || ch == 133 || ch == 134)
							ProcessHotKey(ch);
					}
				}
				}
			}
			else
			{
				// ENTER
				if (ch == 13)
				{
					OnEnter();
					continue;
				}

				// ESCAPE or BACKSPACE
				if (ch == 27 || ch == 8)
				{
					OnBack();
					continue;
				}

				// HOTKEYS
				auto key = ch;

				switch (_hkpolicy)
				{
				case HotkeyPolicy::hp_letters:
				{
					key = toupper(ch);
					break;
				}
				case HotkeyPolicy::hp_numbers:
				{
					key = ch - _T('0');
					break;
				}
				default: continue;
				}
				ProcessHotKey(key);
			}
		}
	}

	void MenuNode::SetNextSelected()
	{
		auto activeIter = GetSelectedMenuIterator();
		if (activeIter != _menuItems.end())
		{
			activeIter->get()->Release();
			if (++activeIter == _menuItems.end())
				SetFirtsSelected();
			else
				activeIter->get()->Select();
		}
	}

	void MenuNode::SetPreviousSelected()
	{
		auto activeIter = GetSelectedMenuIterator();
		if (activeIter != _menuItems.end())
		{
			activeIter->get()->Release();
			if (activeIter == _menuItems.begin())
				SetLastSelected();
			else
				(--activeIter)->get()->Select();
		}
	}

	void MenuNode::ResetSelected()
	{
		for (auto &&item : _menuItems)
			item->Release();
		if (!_menuItems.empty())
			SetFirtsSelected();
	}

	void MenuNode::SetFirtsSelected()
	{
		_menuItems.front()->Select();
	}

	void MenuNode::SetLastSelected()
	{
		_menuItems.back()->Select();
	}

	std::vector<std::shared_ptr<MenuItem>>::iterator MenuNode::GetSelectedMenuIterator()
	{
		return std::find_if(_menuItems.begin(), _menuItems.end(), [](auto&& item) { return item->IsSelected(); });
	}

	void MenuItem::Connect(std::function<bool()> callback)
	{
		_callback = callback;
	}

	void MenuNode::AssignHotkey(std::reference_wrapper<std::shared_ptr<MenuItem>> item)
	{
		switch (_hkpolicy)
		{
		case HotkeyPolicy::hp_letters:
		{
			// assign hotkey by the first available in caption
			for (auto letter : item.get()->GetCaption())
			{
				auto upper = toupper(letter);
				if (IsHotkeyAvailable(upper))
				{
					// add to hotkey map
					_hotkeys[upper] = item.get();
					item.get()->SetHotkey(upper);
					return;
				}
			}
			//todo: assign first available or dont assign at all 
			break;
		}
		case HotkeyPolicy::hp_numbers:
		{
			if (_hotkeys.size() < 10)
			{
				// add to hotkey map
				_hotkeys[_hotkeys.size() + 1u] = item.get();
				item.get()->SetHotkey(_hotkeys.size() + 1u);
			}
			break;
		}
		case HotkeyPolicy::hp_fx_keys:
		{
			auto add = 0;
			if (_hotkeys.size() < 11)
				add = 59;
			else
			{
				if (_hotkeys.size() < 13)
					add = 133;
			}
			// add to hotkey map
			_hotkeys[_hotkeys.size() + add] = item.get();
			item.get()->SetHotkey(_hotkeys.size() + 1u);
			break;
		}
		default:
			break;
		}
	}

	bool MenuNode::IsHotkeyAvailable(size_t hotkey)
	{
		return _hotkeys.find(hotkey) == _hotkeys.end() && (std::find(_illegalList.begin(), _illegalList.end(), hotkey) == _illegalList.end());
	}

	bool MenuNode::IsHotKeyInUse(size_t hotkey)
	{
		return _hotkeys.find(hotkey) != _hotkeys.end();
	}

	std::shared_ptr<MenuItem> MenuNode::GetSelectedItem()
	{
		return *GetSelectedMenuIterator();
	}

	void MenuNode::RemoveSelectedItem()
	{
		auto it = GetSelectedMenuIterator();
		if (it != _menuItems.end())
			it->get()->Delete();
	}

	void MenuNode::Execute()
	{
		if (_hOutput == nullptr)
			return;

		if (!_menuItems.empty())
		{
			Draw();
			_isProcessing = true;
			ProcessKey();
		}
	}

	void MenuNode::Reset()
	{
		if (!_menuItems.empty())
		{
			_menuItems.clear();
			_hotkeys.clear();
			_hotkeyOffset = 0;
		}
	}

	size_t MenuNode::GetSelectedPosition()
	{
		for (auto i = 0u; i < _menuItems.size(); ++i)
		{
			if (_menuItems[i]->IsSelected())
				return i;
		}
		return 0;
	}

	bool MenuNode::Empty() const
	{
		return _menuItems.empty();
	}

	void MenuNode::SetPolicy(HotkeyPolicy policy)
	{
		// 
		if (policy != _hkpolicy)
		{
			_hkpolicy = policy;
			_hotkeys.clear();
			for (auto && item : _menuItems)
				// reassign hotkeys
				AssignHotkey(item);
		}
	}

	void MenuNode::SetMaxVisibleMenuItems(size_t items)
	{
		if (items)
			_maxVisibleItems = items;
	}

	size_t MenuNode::GetMaxVisibleMenuItems() const
	{
		return _maxVisibleItems;
	}

	const std::vector<std::shared_ptr<MenuItem>>& MenuNode::GetItems() const
	{
		return _menuItems;
	}

	void* MenuItem::GetContext() const
	{
		return _assotiatedContext;
	}

	void MenuItem::Delete()
	{
		_pending_delete = true;
	}

	bool MenuItem::Deleted() const
	{
		return _pending_delete;
	}

	unsigned short MenuNode::GetKey()
	{
#ifdef UNICODE
#define GETCH  _getwch
#else
#define GETCH  _getch
#endif
		return GETCH();
	}
}
