// HostTest.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include "F4Packet_generated.h"
#include "HostTest.h"
#include "FtpClient.h"
#include "DataManagerShotSolution.h"
#include "DataManagerBalance.h"

#define MAX_LOADSTRING 100
#define ENET_ADDRESS_INFO_SIZE 48
#define FTPSERVER_IP "192.168.0.19"
#define FTPSERVER_PORT 21
#define FTPSERVER_IP_FOR_CHINA "39.118.204.14"
#define FTPSERVER_PORT_FOR_CHINA 32121

#define FTPID  "anonymous"
#define FTPPW "1234"
#define DESIRED_WINSOCK_VERSION        0x0101
#define MINIMUM_WINSOCK_VERSION        0x0001

#ifdef _WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#include <stdlib.h>



// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
HWND hWnd;

int option = HOST_OPTION_NO_CHECK_REPLAY | HOST_OPTION_NO_CHECK_GOAL | HOST_OPTION_NO_CHECK_TIME | HOST_OPTION_NO_CHECK_SHOTCLOCK | HOST_TIME_MINUTE_3;
int userID = 100;
ENetHost* pEnetHost = nullptr;
ENetHost* pEnetClient = nullptr;
ENetPeer* peer = 0;
CHost* pHost = nullptr;
std::vector<string> messages;
std::map<int, ENetPeer*> UserList;
char buff[100];
int drawTextStartY = 0;
int drawTextUnit = 20;
bool visualDebug = false;
int lastCreatedOption = 0;

string localTestIP = FTPSERVER_IP;
int localTestPORT = FTPSERVER_PORT;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
string              GetInIpAddress();
void                ServiceHost();
void                ServiceClient();
void                LogMessage(const char* msg);

void                AddMessage(string str);
void                SendPacket(HostMessage* pMessage);
CDataManagerBalance*        gBalanceTable = NULL;
CDataManagerShotSolution*   gShotSolution = NULL;
CAnimationController*       gAnimationController = NULL;
bool                UpdateBalanceData();
bool                UpdateShotSolution();
void                LoadData();
void                CreateHost(int option);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    //_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    //_CrtSetBreakAlloc(22623);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_HOSTTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return false;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HOSTTEST));

    MSG msg;
    memset(&msg, 0, sizeof(msg));

    //HINSTANCE    hInstHost;
    //HINSTANCE    hInstEnet;

    //hInstHost = LoadLibrary("host.dll");
    //hInstEnet = LoadLibrary(L"enet.dll");

    AddMessage("Host Test Client");
    
    AddMessage("IP address is " + GetInIpAddress());

    ZeroMemory(buff, sizeof(char) * 100);

    enet_initialize();

    bool done = false;

    DHOST_TYPE_UINT64 dwTime = GetTickCount64();
    DHOST_TYPE_UINT64 dwEverySecondTime = GetTickCount64();

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            DHOST_TYPE_UINT64 dwTimeCurrent = GetTickCount64();

            if (dwTime + 30 < dwTimeCurrent)
            {
                //CreateHost(HOST_OPTION_GAMEMODE_NORMAL | option);
                float timeElapsed = (dwTimeCurrent - dwTime) / 1000.0f;

                
                if (GetAsyncKeyState('K') & 0x0001)
                {
                    SGiveGiftInfo giftInfo;
                    giftInfo.UserID = 100;

                    SAddAbility ability;
                    ability.AbilityName = "GiftBuff_JumpShotValue";
                    ability.AddValue = 99;
                    ability.Times = 10;
                    giftInfo.Attrs.insert(std::make_pair("GiftBuff_JumpShotValue", ability));


                    AddMessage("K key pressed!");

                    //GiftBuff_JumpShotProbability

                    pHost->OnGiveGift(&giftInfo);

                    // 원하는 동작을 여기에 넣으면 됨
                }

                if (GetAsyncKeyState('L') & 0x0001)
                {
                    SGiveGiftInfo giftInfo;
                    giftInfo.UserID = 100;

                    SAddAbility ability;
                    ability.AbilityName = "GiftBuff_NicePassValue";
                    ability.AddValue = 99;
                    ability.Times = 10;
                    giftInfo.Attrs.insert(std::make_pair("GiftBuff_NicePassValue", ability));

                    AddMessage("L key pressed!");
                    pHost->OnGiveGift(&giftInfo);
                }
                

                dwTime = dwTimeCurrent;

                if (pHost != nullptr)
                {
                    pHost->Update(timeElapsed);

                    if (dwEverySecondTime + 1000 < dwTimeCurrent)
                    {
                        dwEverySecondTime = dwTimeCurrent;

                        pHost->DevUpdateEverySecond();
                    }
                    //HostUpdate(pHost, timeElapsed);

                    if (visualDebug == true)
                    {
                        RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
                    }
                }

                ServiceHost();

                ServiceClient();
            }
        }
    }

    enet_deinitialize();

    if (pHost != nullptr)
    {
        pHost->Release();
        SAFE_DELETE(pHost);
        //HostDelete(pHost);
    }
    messages.clear();
    //_CrtDumpMemoryLeaks();

    return (int) msg.wParam;
}


void InitEnet()
{
    enet_host_destroy(pEnetHost);

    enet_initialize();

    ENetAddress address;
    //address.host = ENET_HOST_ANY;
    address.port = 2020;
    address.sin6_scope_id = 0;
    
    enet_address_set_host(&address, GetInIpAddress().c_str());

    pEnetHost = enet_host_create(&address, 16, 2, 0, 0);
}

void LoadOption()
{
    char buff[FILENAME_MAX]; //create string buffer to hold path
    std::string currentpath(GetCurrentDir(buff, sizeof(buff)));
    string iniPath = currentpath + "\\option.ini";

    ifstream inFile;
    inFile.open(iniPath);

    if (inFile)
    {
        string str = "";
        getline(inFile, str);
        option = atoi(str.c_str());
    }

    inFile.close();
}

void SaveOption()
{
    char buff[FILENAME_MAX]; //create string buffer to hold path
    std::string currentpath(GetCurrentDir(buff, sizeof(buff)));
    string iniPath = currentpath + "\\option.ini";

    ofstream outFile;
    outFile.open(iniPath);

    char save[256];

    ZeroMemory(save, sizeof(char) * 10);

    int saveOption = option & ~(HOST_OPTION_GAMEMODE);

    _itoa(saveOption, save, 10);

    outFile.write(save, 10);

    outFile.close();
}

void HostTestInit()
{
    
    if (false == UserList.empty())
    {
        for (auto& it : UserList)
        {
            auto peer = it.second;

            it.second = nullptr;
        }

        UserList.clear();
    }
    if (pHost)
    {
        pHost->DevPlayPacketSave(false);
        //HostDevPlayPacketSave(pHost, false);
        pHost->Release();
    }
    SAFE_DELETE(pHost);
    //HostDelete(pHost);
    //InitEnet();
    
}



