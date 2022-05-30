#define MENU_FIRST_ITEM mDevices
#define MENU_ITEMS_COUNT 5
#include "MyMenu.cpp"

void exitProgram(Menu_Item_t* current_item);
void drawDevices(Menu_Item_t* current_item);
void drawUsersWithWait(Menu_Item_t* current_item);
void drawDeviceTypesWithWait(Menu_Item_t* current_item);
void drawUsers(Menu_Item_t* current_item);
void drawDeviceTypes(Menu_Item_t* current_item);
void drawReportsSortByPrice(Menu_Item_t* current_item);
void drawReportsSortByType(Menu_Item_t* current_item);
void drawReportsSortBySupplyDate(Menu_Item_t* current_item);
void drawReportsCostRemainder(Menu_Item_t* current_item);
void drawReportsUserInfo(Menu_Item_t* current_item);
void drawAbout(Menu_Item_t* current_item);

MENU_ITEM(mDevices, mLists, NULL_MENU, NULL_MENU, NULL_MENU, L"Devices", L"Список устройств", &drawDevices);
MENU_ITEM(mLists, mReports, mDevices, NULL_MENU, mListsBack, L"Lists", L"Допольнительные списки", NULL);
	MENU_ITEM(mListsBack, mListsUsers, NULL_MENU, mLists, mDevices, L"", L"Назад", NULL);
	MENU_ITEM(mListsUsers, mListsDeviceTypes, mListsBack, mLists, NULL_MENU, L"Users", L"Список работников", &drawUsersWithWait);
	MENU_ITEM(mListsDeviceTypes, NULL_MENU, mListsUsers, mLists, NULL_MENU, L"Device types", L"Список типов устройств", &drawDeviceTypesWithWait);
MENU_ITEM(mReports, mAbout, mLists, NULL_MENU, mReportsBack, L"Reports", L"Отчеты", NULL);
	MENU_ITEM(mReportsBack, mReportsSort, NULL_MENU, mReports, mDevices, L"", L"Назад", NULL);
	MENU_ITEM(mReportsSort, mReportsCostRemainder, mReportsBack, mReports, mReportsSortBack, L"Sort", L"Сортировка", NULL);
		MENU_ITEM(mReportsSortBack, mReportsSortByPrice, NULL_MENU, mReportsSort, mReportsBack, L"", L"Назад", NULL);
		MENU_ITEM(mReportsSortByPrice, mReportsSortByType, mReportsSortBack, mReportsSort, NULL_MENU, L"Sort by price", L"Сортировка за ценой", &drawReportsSortByPrice);
		MENU_ITEM(mReportsSortByType, mReportsSortBySupplyDate, mReportsSortByPrice, mReportsSort, NULL_MENU, L"Sort by type", L"Сортировка за типом", &drawReportsSortByType);
		MENU_ITEM(mReportsSortBySupplyDate, NULL_MENU, mReportsSortByType, mReportsSort, NULL_MENU, L"Sort by date", L"Сортировка за датой", &drawReportsSortBySupplyDate);
	MENU_ITEM(mReportsCostRemainder, mReportsUserInfo, mReportsBack, mReports, NULL_MENU, L"Remainder of the cost", L"Остаток от стоимости", &drawReportsCostRemainder);
	MENU_ITEM(mReportsUserInfo, NULL_MENU, mReportsCostRemainder, mReports, NULL_MENU, L"User Info", L"Информация о пользователе", &drawReportsUserInfo);
MENU_ITEM(mAbout, mExit, mReports, NULL_MENU, NULL_MENU, L"About", L"Описание программы", &drawAbout);
MENU_ITEM(mExit, NULL_MENU, mAbout, NULL_MENU, NULL_MENU, L"Exit", L"Выход", &exitProgram);