#include <Uefi.h>
#include <stdlib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/SimpleTextInEx.h>

typedef enum _STATUS{
    WELCOME, GAMING, PAUSE, WIN, STOP, MENU, GAME_EXIT
}STATUS;
//九种游戏状态，欢迎，游戏中，暂停，结束，终止，菜单，退出游戏
typedef enum _DIRECTION{
    UP, LEFT, RIGHT, DOWN
}DIRECTION;//四个方向——上下左右

typedef struct _SHOW{
    UINT32 x;//坐标
    UINT32 y;//内容
    CHAR16 text[3];
}SHOW;
//显示到屏幕上
typedef struct _SNAKE{
    UINT32 x;
    UINT32 y;
}SNAKE;
SNAKE head, tail;//蛇头与蛇尾
/*********************************** Cursor ************************************/
VOID mySetCursorPos(UINT32 x,UINT32 y);
VOID mySetCursorStart(VOID);
VOID mySetCursorRight(UINT32 n);
VOID mySetCursorLeft(UINT32 n);
VOID mySetCursorUp(UINT32 n);
VOID mySetCursorDown(UINT32 n);
VOID createFood(VOID);
/*********************************** Random ************************************/
UINT32 JR_randomIn(UINT32 max);
/**************Game State***********/
STATUS sys_gs; //游戏状态
UINT32 curtime = 0;
UINT32 snakebody = 0;  //蛇有多长
UINT32 record = 0;  //蛇有多长
UINT32 totalSteps = 0; //目前总步数
UINT32 Direc = 0; //蛇头朝向的反方向，0为上，2为右，1左，3下
/* ************ Main *********** */
#define ROW 23
#define COL 29
UINT32 BuffIndex = 0;
UINT32 isRefresh;
SHOW showBuff[ROW*COL*2];//显示界面
VOID init(VOID);
VOID winGame(VOID);
VOID updateKeys(VOID);
VOID showWelcome(VOID);
VOID showMenu(VOID);
VOID render(VOID);
VOID showGame(VOID);
VOID myClear(VOID);
VOID startMenu(UINT32 index);
CHAR16* myStrcpy(CHAR16 *,const CHAR16 *);
VOID showInBuff(SHOW buffer);
/*********************************** Map ******s******************************/
const CHAR16 *WALL= L"1";//墙
const CHAR16 *BAR= L" ";//空地
const CHAR16 *SN = L"*";//蛇的身子
const CHAR16 *FO = L"#";//食物
const CHAR16 *HE = L"O";//蛇的头
static UINT32 BARS[ROW][COL] = {0};
VOID initUI(VOID);
VOID drawBars(VOID);
VOID drawBar(UINT32 i,UINT32 j);
VOID initSnake(VOID);
UINT32 moveSnake(UINT32 dir);
VOID showTips(VOID);
/* ************ MENU *********** */
UINT32 MainMenuNum = 4; 
UINT32 MenuIndex = 1;
UINT32 SaveMenuNum = 11;
VOID startGame(VOID);
VOID viewRecord(VOID);
VOID resetRecord(VOID);
VOID exitGame(VOID);
INTN
EFIAPI
ShellAppMain (
              IN UINTN Argc,
              IN CHAR16 **Argv
              )
{
    EFI_STATUS Status;
    EFI_SIMPLE_TEXT_OUTPUT_MODE SavedConsoleMode;
    CopyMem(&SavedConsoleMode, gST->ConOut->Mode, sizeof(EFI_SIMPLE_TEXT_OUTPUT_MODE));
    Status = gST->ConOut->EnableCursor(gST->ConOut, FALSE);
    Status = gST->ConOut->ClearScreen(gST->ConOut);
    Status = gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
    Status = gST->ConOut->SetCursorPosition(gST->ConOut, 0, 0);

    init();//Welcome 状态，refresh为1
    while(1){
        updateKeys();
        if(isRefresh){
            render();//第一次以welcome状态render。
        }
        if(sys_gs == GAME_EXIT){
            break;//直到游戏退出
        }
    }
    gST->ConOut->EnableCursor(gST->ConOut, SavedConsoleMode.CursorVisible);
    gST->ConOut->SetCursorPosition(gST->ConOut, SavedConsoleMode.CursorColumn, SavedConsoleMode.CursorRow);
    gST->ConOut->SetAttribute(gST->ConOut, SavedConsoleMode.Attribute);
    gST->ConOut->ClearScreen(gST->ConOut);
    return 0;
}

VOID winGame(VOID){
    record = snakebody;
    sys_gs = WIN;
    isRefresh = 1;
}

VOID init(VOID){
    sys_gs = WELCOME;
    isRefresh = 1;
}

UINT32 ticks = 0;