void CreateHost(int option)
{
    LoadData();
    lastCreatedOption = option;
    
    if (!pEnetHost)
    {
        ENetAddress address;
        //address.host = ENET_HOST_ANY;
        address.port = 2020;
        address.sin6_scope_id = 0;

        enet_address_set_host(&address, GetInIpAddress().c_str());

        pEnetHost = enet_host_create(&address, 16, 2, 0, 0);
    }
    else
    {
        HostTestInit();
    }

    int roomid = RANDOM_INT(1000);

    HostMessageCallback callback = SendPacket;
    LogCallback callbackLog = LogMessage;
    
    pHost = new CHost((char*)std::to_string(roomid).c_str(), option | HOST_OPTION_NO_REDIS);

    if (pHost != nullptr)
    {
        pHost->RegistCallbackFuncLog(callbackLog);
    }

    if (pHost != nullptr)
    {
        pHost->RegistCallbackFuncSendPacket(callback);
    }

    pHost->m_pBalance = gBalanceTable;
    pHost->m_ShotSolution = gShotSolution;
    pHost->m_AnimationController = gAnimationController;
    pHost->Initialize();
    pHost->SetIsNormalGame(option & HOST_OPTION_CHOOSE_NORMAL_BETWEEN_NORMAL_AND_RANK);
    //pHost = HostCreateW(name, option | HOST_OPTION_NO_REDIS, gBalanceTable);
    if (!gBalanceTable->HasData())
    {
        UpdateBalanceData();
    }
    if (!gShotSolution->HasData())
    {
        UpdateBalanceData();
    }
    
    UserList.clear();
	
	int stageIndex = 0;
	switch (option & HOST_OPTION_STAGE)
	{
		case HOST_OPTION_STAGE_RANDOM:
		{
			stageIndex = rand() % 4;
		}
		break;
		case HOST_OPTION_STAGE_0:
		{
			stageIndex = 0;
		}
		break;
		case HOST_OPTION_STAGE_1:
		{
			stageIndex = 1;
		}
		break;
		case HOST_OPTION_STAGE_2:
		{
			stageIndex = 2;
		}
		break;
		case HOST_OPTION_STAGE_3:
		{
			stageIndex = 3;
		}
		break;
		case HOST_OPTION_STAGE_4:
		{
			stageIndex = 4;
		}
		break;
		case HOST_OPTION_STAGE_5:
		{
			stageIndex = 5;
		}
		break;
        case HOST_OPTION_STAGE_6:
        {
            stageIndex = 6;
        }
        break;
		case HOST_OPTION_STAGE_100:
		{
			stageIndex = 100;
		}
		break;
	}

    int sceneIndex = 0;
    switch (option & HOST_OPTION_SCENE)
    {
        case HOST_OPTION_SCENE_RANDOM:
        {
            sceneIndex = kDEFAULT_SCENE_INDEX + rand() % HOST_OPTION_SCENE_RANDOM_COUNT;
        }
        break;
        case HOST_OPTION_SCENE_1000:
        {
            sceneIndex = kDEFAULT_SCENE_INDEX;
        }
        break;
        case HOST_OPTION_SCENE_1001:
        {
            sceneIndex = kDEFAULT_SCENE_INDEX + 1;
        }
        break;
        case HOST_OPTION_SCENE_1002:
        {
            sceneIndex = kDEFAULT_SCENE_INDEX + 2;
        }
        break;
    }

    if (option & HOST_OPTION_NO_KICK)
    {
        pHost->SetDevNoKick(true);
    }

    pHost->DevStageSetting(stageIndex);
    pHost->DevSceneSetting(sceneIndex);
}

bool UpdateAnimationData()
{
    CFtpClient clsFtp;
    if (clsFtp.Connect(localTestIP.c_str(), localTestPORT, true))
    {
        int a = 0;
    }
    clsFtp.Login(FTPID, FTPPW);
    char buff[FILENAME_MAX]; //create string buffer to hold path
    GetCurrentDir(buff, sizeof(buff));
    std::string currentpath(buff);
    currentpath.append("/game/cservice/Resource");
    currentpath.append("/AnimData.bin");
    const char* pszRemoteFolder = "/";
    const char* pszRemoteFile = "AnimData.bin";
    const char* pszLocalPath = currentpath.c_str();

    // FTP 서버 폴더를 변경한다.

    if (clsFtp.ChangeFolder(pszRemoteFolder) == false)
    {
        printf("clsFtp.ChangeFolder(%s) error\n", pszRemoteFolder);
        return false;
    }
    if (clsFtp.Download(pszRemoteFile, pszLocalPath) == false)
    {
        printf("clsFtp.Download(%s) error\n", pszRemoteFile);
        return false;
    }
    return true;
}

bool UpdateBalanceData()
{
    CFtpClient clsFtp;
    if (clsFtp.Connect(localTestIP.c_str(), localTestPORT, true))
    {
        int a = 0;
    }
    clsFtp.Login(FTPID, FTPPW);
    char buff[FILENAME_MAX]; //create string buffer to hold path
    GetCurrentDir(buff, sizeof(buff));
    std::string currentpath(buff);
    currentpath.append("/game/cservice/Resource");
    currentpath.append("/BalanceData.bin");
    const char* pszRemoteFolder = "/";
    const char* pszRemoteFile = "BalanceData.bin";
    const char* pszLocalPath = currentpath.c_str();

    // FTP 서버 폴더를 변경한다.

    if (clsFtp.ChangeFolder(pszRemoteFolder) == false)
    {
        printf("clsFtp.ChangeFolder(%s) error\n", pszRemoteFolder);
        return false;
    }
    if (clsFtp.Download(pszRemoteFile, pszLocalPath) == false)
    {
        printf("clsFtp.Download(%s) error\n", pszRemoteFile);
        return false;
    }
    return true;
}
bool UpdateShotSolution()
{
    CFtpClient clsFtp;
    if (clsFtp.Connect(localTestIP.c_str(), localTestPORT, true))
    {
        int a = 0;
    }
    clsFtp.Login(FTPID, FTPPW);
    char buff[FILENAME_MAX]; //create string buffer to hold path
    GetCurrentDir(buff, sizeof(buff));
    std::string currentpath(buff);
    currentpath.append("/game/cservice/Resource");
    currentpath.append("/ShotSolutions.bin");
    const char* pszRemoteFolder = "/";
    const char* pszRemoteFile = "ShotSolutions.bin";
    const char* pszLocalPath = currentpath.c_str();

    // FTP 서버 폴더를 변경한다.

    if (clsFtp.ChangeFolder(pszRemoteFolder) == false)
    {
        printf("clsFtp.ChangeFolder(%s) error\n", pszRemoteFolder);
        return false;
    }
    if (clsFtp.Download(pszRemoteFile, pszLocalPath) == false)
    {
        printf("clsFtp.Download(%s) error\n", pszRemoteFile);
        return false;
    }
    return true;
}

void HostTestPlayPacketSave(bool value)
{
    pHost->DevPlayPacketSave(value);
    //HostDevPlayPacketSave(pHost, value);
}

