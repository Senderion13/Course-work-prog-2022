#include <iostream>
#include "menuitems.cpp"
#include "MyODBC.cpp"

#define WINDOW_HEIGHT 40
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

bool checkDecimal(string ID) {
	bool repeat;
	if (ID == "Q" || ID == "q") return true;
	do {
		repeat = false;
		try {
			stoi(ID);
		}
		catch (exception) {
			repeat = true;
			wprintf(L"Wrong data! Please, try again\n\nEnter device id: ");
			std::getline(cin, ID);
			if (ID == "Q" || ID == "q") return true;
		}
	} while (repeat);
}

void checkDate(wchar_t* date) {
	bool repeat = false;
	do {
		repeat = false;
		if (wcslen(date) != 10) {
			wprintf(L"Wrong date!\ncode: symbols must be 10\nPlease, try again\n\nEnter supply date: ");
			wcin.getline(date, 10);
			repeat = true;
			continue;
		}
		for (int i = 0; i < 10; i++)
		{
			if (i == 4 || i == 7) {
				if (date[i] != 45) {
					wprintf(L"Wrong date!\ncode: use \"-\" like date separator\nPlease, try again\n\nEnter supply date: ");
					wcin.getline(date, 10);
					repeat = true;
					continue;
				}
			}
			else {
				if (date[i] < 48 || date[i] > 57) {
					wprintf(L"Wrong date!\ncode: wrong digits\nPlease, try again\n\nEnter supply date: ");
					wcin.getline(date, 10);
					repeat = true;
					continue;
				}
			}
		}
	} while (repeat);
}

void clearArea(int startX, int startY, int count) {
	SetConsoleTextAttribute(hConsole, 0);
	char blank[MENU_TEXT_SIZE + 3];
	for (int i = 0; i < MENU_TEXT_SIZE + 2; i++)
	{
		blank[i] = ' ';
	}
	blank[MENU_TEXT_SIZE + 2] = 0;
	for (int i = 0; i < count+1; i++)
	{
		SetConsoleCursorPosition(hConsole, { (SHORT)startX, (SHORT)(startY + i)});
		cout << blank;
	}
}

