#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>


typedef enum _STATUS {
	WELCOME, GAMING, PAUSE, WIN, STOP, MENU, EDITMAP, LOADMAP, SAVEMAP, GAME_EXIT
}STATUS;

typedef enum _DIRECTION {
	UP, LEFT, RIGHT, DOWN
}DIRECTION;

typedef struct _SHOWNODE {
	UINT32 x;
	UINT32 y;
	CHAR8 text[3];
}SHOWNODE;

/*********************************** Cursor ************************************/
#define MAXCOL  79  //[0 - 79]
VOID mySetCursorPos(UINT32 x, UINT32 y);

VOID mySetCursorStart(VOID);

VOID mySetCursorRight(UINT32 n);

VOID mySetCursorLeft(UINT32 n);

VOID mySetCursorUp(UINT32 n);

VOID mySetCursorDown(UINT32 n);

/*********************************** Random ************************************/

VOID getRadom(UINT32);

UINT32 JR_random(VOID);

UINT32 JR_randomIn(UINT32 max);

UINT32 JR_randomRange(UINT32 min, UINT32 max);

/**************Game State***********/
STATUS sys_gs;
UINT32 curtime = 0;
UINT32 totalSteps = 0;
/* ************ Main *********** */

#define ROW 23
#define COL 29

UINT32 BuffIndex = 0;
UINT32 isRefresh;

SHOWNODE showBuff[ROW*COL * 2];

VOID init(VOID);

VOID winGame(VOID);

VOID updateKeys(UINT32 index);

VOID showWelcome(VOID);

VOID showMenu(VOID);

VOID render(VOID);

VOID showGame(VOID);

VOID myClear(VOID);

VOID startMenu(UINT32 index);

CHAR8* myStrcpy(CHAR8 *, const CHAR8 *);

VOID showInBuff(SHOWNODE buffer);

/*********************************** Map ************************************/
const CHAR8 *BG = " ";
const CHAR8 *BAR = "1";
const CHAR8 *MAN = "R";
const CHAR8 *EXIT = "E";

