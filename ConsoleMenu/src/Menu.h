#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <map>
#include <list>
#include <windows.h>

#undef GetMessage

class MenuItem
{
protected:

	std::string _caption{ "" };
	int32_t _hotkey{ 0 };
	std::function<bool()> _callback{ nullptr };
	bool _isVisible{ true };
	bool _isSelected{ false };

	// message
	std::string _errorMessage{ "Error" };
	std::string _successMessage{ "Success" };
	bool _showMessage{ false };
	bool _callbackResult{ false };
	bool _alwaysShowMessage{ true };

public:

	// default c-tor
	MenuItem() = default;

	// c-tor
	explicit MenuItem(const std::string &caption) :_caption(caption) {};

	// virtual d-tor
	virtual ~MenuItem() = default;

	// available error or success message to be shown
	void UnlockMessage();

	// disable error or success message to be shown 
	void LockMessage();

	// mark item as selected
	void Select();

	// mark item as not selected
	void Release();

	// mark item as visible
	void SetVisible();

	// mark item as invisible
	void Hide();

	// run callbalck if it is available and make message visible
	void RunCallback();

	// connect callback to menu item or node OnEnter event
	void Connect(std::function<bool()> callback);

	// set hotkey code
	void SetHotkey(int32_t code);

	// return true if item is marked as visible
	bool IsVisible() const;

	// return true if item is marked as selected
	bool IsSelected() const;

	// return true if message is marked as visible and unlocked
	bool IsMessageVisible() const;

	// return caption of item
	const std::string& GetCaption() const;

	// return message base on callback return result
	// if callback executed successfull return success message else error message
	const std::string& GetMessage();

	// return assigned hotkey
	uint32_t GetHotKey() const;

	// return length of item's caption
	size_t GetCaptionLength() const;

	// set error message
	void SetErrorMessage(std::string message);

	// set success message
	void SetSuccessMessage(std::string message);
	
	//
	virtual void Execute() {};
};

class MenuNode : public MenuItem
{
public:

	// hotkeys generates based on
	enum class HotkeyPolicy
	{
		// do not generate hotkeys
		hp_none,
		// the first available letter in caption, A-Z
		hp_letters,
		// index by value from 1 to 9
		hp_numbers,
		// index from F1 to F12
		hp_fx_keys,
	};

	enum class VisibleScrollPolicy
	{
		vsp_center,
		vsp_down,
		vsp_up
	};

	// c-tor
	explicit MenuNode(const std::string &caption);

	// v d-tor
	virtual ~MenuNode();

	// Adds new menu item. If need to delegate ownage use std::move
	void Add(std::shared_ptr<MenuItem> node);

	// Call menu
	void Execute() override;

	// Reset all menu items
	void Reset();

	// returns selected menu index
	size_t GetSelectedPosition();

	// return true if node has no items
	bool Empty() const;

	// sets hotkey generation policy
	void SetPolicy(HotkeyPolicy policy);

	// return const reference to menu items
	const std::vector<std::shared_ptr<MenuItem>>& GetItems() const;

	// set maximum visible menu items in node
	// this one do not change recursively this parameter 
	void SetMaxVisibleMenuItems(uint8_t items);

	// return maximum visible menu items count
	uint8_t GetMaxVisibleMenuItems() const;

private:

	// vector of menu items
	std::vector<std::shared_ptr<MenuItem>> _menuItems;

	// map of hotkeys
	std::map<int32_t, std::weak_ptr<MenuItem>> _hotkeys;

	// list of illegal hotkey letters
	std::list<char> _illegalList{ ' ', '/', '.', ',' };

	//
	HotkeyPolicy _hkpolicy{ HotkeyPolicy::hp_letters };

	//
	VisibleScrollPolicy _vsp{ VisibleScrollPolicy::vsp_center };

	// length of the biggest line in menu items
	uint8_t _hotkeyOffset{ 0u };

	// maximum visible menu items
	uint8_t _maxVisibleItems{ 3u };

	void OnBack();
	void SetNextSelected();
	void SetPreviousSelected();
	void ResetSelected();
	void SetFirtsSelected();
	void SetLastSelected();
	// return selected menu iterator on success or end on failure
	auto GetSelectedMenuIterator()
	{
		return std::find_if(_menuItems.begin(), _menuItems.end(), [](auto&& item) { return item->IsSelected(); });
	}
	void AssignHotkey(std::reference_wrapper<std::shared_ptr<MenuItem>> item);

	bool IsHotkeyAvailable(int32_t hotkey);

public:

	// return mutable selected item
	std::shared_ptr<MenuItem> GetSelectedItem()
	{
		return *GetSelectedMenuIterator();
	}

protected:

	// 
	bool _isProcessing{ false };

	//
	HANDLE _hOutput{ GetStdHandle(STD_OUTPUT_HANDLE) };

	// clear screen and draw menu items
	void Draw();

	// clear screen or part of it
	void Clear() const;

	// run callback if it is available and draw menu items
	void OnEnter();

	// execute key processing 
	void ProcessKey();

	// if hotkey is available set its menu item selected and redraw menu
	void ProcessHotKey(int32_t code);

	// draw single men item
	void PrintMenuItem(const std::shared_ptr<MenuItem>& item) const;

};