void drawMenu(COORD c, COORD defC) {
	if (GetLevelItemsCount(CurrentMenu) < lastPosition.count) {
		clearArea((WINDOW_WIDTH - MENU_TEXT_SIZE) / 2 - 1, lastPosition.pos, lastPosition.count);
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
	
	clearArea(0, WINDOW_HEIGHT - 1, 0);
	SetConsoleCursorPosition(hConsole, { 0, WINDOW_HEIGHT - 1});
	SetConsoleTextAttribute(hConsole, 240);
	wprintf(CurrentMenuItem->Hint);
	SetConsoleTextAttribute(hConsole, 15);
}

void exitProgram(Menu_Item_t* current_item) {
	SetConsoleTextAttribute(hConsole, 15);
	exit(0);
}

void addRecord(Menu_Item_t* current_item) {
	string typeID, userID, price, lifeTime;
	wchar_t supplyDate[11];
	wchar_t description[100];
	system("cls");
	drawDeviceTypes(current_item);
	wprintf(L"Enter Q to quit\n\nEnter type id: ");
	getline(cin, typeID);
	if (typeID == "Q" || typeID == "q") {
	}
	else {
		checkDecimal(typeID);
		drawUsers(current_item);
		wprintf(L"Enter user id: ");
		getline(cin, userID);
		checkDecimal(userID);
		wprintf(L"Enter price: ");
		getline(cin, price);
		checkDecimal(price);
		wprintf(L"Enter life time: ");
		getline(cin, lifeTime);
		checkDecimal(lifeTime);
		wprintf(L"Enter supply date(YYYY-MM-DD): ");
		wcin >> supplyDate;
		checkDate(supplyDate);
		wprintf(L"Enter description: ");
		wcin.getline(description, 100);
		wcin.getline(description, 100);
		wchar_t sqlRequest[SQL_QUERY_SIZE] = L"";
		swprintf(sqlRequest, SQL_QUERY_SIZE, L"INSERT INTO devices ( userID, typeID, price, supplyDate, lifeTime, description) VALUES (%d,%d,%d,\'%ls\',%d,\'%ls\')", stoi(userID), stoi(typeID), stoi(price), supplyDate, stoi(lifeTime), description);
		getTable(sqlRequest, false);
	}
}

void updateRecord(Menu_Item_t* current_item) {
	string ID, typeID, userID, price, lifeTime;
	wchar_t supplyDate[11];
	wchar_t description[100];
	system("cls");
	wprintf(L"Enter Q to quit\n\nEnter device id: ");
	getline(cin, ID);
	if (ID == "Q" || ID == "q") {

	}
	else {
		checkDecimal(ID);
		drawDeviceTypes(current_item);
		wprintf(L"Enter new type id: ");
		getline(cin, typeID);
		checkDecimal(typeID);
		drawUsers(current_item);
		wprintf(L"Enter new user id: ");
		getline(cin, userID);
		checkDecimal(ID);
		wprintf(L"Enter new price: ");
		getline(cin, price);
		checkDecimal(ID);
		wprintf(L"Enter new life time: ");
		getline(cin, lifeTime);
		checkDecimal(ID);
		wprintf(L"Enter new supply date(YYYY-MM-DD): ");
		wcin >> supplyDate;
		checkDate(supplyDate);
		wprintf(L"Enter new description: ");
		wcin.getline(description, 100);
		wcin.getline(description, 100);
		wchar_t sqlRequest[SQL_QUERY_SIZE] = L"";
		swprintf(sqlRequest, SQL_QUERY_SIZE, L"UPDATE devices SET userID=%d, typeID=%d, price=%d, supplyDate=\'%ls\', lifeTime=%d, description=\'%ls\' WHERE ID=%d", stoi(userID), stoi(typeID), stoi(price), supplyDate, stoi(lifeTime), description, stoi(ID));
		getTable(sqlRequest, false);
	}
}

void deleteRecord(Menu_Item_t* current_item) {
	SQLWCHAR sqlRequestPrint[SQL_QUERY_SIZE] = L"SELECT devices.id, users.name AS user, devicetypes.name AS `device type`, Devices.price, Devices.supplyDate, Devices.lifeTime, Devices.description FROM Devices INNER JOIN users ON devices.userID=users.id INNER JOIN devicetypes ON devices.typeID=devicetypes.id";
	getTable(sqlRequestPrint, false);
	string ID = "";
	wprintf(L"Enter Q to quit\n\nEnter device id: ");
	cin >> ID;
	cin.get();
		if (checkDecimal(ID)) {}
		else {
			wchar_t sqlRequestDelete[SQL_QUERY_SIZE] = L"";
			swprintf(sqlRequestDelete, SQL_QUERY_SIZE, L"DELETE FROM devices WHERE ID = %d", stoi(ID));
			if (getTable(sqlRequestDelete, false) == 0) {
				wprintf(L"Deleting success\nPress ESC to quit");
				wait();
			}
		}
}

void drawDevices(Menu_Item_t* current_item) {
	SQLWCHAR sqlRequest[SQL_QUERY_SIZE] = L"SELECT devices.id, users.name AS user, devicetypes.name AS `device type`, Devices.price, Devices.supplyDate, Devices.lifeTime, Devices.description FROM Devices INNER JOIN users ON devices.userID=users.id INNER JOIN devicetypes ON devices.typeID=devicetypes.id";
	int currkey, extkey;
	bool exit = false;
	system("cls");
	getTable(sqlRequest, false);
	SetConsoleCursorPosition(hConsole, { 0, WINDOW_HEIGHT - 1 });
	SetConsoleTextAttribute(hConsole, 240);
	wprintf(L" F4 - add new record | F5 - update existed record (by id) | F6 - delete record (by id) ");
	SetConsoleTextAttribute(hConsole, 15);
	SetConsoleCursorPosition(hConsole, { 0, 0 });
	while (!exit) {
		currkey = _getch();
		switch (currkey) {
			case 0:
			{
				extkey = _getch();
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
				exit = true;
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