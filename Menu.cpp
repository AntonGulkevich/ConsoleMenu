#include "Menu.h"

#include <iostream> // cout
#include <conio.h>
#include <algorithm>
#include <iomanip>
#include <cctype>


MenuNode::MenuNode(const std::string& caption) :MenuItem(caption)
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

void MenuItem::SetErrorMessage(std::string message)
{
	_errorMessage = message;
}

void MenuItem::SetSuccessMessage(std::string message)
{
	_successMessage = message;
}

const std::string& MenuItem::GetMessage()
{
	_showMessage = false;
	return  _callbackResult ? _successMessage : _errorMessage;
}

const std::string& MenuItem::GetCaption() const
{
	return _caption;
}

uint32_t MenuItem::GetHotKey() const
{
	return _hotkey;
}

size_t MenuItem::GetCaptionLength() const
{
	return _caption.length();
}

void MenuItem::SetHotkey(int32_t code)
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
		ptr->SetMaxVisibleMenuItems(_maxVisibleItems);

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

	if (_maxVisibleItems < _menuItems.size())
	{
		auto fromIt = GetSelectedMenuIterator();
		auto toIt = std::next(fromIt);
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

		for (auto it = fromIt; it != toIt; ++it)
			PrintMenuItem(*it);
	}
	else
		for (const auto &item : _menuItems)
			PrintMenuItem(item);
}

void MenuNode::Clear() const
{
	COORD coord = { 0, 0 };
	SetConsoleCursorPosition(_hOutput, coord);

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

	for (auto i = 0u; i < _maxVisibleItems; ++i)
		std::cout << std::setfill(' ') << std::setw(csbi.dwSize.X - 1) << " " << std::endl;

	SetConsoleCursorPosition(_hOutput, coord);
}

void MenuNode::ProcessHotKey(int32_t code)
{
	if (!IsHotkeyAvailable(code))
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
		std::cout << (item->IsSelected() ? "->" : "  ") << std::left << std::setw(_hotkeyOffset + 3) << item->GetCaption();
		char hotkey = item->GetHotKey();
		if (hotkey)
		{
			std::cout << "[";
			switch (_hkpolicy)
			{
			case HotkeyPolicy::hp_letters: std::cout << hotkey; break;
			case HotkeyPolicy::hp_fx_keys: std::cout << "F";
			case HotkeyPolicy::hp_numbers: std::cout << std::to_string(hotkey - 1); break;
			default: break;
			}

			std::cout << "]  ";
		}
		// show message
		if (item->IsMessageVisible())
		{
			std::cout << item->GetMessage();
		}
		std::cout << std::endl;
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
		it->get()->Execute();
	}
	Draw();
}

void MenuNode::ProcessKey()
{
	while (_isProcessing)
	{
		//Draw();
		auto ch = _getch();
		if (ch == 0 || ch == 224)
		{
			ch = _getch();
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
			}

			// ESCAPE or BACKSPACE
			if (ch == 27 || ch == 8)
			{
				OnBack();
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
				key = ch - '0';
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
			_hotkeys[_hotkeys.size() + 1] = item.get();
			item.get()->SetHotkey(_hotkeys.size() + 1);
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
		item.get()->SetHotkey(_hotkeys.size() + 1);
		break;
	}
	default: 
		break;
	}
}

bool MenuNode::IsHotkeyAvailable(int32_t hotkey)
{
	return _hotkeys.find(hotkey) == _hotkeys.end() && (std::find(_illegalList.begin(), _illegalList.end(), hotkey) == _illegalList.end());
}

void MenuNode::Execute()
{
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
	for (auto i = 0; i < _menuItems.size(); ++i)
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

void MenuNode::SetMaxVisibleMenuItems(uint8_t items)
{
	if (items)
		_maxVisibleItems = items;
}

uint8_t MenuNode::GetMaxVisibleMenuItems() const
{
	return _maxVisibleItems;
}

const std::vector<std::shared_ptr<MenuItem>>& MenuNode::GetItems() const
{
	return _menuItems;
}
