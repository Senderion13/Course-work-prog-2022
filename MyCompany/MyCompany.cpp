#include <iostream>
#include <format>
#include <string>
#include "menuitems.cpp"
#include "MyODBC.cpp"

#define WINDOW_HEIGHT 50
#define WINDOW_WIDTH 200
#define MENU_HEADER_TEXT L"MAIN MENU"
using namespace std;

ofstream file;
WORD color = BACKGROUND_RED | BACKGROUND_GREEN;
WORD defColor = BACKGROUND_BLUE | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;

struct MenuPositionInfo {
	int pos;
	int count;
};

MenuPositionInfo lastPosition;
void wait() {
	int currkey;
	while (1) {
		currkey = _getch();
		if (currkey == 27) {
			system("cls");
			return;
		}
	}
}

int getMenuItemsCount() {
	Menu_Item_t* item = CurrentMenu;
	int menuCount = 0;
	do {
		menuCount++;
	} while ((item = item->Next) != &NULL_MENU);
	return menuCount;
}

void clearArea(int start, int count) {
	SetConsoleTextAttribute(hConsole, 0);
	char blank[MENU_TEXT_SIZE + 3];
	for (int i = 0; i < MENU_TEXT_SIZE + 2; i++)
	{
		blank[i] = ' ';
	}
	blank[MENU_TEXT_SIZE + 2] = 0;
	for (int i = 0; i < count+1; i++)
	{
		SetConsoleCursorPosition(hConsole, { (SHORT)((WINDOW_WIDTH - MENU_TEXT_SIZE) / 2 - 1), (SHORT)(start + i)});
		cout << blank;
	}
}

void drawMenu(COORD c, COORD defC) {
	if (getMenuItemsCount() < lastPosition.count) {
		clearArea(lastPosition.pos, lastPosition.count);
	}
	Menu_Item_t* item = CurrentMenu;
	int count = GetLevelItemsCount(CurrentMenu);
	SetConsoleCursorPosition(hConsole, { (SHORT)((WINDOW_WIDTH - MENU_TEXT_SIZE) / 2 - 1), (SHORT)((WINDOW_HEIGHT - count - 1) / 2)});
	SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN);
	for (int i = 0; i < (MENU_TEXT_SIZE - wcslen(MENU_HEADER_TEXT) + 2) / 2 + 1; i++) {
		wcout << " ";
	}
	std::wprintf(MENU_HEADER_TEXT);
	for (int i = 0; i < (MENU_TEXT_SIZE - wcslen(MENU_HEADER_TEXT) + 2) / 2; i++) cout << " ";
	SetConsoleTextAttribute(hConsole, 0);
	int menuCount = 0;
	do {
		menuCount++;
		SetConsoleCursorPosition(hConsole, { (SHORT)((WINDOW_WIDTH - MENU_TEXT_SIZE) / 2 - 1), (SHORT)((WINDOW_HEIGHT - count - 1) / 2 + menuCount) });
		SetConsoleTextAttribute(hConsole, item == CurrentMenuItem ? color : defColor);
		wcout << " ";
		int text_len = wcslen(item->Text);
		for (int i = 0; i < MENU_TEXT_SIZE; i++) {
			if (i < text_len) 
				wcout << item->Text[i];
			else
			{
				if (item->Child != &NULL_MENU && item->Child != NULL && (i >= text_len && i < text_len + 3) ) {
					wcout << ".";
				} else wcout << " ";
			}
		}
		wcout << " ";
		SetConsoleTextAttribute(hConsole, 0);
	} while ((item = item->Next) != &NULL_MENU);

	lastPosition.pos = (WINDOW_HEIGHT - count - 1) / 2;
	lastPosition.count = menuCount;

	SetConsoleCursorPosition(hConsole, {0, WINDOW_HEIGHT - 1});
	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY);
	wprintf(CurrentMenuItem->Hint);
}

void exitProgram(Menu_Item_t* current_item) {
	exit(0);
}