//UINT32 BARS[ROW][COL];  
//init random after
static UINT8 BARS[ROW][COL] = {
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0},
		{0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0},
		{0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0},
		{0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0},
		{0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0},
		{0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0},
		{0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0},
		{0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0},
		{0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0},
		{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0},
		{0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0},
		{0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0},
		{0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0},
		{0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0},
		{0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0},
		{0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0},
		{0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0},
		{0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0},
		{0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

UINT32 VALUE[(ROW / 2)*(COL / 2)];
UINT32 FLAG[4];
SHOWNODE CurMan;
SHOWNODE CurEdit;
UINT32 IsEditMap = 0;

VOID EditMaped(VOID);

UINT32 saveMap(UINT32 index);

UINT32 loadMap(UINT32 index);

VOID clearBar(VOID);

VOID initMaze(VOID);

UINT32 initBar(UINT32);

VOID drawBars(VOID);

VOID initMan(VOID);

VOID showMan(VOID);

UINT32 createMap(UINT32);

VOID initEditCursor(VOID);

VOID showEditCursor(VOID);

UINT32 moveMan(UINT32 dir);

UINT32 moveEditCursor(UINT32 dir);

VOID setEditText(VOID);

VOID radomEdit(VOID);

VOID linkPath(UINT32, UINT32);

VOID showTips(VOID);

VOID initVALUE(VOID);

VOID visitPoint(UINT32);

UINT32 checkNeighbour(UINT32, UINT32);

/* ************ MENU *********** */

UINT32 MainMenuNum = 4;
UINT32 MenuIndex = 1;
UINT32 SaveMenuNum = 11;

VOID startGame(VOID);

VOID startEdit(VOID);

VOID startLoad(VOID);

VOID exitGame(VOID);



INTN ShellAppMain(UINTN Argc, CHAR16 **Argv)
{
	EFI_STATUS Status;
	EFI_SIMPLE_TEXT_OUTPUT_MODE SavedConsoleMode;
	// Save the current console cursor position and attributes
	CopyMem(&SavedConsoleMode, gST->ConOut->Mode, sizeof(EFI_SIMPLE_TEXT_OUTPUT_MODE));
	Status = gST->ConOut->EnableCursor(gST->ConOut, FALSE);
	ASSERT_EFI_ERROR(Status);
	Status = gST->ConOut->ClearScreen(gST->ConOut);
	ASSERT_EFI_ERROR(Status);
	Status = gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
	ASSERT_EFI_ERROR(Status);
	Status = gST->ConOut->SetCursorPosition(gST->ConOut, 0, 0);
	ASSERT_EFI_ERROR(Status);

	init();
	while (1) {
		updateKeys(3);
		if (isRefresh) {
			render();
		}
		if (sys_gs == GAME_EXIT) {
			break;
		}
	}
	// Restore the cursor visibility, position, and attributes
	gST->ConOut->EnableCursor(gST->ConOut, SavedConsoleMode.CursorVisible);
	gST->ConOut->SetCursorPosition(gST->ConOut, SavedConsoleMode.CursorColumn, SavedConsoleMode.CursorRow);
	gST->ConOut->SetAttribute(gST->ConOut, SavedConsoleMode.Attribute);
	gST->ConOut->ClearScreen(gST->ConOut);
	return 0;
}

VOID winGame(VOID) {
	sys_gs = WIN;
	isRefresh = 1;
}

VOID init(VOID) {
	sys_gs = WELCOME;
	isRefresh = 1;
}

UINT32 ticks = 0;

VOID updateKeys(UINT32 ms) {

	EFI_STATUS Status;
	EFI_INPUT_KEY Key;

	Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
	ASSERT_EFI_ERROR(Status);
	if (EFI_ERROR(Status))
		return;

	switch (sys_gs)
	{
	case WELCOME:
		if (Key.ScanCode == SCAN_ESC) {
			exitGame();

		}
		else if (Key.ScanCode == SCAN_PAGE_UP) {
			startMenu(1);
		}
		break;
	case MENU:
		if (Key.ScanCode == SCAN_UP)
		{
			MenuIndex--;
			if (MenuIndex <= 0)
				MenuIndex = MainMenuNum;
			isRefresh = 1;
		}
		else if (Key.ScanCode == SCAN_DOWN) {
			MenuIndex++;
			if (MenuIndex > MainMenuNum)
				MenuIndex = 1;
			isRefresh = 1;
		}
		else if (Key.ScanCode == SCAN_PAGE_UP) {
			switch (MenuIndex) {
			case 1:    //Start
				startGame();
				break;
			case 2:    //Edit
			   //startEdit();
				break;
			case 3:    //Load
			   //startLoad();
				break;
			case 4:    //Exit
				exitGame();
				break;




			}
		}
		break;
	case GAMING:
		if (Key.ScanCode == SCAN_ESC) {
			startMenu(1);

		}
		else if (Key.ScanCode == SCAN_PAGE_UP) {
			sys_gs = PAUSE;
			isRefresh = 1;
		}
		else if (Key.ScanCode == SCAN_UP)
		{
			int result = moveMan(UP);
			if (result == 99) {
				winGame();
				return;
			}
			else if (result) {
				totalSteps++;
				isRefresh = 1;
			}
		}
		else if (Key.ScanCode == SCAN_DOWN)
		{
			int result = moveMan(DOWN);
			if (result == 99) {
				winGame();
				return;
			}
			else if (result) {
				totalSteps++;
				isRefresh = 1;
			}
		}
		else if (Key.ScanCode == SCAN_LEFT)
		{
			int result = moveMan(LEFT);
			if (result == 99) {
				winGame();
				return;
			}
			else if (result) {
				totalSteps++;
				isRefresh = 1;
			}
		}
		else if (Key.ScanCode == SCAN_RIGHT)
		{
			int result = moveMan(RIGHT);
			if (result == 99) {
				winGame();
				return;
			}
			else if (result) {
				totalSteps++;
				isRefresh = 1;
			}
		}
		break;
	case PAUSE:
		if (Key.ScanCode == SCAN_PAGE_UP || Key.ScanCode == SCAN_ESC) {   //Resume
			mySetCursorPos(0, ROW);
			gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t\t\t\t");
			sys_gs = GAMING;
			isRefresh = 1;
		}
	case STOP:
		if (Key.ScanCode == SCAN_PAGE_UP) { //reStart
			startGame();
		}
		else if (Key.ScanCode == SCAN_ESC) {
			startMenu(1);
		}
		break;
	default:
		break;
	}
}

VOID render(VOID) {
	isRefresh = 0;
	switch (sys_gs)
	{
	case WELCOME:
		showWelcome();
		mySetCursorPos(0, ROW + 1);
		break;
	case MENU:
		showMenu();
		mySetCursorPos(0, ROW + 1);
		break;
	case GAMING:
	case WIN:
		showGame();
		mySetCursorPos(0, ROW + 1);
		break;
	case PAUSE:
		mySetCursorPos(0, ROW);
		gST->ConOut->OutputString(gST->ConOut, L"Pause! Press Page_Up to Continue.\t\t\t\t");
		mySetCursorPos(0, ROW + 1);
		break;
	case STOP:
		mySetCursorPos(0, ROW);
		gST->ConOut->OutputString(gST->ConOut, L"You Win! Press Page_Up to Restart or Esc to back to Menu.\t\t");
		mySetCursorPos(0, ROW + 1);
		break;
	}
}

VOID showGame(VOID) {
	UINT32 i = 0;
	for (i = 0; i < BuffIndex; i++) {
		mySetCursorPos(showBuff[i].x, showBuff[i].y);
		if ((showBuff[i].text)[0] == '1')
		{
			/* code */
			gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_YELLOW));
		}if ((showBuff[i].text)[0] == ' ')
		{
			gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
		}if ((showBuff[i].text)[0] == 'E')
		{
			gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLUE));
		}if ((showBuff[i].text)[0] == 'R')
		{
			gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_RED));
		}
		gST->ConOut->OutputString(gST->ConOut, showBuff[i].text);
	}

	BuffIndex = 0;

	if (sys_gs == WIN) {
		sys_gs = STOP;
		isRefresh = 1;
	}
}

