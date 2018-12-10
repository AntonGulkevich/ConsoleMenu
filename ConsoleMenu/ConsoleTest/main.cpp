#pragma once
#include <TCHAR.h>
#include "../src/Menu.h"
#pragma comment(lib, "ConsoleMenu.lib")

using namespace  Menu;

bool Delete(std::shared_ptr<MenuNode> node)
{
	node->RemoveSelectedItem();
	return true;
}

void DeleteTest()
{
	MenuNode main_menu{ _T("main") };

	main_menu.SetMaxVisibleMenuItems(10);

	auto node_1{ std::make_shared<MenuNode>(_T("Node 1")) };
	auto node_2{ std::make_shared<MenuNode>(_T("Node 2")) };

	auto item11{ std::make_shared<MenuNode>(_T("Item 1")) };
	auto item12{ std::make_shared<MenuNode>(_T("Item 2")) };
	auto item13{ std::make_shared<MenuNode>(_T("Item 3")) };
	auto item14{ std::make_shared<MenuNode>(_T("Item 4")) };
	auto item15{ std::make_shared<MenuNode>(_T("Item 5")) };

	auto item21{ std::make_shared<MenuNode>(_T("Item 1")) };
	auto item22{ std::make_shared<MenuNode>(_T("Item 2")) };
	auto item23{ std::make_shared<MenuNode>(_T("Item 3")) };
	auto item24{ std::make_shared<MenuNode>(_T("Item 4")) };
	auto item25{ std::make_shared<MenuNode>(_T("Item 5")) };

	auto removeThisItem{ std::make_shared<MenuItem>(_T("Delete")) };
	removeThisItem->Connect([&main_menu]()
	{
		auto node = std::dynamic_pointer_cast<MenuNode>(main_menu.GetSelectedItem());
		return node != nullptr && !node->Empty() ? Delete(node) : false;
	}
	);

	main_menu.Add(node_1);
	main_menu.Add(node_2);

	item11->Add(removeThisItem);
	item12->Add(removeThisItem);
	item13->Add(removeThisItem);
	item14->Add(removeThisItem);
	item15->Add(removeThisItem);

	item21->Add(removeThisItem);
	item22->Add(removeThisItem);
	item23->Add(removeThisItem);
	item24->Add(removeThisItem);
	item25->Add(removeThisItem);

	node_1->Add(std::move(item11));
	node_1->Add(std::move(item12));
	node_1->Add(std::move(item13));
	node_1->Add(std::move(item14));
	node_1->Add(std::move(item15));

	node_2->Add(std::move(item21));
	node_2->Add(std::move(item22));
	node_2->Add(std::move(item23));
	node_2->Add(std::move(item24));
	node_2->Add(std::move(item25));



	main_menu.Execute();
}

int _tmain(int argc, TCHAR *argv[])
{
	DeleteTest();

	return 0;
}