void addRecord(Menu_Item_t* current_item) {
	int typeID, userID, price, lifeTime;
	wchar_t supplyDate[11];
	wchar_t description[100];
	drawDeviceTypes(current_item);
	wprintf(L"Enter type id: ");
	cin >> typeID;
	drawUsers(current_item);
	wprintf(L"Enter user id: ");
	cin >> userID;
	wprintf(L"Enter price: ");
	cin >> price;
	wprintf(L"Enter life time: ");
	cin >> lifeTime;
	wprintf(L"Enter supply date: ");
	wcin >> supplyDate;
	wprintf(L"Enter description: ");
	wcin.getline(description, 100);
	wcin.getline(description, 100);
	wchar_t sqlRequest[SQL_QUERY_SIZE] = L"";
	swprintf(sqlRequest, SQL_QUERY_SIZE, L"INSERT INTO devices ( userID, typeID, price, supplyDate, lifeTime, description) VALUES (%d,%d,%d,\'%ls\',%d,\'%ls\')", userID, typeID, price, supplyDate, lifeTime, description);
	getTable(sqlRequest, false);
}

void updateRecord(Menu_Item_t* current_item) {
	int ID, typeID, userID, price, lifeTime;
	wchar_t supplyDate[11];
	wchar_t description[100];
	wprintf(L"Enter device id: ");
	cin >> ID;
	drawDeviceTypes(current_item);
	wprintf(L"Enter new type id: ");
	cin >> typeID;
	drawUsers(current_item);
	wprintf(L"Enter new user id: ");
	cin >> userID;
	wprintf(L"Enter new price: ");
	cin >> price;
	wprintf(L"Enter new life time: ");
	cin >> lifeTime;
	wprintf(L"Enter new supply date: ");
	wcin >> supplyDate;
	wprintf(L"Enter new description: ");
	wcin.getline(description, 100);
	wcin.getline(description, 100);
	wchar_t sqlRequest[SQL_QUERY_SIZE] = L"";
	swprintf(sqlRequest, SQL_QUERY_SIZE, L"UPDATE devices SET userID=%d, typeID=%d, price=%d, supplyDate=\'%ls\', lifeTime=%d, description=\'%ls\' WHERE ID=%d", userID, typeID, price, supplyDate, lifeTime, description, ID);
	getTable(sqlRequest, false);
}

void deleteRecord(Menu_Item_t* current_item) {
	int ID;
	wprintf(L"Enter device id: ");
	cin >> ID;
	wchar_t sqlRequest[SQL_QUERY_SIZE] = L"";
	swprintf(sqlRequest, SQL_QUERY_SIZE, L"DELETE FROM devices WHERE ID = %d", ID);
	getTable(sqlRequest, false);
}

void drawDevices(Menu_Item_t* current_item) {
	SQLWCHAR sqlRequest[SQL_QUERY_SIZE] = L"SELECT devices.id, users.name AS user, devicetypes.name AS `device type`, Devices.price, Devices.supplyDate, Devices.lifeTime, Devices.description FROM Devices INNER JOIN users ON devices.userID=users.id INNER JOIN devicetypes ON devices.typeID=devicetypes.id";
	int currkey, extkey;
	bool exit = false;
	while (!exit) {
		system("cls");
		getTable(sqlRequest, false);
		SetConsoleCursorPosition(hConsole, { 0, 48 });
		SetConsoleTextAttribute(hConsole, 240);
		wprintf(L" F4 - add new record | F5 - update existed record (by id) | F6 - delete record (by id) ");
		SetConsoleTextAttribute(hConsole, 15);
		SetConsoleCursorPosition(hConsole, { 0, 0 });
		currkey = _getch();
		switch (currkey) {
			case 0:
			{
				extkey = _getch();
				system("cls");
				switch (extkey) {
					case 62:
					{
						addRecord(current_item);
						break;
					}
					case 63:
					{
						updateRecord(current_item);
						break;
					}
					case 64:
					{
						deleteRecord(current_item);
						break;
					}
				}
				break;
			}
			case 13:
			{

				break;
			}
			case 27:
			{
				exit = true;
				break;
			}
			
		}
	}
	system("cls");
}

void drawUsersWithWait(Menu_Item_t* current_item) {
	SQLWCHAR sqlRequest[SQL_QUERY_SIZE] = L"SELECT * FROM users";
	system("cls");
	getTable(sqlRequest, false);
	wait();
}