void HostTestNoKick(bool value)
{
    if (pHost != nullptr)
    {
        pHost->SetDevNoKick(value);
    }
}

void HostTestAiLevelSetting(uint32_t team, uint32_t value)
{
    pHost->DevAiLevelSetting(team, value);
    //HostDevAiLevelSetting(pHost, team, value);
}

//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HOSTTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HOSTTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//

int scroll;

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_VSCROLL,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   SetScrollRange(hWnd, SB_VERT, 0, static_cast<int>(messages.size()), TRUE);
   SetScrollPos(hWnd, SB_VERT, scroll, TRUE);

   if (!hWnd)
   {
      return FALSE;
   }

   LoadOption();

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   
   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//

vector<int> ids;
int centerX = 450;
int centerY = 650;
float magnification = 50;
float playerSize = 20;
float ballSize = 15;

void DrawPlayer(HDC hdc, float x, float z, float yaw)
{
    float startX = x * magnification;
    float startZ = z * magnification;

    float angleRad = yaw * 0.0174532924f;
    float cosA = cos(angleRad);
    float sinA = -sin(angleRad);

    float endX = -playerSize * sinA + startX;
    float endZ = playerSize * cosA + startZ;

    Ellipse(hdc, static_cast<int>(startX - playerSize + centerX), static_cast<int>(-(startZ - playerSize) + centerY), 
        static_cast<int>(startX + playerSize + centerX), static_cast<int>(-(startZ + playerSize) + centerY));
    MoveToEx(hdc, static_cast<int>(startX + centerX), static_cast<int>(-startZ + centerY), nullptr);
    LineTo(hdc, static_cast<int>(endX + centerX), static_cast<int>(-endZ + centerY));
}

void DrawPlayerEx(HDC hdc, float x, float z, float yaw)
{
    float startX = x * magnification;
    float startZ = z * magnification;

    float angleRad = yaw * 0.0174532924f;
    float cosA = cos(angleRad);
    float sinA = -sin(angleRad);

    float endX = -playerSize * sinA + startX;
    float endZ = playerSize * cosA + startZ;

    Ellipse(hdc, static_cast<int>(startX - playerSize + centerX), static_cast<int>(-(startZ - playerSize) + centerY),
        static_cast<int>(startX + playerSize + centerX), static_cast<int>(-(startZ + playerSize) + centerY));
    MoveToEx(hdc, static_cast<int>(startX + centerX), static_cast<int>(-startZ + centerY), nullptr);
    LineTo(hdc, static_cast<int>(endX + centerX), static_cast<int>(-endZ + centerY));
}

void DrawBall(HDC hdc, float x, float z)
{
    float startX = x * magnification;
    float startZ = z * magnification;

    Ellipse(hdc, static_cast<int>(startX - ballSize + centerX), static_cast<int>(-(startZ - ballSize) + centerY),
        static_cast<int>(startX + ballSize + centerX), static_cast<int>(-(startZ + ballSize) + centerY));
}