VOID myClear(VOID) {
	gST->ConOut->SetAttribute(gST->ConOut, 0x1);
	gST->ConOut->ClearScreen(gST->ConOut);
}

/***************************************** Map Function ***************************************/

VOID initMaze(VOID) {
	//initBar(1);
	drawBars();

	initMan();
	showMan();
}

UINT32 checkMan(SHOWNODE node) {
	if (node.x == COL * 2 - 4 && node.y == ROW - 2) {
		return 99;
	}
	else if (node.x < 0 || node.x >= COL * 2
		|| node.y < 0 || node.y >= ROW
		|| !BARS[node.y][node.x / 2]
		) return 0;
	return 1;
}

UINT32 moveMan(UINT32 dir) {
	UINT32 result = 0;
	SHOWNODE node;
	node.x = CurMan.x;
	node.y = CurMan.y;
	myStrcpy(node.text, MAN);
	switch (dir) {
	case 0:   //UP
		node.y--;
		break;
	case 1:   //LEFT
		node.x -= 2;
		break;
	case 2:   //RIGHT
		node.x += 2;
		break;
	case 3:   //DOWN
		node.y++;
		break;
	}
	result = checkMan(node);
	if (result) {
		myStrcpy(CurMan.text, BG);
		showInBuff(CurMan);
		CurMan.x = node.x;
		CurMan.y = node.y;
		myStrcpy(CurMan.text, node.text);
		showInBuff(CurMan);
		return result;
	}
	return 0;
}

UINT32 initBar(UINT32 flag) {
	UINT32 i = 0, j = 0;
	if (IsEditMap) {
		IsEditMap = 0;
		return 1;
	}

	for (i = 0; i < ROW; i++) {
		for (j = 0; j < COL; j++) {
			BARS[i][j] = 0;
		}
	}
	return createMap(flag);
}

VOID drawBars(VOID) {

	UINT32 i = 0, j = 0;
	SHOWNODE node;
	for (i = 0; i < ROW; i++) {
		for (j = 0; j < COL; j++) {
			node.x = j * 2;
			node.y = i;
			if (BARS[i][j])
				myStrcpy(node.text, BG);
			else
				myStrcpy(node.text, BAR);
			showInBuff(node);
		}
	}
	node.x = COL * 2 - 4;
	node.y = ROW - 2;
	myStrcpy(node.text, EXIT);
	showInBuff(node);
}