void drawDeviceTypesWithWait(Menu_Item_t* current_item) {
	SQLWCHAR sqlRequest[SQL_QUERY_SIZE] = L"SELECT * FROM devicetypes";
	system("cls");
	getTable(sqlRequest, false);
	wait();
}

void drawUsers(Menu_Item_t* current_item) {
	SQLWCHAR sqlRequest[SQL_QUERY_SIZE] = L"SELECT * FROM users";
	getTable(sqlRequest, false);
}

void drawDeviceTypes(Menu_Item_t* current_item) {
	SQLWCHAR sqlRequest[SQL_QUERY_SIZE] = L"SELECT * FROM devicetypes";
	getTable(sqlRequest, false);
}
// -------------------------------------- SORTING ------------------------------- //
void drawReportsSortByPrice(Menu_Item_t* current_item) {
	SQLWCHAR sqlRequest[SQL_QUERY_SIZE] = L"SELECT devices.id, users.name AS user, devicetypes.name AS `device type`, Devices.price, Devices.supplyDate, Devices.lifeTime, Devices.description FROM Devices INNER JOIN users ON devices.userID=users.id INNER JOIN devicetypes ON devices.typeID=devicetypes.id ORDER BY price";
	system("cls");
	getTable(sqlRequest, true);
	wait();
}
void drawReportsSortByType(Menu_Item_t* current_item) {
	SQLWCHAR sqlRequest[SQL_QUERY_SIZE] = L"SELECT devices.id, users.name AS user, devicetypes.name AS `device type`, Devices.price, Devices.supplyDate, Devices.lifeTime, Devices.description FROM Devices INNER JOIN users ON devices.userID=users.id INNER JOIN devicetypes ON devices.typeID=devicetypes.id ORDER BY typeID";
	system("cls");
	getTable(sqlRequest, true);
	wait();
}
void drawReportsSortBySupplyDate(Menu_Item_t* current_item) {
	SQLWCHAR sqlRequest[SQL_QUERY_SIZE] = L"SELECT devices.id, users.name AS user, devicetypes.name AS `device type`, Devices.price, Devices.supplyDate, Devices.lifeTime, Devices.description FROM Devices INNER JOIN users ON devices.userID=users.id INNER JOIN devicetypes ON devices.typeID=devicetypes.id ORDER BY supplyDate";
	system("cls");
	getTable(sqlRequest, true);
	wait();
}
// --------------------------- Cost Remainder ----------------------- //
void drawReportsCostRemainder(Menu_Item_t* current_item) {
	SQLWCHAR sqlRequest[SQL_QUERY_SIZE] = L"SELECT users.name AS \`employee\'s name\`, devicetypes.name AS \'device type\', devices.description, FORMAT(GREATEST(( ( price / lifeTime ) * ( lifeTime - ( ( YEAR(CURDATE())*12+MONTH(CURDATE()) ) - ( YEAR(supplyDate)*12+MONTH(supplyDate) ) ) ) ), 0), 2) AS \`remainder of the cost\` FROM devices JOIN devicetypes ON devicetypes.ID=devices.typeID JOIN users ON users.ID=devices.userID";
	system("cls");
	getTable(sqlRequest, true);
	wait();
}
// ---------------------------------- USER INFO --------------------- //
void drawReportsUserInfo(Menu_Item_t* current_item) {
	system("cls");
	wprintf(L"Enter user name: ");
	wchar_t name[60] = L"SELECT * FROM users WHERE name=\"";
	wcin >> name + wcslen(name);
	name[wcslen(name)] = 34;
	system("cls");
	getTable(name, true);
	wait();
}
// ---------------------------------------- ABOUT ----------------- //
void drawAbout(Menu_Item_t* current_item) {
	system("cls");
	wprintf(L"This program was created by Sierov Andrii. \nFor another information contact with me - TG: @Senderion\n");

	wprintf(L"\n\n____________________$$$$$$$$$$\n_________________$$$__________$$$\n_______________$$________________$$__________$$$\n____________$$$____________________$$_______$_$_$\n__$$$______$$$$____________$$$$$$$$__$_____$_$__$\n_$__$$____$____$$$_$____$$$$$$________$___$__$__$\n$___$_$_$$$________$___$$$_____________$_$___$$$\n$__$__$$$$$__$$$$__$________$$$$$$______$$_$$$\n_$$$$$$$$_$_$$$$_$_$______$$$$$$__$_______$\n_________$__$$$$$$$_$$$$__$$$$$$$$$_______$\n________$________$$$____$$$$______________$\n_______$_______$$___________$$____________$\n______$_______$_______________$___________$\n______$______$_$$$$_____$$$$$$_____________$\n_____$_____$$______$$$$$______$$$__________$\n_____$____$________$___$_________$$$________$\n_____$___$__$_____$___$_____________$_______$\n_____$_______$$$$$$$$$$$$$$$$$$$$$__________$\n_____$______________________________________$\n______$________$$$$$$$$$$$$$$_______________$\n______$____________$$$$$$___________________$$\n_______$______________________________________$\n___$$$$$$______________________________________$\n$$$______$_____________________________________$\n__________$$_______________________$$___________$\n____________$$$__________________$$_____________$\n_______________$$$$$$$$$$$$$$$$$$");
	wprintf(L"\n\nPress ESC to go back");
	wait();
}


