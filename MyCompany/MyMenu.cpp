#define MENU_TEXT_SIZE 30
#define MENU_HINT_SIZE 80

typedef struct Menu_Item
{
	struct Menu_Item* Next;
	struct Menu_Item* Previous;
	struct Menu_Item* Parent;
	struct Menu_Item* Child;
	wchar_t Text[MENU_TEXT_SIZE];
	wchar_t Hint[MENU_HINT_SIZE];
	void (*onEnter)(Menu_Item*);
} Menu_Item_t;
#define MENU_ITEM_READ_POINTER(Addr) *(Addr)
#define MENU_ITEM(Name, Next, Previous, Parent, Child, Text, Hint, onEnter)								                \
  extern Menu_Item_t Next;                                                                                              \
  extern Menu_Item_t Previous;                                                                                          \
  extern Menu_Item_t Parent;                                                                                            \
  extern Menu_Item_t Child;                                                                                             \
  Menu_Item Name = {&Next, &Previous, &Parent, &Child, Text, Hint, onEnter}

Menu_Item_t NULL_MENU = { 0 };

#define MENU_PARENT MENU_ITEM_READ_POINTER(&Menu_GetCurrentMenu()->Parent)
#define MENU_CHILD MENU_ITEM_READ_POINTER(&Menu_GetCurrentMenu()->Child)
#define MENU_NEXT MENU_ITEM_READ_POINTER(&Menu_GetCurrentMenu()->Next)
#define MENU_PREVIOUS MENU_ITEM_READ_POINTER(&Menu_GetCurrentMenu()->Previous)

Menu_Item_t* Menu_GetCurrentMenu(void);

void Menu_EnterCurrentItem(void);

static Menu_Item_t* CurrentMenuItem = &NULL_MENU;
static Menu_Item_t* CurrentMenu = &NULL_MENU;

Menu_Item_t* Menu_GetCurrentMenu(void)
{
    return CurrentMenuItem;
}

void Menu_EnterCurrentItem(void)
{
    if ((CurrentMenuItem == &NULL_MENU) || (CurrentMenuItem == NULL))
        return;

    void (*onEnter)(Menu_Item*) = MENU_ITEM_READ_POINTER(&CurrentMenuItem->onEnter);

    if (onEnter)
        onEnter((Menu_Item*)Menu_GetCurrentMenu());
    if (CurrentMenuItem->Child != &NULL_MENU && CurrentMenuItem->Child != NULL) {
        CurrentMenu = CurrentMenuItem->Child;
        CurrentMenuItem = CurrentMenu;
    }
}

int GetLevelItemsCount(Menu_Item_t* menu) {
    Menu_Item_t* item = menu;
    int i = 0;
    do {
        i++;
    } while ((item = item->Next) != &NULL_MENU);
    return i;
}