VOID updateKeys(VOID){ //根据键盘移动光标或蛇位置
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    switch (sys_gs)
    {
        case WELCOME:
            if (Key.ScanCode == SCAN_ESC){
                 exitGame();
            }//如果ESC，退出游戏
            else if(Key.ScanCode == SCAN_F1){
                startMenu(1);
            }//如果按键向上，状态改为Menu,isfresh仍然是1。
            break;
        case MENU:
            if (Key.ScanCode == SCAN_UP)
            {
                MenuIndex --;
                if(MenuIndex <= 0)
                    MenuIndex = MainMenuNum;
                isRefresh = 1;
            }
            else if(Key.ScanCode == SCAN_DOWN){
              MenuIndex ++;
              if(MenuIndex > MainMenuNum)
                    MenuIndex = 1;
              isRefresh = 1;
            }
            else if(Key.ScanCode == SCAN_F1){
                switch(MenuIndex){
                 case 1:    //Start
                    startGame();
                    break;
                 case 2:    //Edit
                    viewRecord ();//查看最高纪录
                    break;
                 case 3:
                    resetRecord();//重置记录
                    break;
                 case 4:    //Exit
                    exitGame();
                    break;
                }
            }
            break;
        case GAMING:
            if (Key.ScanCode == SCAN_ESC){
                if(snakebody>record) record=snakebody;
                    startMenu(1);
            }
            else if(Key.ScanCode == SCAN_F1){
                    sys_gs = PAUSE;
                    isRefresh = 1;
            }
            else if (Key.ScanCode == SCAN_UP)
                {
                    UINT32 result = moveSnake(UP);
                    if(result == 2){
                        winGame();
                        return ;
                    }
                    else if(result != 3) totalSteps ++;
                }
             else if (Key.ScanCode == SCAN_DOWN)
                {
                    UINT32 result = moveSnake(DOWN);
                    if(result == 2){
                        winGame();
                        return ;
                    }
                    else if(result != 3) totalSteps ++;
                } 
             else if (Key.ScanCode == SCAN_LEFT)
                {
                    UINT32 result = moveSnake(LEFT);
                    if(result == 2){
                         winGame();
                         return ;
                    }
                    else if(result != 3) totalSteps ++;
                }
              else if (Key.ScanCode == SCAN_RIGHT)
                {
                    UINT32 result = moveSnake(RIGHT);
                    if(result == 2){
                         winGame();
                         return ;
                    }
                    else if(result != 3) totalSteps ++;
                }
            isRefresh = 1;
            break;
        case PAUSE:
            if(Key.ScanCode == SCAN_F1 || Key.ScanCode == SCAN_ESC){   //Resume
                mySetCursorPos(0, ROW);
                gST->ConOut->OutputString(gST->ConOut,L"\t\t\t\t\t\t\t");
                sys_gs = GAMING;
                isRefresh = 1;
            }
        case STOP:
            if(Key.ScanCode == SCAN_F1){ //reStart
                startGame();
            }
            else if(Key.ScanCode == SCAN_ESC){
                startMenu(1);
            }
            break;
        default:
            break;
    }
}
VOID createFood(VOID){
    UINT32 i=0,j=0;
    i=JR_randomIn(ROW);
    j=JR_randomIn(COL);
    if(BARS[i][j]) createFood();
    else {
        BARS[i][j]=580;
        drawBar(i,j);
    }
}
VOID render(VOID){//游戏状态切换
    isRefresh = 0;
    switch (sys_gs)
    {
        case WELCOME:
            showWelcome();
            mySetCursorPos(0, ROW+1);
            break;
        case MENU:
            showMenu();
            mySetCursorPos(0, ROW+1);
            break;
        case GAMING:
            showGame();
        case WIN:
            showGame();
            mySetCursorPos(0, ROW+1);
            break;
        case PAUSE:
            mySetCursorPos(0, ROW);
            gST->ConOut->OutputString(gST->ConOut,L"Pause! Press Page_Up to Continue.\t\t\t\t");
            mySetCursorPos(0, ROW+1);
            break;
        case STOP:
            mySetCursorPos(0, ROW);
            gST->ConOut->OutputString(gST->ConOut,L"Great! Press Page_Up to Restart or Esc to back to Menu.\t\t");
            mySetCursorPos(0, ROW+1);
            break; 
    }
}
VOID showGame(VOID){
    UINT32 i = 0;
    for(i=0; i<BuffIndex; i++){
        mySetCursorPos(showBuff[i].x, showBuff[i].y);
        if ((showBuff[i].text)[0] == '1')//墙壁
        {
             gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_YELLOW));
        }
        if ((showBuff[i].text)[0] == ' ')//空格
        {
            gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
        }
        if ((showBuff[i].text)[0] == '*')//蛇身
        {
            gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_RED));
        }
        if ((showBuff[i].text)[0] == 'O')//蛇头
        {
            gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_RED));
        }
        if ((showBuff[i].text)[0] == '#')//食物
        {
            gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLUE));
        }
        gST->ConOut->OutputString(gST->ConOut,showBuff[i].text);
    }
    BuffIndex = 0;
    if(sys_gs == WIN){
        sys_gs = STOP;
        isRefresh = 1;
    }
}
VOID myClear(VOID){
    gST->ConOut->SetAttribute(gST->ConOut,0x1);
    gST->ConOut->ClearScreen(gST->ConOut);
}
/***************************************** Map Function ***************************************/