void LoadData()
{
    if (gBalanceTable)
    {
        delete(gBalanceTable);
    }

    gBalanceTable = new CDataManagerBalance();

    if (!gBalanceTable->HasData())
    {
        delete(gBalanceTable);
        UpdateBalanceData();
        gBalanceTable = new CDataManagerBalance();
    }

    if (gShotSolution)
    {
        delete(gShotSolution);
    }

    gShotSolution = new CDataManagerShotSolution();

    if (!gShotSolution->HasData())
    {
        delete(gShotSolution);
        UpdateShotSolution();
        gShotSolution = new CDataManagerShotSolution();
    }
    if (gAnimationController)
    {
        delete(gAnimationController);
    }
    gAnimationController = new CAnimationController();
    gAnimationController->ReadBinary();

    size_t AnimationbinarySize = gAnimationController->GetAnimationInfoSize();

    for (int i = 0; i < AnimationbinarySize; ++i)
    {
        int result = gAnimationController->CheckAnimationData(i);
        const char* name = nullptr;
        string str = "";
        string error_code = "";
        switch (result)
        {
            case -1:
            {
                error_code = "Index is null";
            }
            break;
            case -2:
            {
                error_code = "Curve is null";
            }
            break;
            case -3:
            {
                error_code = "Curve X or Y or Z is null";
            }
            case -4:
            {
                error_code = "Key is null";
            }
            break;
            case -5:
            {
                error_code = "Length is null";
            }
            break;
            default:
                error_code = "i don't know";
            break;
        }

        if (result < 0)
        {
            name = gAnimationController->ValidationAnimationInfo(i);

            if (name == nullptr)
            {
                name = "null";
            }

            str = "Invalid AnimationData Index : " + std::to_string(i) + ", Name : " + name + ", Error : " + error_code.c_str();

            LogMessage(str.c_str());
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITMENU:
        {
            if ((option & HOST_OPTION_LOG_ALIVE_CHECK) != 0)
            {
                CheckMenuItem((HMENU)wParam, ID_LOG_ALIVE_CHECK, MF_BYCOMMAND | MF_CHECKED);
            }
            else
            {
                CheckMenuItem((HMENU)wParam, ID_LOG_ALIVE_CHECK, MF_BYCOMMAND | MF_UNCHECKED);
            }

            if ((option & HOST_OPTION_LOG_SHOT_CHECK) != 0)
            {
                CheckMenuItem((HMENU)wParam, ID_LOG_SHOT_CHECK, MF_BYCOMMAND | MF_CHECKED);
            }
            else
            {
                CheckMenuItem((HMENU)wParam, ID_LOG_SHOT_CHECK, MF_BYCOMMAND | MF_UNCHECKED);
            }

            if ((option & HOST_OPTION_CHOOSE_NORMAL_BETWEEN_NORMAL_AND_RANK) != 0)
            {
                CheckMenuItem((HMENU)wParam, ID_OPTION_CHOOSE_NORMAL_BETWEEN_NORMAL_AND_RANK, MF_BYCOMMAND | MF_CHECKED);
            }
            else
            {
                CheckMenuItem((HMENU)wParam, ID_OPTION_CHOOSE_NORMAL_BETWEEN_NORMAL_AND_RANK, MF_BYCOMMAND | MF_UNCHECKED);
            }

            if ((option & HOST_OPTION_NO_CHECK_REPLAY) != 0)
            {
                CheckMenuItem((HMENU)wParam, ID_OPTION_REPLAYCHECK, MF_BYCOMMAND | MF_CHECKED);
            }
            else
            {
                CheckMenuItem((HMENU)wParam, ID_OPTION_REPLAYCHECK, MF_BYCOMMAND | MF_UNCHECKED);
            }

            if ((option & HOST_OPTION_NO_CHECK_GOAL) != 0)
            {
                CheckMenuItem((HMENU)wParam, ID_NO_CHECK_GOAL, MF_BYCOMMAND | MF_CHECKED);
            }
            else
            {
                CheckMenuItem((HMENU)wParam, ID_NO_CHECK_GOAL, MF_BYCOMMAND | MF_UNCHECKED);
            }

            if ((option & HOST_OPTION_NO_CHECK_TIME) != 0)
            {
                CheckMenuItem((HMENU)wParam, ID_NO_CHECK_TIME, MF_BYCOMMAND | MF_CHECKED);
            }
            else
            {
                CheckMenuItem((HMENU)wParam, ID_NO_CHECK_TIME, MF_BYCOMMAND | MF_UNCHECKED);
            }

            if ((option & HOST_OPTION_NO_CHECK_SHOTCLOCK) != 0)
            {
                CheckMenuItem((HMENU)wParam, ID_NO_CHECK_SHOTCLOCK, MF_BYCOMMAND | MF_CHECKED);
            }
            else
            {
                CheckMenuItem((HMENU)wParam, ID_NO_CHECK_SHOTCLOCK, MF_BYCOMMAND | MF_UNCHECKED);
            }

            if ((option & HOST_OPTION_NO_AUTO_AI) != 0)
            {
                CheckMenuItem((HMENU)wParam, ID_OPTION_AUTOAICHANGE, MF_BYCOMMAND | MF_CHECKED);
            }
            else
            {
                CheckMenuItem((HMENU)wParam, ID_OPTION_AUTOAICHANGE, MF_BYCOMMAND | MF_UNCHECKED);
            }

            if ((option & HOST_FUNC_SHOTFAIL) != 0)
            {
                CheckMenuItem((HMENU)wParam, ID_SHOTFAIL, MF_BYCOMMAND | MF_CHECKED);
            }
            else
            {
                CheckMenuItem((HMENU)wParam, ID_SHOTFAIL, MF_BYCOMMAND | MF_UNCHECKED);
            }

            if ((option & HOST_OPTION_NO_KICK) != 0)
            {
                CheckMenuItem((HMENU)wParam, ID_DEBUG_NOKICK, MF_BYCOMMAND | MF_CHECKED);
            }
            else
            {
                CheckMenuItem((HMENU)wParam, ID_DEBUG_NOKICK, MF_BYCOMMAND | MF_UNCHECKED);
            }

            if ((option & HOST_OPTION_PREVENT_AFK) != 0)
            {
                CheckMenuItem((HMENU)wParam, ID_DEBUG_PREVENT_AFK, MF_BYCOMMAND | MF_CHECKED);
            }
            else
            {
                CheckMenuItem((HMENU)wParam, ID_DEBUG_PREVENT_AFK, MF_BYCOMMAND | MF_UNCHECKED);
            }

            if (visualDebug == true)
            {
                CheckMenuItem((HMENU)wParam, ID_DEBUG_PLAYMONITOR, MF_BYCOMMAND | MF_CHECKED);
            }
            else
            {
                CheckMenuItem((HMENU)wParam, ID_DEBUG_PLAYMONITOR, MF_BYCOMMAND | MF_UNCHECKED);
            }

            switch (option & HOST_OPTION_STAGE)
            {
                case HOST_OPTION_STAGE_RANDOM:
                    CheckMenuItem((HMENU)wParam, ID_STAGE_RANDOM, MF_BYCOMMAND | MFS_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_0, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_100, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_OPTION_STAGE_0:
                    CheckMenuItem((HMENU)wParam, ID_STAGE_RANDOM, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_0, MF_BYCOMMAND | MFS_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_100, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_OPTION_STAGE_1:
                    CheckMenuItem((HMENU)wParam, ID_STAGE_RANDOM, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_0, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_1, MF_BYCOMMAND | MFS_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_100, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_OPTION_STAGE_2:
                    CheckMenuItem((HMENU)wParam, ID_STAGE_RANDOM, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_0, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_2, MF_BYCOMMAND | MFS_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_100, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_OPTION_STAGE_3:
                    CheckMenuItem((HMENU)wParam, ID_STAGE_RANDOM, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_0, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_3, MF_BYCOMMAND | MFS_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_100, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_OPTION_STAGE_4:
                    CheckMenuItem((HMENU)wParam, ID_STAGE_RANDOM, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_0, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_4, MF_BYCOMMAND | MFS_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_100, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_OPTION_STAGE_5:
                    CheckMenuItem((HMENU)wParam, ID_STAGE_RANDOM, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_0, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_5, MF_BYCOMMAND | MFS_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_100, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_OPTION_STAGE_6:
                    CheckMenuItem((HMENU)wParam, ID_STAGE_RANDOM, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_0, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_6, MF_BYCOMMAND | MFS_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_100, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_OPTION_STAGE_100:
                    CheckMenuItem((HMENU)wParam, ID_STAGE_RANDOM, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_0, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_STAGE_100, MF_BYCOMMAND | MFS_CHECKED);
                    break;
            }

            switch (option & HOST_OPTION_SCENE)
            {
                case HOST_OPTION_SCENE_RANDOM:
                    CheckMenuItem((HMENU)wParam, ID_SCENE_RANDOM, MF_BYCOMMAND | MFS_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_SCENE_1000, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_SCENE_1001, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_SCENE_1002, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_OPTION_SCENE_1000:
                    CheckMenuItem((HMENU)wParam, ID_SCENE_RANDOM, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_SCENE_1000, MF_BYCOMMAND | MFS_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_SCENE_1001, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_SCENE_1002, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_OPTION_SCENE_1001:
                    CheckMenuItem((HMENU)wParam, ID_SCENE_RANDOM, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_SCENE_1000, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_SCENE_1001, MF_BYCOMMAND | MFS_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_SCENE_1002, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_OPTION_SCENE_1002:
                    CheckMenuItem((HMENU)wParam, ID_SCENE_RANDOM, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_SCENE_1000, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_SCENE_1001, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_SCENE_1002, MF_BYCOMMAND | MFS_CHECKED);
                    break;
            }


            switch ((option & HOST_TIME))
            {
                case HOST_TIME_MINUTE_1:
                    CheckMenuItem((HMENU)wParam, ID_TIME_1, MF_BYCOMMAND | MF_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_7, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_8, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_9, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_10, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_TIME_MINUTE_2:
                    CheckMenuItem((HMENU)wParam, ID_TIME_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_2, MF_BYCOMMAND | MF_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_7, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_8, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_9, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_10, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_TIME_MINUTE_3:
                    CheckMenuItem((HMENU)wParam, ID_TIME_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_3, MF_BYCOMMAND | MF_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_7, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_8, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_9, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_10, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_TIME_MINUTE_4:
                    CheckMenuItem((HMENU)wParam, ID_TIME_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_4, MF_BYCOMMAND | MF_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_5, MF_BYCOMMAND | MF_UNCHECKED); 
                    CheckMenuItem((HMENU)wParam, ID_TIME_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_7, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_8, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_9, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_10, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_TIME_MINUTE_5:
                    CheckMenuItem((HMENU)wParam, ID_TIME_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_5, MF_BYCOMMAND | MF_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_7, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_8, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_9, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_10, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_TIME_MINUTE_6:
                    CheckMenuItem((HMENU)wParam, ID_TIME_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_6, MF_BYCOMMAND | MF_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_7, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_8, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_9, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_10, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_TIME_MINUTE_7:
                    CheckMenuItem((HMENU)wParam, ID_TIME_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_7, MF_BYCOMMAND | MF_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_8, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_9, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_10, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_TIME_MINUTE_8:
                    CheckMenuItem((HMENU)wParam, ID_TIME_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_7, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_8, MF_BYCOMMAND | MF_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_9, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_10, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_TIME_MINUTE_9:
                    CheckMenuItem((HMENU)wParam, ID_TIME_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_7, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_8, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_9, MF_BYCOMMAND | MF_CHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_10, MF_BYCOMMAND | MF_UNCHECKED);
                    break;
                case HOST_TIME_MINUTE_10:
                    CheckMenuItem((HMENU)wParam, ID_TIME_1, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_2, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_3, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_4, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_5, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_6, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_7, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_8, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_9, MF_BYCOMMAND | MF_UNCHECKED);
                    CheckMenuItem((HMENU)wParam, ID_TIME_10, MF_BYCOMMAND | MF_CHECKED);
                    break;
                default:
                    break;
            }
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
                case ID_NO_CHECK_GOAL:
                    {
                        if ((option & HOST_OPTION_NO_CHECK_GOAL) != 0)
                        {
                            option ^= HOST_OPTION_NO_CHECK_GOAL;
                        }
                        else
                        {
                            option |= HOST_OPTION_NO_CHECK_GOAL;
                        }
                    }
                    break;
                case ID_NO_CHECK_TIME:
                    {
                        if ((option & HOST_OPTION_NO_CHECK_TIME) != 0)
                        {
                            option ^= HOST_OPTION_NO_CHECK_TIME;
                        }
                        else
                        {
                            option |= HOST_OPTION_NO_CHECK_TIME;
                        }
                    }
                    break;
                case ID_NO_CHECK_SHOTCLOCK:
                    {
                        if ((option & HOST_OPTION_NO_CHECK_SHOTCLOCK) != 0)
                        {
                            option ^= HOST_OPTION_NO_CHECK_SHOTCLOCK;
                        }
                        else
                        {
                            option |= HOST_OPTION_NO_CHECK_SHOTCLOCK;
                        }
                    }
                    break;
                case ID_OPTION_REPLAYCHECK:
                    {
                        if ((option & HOST_OPTION_NO_CHECK_REPLAY) != 0)
                        {
                            option ^= HOST_OPTION_NO_CHECK_REPLAY;
                        }
                        else
                        {
                            option |= HOST_OPTION_NO_CHECK_REPLAY;
                        }

                    }
                    break;
                case ID_OPTION_AUTOAICHANGE:
                    {
                        if ((option & HOST_OPTION_NO_AUTO_AI) != 0)
                        {
                            option ^= HOST_OPTION_NO_AUTO_AI;
                        }
                        else
                        {
                            option |= HOST_OPTION_NO_AUTO_AI;
                        }
                    }
                    break;
                case ID_OPTION_CHOOSE_NORMAL_BETWEEN_NORMAL_AND_RANK:
                    {
                        if ((option & HOST_OPTION_CHOOSE_NORMAL_BETWEEN_NORMAL_AND_RANK) != 0)
                        {
                            option ^= HOST_OPTION_CHOOSE_NORMAL_BETWEEN_NORMAL_AND_RANK;
                        }
                        else
                        {
                            option |= HOST_OPTION_CHOOSE_NORMAL_BETWEEN_NORMAL_AND_RANK;
                        }
                    }
                    break;
                case ID_LOG_ALIVE_CHECK:
                    {
                        if ((option & HOST_OPTION_LOG_ALIVE_CHECK) != 0)
                        {
                            option ^= HOST_OPTION_LOG_ALIVE_CHECK;
                        }
                        else
                        {
                            option |= HOST_OPTION_LOG_ALIVE_CHECK;
                        }
                    }
                    break;
                case ID_LOG_SHOT_CHECK:
                    {
                        if ((option & HOST_OPTION_LOG_SHOT_CHECK) != 0)
                        {
                            option ^= HOST_OPTION_LOG_SHOT_CHECK;
                        }
                        else
                        {
                            option |= HOST_OPTION_LOG_SHOT_CHECK;
                        }
                    }
                    break;
                case ID_SHOTFAIL:
                    {
                        if ((option & HOST_FUNC_SHOTFAIL) != 0)
                        {
                            option ^= HOST_FUNC_SHOTFAIL;
                        }
                        else
                        {
                            option |= HOST_FUNC_SHOTFAIL;
                        }
                    }
                    break;
                case ID_DEBUG_NOKICK:
                    {
                        if ((option & HOST_OPTION_NO_KICK) != 0)
                        {
                            option ^= HOST_OPTION_NO_KICK;
                        }
                        else
                        {
                            option |= HOST_OPTION_NO_KICK;
                        }
                    }
                    break;
                case ID_DEBUG_PREVENT_AFK:
                    {
                        if ((option & HOST_OPTION_PREVENT_AFK) != 0)
                        {
                            option ^= HOST_OPTION_PREVENT_AFK;
                        }
                        else
                        {
                            option |= HOST_OPTION_PREVENT_AFK;
                        }
                    }
                    break;
                case ID_CREATE_NORMAL:
                    {
                        
                        CreateHost(HOST_OPTION_GAMEMODE_NORMAL | option);

                        LogMessage("Normal mode creation completed.");

                        SaveOption();
                    }
                    break;
                case ID_CREATE_TRIO:
                    {
                        CreateHost(HOST_OPTION_GAMEMODE_TRIO | option);

                        LogMessage("Trio mode creation completed.");

                        SaveOption();
                    }
                    break;
                case ID_CREATE_TRIO_PVP_MIXED:
                    {
                        CreateHost(HOST_OPTION_GAMEMODE_TRIO_PVP_MIXED | option);

                        LogMessage("Trio_PVP_MIXED mode creation completed.");

                        SaveOption();
                    }
                    break;

                case ID_CREATE_MINIGAME:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_MINIGAME_CUSTOM | option);

                    LogMessage("MinigameCustom mode creation completed.");

                    SaveOption();
                }
                break;


                case ID_CREATE_CONTINUOUS:
                    {
                        CreateHost(HOST_OPTION_GAMEMODE_CONTINUOUS | option);

                        LogMessage("Continunous mode creation completed.");

                        SaveOption();
                    }
                    break;
                case ID_CREATE_TRAINING:
                    {
                        
                        CreateHost(HOST_OPTION_GAMEMODE_TRAINING | option);

                        LogMessage("Training mode creation completed.");

                        SaveOption();
                    }
                    break;
                case ID_CREATE_TRAINING_REBOUND:
                    {

                        CreateHost(HOST_OPTION_GAMEMODE_TRAINING_REBOUND | option);

                        LogMessage("Training Rebound mode creation completed.");

                        SaveOption();
                    }
                    break;
                case ID_CREATE_TRAINING_JUMPSHOTBLOCK:
                    {

                        CreateHost(HOST_OPTION_GAMEMODE_TRAINING_JUMPSHOTBLOCK | option);

                        LogMessage("Training JumpShotBlock mode creation completed.");

                        SaveOption();
                    }
                    break;
                case ID_CREATE_TRAINING_RIMATTACKBLOCK:
                    {

                        CreateHost(HOST_OPTION_GAMEMODE_TRAINING_RIMATTACKBLOCK | option);

                        LogMessage("Training RimAttackBlock mode creation completed.");

                        SaveOption();
                    }
                    break;
                case ID_CREATE_TRAINING_OFFBALLMOVE:
                    {

                        CreateHost(HOST_OPTION_GAMEMODE_TRAINING_OFFBALLMOVE | option);

                        LogMessage("Training OffBallMove mode creation completed.");

                        SaveOption();
                    }
                    break;
                case ID_UPDATEDATA_ANIMATIONDATA:
                    {
                        if(UpdateAnimationData())
                            LogMessage("Animation Data Updated.");
                    }
                    break;    
                case ID_UPDATEDATA_BALANCEDATA:
                    {
                        if(UpdateBalanceData())
                            LogMessage("Balance Data Updated.");
                    }
                    break;
                case ID_UPDATEDATA_SHOTSOLUTION:
                    {
                        if (UpdateShotSolution())
                            LogMessage("ShotSolution Updated.");
                    }
                    break;
                case ID_CREATE_TUTORIAL_BASIC:
                {
                    
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_BASIC | option);

                    LogMessage("Tutorial basic mode creation completed.");

                    SaveOption();
                }
                break;
                case ID_CREATE_TUTORIAL_PASS:
                {
                   
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_PASS | option);

                    LogMessage("Tutorial pass mode creation completed.");

                    SaveOption();
                }
                break;
                case ID_CREATE_TUTORIAL_STEAL:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_STEAL | option);

                    LogMessage("Tutorial steal mode creation completed.");

                    SaveOption();
                }
                break;
                case ID_CREATE_TUTORIAL_DIVINGCATCH:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_DIVINGCATCH | option);

                    LogMessage("Tutorial DivingCatch mode creation completed.");

                    SaveOption();
                }
                break;
                case ID_CREATE_TUTORIAL_JUMPSHOT:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_JUMPSHOT | option);

                    LogMessage("Tutorial JumpShot mode creation completed.");

                    SaveOption();
                }
                break;
                case ID_CREATE_TUTORIAL_RIMATTACK:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_RIMATTACK | option);

                    LogMessage("Tutorial RimAttack mode creation completed.");

                    SaveOption();
                }
                break;
                case ID_CREATE_TUTORIAL_JUMPSHOTBLOCK:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_JUMPSHOTBLOCK | option);

                    LogMessage("Tutorial JumpShotBlock mode creation completed.");

                    SaveOption();
                }
                break;
                case ID_CREATE_TUTORIAL_REBOUND:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_REBOUND | option);

                    LogMessage("Tutorial Rebound mode creation completed.");

                    SaveOption();
                }
                break;

                case ID_CREATE_TUTORIAL_REBOUNDBEGINNER:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_REBOUND_BEGINNER | option);

                    LogMessage("Tutorial Rebound Beginner mode creation completed.");

                    SaveOption();
                }
                break;

                case ID_CREATE_TUTORIAL_BEGINNERBLOCK:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_BLOCK_BEGINNER | option);

                    LogMessage("Tutorial Block Beginner mode creation completed.");

                    SaveOption();
                }
                break; 

                case ID_CREATE_TUTORIAL_RIMATTACKBLOCK:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_RIMATTACKBLOCK | option);

                    LogMessage("Tutorial RimAttackBlock mode creation completed.");

                    SaveOption();
                }
                break;
                case ID_CREATE_TUTORIAL_BOXOUT:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_BOXOUT | option);

                    LogMessage("Tutorial BoxOut mode creation completed.");

                    SaveOption();
                }
                break;
                case ID_CREATE_TUTORIAL_PENETRATE:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_PENETRATE | option);

                    LogMessage("Tutorial Penetrate mode creation completed.");

                    SaveOption();
                }
                break;
                case ID_CREATE_TUTORIAL_SHOOTINGDISTURB:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_TUTORIAL_SHOOTINGDISTURB | option);

                    LogMessage("Tutorial ShootingDisturb mode creation completed.");

                    SaveOption();
                }
                break;
                case ID_CREATE_CHALLENGE_1:
                    {
                        CreateHost(HOST_OPTION_GAMEMODE_CHALLENGE1 | option);

                        LogMessage("Challenge mode 1 creation completed.");

                        SaveOption();
                    }
                    break;
                case ID_CREATE_CHALLENGE_2:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_CHALLENGE2 | option);

                    LogMessage("Challenge mode 2 creation completed.");

                    SaveOption();
                }
                break;
                case ID_CREATE_CHALLENGE_3:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_CHALLENGE3 | option);

                    LogMessage("Challenge mode 3 creation completed.");

                    SaveOption();
                }
                break;
                case ID_CREATE_CHALLENGE_4:
                {
                    CreateHost(HOST_OPTION_GAMEMODE_CHALLENGE4 | option);

                    LogMessage("Challenge mode 4 creation completed.");

                    SaveOption();
                }
                case ID_TIME_1:
                    {
                        option &= HOST_TIME_RESET;
                        option |= HOST_TIME_MINUTE_1;
                    }
                    break;
                case ID_TIME_2:
                    {
                        option &= HOST_TIME_RESET;
                        option |= HOST_TIME_MINUTE_2;
                    }
                    break;
                case ID_TIME_3:
                    {
                        option &= HOST_TIME_RESET;
                        option |= HOST_TIME_MINUTE_3;
                    }
                    break; 
                case ID_TIME_4:
                    {
                        option &= HOST_TIME_RESET;
                        option |= HOST_TIME_MINUTE_4;
                    }
                    break;
                case ID_TIME_5:
                    {
                        option &= HOST_TIME_RESET;
                        option |= HOST_TIME_MINUTE_5;
                    }
                    break;
                case ID_TIME_6:
                    {
                        option &= HOST_TIME_RESET;
                        option |= HOST_TIME_MINUTE_6;
                    }
                    break;
                case ID_TIME_7:
                    {
                        option &= HOST_TIME_RESET;
                        option |= HOST_TIME_MINUTE_7;
                    }
                    break;
                case ID_TIME_8:
                    {
                        option &= HOST_TIME_RESET;
                        option |= HOST_TIME_MINUTE_8;
                    }
                    break;
                case ID_TIME_9:
                    {
                        option &= HOST_TIME_RESET;
                        option |= HOST_TIME_MINUTE_9;
                    }
                    break;
                case ID_TIME_10:
                    {
                        option &= HOST_TIME_RESET;
                        option |= HOST_TIME_MINUTE_10;
                    }
                    break;
                case ID_PLAY_PACKET_SAVE:
                    {
                        HostTestPlayPacketSave(true);
                        LogMessage("PlayerPacket Save Start...");
                    }
                    break;
				case ID_LOCAL_TEST_FOR_CHINA:
				    {
                        localTestIP = FTPSERVER_IP_FOR_CHINA;
                        localTestPORT = FTPSERVER_PORT_FOR_CHINA;
                        string str_log = "Connect IP : " + localTestIP + ", PORT : " + std::to_string(localTestPORT);
					    LogMessage(str_log.c_str());
				    }
				    break;
				case ID_STAGE_RANDOM:
				{
					option &= HOST_OPTION_STAGE_RESET;
					option |= HOST_OPTION_STAGE_RANDOM;
				}
				break;
				case ID_STAGE_0:
				{
					option &= HOST_OPTION_STAGE_RESET;
					option |= HOST_OPTION_STAGE_0;
				}
				break;
				case ID_STAGE_1:
				{
					option &= HOST_OPTION_STAGE_RESET;
					option |= HOST_OPTION_STAGE_1;
				}
				break;
				case ID_STAGE_2:
				{
					option &= HOST_OPTION_STAGE_RESET;
					option |= HOST_OPTION_STAGE_2;
				}
				break;
				case ID_STAGE_3:
				{
					option &= HOST_OPTION_STAGE_RESET;
					option |= HOST_OPTION_STAGE_3;
				}
				break;
				case ID_STAGE_4:
				{
					option &= HOST_OPTION_STAGE_RESET;
					option |= HOST_OPTION_STAGE_4;
				}
				break;
				case ID_STAGE_5:
				{
					option &= HOST_OPTION_STAGE_RESET;
					option |= HOST_OPTION_STAGE_5;
				}
				break;
                case ID_STAGE_6:
                {
                    option &= HOST_OPTION_STAGE_RESET;
                    option |= HOST_OPTION_STAGE_6;
                }
                break;
				case ID_STAGE_100:
				{
					option &= HOST_OPTION_STAGE_RESET;
					option |= HOST_OPTION_STAGE_100;
				}
				break;
				case ID_SCENE_RANDOM:
				{
					option &= HOST_OPTION_SCENE_RESET;
					option |= HOST_OPTION_SCENE_RANDOM;
				}
				break;
				case ID_SCENE_1000:
				{
					option &= HOST_OPTION_SCENE_RESET;
					option |= HOST_OPTION_SCENE_1000;
				}
				break;
				case ID_SCENE_1001:
				{
					option &= HOST_OPTION_SCENE_RESET;
					option |= HOST_OPTION_SCENE_1001;
				}
				break;
				case ID_SCENE_1002:
				{
					option &= HOST_OPTION_SCENE_RESET;
					option |= HOST_OPTION_SCENE_1002;
				}
				break;
				case ID_DEBUG_PLAYMONITOR:
				{
					visualDebug = !visualDebug;
					RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
				}
				break;
				case ID_DEBUG_PACKETTEST:
				{
					for (size_t i = 0; i < 10; i++)
					{
						string n = CHost::TestPacket();
						LogMessage(n.c_str());
					}
				}
				break;
				case IDM_ABOUT:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
        }
        break;
    case WM_MOUSEHWHEEL:
        {
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            scroll += min(100, max(0, zDelta));
        }
        break;
    case WM_VSCROLL:
        {
            int size = (int)messages.size();

            switch (LOWORD(wParam))
            {
                case SB_LINEUP:
                    {
                        scroll = 0;
                    }
                    break;
                case SB_PAGEUP:
                    {
                        scroll = max(0, scroll - 1);
                    }
                    break;
                case SB_PAGEDOWN:
                    {
                        scroll = min(size, scroll + 1);
                    }
                    break;
                case SB_LINEDOWN:
                    {
                        scroll = size;
                    }
                    break;
                case SB_THUMBTRACK:
                    {
                        scroll = HIWORD(wParam);
                    }
                    break;
            }
            
            SetScrollRange(hWnd, SB_VERT, 0, size, TRUE);
            SetScrollPos(hWnd, SB_VERT, scroll, TRUE);

            RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
        }
        break;
    case WM_PAINT:
    {
        RECT rt;
        GetClientRect(hWnd, &rt);

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        if (visualDebug == true)
        {
            if (pHost != nullptr)
            {
                if (ids.size() == 0)
                {
                    //HostDevGetPlayerID(pHost, &ids);
                    std::vector<F4PACKET::SPlayerInformationT*> vecCharacters;
                    vecCharacters.clear();

                    pHost->GetCharacterInformationVec(vecCharacters);

                    for (int i = 0; i < vecCharacters.size(); ++i)
                    {
                        ids.push_back(vecCharacters[i]->id);
                    }
                }
                else
                {
                    std::vector<F4PACKET::SPlayerInformationT*> vecCharacters;
                    vecCharacters.clear();

                    pHost->GetCharacterInformationVec(vecCharacters);

                    if (ids.size() != vecCharacters.size())
                    {
                        ids.clear();

                        for (int i = 0; i < vecCharacters.size(); ++i)
                        {
                            ids.push_back(vecCharacters[i]->id);
                        }
                    }
                }

                Rectangle(hdc, 69, 58, 831, 650);


                //for (int i = 0; i < ids.size(); i++)
                //{
                //    float x, y, yaw;

                //    //HostDevGetPlayerPosition(pHost, ids[i], &x, &y, &yaw);
                //    F4PACKET::SPlayerAction* pInfo = pHost->GetCharacterAction(ids[i]);

                //    x = pInfo->positionlogic().x();
                //    y = pInfo->positionlogic().z();
                //    yaw = pInfo->yawlogic();


                //    DrawPlayer(hdc, x, y, yaw);
                //}

                for (int i = 0; i < ids.size(); i++)
                {
                    float x, y, yaw;

                    
                    F4PACKET::SPlayerAction* pInfo = pHost->GetCharacterStateAction(ids[i]);

                    x = pInfo->positionlogic().x();
                    y = pInfo->positionlogic().z();
                    yaw = pInfo->yawlogic();

                    DrawPlayerEx(hdc, x, y, yaw);
                    
                    
                    
                    /*
                    CCharacter* pCharacter = pHost->GetCharacterManager()->GetCharacter(ids[i]);
                    if (pCharacter)
                    {
                        x = pCharacter->GetPosition().fX;
                        y = pCharacter->GetPosition().fZ;
                        yaw = pCharacter->GetDirYaw();

                        DrawPlayerEx(hdc, x, y, yaw);
                    }
                    */
                    

                }


                float ball_x, ball_z;
                //HostDevGetBallPosition(pHost, &ball_x, &ball_z);
                auto pos = pHost->GetBallPosition();
                ball_x = pos.x();
                ball_z = pos.z();

                DrawBall(hdc, ball_x, ball_z);
            }
        }
        else
        {
            int y = 5;

            int vertical = (int)(rt.bottom - rt.top) / drawTextUnit;
            int start = max(0, scroll - vertical);

            for (int i = start; i < messages.size(); i++)
            {
                TextOut(hdc, 5, y, messages[i].c_str(), static_cast<int>(messages[i].size()));

                y += drawTextUnit;
            }

            EndPaint(hWnd, &ps);
        }
    }
    break;
    case WM_DESTROY:
        {
            SaveOption();
            PostQuitMessage(0);
            if (gBalanceTable)
                delete(gBalanceTable);
            if (gShotSolution)
                delete(gShotSolution);
            if (gAnimationController)
                delete(gAnimationController);
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void AddMessage(string str)
{
    //string* strmsg = new string(str);
    messages.push_back(str);
    scroll = static_cast<int>(messages.size());
    SetScrollRange(hWnd, SB_VERT, 0, static_cast<int>(messages.size()), TRUE);
    SetScrollPos(hWnd, SB_VERT, scroll, TRUE);
    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

void LogMessage(const char* msg)
{
    AddMessage(msg);
}




string GetInIpAddress()
{
    WSADATA wsadata;
    string strIP; // 이 변수에 IP주소가 저장된다.
    strIP = "";

    if (!WSAStartup(DESIRED_WINSOCK_VERSION, &wsadata))
    {
        if (wsadata.wVersion >= MINIMUM_WINSOCK_VERSION)
        {
            HOSTENT* p_host_info;
            IN_ADDR in;
            char host_name[128] = { 0, };

            gethostname(host_name, 128);

            p_host_info = gethostbyname(host_name);
            
            if (p_host_info != NULL)
            {
                for (int i = 0; p_host_info->h_addr_list[i]; i++)
                {
                    memcpy(&in, p_host_info->h_addr_list[i], 4);
                    strIP = inet_ntoa(in);
                    break;
                }
            }
        }
        WSACleanup();
    }
    return strIP;
}

void SendPacket(HostMessage * pMessage)
{
    ENetPeer* enetPeer = (ENetPeer * )pMessage->peer;

    ENetPacket* packet = enet_packet_create(pMessage->data, pMessage->size, ENET_PACKET_FLAG_RELIABLE);

    enet_peer_send(enetPeer, 0, packet);
}

void ServiceHost()
{
    if (pEnetHost != nullptr)
    {
        ENetEvent event;

        while (enet_host_service(pEnetHost, &event, 0) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                {
                    char buff[100];

                    // 접속하는 사람 IP 찍어줄라고 새로 넣음
                    char* buf = new char[ENET_ADDRESS_INFO_SIZE] {0,};

                    enet_peer_get_address_info(buf, ENET_ADDRESS_INFO_SIZE + 1, event.peer);

                    snprintf(buff, sizeof(buff), "A new client connected from %s", buf);

                    AddMessage(buff);

                    delete[] buf;

                    //HostAddUser(pHost, userID, event.peer);
                    if (pHost != nullptr)
                    {
                        pHost->UserAdd(userID, event.peer);
                    }
                    //HostOnConnect(pHost, userID);
                    pHost->OnHostConnect(userID);
                    userID++;

                    /* Store any relevant client information here. */
                    event.peer->data = (void*)"Client information";
                }
                break;
                case ENET_EVENT_TYPE_RECEIVE:
                {
                    auto iter = UserList.find(userID);
                    if (iter == UserList.end())
                    {
                        std::string handshake = "handshake";

                        HostMessage hostMessage;

                        hostMessage.data = (void*)handshake.c_str();
                        hostMessage.size = static_cast<int>(handshake.size());
                        hostMessage.userID = userID;
                        hostMessage.peer = event.peer;

                        SendPacket(&hostMessage);

                        UserList.insert(std::pair<int, ENetPeer*>(userID, event.peer));
                    }

                    //HostProcessPacket(pHost, (char*)event.packet->data, (int)event.packet->dataLength, event.peer);
                    pHost->ProcessPacket((char*)event.packet->data, (int)event.packet->dataLength, event.peer);
                    /* Clean up the packet now that we're done using it. */
                    enet_packet_destroy(event.packet);
                }
                break;

                case ENET_EVENT_TYPE_DISCONNECT:
                {
                    char* buf = new char[ENET_ADDRESS_INFO_SIZE] { 0, };

                    enet_peer_get_address_info(buf, ENET_ADDRESS_INFO_SIZE + 1, event.peer);

                    snprintf(buff, sizeof(buff), "%s disconnected", buf);

                    AddMessage(buff);

                    delete[] buf;


                    /* Reset the peer's client information. */
                    event.peer->data = NULL;

                    if (false == UserList.empty())
                    {
                        //for (auto& it : UserList)
                        //{
                        //    auto peer = it.second;
                        //
                        //    it.second = nullptr;
                        //    UserList.erase(it);
                        //}
                        for (auto pos = UserList.begin(); pos != UserList.end(); pos++) {
                            if (pos->second == event.peer) {
                                UserList.erase(pos);
                                break;
                            }
                        }
                        if (UserList.size() == 0)
                        {
                            UserList.clear();
                            CreateHost(lastCreatedOption);
                        }
                    }

                    
                }
                break;
            }
        }
    }
}

void ServiceClient()
{
    if (pEnetClient != nullptr)
    {
        ENetEvent event;

        while (enet_host_service(pEnetClient, &event, 0) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                {
                    char buff[100];

                    char* buf = new char[ENET_ADDRESS_INFO_SIZE] { 0, };

                    enet_peer_get_address_info(buf, ENET_ADDRESS_INFO_SIZE + 1, event.peer);

                    snprintf(buff, sizeof(buff), "A new client connected from %s", buf);

                    AddMessage(buff);

                    delete[] buf;

                    /* Store any relevant client information here. */
                    event.peer->data = (void*)"Client information";
                }
                break;
                case ENET_EVENT_TYPE_RECEIVE:
                {
                    /*char buff[100];

                    char* buf = new char[ENET_ADDRESS_INFO_SIZE] { 0, };

                    enet_peer_get_address_info(buf, ENET_ADDRESS_INFO_SIZE + 1, event.peer);*/

                    //snprintf(buff, sizeof(buff), "A packet of length %u containing %s was received from %s on channel %u.\n",
                    //    event.packet->dataLength,
                    //    event.packet->data,
                    //    event.peer->data,
                    //    event.channelID);

                    //AddMessage(buff);

                    /* Clean up the packet now that we're done using it. */
                    enet_packet_destroy(event.packet);
                }
                break;

                case ENET_EVENT_TYPE_DISCONNECT:
                {
                    char buff[100];

                    char* buf = new char[ENET_ADDRESS_INFO_SIZE] { 0, };

                    enet_peer_get_address_info(buf, ENET_ADDRESS_INFO_SIZE + 1, event.peer);

                    snprintf(buff, sizeof(buff), "%s disconnected", buf);

                    AddMessage(buff);

                    delete[] buf;

                    /* Reset the peer's client information. */
                    event.peer->data = NULL;
                }
                break;
            }
        }
    }
}