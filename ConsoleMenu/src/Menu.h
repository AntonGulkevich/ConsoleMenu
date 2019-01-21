#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <map>
#include <list>
#include <windows.h>
#include <TCHAR.h>
#include <iostream>

#undef GetMessage

namespace Menu
{
	using tstring = std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>>;
	using tcout = std::basic_ostream<TCHAR, std::char_traits<TCHAR>>;

	class MenuFrame
	{
		/*
		 * Todo
		 * 1. Aligement
		 * 2. Moving
		 * 3. Dinamic show/hide
		 * 4. Drag&drop
		 * 5. Resizing
		 * 6. Ostream syntax (<< variadic templates)
		 * 7. Policy (external / internal )
		 * 8. Optimization (update only if info changed)
		 *
		 */

		 // menu text 
		tstring _caption{ _T("New frame.") };

		//
		bool _show_caption{ true };

		//
		bool _is_visible{ true };

		//
		short _width{ 40u };

		//
		short _height{ 10u };

#ifdef UNICODE 
		//
		//TCHAR _vertical_border_symbol{ _T('\x2502') };
		//TCHAR _top_left_border_symbol{ _T('\x250E') };
		//TCHAR _top_right_border_symbol{ _T('\x2512') };
		//TCHAR _bot_left_border_symbol{ _T('\x2516') };
		//TCHAR _bot_right_border_symbol{ _T('\x251A') };
		TCHAR _vertical_border_symbol{ _T('\x2502') };
		TCHAR _top_left_border_symbol{ _T('\x2552') };
		TCHAR _top_right_border_symbol{ _T('\x2555') };
		TCHAR _bot_left_border_symbol{ _T('\x2558') };
		TCHAR _bot_right_border_symbol{ _T('\x255B') };
		TCHAR _horizontal_border_symbol{ _T('\x2550') };
#else
		//
		TCHAR _vertical_border_symbol{ _T('|') };

		//
		TCHAR _horizontal_border_symbol{ _T('-') };
#endif

		//
		bool _show_vertical_border{ true };

		//
		bool _show_horizontal_border{ true };

		//
		short _top_offet{ 0u };

		//
		short _left_offset{ 0u };

		//
		std::vector <std::unique_ptr <tstring>> _list_string;

	protected:

		//
		tcout & _outstream
		{
#ifdef UNICODE
			std::wcout
#else
			std::cout
#endif
		};

		// hadle for console output
		HANDLE _hOutput{ nullptr };

	public:

		// c-tor
		explicit MenuFrame(const tstring& str);

		// d-tor
		~MenuFrame() = default;

		//
		void SetConsole(HANDLE console_handle);

		void ClearList();

		void AddLine(const tstring & str);
		void AddLine(std::unique_ptr<tstring> str);
		void AddLine(const TCHAR * _pstr);

		void SetHeight(short heigth);
		void SetWidth(short width);

		short GetHeight() const;
		short GetWidth() const;

		void SetCaption(const tstring & str);
		void SetCaption(const TCHAR * _pstr);

		//
		void Draw();

		//
		void SetLeftOffset(short left_offset);
		void SetTopOffset(short top_offset);

		//
		void Clear() const;

		//
		bool IsVisible() const;
		void Hide();
		void Show();

		//
		void Update();
		};

	class MenuItem
	{
		// true if marked as deleted
		bool _pending_delete{ false };

		// 
		bool _callbackResult{ false };

		// 
		bool _showMessage{ false };

	protected:

		// menu text 
		tstring _caption{ _T("") };

		// hotkey 0-if not in use
		size_t _hotkey{ 0u };

		// callback
		std::function<bool()> _callback{ nullptr };

		// true if menu item is visible
		bool _isVisible{ true };

		// true if item is selected
		bool _isSelected{ false };

		// message shown after callback executes with error
		tstring _errorMessage{ _T("Error") };