VOID initMan(VOID) {
	CurMan.x = 2;
	CurMan.y = 1;
	myStrcpy(CurMan.text, MAN);
}

VOID showMan(VOID) {
	showInBuff(CurMan);
}

UINT32 createMap(UINT32 flag) {

	switch (flag) {
	case 1:
		initVALUE();
		//visitPoint(0);
		break;
	default:
		break;
	}
	return 1;
}

VOID initVALUE(VOID) {
	UINT32 i = 0;
	for (i = 0; i < (ROW / 2)*(COL / 2); i++) {
		VALUE[i] = 0;
	}
}

VOID visitPoint(UINT32 a) {
	UINT32 i = 0, node = 0;
	UINT32 x = 0, y = 0;
	if (VALUE[a]) return;

	x = (a % (COL / 2)) * 2 + 1;
	y = (a / (COL / 2)) * 2 + 1;
	BARS[y][x] = 1;
	VALUE[a] = 2;
	//getRadom(5);
	getRadom(JR_randomRange(1, 10));
	for (i = 0; i < 4; i++) {
		if ((node = checkNeighbour(a, FLAG[i])) != 0) {
			linkPath(a, node);
			visitPoint(node);
		}
	}
}

VOID linkPath(UINT32 a, UINT32 b) {
	UINT32 x1 = (a % (COL / 2)) * 2 + 1;
	UINT32 x2 = (b % (COL / 2)) * 2 + 1;
	UINT32 y1 = (a / (COL / 2)) * 2 + 1;
	UINT32 y2 = (b / (COL / 2)) * 2 + 1;
	UINT32 x = (x1 + x2) / 2;
	UINT32 y = (y1 + y2) / 2;
	BARS[y][x] = 1;
}

UINT32 checkNeighbour(UINT32 a, UINT32 dir) {
	UINT32 col = COL / 2;
	UINT32 row = ROW / 2;
	UINT32 index = 0;

	switch (dir) {
	case 0:
		index = a - col;
		if (index > 0 && !VALUE[index]) return (index);
		break;
	case 1:
		index = a - 1;
		if ((a % col) != 0 && !VALUE[index]) return (index);
		break;
	case 2:
		index = a + 1;
		if ((index % col) != 0 && !VALUE[index]) return (index);
		break;
	case 3:
		index = a + col;
		if (index < (row * col) && !VALUE[index]) return (index);
		break;
	}
	return 0;
}

VOID getRadom(UINT32 time) {
	UINT32 i = 0, j = 0, k = 0;
	UINT32 temp = 0;
	for (i = 0; i < 4; i++)
		FLAG[i] = i;
	for (i = 0; i < time; i++) {
		for (j = 0; j < 4; j++) {
			k = JR_randomIn(3);
			if (k != j) {
				temp = FLAG[j];
				FLAG[j] = FLAG[k];
				FLAG[k] = temp;
			}
		}
	}
}



VOID startMenu(UINT32 index) {
	sys_gs = MENU;
	MenuIndex = index > MainMenuNum ? 1 : index;
	isRefresh = 1;
}

/* SHOW WELCOME */
VOID showWelcome(VOID) {

	gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
	gST->ConOut->OutputString(gST->ConOut, L"\n\n\r");
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t********************************** Maze ************************************\n\r");
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           Welcome to uefi Gameing.                       *\n\r");
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           By Liuhaodong 2015-1                           *\n\r");
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           liute62@sina.cn                                *\n\r");
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           PgUP Menu                                      *\n\r");
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           Esc Exit                                       *\n\r");
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*********************************** Maze ***********************************\n\r");
}

VOID showMenu(VOID) {
	myClear();
	mySetCursorStart();
	gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
	gST->ConOut->OutputString(gST->ConOut, L"\n\n\r");
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t********************************** MENU ************************************\n\r");
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           Start Game                                     *\n\r");
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           Edit Map.                                      *\n\r");
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           Load Map.                                      *\n\r");
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           Exit.                                          *\n\r");
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t********************************** MENU ************************************\n\r");
	mySetCursorPos(26, MenuIndex * 2 + 2);
	gST->ConOut->OutputString(gST->ConOut, L"=>");
}