VOID initUI(VOID){
    UINT32 i=0,j=0;
    for(i=0; i<ROW; i++){
        j=0;
        BARS[i][j]=600;
        j=COL-1;
        BARS[i][j]=600;
    }
    for(j=0; j<COL; j++){
        i=0;
        BARS[i][j]=600;
        i=ROW-1;
        BARS[i][j]=600;
    }
    initSnake();//初始化蛇
    drawBars();
}

UINT32 checkSnake(SNAKE node){
    if(BARS[node.x][node.y]==580) return 1; //蛇吃到了食物，变长
    else if(BARS[node.x][node.y]!=0) return 2;//蛇撞墙。
    return 0;//继续走
}

UINT32 moveSnake(UINT32 dir){
    UINT32 result = 0;
    SNAKE node;
    node.x = head.x;
    node.y = head.y;
    if(dir == Direc) return 3;//移动不成功
    else if (dir == 0){
        node.x --;
        Direc = 3;//UP
    }
    else if (dir == 1){
        node.y --;
        Direc = 2;//LEFT
    }
    else if (dir == 2){
        node.y ++;
        Direc = 1;//RIGHT
    }
    else if (dir == 3){
        node.x ++;
        Direc = 0;//DOWN
    }
    //p为移动指针，p指向头结点
    result = checkSnake(node);//0是继续走。1是变长，2是撞墙或者撞自己身子。
    if(result == 1){//蛇变长
        snakebody++;
        createFood();//吃掉旧食物，创造新食物
    }
    else{//遍历
        UINT32 i,j,k;
        for(i=1; i<ROW-1; i++){
            for(j=1; j<COL-1; j++){
                k=BARS[i][j];
                if((k<=snakebody)&&(k>0)) {
                    BARS[i][j]--;
                    drawBar(i,j);
                }
                if(BARS[i][j]==1) {
                    tail.x=i;
                    tail.y=j;
                }
            }
        }
    }
    BARS[node.x][node.y]=snakebody;
    drawBar(node.x,node.y);
    head=node;//换个头就行
    return result;
}
VOID drawBar(UINT32 i,UINT32 j){
    SHOW node;
    node.x = j * 2;
    node.y = i;
    if(BARS[i][j]==600)
    myStrcpy(node.text, WALL);
    else if(BARS[i][j]==0)
    myStrcpy(node.text, BAR);
    else if(BARS[i][j]==580)
    myStrcpy(node.text, FO);
    else if(BARS[i][j]==snakebody)
    myStrcpy(node.text, HE);
    else myStrcpy(node.text, SN);
    showInBuff(node);
}
VOID drawBars(VOID){
    UINT32 i = 0, j = 0;
    SHOW node;
    for(i=0; i<ROW; i++){
        for(j=0; j<COL; j++){
            node.x = j * 2;
            node.y = i;
            if(BARS[i][j]==600)
                myStrcpy(node.text, WALL);
            else if(BARS[i][j]==0)
                myStrcpy(node.text, BAR);
            else if(BARS[i][j]==580)
                myStrcpy(node.text, FO);
            else if(BARS[i][j]==snakebody)
                myStrcpy(node.text, HE);
            else myStrcpy(node.text, SN);
            showInBuff(node);
        }
    }
}
VOID initSnake(VOID){
    UINT32 i=1,j=1;
    for(i=1;i<ROW-1;i++){
           for(j=1; j<COL-1; j++){
           BARS[i][j]=0;
        }
    }
    BARS[3][3]=2;
    BARS[2][3]=1;
    BARS[10][10]=580;
    snakebody = 2;
    head.x = 3;
    head.y = 3;
    tail.x = 2;
    tail.y = 3;
}
VOID startMenu(UINT32 index){
    sys_gs = MENU;
    MenuIndex = index > MainMenuNum ? 1 : index;
    isRefresh = 1;
}
/* SHOW WELCOME */
VOID showWelcome(VOID){
    gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
    gST->ConOut->OutputString(gST->ConOut, L"\n\n\r");
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t********************************** SNAKE ***********************************\n\r");
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           Welcome To Our UEFI Game                       *\n\r");
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           By LiWanting(1652724)                          *\n\r");
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           School Of SoftWare Engineering                 *\n\r");
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           Tongji University                              *\n\r");
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           Press ESC To Exit                              *\n\r");
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*********************************** SNAKE **********************************\n\r");
}
VOID showMenu(VOID){
    myClear();
    mySetCursorStart();
    gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK));
    gST->ConOut->OutputString(gST->ConOut, L"\n\n\r");
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t******************************** MENU **************************************\n\r");
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           Start Game                                     *\n\r");
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           View Record                                    *\n\r");
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           Reset Record                                   *\n\r");
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t*                           Exit                                           *\n\r");
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t******************************** MENU **************************************\n\r");
    mySetCursorPos(26 ,MenuIndex * 2+2);
    gST->ConOut->OutputString(gST->ConOut,L"=>");
}
VOID showInBuff(SHOW buffer){
    showBuff[BuffIndex].x = buffer.x;
    showBuff[BuffIndex].y = buffer.y;
    myStrcpy(showBuff[BuffIndex].text, buffer.text);
    BuffIndex ++;
}
CHAR16* myStrcpy(CHAR16 * strDest,const CHAR16 * strSrc){
    UINT32 i;
    for(i = 0; *(strSrc+i)!= '\0'; i++)
    { *(strDest+i)=*(strSrc+i);}
        *(strDest+i)= '\0';
    return strDest;
}
/********************* Cursor *********************/
VOID mySetCursorPos(UINT32 x,UINT32 y){
    gST->ConOut->SetCursorPosition(gST->ConOut, x, y);
}
VOID mySetCursorStart(VOID){
    mySetCursorPos(0,0);
}
VOID startGame(VOID){
    BuffIndex = 0;
    isRefresh = 1;
    sys_gs = GAMING;
    myClear();
    initUI();
    showTips();
    totalSteps = 0;
    mySetCursorPos(0, ROW);
    gST->ConOut->OutputString(gST->ConOut, L"\t\t\t\t\t\t\t");
}
VOID viewRecord(VOID){
    mySetCursorPos(0, 16);
    UINT32 i = 0;
    for(i=0; i<record; i++){
        mySetCursorPos(10+2*i, 13);
        gST->ConOut->SetAttribute(gST->ConOut, EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_RED));
        if(i==0) gST->ConOut->OutputString(gST->ConOut,L"O");
        else gST->ConOut->OutputString(gST->ConOut,L"*");
    }
}
VOID resetRecord(VOID){
    record = 2;
    mySetCursorPos(0, 16);
    gST->ConOut->OutputString(gST->ConOut,L"Success! Yout past record has been resetted!");
}
VOID exitGame(VOID){
    sys_gs = GAME_EXIT;
}
VOID showTips(VOID){
    UINT32 y = 1;
    if(sys_gs != GAMING) return ;
    mySetCursorPos(COL*2+1, y++);
    mySetCursorPos(COL*2+1, y++);
    y ++;
    mySetCursorPos(COL*2, y++);
    gST->ConOut->OutputString(gST->ConOut, L" UP ");
    mySetCursorPos(COL*2, y++);
    gST->ConOut->OutputString(gST->ConOut, L" LEFT ");
    mySetCursorPos(COL*2, y++);
    gST->ConOut->OutputString(gST->ConOut, L" DOWN ");
    mySetCursorPos(COL*2, y++);
    gST->ConOut->OutputString(gST->ConOut, L" RIGHT ");
    y ++;
    mySetCursorPos(COL*2, y++);
    gST->ConOut->OutputString(gST->ConOut, L" Press F1 To PAUSE");
    mySetCursorPos(COL*2, y++);
    gST->ConOut->OutputString(gST->ConOut, L" Press Esc To Return");
}//操作提示部分
/*********************************** Random ************************************/
#define JR_RANDOM_NUM   100
//static UINT32 * RandomPool;
static UINT32 RandomPool[JR_RANDOM_NUM];
static UINT32 JR_index = JR_RANDOM_NUM;
static UINT32 RandomResult = 0;
//生成随机数
static VOID JR_InitRandom(){
    UINT32 i = 0;
    EFI_TIME  Time;
    if(JR_index != JR_RANDOM_NUM) return ;
    JR_index = 0;
    for(i=0; i<JR_RANDOM_NUM; i++){
        if(RandomResult ==0)
        {
            gRT->GetTime(&Time, NULL);
            RandomResult  = Time.Second;
        }
        RandomResult = (RandomResult<<1) | (((RandomResult&0x80)>>7)^((RandomResult&0x40)>>6));
        RandomPool[i] = RandomResult;
    }
}
UINT32 JR_randomIn(UINT32 max){
    JR_InitRandom();
    return (RandomPool[JR_index++] % (max));
}//取一个不超过max的随机数，测试没有问题