		// message shown after callback executes with success
		tstring _successMessage{ _T("Success") };

		// true if error or success message are visible
		bool _alwaysShowMessage{ true };

		// context assotiated with this menu
		void * _assotiatedContext{ nullptr };

	public:

		// default c-tor
		MenuItem() = default;

		// c-tor
		explicit MenuItem(const tstring &caption) :_caption(caption) {};

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
		void SetHotkey(size_t code);

		// return true if item is marked as visible
		bool IsVisible() const;

		// return true if item is marked as selected
		bool IsSelected() const;

		// return true if message is marked as visible and unlocked
		bool IsMessageVisible() const;

		// return caption of item
		const tstring& GetCaption() const;

		// return message base on callback return result
		// if callback executed successfull return success message else error message
		const tstring& GetMessage();

		// return assigned hotkey
		size_t GetHotKey() const;

		// return length of item's caption
		size_t GetCaptionLength() const;

		// set error message
		void SetErrorMessage(tstring message);

		// set success message
		void SetSuccessMessage(tstring message);

		//
		virtual void Execute() {};

		//
		void SetContext(void* context);

		//
		void * GetContext() const;

		//
		void Delete();

		//
		bool Deleted() const;
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
		explicit MenuNode(const tstring &caption);

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
		void SetMaxVisibleMenuItems(size_t items);

		// return maximum visible menu items count
		size_t GetMaxVisibleMenuItems() const;

		// return mutable selected item
		std::shared_ptr<MenuItem> GetSelectedItem();

		// removes selected item if it exists
		void RemoveSelectedItem();

		//
		void AddFrame(std::shared_ptr<MenuFrame> frame);



	private:

		// vector of frames
		std::vector<std::shared_ptr<MenuFrame>> _menuFrames;

		// vector of menu items
		std::vector<std::shared_ptr<MenuItem>> _menuItems;

		// map of hotkeys
		std::map<size_t, std::weak_ptr<MenuItem>> _hotkeys;

		// list of illegal hotkey letters
		std::list<TCHAR> _illegalList{ _T(' '), _T('/'), _T('.'), _T(',') };

		//
		HotkeyPolicy _hkpolicy{ HotkeyPolicy::hp_letters };

		//
		VisibleScrollPolicy _vsp{ VisibleScrollPolicy::vsp_center };

		// length of the biggest line in menu items
		size_t _hotkeyOffset{ 0u };

		// maximum visible menu items
		size_t _maxVisibleItems{ 3u };

		//
		void OnBack();

		// selection modifiers
		void SetNextSelected();
		void SetPreviousSelected();
		void ResetSelected();
		void SetFirtsSelected();
		void SetLastSelected();

		// return selected menu iterator on success or end on failure
		std::vector<std::shared_ptr<MenuItem>>::iterator GetSelectedMenuIterator();

		void AssignHotkey(std::reference_wrapper<std::shared_ptr<MenuItem>> item);

		bool IsHotkeyAvailable(size_t hotkey);

		bool IsHotKeyInUse(size_t hotkey);

		// clear screen and draw menu items
		void Draw();

		// run callback if it is available and draw menu items
		void OnEnter();

		// if hotkey is available set its menu item selected and redraw menu
		void ProcessHotKey(int32_t code);

		// true if node is processing keys input
		bool _isProcessing{ false };

		//
		void ClearFrameOnScreen();

	protected:

		//
		tcout & _outstream
		{
	#ifdef UNICODE
			std::wcout
	#else
			std::cout
	#endif
		};

		// hadle for console output
		HANDLE _hOutput{ GetStdHandle(STD_OUTPUT_HANDLE) };

		// clear screen or part of it
		void Clear() const;

		// blocking function that await key input
		// return key code
		static unsigned short GetKey();

		// execute key processing
		void ProcessKey();

		// draw single menu item
		void PrintMenuItem(const std::shared_ptr<MenuItem>& item) const;

		};

	}