VOID showInBuff(SHOWNODE buffer) {
	showBuff[BuffIndex].x = buffer.x;
	showBuff[BuffIndex].y = buffer.y;
	myStrcpy(showBuff[BuffIndex].text, buffer.text);
	BuffIndex++;
}

CHAR8* myStrcpy(CHAR8 * strDest, const CHAR8 * strSrc) {
	UINT32 i;
	CHAR8 *address = strDest;
	for (i = 0; strSrc[i] != '\0'; i++)
		strDest[i] = strSrc[i];
	strDest[i] = '\0';
	return address;
}
/********************* Cursor *********************/

VOID mySetCursorPos(UINT32 x, UINT32 y) {
	gST->ConOut->SetCursorPosition(gST->ConOut, x, y);
}

VOID mySetCursorStart(VOID) {
	mySetCursorPos(0, 0);
}

VOID startGame(VOID) {

	BuffIndex = 0;
	isRefresh = 1;
	sys_gs = GAMING;

	myClear();
	initMaze();
	showTips();

	totalSteps = 0;
	mySetCursorPos(0, ROW);
	gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t\t\t\t");

}

VOID startEdit(VOID) {

}

VOID startLoad(VOID) {

}

VOID exitGame(VOID) {
	sys_gs = GAME_EXIT;
}

VOID showTips(VOID) {
	UINT32 y = 1;
	if (sys_gs != GAMING) return;
	mySetCursorPos(COL * 2 + 1, y++);

	mySetCursorPos(COL * 2 + 1, y++);
	/**CHAR8 stepResult[3];
	stepResult[0] = 'a';
	stepResult[1] = 'b';
	stepResult[2] = 'c';
	gST->ConOut->OutputString(gST->ConOut,L"Steps:  ");
	gST->ConOut->OutputString(gST->ConOut,stepResult);**/
	y++;
	mySetCursorPos(COL * 2, y++);
	gST->ConOut->OutputString(gST->ConOut, L" Up ");
	mySetCursorPos(COL * 2, y++);
	gST->ConOut->OutputString(gST->ConOut, L" Left ");
	mySetCursorPos(COL * 2, y++);
	gST->ConOut->OutputString(gST->ConOut, L" Down ");
	mySetCursorPos(COL * 2, y++);
	gST->ConOut->OutputString(gST->ConOut, L" Right ");
	y++;
	mySetCursorPos(COL * 2, y++);
	gST->ConOut->OutputString(gST->ConOut, L" Save Map");
	mySetCursorPos(COL * 2, y++);
	gST->ConOut->OutputString(gST->ConOut, L" Esc : Back");
}


/*********************************** Random ************************************/
#define JR_RANDOM_NUM   100

//static UINT32 * RandomPool;
static UINT32 RandomPool[JR_RANDOM_NUM];
static UINT32 JR_index = JR_RANDOM_NUM;
static UINT8 RandomResult = 0;

static VOID JR_InitRandom() {
	UINT32 i = 0;
	EFI_TIME  Time;
	if (JR_index != JR_RANDOM_NUM) return;

	JR_index = 0;
	// memset(RandomPool, 0, sizeof(RandomPool));
	 //srand(time(0) + rand());
	 //RandomPool = AllocatePool(sizeof(UINT32));
	 //ZeroMem(RandomPool, sizeof(UINT32));

	for (i = 0; i < JR_RANDOM_NUM; i++) {
		if (RandomResult == 0)
		{
			gRT->GetTime(&Time, NULL);
			RandomResult = Time.Second;
		}
		RandomResult = (RandomResult << 1) | (((RandomResult & 0x80) >> 7) ^ ((RandomResult & 0x40) >> 6));
		RandomPool[i] = RandomResult;
	}
}

UINT32 JR_random(VOID) {
	JR_InitRandom();
	return (RandomPool[JR_index++]);
}

UINT32 JR_randomIn(UINT32 max) {
	JR_InitRandom();
	return (RandomPool[JR_index++] % (max + 1));
}

UINT32 JR_randomRange(UINT32 min, UINT32 max) {
	JR_InitRandom();
	return (min + (RandomPool[JR_index++] % (max - min + 1)));
}