int main()
{
	setlocale(LC_ALL, "rus_rus.866");
	perror("setlocale");
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTitle(L"My Program");
	CONSOLE_SCREEN_BUFFER_INFOEX consolesize;
	CurrentMenuItem = &MENU_FIRST_ITEM;
	CurrentMenu = &MENU_FIRST_ITEM;
	consolesize.cbSize = sizeof(consolesize);

	GetConsoleScreenBufferInfoEx(hConsole, &consolesize);

	COORD c;
	COORD defC;
	c.X = WINDOW_WIDTH;
	c.Y = WINDOW_HEIGHT;
	defC.X = 0;
	defC.Y = 0;
	consolesize.dwSize = c;

	consolesize.srWindow.Left = 0;
	consolesize.srWindow.Right = c.X;
	consolesize.srWindow.Top = 0;
	consolesize.srWindow.Bottom = c.Y;
	SetConsoleCursorPosition(hConsole, defC);
	SetConsoleScreenBufferInfoEx(hConsole, &consolesize);
	int currkey;
	bool needRedraw = true;
	while (1) {
		if(needRedraw)
			drawMenu(c, defC);
		
		currkey = _getch();
		switch (currkey) {
		case 13:
				Menu_EnterCurrentItem();
				needRedraw = true;
			break;
		case 72:
			if (CurrentMenuItem->Previous != &NULL_MENU) {
				needRedraw = true;
				CurrentMenuItem = CurrentMenuItem->Previous;
			}
			else needRedraw = false;
			break;
		case 80:
			if (CurrentMenuItem->Next != &NULL_MENU) {
				needRedraw = true;
				CurrentMenuItem = CurrentMenuItem->Next;
			} else needRedraw = false;
			break;
		case 27:
			{
				
				break;
			}
		}
		
	}
	
}

/*
up 72
down 80
enter 13
esc 27
*/

/*
____________________$$$$$$$$$$
_________________$$$__________$$$
_______________$$________________$$__________$$$
____________$$$____________________$$_______$_$_$
__$$$______$$$$____________$$$$$$$$__$_____$_$__$
_$__$$____$____$$$_$____$$$$$$________$___$__$__$
$___$_$_$$$________$___$$$_____________$_$___$$$
$__$__$$$$$__$$$$__$________$$$$$$______$$_$$$
_$$$$$$$$_$_$$$$_$_$______$$$$$$__$_______$
_________$__$$$$$$$_$$$$__$$$$$$$$$_______$
________$________$$$____$$$$______________$
_______$_______$$___________$$____________$
______$_______$_______________$___________$
______$______$_$$$$_____$$$$$$_____________$
_____$_____$$______$$$$$______$$$__________$
_____$____$________$___$_________$$$________$
_____$___$__$_____$___$_____________$_______$
_____$_______$$$$$$$$$$$$$$$$$$$$$__________$
_____$______________________________________$
______$________$$$$$$$$$$$$$$_______________$
______$____________$$$$$$___________________$$
_______$______________________________________$
___$$$$$$______________________________________$
$$$______$_____________________________________$
__________$$_______________________$$___________$
____________$$$__________________$$_____________$
_______________$$$$$$$$$$$$$$$$$$
*/