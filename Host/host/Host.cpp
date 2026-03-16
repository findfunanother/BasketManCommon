#include <cstdint>
#include <iomanip>
#include "Host.h"
#include "State.h"
#include "VerifyManager.h"
#include "FlatBufProtocol.h"
#include "HostUserInfo.h"
#include "PhysicsHandler.h"
#include "BallController.h"
#include "State_BreakTime.h"
#include "State_Challenge_OffBallMove_Init.h"
#include "State_Challenge_OffBallMove_Ready.h"
#include "State_Challenge_OffBallMove_Play.h"
#include "State_Challenge_OffBallMove_End.h"
#include "State_Challenge_Rebound_Init.h"
#include "State_Challenge_Rebound_Ready.h"
#include "State_Challenge_Rebound_Play.h"
#include "State_Challenge_Rebound_End.h"
#include "State_Challenge_Pass_Init.h"
#include "State_Challenge_Pass_Ready.h"
#include "State_Challenge_Pass_Play.h"
#include "State_Challenge_Pass_End.h"
#include "State_Challenge_JumpShotBlock_Init.h"
#include "State_Challenge_JumpShotBlock_Ready.h"
#include "State_Challenge_JumpShotBlock_Play.h"
#include "State_Challenge_JumpShotBlock_BreakTime.h"
#include "State_Challenge_JumpShotBlock_End.h"
#include "State_ChallengeResult.h"
#include "State_End.h"
#include "State_Init.h"
#include "State_JumpBall.h"
#include "State_LineUp.h"
#include "State_Load.h"
#include "State_Play.h"
#include "State_Ready.h"
#include "State_Result.h"
#include "State_SceneStart.h"
#include "State_Score.h"
#include "State_Arrange.h"
#include "State_Training.h"
#include "State_MiniGameCustom.h"
#include "CAnimationController.h"
#include "State_Tutorial.h"
#include "DataManagerShotSolution.h"
#include "DataManagerBalance.h"
#include "Character.h"

#include <fstream>
#include <array>
#include <cstdint>
#include <mutex>
#include <direct.h>

#ifdef _MSC_VER
#else
#include <dlfcn.h>
#define BUFF_SIZE 255
#endif

namespace
{
	const char* kHostLogFilePath = "D:/04.BasketMan/BasketMan.00.Client/Logs/Host/host.log";
	std::mutex g_hostLogMutex;

	int ParseHostIdToInt(const std::string& hostId)
	{
		try
		{
			return std::stoi(hostId);
		}
		catch (...)
		{
			int fallback = 0;
			for (size_t i = 0; i < hostId.size(); ++i)
			{
				fallback = (fallback * 131) + static_cast<unsigned char>(hostId[i]);
			}

			if (fallback < 0)
			{
				fallback = -fallback;
			}

			return fallback;
		}
	}

	void AppendHostFileLog(const std::string& line)
	{
		std::lock_guard<std::mutex> lock(g_hostLogMutex);

		// C++17 filesystem 미사용 환경을 위해 고정 경로를 순차 생성한다.
		_mkdir("D:/04.BasketMan/BasketMan.00.Client/Logs");
		_mkdir("D:/04.BasketMan/BasketMan.00.Client/Logs/Host");

		std::ofstream out(kHostLogFilePath, std::ios::app);
		if (!out.is_open())
		{
			return;
		}

		out << line << "\n";
	}

	uint64_t Mix64(uint64_t x)
	{
		x += 0x9e3779b97f4a7c15ULL;
		x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
		x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
		return x ^ (x >> 31);
	}

	std::array<uint8_t, 32> BuildDigest(const uint8_t* key, size_t keyLen, const uint8_t* data, size_t dataLen)
	{
		uint64_t state = 1469598103934665603ULL;

		for (size_t i = 0; i < keyLen; ++i)
		{
			state ^= key[i];
			state *= 1099511628211ULL;
		}

		state ^= 0xA5;
		state *= 1099511628211ULL;

		for (size_t i = 0; i < dataLen; ++i)
		{
			state ^= data[i];
			state *= 1099511628211ULL;
		}

		std::array<uint8_t, 32> out{};
		uint64_t x = state;
		for (size_t block = 0; block < 4; ++block)
		{
			x = Mix64(x + (block * 0x9e3779b97f4a7c15ULL));
			for (size_t byteIdx = 0; byteIdx < 8; ++byteIdx)
			{
				out[block * 8 + byteIdx] = static_cast<uint8_t>((x >> (byteIdx * 8)) & 0xFF);
			}
		}

		return out;
	}
}


CHost::CHost(char* hostID, int option) : m_HostID(hostID)
{
	m_PacketDataBuffer = nullptr;
	m_CallbackBroadcast = nullptr;
	m_CallbackCpp = nullptr;
	m_CallbackRedis = nullptr;
	m_CallbackRedisSend = nullptr;
	m_CallbackLog = nullptr;
	m_DevManager = nullptr;

	m_CurrentState = EHOST_STATE::NONE;
	m_GameManager = nullptr;
	m_pBallController = nullptr;
	m_pBalance = nullptr;
	m_ShotSolution = nullptr;
	m_AnimationController = nullptr;
	m_TimeManager = nullptr;
	m_CharacterManager = nullptr;
	m_PhysicsHandler = nullptr;

	m_GameManager = new CGameManager(this, option);
	m_pBallController = new CBallController(this);
	m_pBalance = new CDataManagerBalance();
	m_ShotSolution = new CDataManagerShotSolution();
	m_AnimationController = new CAnimationController();
	if (m_AnimationController != nullptr)
	{
		m_AnimationController->ReadBinary();
	}
	
	m_TimeManager = new CTimeManager();
	m_CharacterManager = new CCharacterManager();
	if (m_CharacterManager != nullptr)
	{
		m_CharacterManager->RegistCallbackFuncOnFireMode(std::bind(&CHost::SendOnFireModeCharacterInfo, this));
	}

	if (m_CharacterManager != nullptr)
	{
		m_CharacterManager->RegistCallbackFuncBadManner(std::bind(&CHost::BadMannerNotice, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
	m_PhysicsHandler = new CPhysicsHandler(); 

#ifdef _DEBUG
	m_DevManager = new CDevManager();
	SetDebug(true);
#endif

	m_VerifyManager = new CVerifyManager(this);
	bTreatDelayMode = true; // 리바운드, 픽을 서버에 응답을 받고 처리, 스피드핵 대응 
	bServerSyncPos  = false;
}

CHost::~CHost(void)
{
	string str = "Delete Host";
	ToLog(str.c_str());

	if (m_PacketDataBuffer)
	{
		free(m_PacketDataBuffer);
	}

	m_PacketDataBuffer = nullptr;
}

void CHost::Initialize()
{
#ifdef BINARY_SAVE
	//LoadBinary();
#endif

	// 1. 밸런스 파일 무결성 검사를 위한 해쉬 값
	CDataManagerBalance* pMB = GetBalanceTable();
	if (pMB)
	{
		if (!pMB->HasData())
		{
			string str_log = "[FILE_ERROR] balance table dictionary is empty";
			ToLog(str_log.c_str());
		}

		const unsigned char* pData = pMB->GetBalanceRawData();
		int datalen = pMB->GetBalanceRawDataLen();
		if (pData && datalen > 0)
		{
			if (!Sha256FromMemory_OpenSSL(
				reinterpret_cast<const uint8_t*>(pData),
				datalen,
				m_serverBalanceDataHash))
			{
				string str_log = "[FILE_ERROR] make balance hash file error";
				ToLog(str_log.c_str());
			}
		}
		else
		{
			string str_log = "[FILE_ERROR] balance raw data is null or empty";
			ToLog(str_log.c_str());
		}
	}
	else
	{
		string str_log = "[FILE_ERROR] balance manager is null";
		ToLog(str_log.c_str());
	}

	// 2. 애니메이션 파일 무결성 검사를 위한 해쉬 값 
	CAnimationController* pAnimController =  GetAnimationController();
	unsigned char* pAnimData = (pAnimController != nullptr) ? pAnimController->GetData() : nullptr;
	if (pAnimData)
	{
		int animdataLen = pAnimController->GetDataLength();

		if (!Sha256FromMemory_OpenSSL(
			reinterpret_cast<const uint8_t*>(pAnimData),
			animdataLen,
			m_serverAnimDataHash))
		{
			// 데이터가 없는 경우 
			string str_log = "[FILE_ERROR] make animdata hash file error";
			ToLog(str_log.c_str());
		}
	}
	else
	{
		string str_log = "[FILE_ERROR] animdata is null";
		ToLog(str_log.c_str());
	}


	m_PacketDataBuffer = (char*)malloc(gPacketBufferSize + kPACKET_HEADER_BUFFER_SIZE);
	
	//string iniPath = currentpath + "\\AnimData.bin"; 
	
	switch (GetOption() & HOST_OPTION_GAMEMODE)
	{
/*
		m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
		m_States.insert(HostStateMapType::value_type(EHOST_STATE::ARRANGE, new CState_Arrange(this)));
		m_States.insert(HostStateMapType::value_type(EHOST_STATE::BREAK_TIME, new CState_BreakTime(this)));
		m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
		m_States.insert(HostStateMapType::value_type(EHOST_STATE::INIT, new CState_Init(this)));
		m_States.insert(HostStateMapType::value_type(EHOST_STATE::LINE_UP, new CState_LineUp(this)));
		m_States.insert(HostStateMapType::value_type(EHOST_STATE::JUMP_BALL, new CState_JumpBall(this)));
		m_States.insert(HostStateMapType::value_type(EHOST_STATE::LOAD, new CState_Load(this)));
		m_States.insert(HostStateMapType::value_type(EHOST_STATE::PLAY, new CState_Play(this)));
		m_States.insert(HostStateMapType::value_type(EHOST_STATE::READY, new CState_Ready(this)));
		m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));
		m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCENE_START, new CState_SceneStart(this)));
		m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCORE, new CState_Score(this)));

		// 트레이닝, 챌린지 모드가 아닌경우 일반 3:3 경기로 취급한다. Redis 에서 정보를 받아오면 그 값으로 모드 설정
		SetModeType(EMODE_TYPE::TRIO);
		ChangeState(EHOST_STATE::INIT);
*/
		
		case HOST_OPTION_GAMEMODE_TUTORIAL_BASIC:		// 튜토리얼
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_BASIC, new CState_Tutorial(this)));

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_BASIC);
			ChangeState(EHOST_STATE::TUTORIAL_BASIC);
		}
		break;
		
		
		case HOST_OPTION_GAMEMODE_TUTORIAL_MINIGAME_CUSTOM:		// 튜토리얼
		{
		/*
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_BASIC, new CState_Tutorial(this)));

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_BASIC);
			ChangeState(EHOST_STATE::TUTORIAL_BASIC);
		*/
			switch (GetOption() & HOST_TIME)
			{
			case HOST_TIME_MINUTE_1: SetGameTimeInit(kHOST_TIME_MINUTE_1); break;
			case HOST_TIME_MINUTE_2: SetGameTimeInit(kHOST_TIME_MINUTE_2); break;
			case HOST_TIME_MINUTE_3: SetGameTimeInit(kHOST_TIME_MINUTE_3); break;
			case HOST_TIME_MINUTE_4: SetGameTimeInit(kHOST_TIME_MINUTE_4); break;
			case HOST_TIME_MINUTE_5: SetGameTimeInit(kHOST_TIME_MINUTE_5); break;
			case HOST_TIME_MINUTE_6: SetGameTimeInit(kHOST_TIME_MINUTE_6); break;
			case HOST_TIME_MINUTE_7: SetGameTimeInit(kHOST_TIME_MINUTE_7); break;
			case HOST_TIME_MINUTE_8: SetGameTimeInit(kHOST_TIME_MINUTE_8); break;
			case HOST_TIME_MINUTE_9: SetGameTimeInit(kHOST_TIME_MINUTE_9); break;
			case HOST_TIME_MINUTE_10: SetGameTimeInit(kHOST_TIME_MINUTE_10); break;
			default: SetGameTimeInit(kHOST_TIME_MINUTE_4); break;
			}

		/*
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::MINIGAME_CUSTOM, new CState_MiniGameCustom(this)));

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCENE_START, new CState_SceneStart(this)));

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::LINE_UP, new CState_LineUp(this)));

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::JUMP_BALL, new CState_JumpBall(this)));

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::PLAY, new CState_Play(this)));

			//m_States.insert(HostStateMapType::value_type(EHOST_STATE::ARRANGE, new CState_Arrange(this)));

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::MINIGAME_CUSTOM);
			ChangeState(EHOST_STATE::MINIGAME_CUSTOM);
			*/

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::ARRANGE, new CState_Arrange(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::BREAK_TIME, new CState_BreakTime(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIA_MINIGAME_CUSTOM, new CState_MiniGameCustom(this)));
			
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::INIT, new CState_Init(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::LINE_UP, new CState_LineUp(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::JUMP_BALL, new CState_JumpBall(this)));

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::LOAD, new CState_Load(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::PLAY, new CState_Play(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::READY, new CState_Ready(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCENE_START, new CState_SceneStart(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCORE, new CState_Score(this)));

			// 트레이닝, 챌린지 모드가 아닌경우 일반 3:3 경기로 취급한다. Redis 에서 정보를 받아오면 그 값으로 모드 설정
			SetModeType(EMODE_TYPE::TUTORIAL_MINIGAME_CUSTOM);
			//ChangeState(EHOST_STATE::INIT);
			ChangeState(EHOST_STATE::TUTORIA_MINIGAME_CUSTOM);
		}
		break;
		

		case HOST_OPTION_GAMEMODE_TUTORIAL_PASS:
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_PASS, new CState_Tutorial(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_PASS);

			ChangeState(EHOST_STATE::TUTORIAL_PASS);
		}
		break;
		case HOST_OPTION_GAMEMODE_TUTORIAL_STEAL:
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_STEAL, new CState_Tutorial(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_STEAL);

			ChangeState(EHOST_STATE::TUTORIAL_STEAL);
		}
		break;
		case HOST_OPTION_GAMEMODE_TUTORIAL_DIVINGCATCH:
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_DIVINGCATCH, new CState_Tutorial(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_DIVINGCATCH);

			ChangeState(EHOST_STATE::TUTORIAL_DIVINGCATCH);
		}
		break;
		case HOST_OPTION_GAMEMODE_TUTORIAL_JUMPSHOT:
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_JUMPSHOT, new CState_Tutorial(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_JUMPSHOT);

			ChangeState(EHOST_STATE::TUTORIAL_JUMPSHOT);
		}
		break;
		case HOST_OPTION_GAMEMODE_TUTORIAL_RIMATTACK:
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_RIMATTACK, new CState_Tutorial(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_RIMATTACK);

			ChangeState(EHOST_STATE::TUTORIAL_RIMATTACK);
		}
		break;
		case HOST_OPTION_GAMEMODE_TUTORIAL_JUMPSHOTBLOCK:
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_JUMPSHOTBLOCK, new CState_Tutorial(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_JUMPSHOTBLOCK);

			ChangeState(EHOST_STATE::TUTORIAL_JUMPSHOTBLOCK);
		}
		break;
		case HOST_OPTION_GAMEMODE_TUTORIAL_REBOUND:
		{
			SetOption(HOST_FUNC_SHOTFAIL);

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_REBOUND, new CState_Tutorial(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_REBOUND);

			ChangeState(EHOST_STATE::TUTORIAL_REBOUND);
		}
		break;

		case HOST_OPTION_GAMEMODE_TUTORIAL_REBOUND_BEGINNER:
		{
			SetOption(HOST_FUNC_SHOTFAIL);
			SetOption(HOST_OPTION_ONLY_SHOT_SOLUTION);

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_REBOUND_BEGINNER, new CState_Tutorial(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_REBOUND_BEGINNER);

			ChangeState(EHOST_STATE::TUTORIAL_REBOUND_BEGINNER);
		}
		break; 

		case HOST_OPTION_GAMEMODE_TUTORIAL_BLOCK_BEGINNER:
		{
			SetOption(HOST_FUNC_SHOTFAIL);

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_BLOCK_BEGINNER, new CState_Tutorial(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_BLOCK_BEGINNER);

			ChangeState(EHOST_STATE::TUTORIAL_BLOCK_BEGINNER);
		}
		break;

		case HOST_OPTION_GAMEMODE_TUTORIAL_RIMATTACKBLOCK:
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_RIMATTACKBLOCK, new CState_Tutorial(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_RIMATTACKBLOCK);

			ChangeState(EHOST_STATE::TUTORIAL_RIMATTACKBLOCK);
		}
		break;
		case HOST_OPTION_GAMEMODE_TUTORIAL_BOXOUT:
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_BOXOUT, new CState_Tutorial(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_BOXOUT);

			ChangeState(EHOST_STATE::TUTORIAL_BOXOUT);
		}
		break;
		case HOST_OPTION_GAMEMODE_TUTORIAL_PENETRATE:
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_PENETRATE, new CState_Tutorial(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_PENETRATE);

			ChangeState(EHOST_STATE::TUTORIAL_PENETRATE);
		}
		break;
		case HOST_OPTION_GAMEMODE_TUTORIAL_SHOOTINGDISTURB:
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TUTORIAL_SHOOTINGDISTURB, new CState_Tutorial(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TUTORIAL_SHOOTINGDISTURB);

			ChangeState(EHOST_STATE::TUTORIAL_SHOOTINGDISTURB);
		}
		break;
		case HOST_OPTION_GAMEMODE_TRAINING:	// 트레이닝
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TRAINING, new CState_Training(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TRAINING);

			ChangeState(EHOST_STATE::TRAINING);
		}
		break;
		case HOST_OPTION_GAMEMODE_TRAINING_REBOUND:	// 트레이닝 리바운드
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TRAINING_REBOUND, new CState_Training(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TRAINING_REBOUND);

			ChangeState(EHOST_STATE::TRAINING_REBOUND);
		}
		break;
		case HOST_OPTION_GAMEMODE_TRAINING_JUMPSHOTBLOCK:	// 트레이닝 점프샷 블록
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TRAINING_JUMPSHOTBLOCK, new CState_Training(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TRAINING_JUMPSHOTBLOCK);

			ChangeState(EHOST_STATE::TRAINING_JUMPSHOTBLOCK);
		}
		break;
		case HOST_OPTION_GAMEMODE_TRAINING_RIMATTACKBLOCK:	// 트레이닝 림어택 블록
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TRAINING_RIMATTACKBLOCK, new CState_Training(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TRAINING_RIMATTACKBLOCK);

			ChangeState(EHOST_STATE::TRAINING_RIMATTACKBLOCK);
		}
		break;
		case HOST_OPTION_GAMEMODE_TRAINING_OFFBALLMOVE:	// 트레이닝 오프 볼 무브
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::TRAINING_OFFBALLMOVE, new CState_Training(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));

			SetModeType(EMODE_TYPE::TRAINING_OFFBALLMOVE);

			ChangeState(EHOST_STATE::TRAINING_OFFBALLMOVE);
		}
		break;
		case HOST_OPTION_GAMEMODE_CHALLENGE1:	// 점프슛 챌린지
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_INIT, new CState_Challenge_OffBallMove_Init(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_READY, new CState_Challenge_OffBallMove_Ready(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_PLAY, new CState_Challenge_OffBallMove_Play(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_END, new CState_Challenge_OffBallMove_End(this)));

			SetModeType(EMODE_TYPE::SKILL_CHALLENGE_OFF_BALL_MOVE);

			ChangeState(EHOST_STATE::CHALLENGE_INIT);
		}
		break;
		case HOST_OPTION_GAMEMODE_CHALLENGE2:	// 리바운드 챌린지
		{
			SetOption(HOST_FUNC_SHOTFAIL);

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_INIT, new CState_Challenge_Rebound_Init(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_READY, new CState_Challenge_Rebound_Ready(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_PLAY, new CState_Challenge_Rebound_Play(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_END, new CState_Challenge_Rebound_End(this)));

			SetModeType(EMODE_TYPE::SKILL_CHALLENGE_REBOUND);

			ChangeState(EHOST_STATE::CHALLENGE_INIT);
		}
		break;
		case HOST_OPTION_GAMEMODE_CHALLENGE3:	// 패스 챌린지
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_INIT, new CState_Challenge_Pass_Init(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_READY, new CState_Challenge_Pass_Ready(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_PLAY, new CState_Challenge_Pass_Play(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_END, new CState_Challenge_Pass_End(this)));

			SetModeType(EMODE_TYPE::SKILL_CHALLENGE_PASS);

			ChangeState(EHOST_STATE::CHALLENGE_INIT);
		}
		break;
		case HOST_OPTION_GAMEMODE_CHALLENGE4:	// 블락 챌린지
		{
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_INIT, new CState_Challenge_JumpShotBlock_Init(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_READY, new CState_Challenge_JumpShotBlock_Ready(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_PLAY, new CState_Challenge_JumpShotBlock_Play(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_BREAKTIME, new CState_Challenge_JumpShotBlock_BreakTime(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::CHALLENGE_END, new CState_Challenge_JumpShotBlock_End(this)));

			SetModeType(EMODE_TYPE::SKILL_CHALLENGE_JUMP_SHOT_BLOCK);

			ChangeState(EHOST_STATE::CHALLENGE_INIT);
		}
		break;
		case HOST_OPTION_GAMEMODE_TRIO:
		{
			switch (GetOption() & HOST_TIME)
			{
			case HOST_TIME_MINUTE_1: SetGameTimeInit(kHOST_TIME_MINUTE_1); break;
			case HOST_TIME_MINUTE_2: SetGameTimeInit(kHOST_TIME_MINUTE_2); break;
			case HOST_TIME_MINUTE_3: SetGameTimeInit(kHOST_TIME_MINUTE_3); break;
			case HOST_TIME_MINUTE_4: SetGameTimeInit(kHOST_TIME_MINUTE_4); break;
			case HOST_TIME_MINUTE_5: SetGameTimeInit(kHOST_TIME_MINUTE_5); break;
			case HOST_TIME_MINUTE_6: SetGameTimeInit(kHOST_TIME_MINUTE_6); break;
			case HOST_TIME_MINUTE_7: SetGameTimeInit(kHOST_TIME_MINUTE_7); break;
			case HOST_TIME_MINUTE_8: SetGameTimeInit(kHOST_TIME_MINUTE_8); break;
			case HOST_TIME_MINUTE_9: SetGameTimeInit(kHOST_TIME_MINUTE_9); break;
			case HOST_TIME_MINUTE_10: SetGameTimeInit(kHOST_TIME_MINUTE_10); break;
			default: SetGameTimeInit(kHOST_TIME_MINUTE_4); break;
			}

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::ARRANGE, new CState_Arrange(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::BREAK_TIME, new CState_BreakTime(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::INIT, new CState_Init(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::LINE_UP, new CState_LineUp(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::JUMP_BALL, new CState_JumpBall(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::LOAD, new CState_Load(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::PLAY, new CState_Play(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::READY, new CState_Ready(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCENE_START, new CState_SceneStart(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCORE, new CState_Score(this)));
			
			// 트레이닝, 챌린지 모드가 아닌경우 일반 3:3 경기로 취급한다. Redis 에서 정보를 받아오면 그 값으로 모드 설정
			SetModeType(EMODE_TYPE::TRIO);
			ChangeState(EHOST_STATE::INIT);
		}
		break;
		case HOST_OPTION_GAMEMODE_TRIO_PVP_MIXED:
		{
			switch (GetOption() & HOST_TIME)
			{
			case HOST_TIME_MINUTE_1: SetGameTimeInit(kHOST_TIME_MINUTE_1); break;
			case HOST_TIME_MINUTE_2: SetGameTimeInit(kHOST_TIME_MINUTE_2); break;
			case HOST_TIME_MINUTE_3: SetGameTimeInit(kHOST_TIME_MINUTE_3); break;
			case HOST_TIME_MINUTE_4: SetGameTimeInit(kHOST_TIME_MINUTE_4); break;
			case HOST_TIME_MINUTE_5: SetGameTimeInit(kHOST_TIME_MINUTE_5); break;
			case HOST_TIME_MINUTE_6: SetGameTimeInit(kHOST_TIME_MINUTE_6); break;
			case HOST_TIME_MINUTE_7: SetGameTimeInit(kHOST_TIME_MINUTE_7); break;
			case HOST_TIME_MINUTE_8: SetGameTimeInit(kHOST_TIME_MINUTE_8); break;
			case HOST_TIME_MINUTE_9: SetGameTimeInit(kHOST_TIME_MINUTE_9); break;
			case HOST_TIME_MINUTE_10: SetGameTimeInit(kHOST_TIME_MINUTE_10); break;
			default: SetGameTimeInit(kHOST_TIME_MINUTE_4); break;
			}

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::ARRANGE, new CState_Arrange(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::BREAK_TIME, new CState_BreakTime(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::INIT, new CState_Init(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::LINE_UP, new CState_LineUp(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::JUMP_BALL, new CState_JumpBall(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::LOAD, new CState_Load(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::PLAY, new CState_Play(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::READY, new CState_Ready(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCENE_START, new CState_SceneStart(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCORE, new CState_Score(this)));

			// 트레이닝, 챌린지 모드가 아닌경우 일반 3:3 경기로 취급한다. Redis 에서 정보를 받아오면 그 값으로 모드 설정
			SetModeType(EMODE_TYPE::TRIO_PVP_MIXED);
			ChangeState(EHOST_STATE::INIT);
		}
		break;
		case HOST_OPTION_GAMEMODE_CONTINUOUS:
		{
			SetGameTimeInit(kHOST_TIME_MINUTE_100);
			
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::ARRANGE, new CState_Arrange(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::BREAK_TIME, new CState_BreakTime(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::INIT, new CState_Init(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::LINE_UP, new CState_LineUp(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::JUMP_BALL, new CState_JumpBall(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::LOAD, new CState_Load(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::PLAY, new CState_Play(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::READY, new CState_Ready(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCENE_START, new CState_SceneStart(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCORE, new CState_Score(this)));
			
			// 트레이닝, 챌린지 모드가 아닌경우 일반 3:3 경기로 취급한다. Redis 에서 정보를 받아오면 그 값으로 모드 설정
			SetModeType(EMODE_TYPE::CONTINUOUS);
			ChangeState(EHOST_STATE::INIT);
		}
		break;
		case HOST_OPTION_GAMEMODE_CUSTOM:
		{
			switch (GetOption() & HOST_TIME)
			{
				case HOST_TIME_MINUTE_1: SetGameTimeInit(kHOST_TIME_MINUTE_1); break;
				case HOST_TIME_MINUTE_2: SetGameTimeInit(kHOST_TIME_MINUTE_2); break;
				case HOST_TIME_MINUTE_3: SetGameTimeInit(kHOST_TIME_MINUTE_3); break;
				case HOST_TIME_MINUTE_4: SetGameTimeInit(kHOST_TIME_MINUTE_4); break;
				case HOST_TIME_MINUTE_5: SetGameTimeInit(kHOST_TIME_MINUTE_5); break;
				case HOST_TIME_MINUTE_6: SetGameTimeInit(kHOST_TIME_MINUTE_6); break;
				case HOST_TIME_MINUTE_7: SetGameTimeInit(kHOST_TIME_MINUTE_7); break;
				case HOST_TIME_MINUTE_8: SetGameTimeInit(kHOST_TIME_MINUTE_8); break;
				case HOST_TIME_MINUTE_9: SetGameTimeInit(kHOST_TIME_MINUTE_9); break;
				case HOST_TIME_MINUTE_10: SetGameTimeInit(kHOST_TIME_MINUTE_10); break;
				default: SetGameTimeInit(kHOST_TIME_MINUTE_4); break;
			}

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::ARRANGE, new CState_Arrange(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::BREAK_TIME, new CState_BreakTime(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::INIT, new CState_Init(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::LINE_UP, new CState_LineUp(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::JUMP_BALL, new CState_JumpBall(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::LOAD, new CState_Load(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::PLAY, new CState_Play(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::READY, new CState_Ready(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCENE_START, new CState_SceneStart(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCORE, new CState_Score(this)));

			// 트레이닝, 챌린지 모드가 아닌경우 일반 3:3 경기로 취급한다. Redis 에서 정보를 받아오면 그 값으로 모드 설정
			SetModeType(EMODE_TYPE::CUSTOM);
			ChangeState(EHOST_STATE::INIT);
		}
		break;
		default:
		{
			switch (GetOption() & HOST_TIME)
			{
				case HOST_TIME_MINUTE_1: SetGameTimeInit(kHOST_TIME_MINUTE_1); break;
				case HOST_TIME_MINUTE_2: SetGameTimeInit(kHOST_TIME_MINUTE_2); break;
				case HOST_TIME_MINUTE_3: SetGameTimeInit(kHOST_TIME_MINUTE_3); break;
				case HOST_TIME_MINUTE_4: SetGameTimeInit(kHOST_TIME_MINUTE_4); break;
				case HOST_TIME_MINUTE_5: SetGameTimeInit(kHOST_TIME_MINUTE_5); break;
				case HOST_TIME_MINUTE_6: SetGameTimeInit(kHOST_TIME_MINUTE_6); break;
				case HOST_TIME_MINUTE_7: SetGameTimeInit(kHOST_TIME_MINUTE_7); break;
				case HOST_TIME_MINUTE_8: SetGameTimeInit(kHOST_TIME_MINUTE_8); break;
				case HOST_TIME_MINUTE_9: SetGameTimeInit(kHOST_TIME_MINUTE_9); break;
				case HOST_TIME_MINUTE_10: SetGameTimeInit(kHOST_TIME_MINUTE_10); break;
				default: SetGameTimeInit(kHOST_TIME_MINUTE_4); break;
			}

			m_States.insert(HostStateMapType::value_type(EHOST_STATE::ARRANGE, new CState_Arrange(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::NONE, new CState(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::BREAK_TIME, new CState_BreakTime(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::END, new CState_End(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::INIT, new CState_Init(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::LINE_UP, new CState_LineUp(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::JUMP_BALL, new CState_JumpBall(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::LOAD, new CState_Load(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::PLAY, new CState_Play(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::READY, new CState_Ready(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::RESULT, new CState_Result(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCENE_START, new CState_SceneStart(this)));
			m_States.insert(HostStateMapType::value_type(EHOST_STATE::SCORE, new CState_Score(this)));
			
			// 트레이닝, 챌린지 모드가 아닌경우 일반 3:3 경기로 취급한다. Redis 에서 정보를 받아오면 그 값으로 모드 설정
			SetModeType(EMODE_TYPE::THREE_ON_THREE);
			SetIsNormalGame(true);
			ChangeState(EHOST_STATE::INIT);
		}
		break;
	}

	if (m_CharacterManager != nullptr)
	{
		m_CharacterManager->CharacterManagerInit(this, GetBalanceTable(), GetAnimationController(), BallControllerGet());
	}


	ResetShotClock();

	m_GameCap = kINT32_INIT;
	m_AliveCheckTime = kALIVE_CHECK_TIME_INIT;
	m_AliveCheckIndex = kINT32_INIT;
	m_AlivePingMaxCnt = kMAX_PING_LATENCY_DEQUE;

	m_ListBadConnectUser.clear();
	m_ListBadConnectUserCheck.clear();
	m_LoginCompleteUser.clear();
	m_SendLoadCompleteUser.clear();
	m_ReConnectUser.clear();
	m_HostJoinUser.clear();

	m_VecAiHostCandidateUser.clear();
	m_VecCallbackEnetDisConnectUser.clear();

	m_BallShotHindrance.clear();

	m_Key = kINT32_INIT;
	m_KeyStr = "";
}

void CHost::Release()
{
	//m_ShotSolutionTable.Release();

	SAFE_DELETE(m_pEnvironmentObject);

	if (false == m_States.empty())
	{
		for (auto& it : m_States)
		{
			auto pState = it.second;

			SAFE_DELETE(pState);
		}
		m_States.clear();
	}

	if (false == m_HostUserMap.empty())
	{
		for (auto& it : m_HostUserMap)
		{
			auto pUser = it.second;

			SAFE_DELETE(pUser);
		}
		m_HostUserMap.clear();
	}

	if (m_CharacterManager != nullptr)
	{
		SAFE_DELETE(m_CharacterManager);
	}

	if (m_GameManager != nullptr)
	{
		SAFE_DELETE(m_GameManager);
	}

	if (m_TimeManager != nullptr)
	{
		SAFE_DELETE(m_TimeManager);
	}

	if (m_DevManager != nullptr)
	{
		SAFE_DELETE(m_DevManager);
	}

	if (m_VerifyManager != nullptr)
	{
		SAFE_DELETE(m_VerifyManager);
	}

	SAFE_DELETE(m_AnimationController);
	SAFE_DELETE(m_ShotSolution);
	SAFE_DELETE(m_pBalance);

	SAFE_DELETE(m_PhysicsHandler);
	SAFE_DELETE(m_pBallController);
}


CState* CHost::GetState(EHOST_STATE state)
{
	auto it = m_States.find(state);
	if (it == m_States.end())
		throw std::out_of_range("Invalid state in GetState");

	return it->second;
}

DHOST_TYPE_HOST_ID CHost::GetHostID()
{
	return m_HostID;
}

void CHost::Update(float timeDelta)
{
	m_pBallController->Update(timeDelta);

	if (m_pEnvironmentObject != nullptr)
	{
		m_pEnvironmentObject->Update(timeDelta);
	}

	m_States[m_CurrentState]->OnUpdate(timeDelta);

	//if (m_CurrentState == EHOST_STATE::PLAY || m_CurrentState == EHOST_STATE::CHALLENGE_PLAY)
	{
		m_CharacterManager->CharacterManagerUpdate(timeDelta, GetGameTime(), GetRoomElapsedTime(), m_pBallController->GetBallPosition());
		m_CharacterManager->CharacterManagerLateUpdateEx(timeDelta);
	}
	
	m_AliveCheckTime -= timeDelta;

	if (m_AliveCheckTime <= kFLOAT_INIT && m_HostUserMap.size() > 0 && GetRedisSaveMatchResult() == false)
	{
		m_AliveCheckTime = kALIVE_CHECK_TIME_INIT;

		SendClientAliveCheck();
	}

	// 기권하기 체크
	CheckTeamSurrenderActivation(timeDelta, GetGameTime());
}

static std::string ToPrintableAscii(const uint8_t* data, int len)
{
	std::string out;
	out.reserve(len);
	for (int i = 0; i < len; ++i)
	{
		uint8_t c = data[i];
		if (c >= 32 && c <= 126) out.push_back((char)c);
		else out.push_back('.');
	}
	return out;
}

// 유틸: 앞부분 hex dump (최대 maxBytes)
static std::string HexPreview(const uint8_t* data, int len, int maxBytes = 32)
{
	int n = (len < maxBytes) ? len : maxBytes;
	char buf[512];
	int pos = 0;
	for (int i = 0; i < n; ++i)
	{
		pos += snprintf(buf + pos, sizeof(buf) - pos, "%02X ", data[i]);
		if (pos >= (int)sizeof(buf) - 4) break;
	}
	if (len > n) pos += snprintf(buf + pos, sizeof(buf) - pos, "...(+%d)", len - n);
	return std::string(buf);
}

static inline uint16_t ReadBE16(const uint8_t* p)
{
	return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}

static inline uint32_t ReadBE32(const uint8_t* p)
{
	return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}


void CHost::ProcessPacketWithMagic(const char* pData, DHOST_TYPE_INT32 size, void* peer)
{
	uint16_t MAGIC = m_PacketGuardRule.magic;
	uint32_t WATERMARK_MASK = m_PacketGuardRule.watermarkMask;
	uint32_t WATERMARK_RESULT = m_PacketGuardRule.watermarkResult;
	// (RESULT & ~MASK) == 0 여야 함

	if (pData == nullptr || size <= 0)
		return;


	bool hasMagic = false;

	const uint8_t* u = reinterpret_cast<const uint8_t*>(pData);

	DHOST_TYPE_USER_ID userID = UserGetID(peer);

	// === 기본(원본) 포인터/사이즈를 "일반 처리용"으로 보관 ===
	const char* parseData = pData;
	DHOST_TYPE_INT32 parseSize = size;

	// === 1) MAGIC+워터마크 헤더가 "있는 경우에만" 검사/스킵 ===
	// 최소 6바이트가 있고, 앞 2바이트가 MAGIC이면 워터마크 검사 시도
	if (size >= 6)
	{
		uint16_t magic = ReadBE16(u);

		if (magic == MAGIC)
		{
			hasMagic = true;

			uint32_t dynData = ReadBE32(u + 2); // 2바이트 뒤부터 4바이트를 읽는 다는 뜻임 

			if ((dynData & WATERMARK_MASK) == WATERMARK_RESULT)
			{
				parseData = pData + 6;
				parseSize = size - 6;
			}
			else
			{
				// ❗ MAGIC은 있는데 워터마크가 틀림
				// 여기 정책은 선택인데, 보통은 "차단"이 맞음 (위조/오염 가능성)
				ToLog("[PACKET_ERROR] invalid dynamic watermark");
				KickToTheLobby(userID, F4PACKET::EKICK_TYPE::version_kick);
				return;

				// 만약 이것도 일반처리로 넘기고 싶으면 return 대신 아래처럼:
				// ToLog("[PKT_WARN] watermark mismatch, fallback to normal parse");
			}
		}
		else
		{
			ToLog("[PACKET_ERROR] invalid dynamic watermark");
			//KickToTheLobby(userID, F4PACKET::EKICK_TYPE::version_kick);
			return;
		}
	}

	// magic != MAGIC 이면 그냥 일반 처리로 진행 (fallback)
	// 1. 이전 버전의 유저는 이미 방화벽에 매직이 없어서 게임을 할 수없다.
	// 2. 정상적인 버전의 유저의 패킷 중 빠트린 것들은 무사히 넘어간다. 그럴 가능성은없음. 현재 skynetkey 만 발견이 됐으나 여기에 들어올지는 모르겠음 

	if (!hasMagic)
	{
		string log_message = "Magic value no have : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(userID);
		ToLog(log_message.c_str());
	}

	// === 2) 아래부터는 "parseData/parseSize"로 기존 코드 그대로 ===
	FlatBufProtocol::Header header_info;

	if (!FlatBufProtocol::GetHeadInfo(header_info, parseData, parseSize))
	{
		string str_log = "[PACKET_ERROR] packet_header parse fail";
		ToLog(str_log.c_str());
		return;
	}

	CPacketImplement<F4PACKET::PACKET_ID>* pPacket = nullptr;

	auto result = FlatBufProtocol::GetPacket(pPacket, parseData, parseSize);

	if (result != FlatBufProtocol::ERESULT::SUCCESS)
	{
		string str_log = "[PACKET_ERROR] result : " + std::to_string((int)result) +
			", PacketID : " + std::to_string(header_info.packetID) +
			", UserID : " + std::to_string(userID);
		ToLog(str_log.c_str());
		return;
	}

	if (nullptr == pPacket)
	{
		ToLog("[PACKET_ERROR] pPacket is null");
		return;
	}

	pPacket->SetUserID(userID);

	CHostUserInfo* pUser = FindUser(userID);

	if (pUser != nullptr && pUser->GetUserType() == EUSER_TYPE::OBSERVER)
	{
		OnPacketObserverUser(pPacket, peer, GetRoomElapsedTime());
		SAFE_DELETE(pPacket);
		return;
	}

	IncreasePacketReceiveCount(header_info.packetID, parseSize);

	m_TimeManager->SetLastPacketReceiveTime(GetRoomElapsedTime());

	m_States[m_CurrentState]->OnPacket(pPacket, peer, GetRoomElapsedTime());
	m_States[m_CurrentState]->OnPacket(pPacket, parseData, parseSize, peer, GetRoomElapsedTime());

	SAFE_DELETE(pPacket);
}


void CHost::ProcessPacketWaterMarkOnly(const char* pData, DHOST_TYPE_INT32 size, void* peer)
{
	const uint16_t MAGIC = m_PacketGuardRule.magic;
	const uint32_t WATERMARK_MASK = m_PacketGuardRule.watermarkMask;
	const uint32_t WATERMARK_RESULT = m_PacketGuardRule.watermarkResult;
	// (RESULT & ~MASK) == 0 여야 함

	if (pData == nullptr || size <= 0)
		return;

	const uint8_t* u = reinterpret_cast<const uint8_t*>(pData);
	DHOST_TYPE_USER_ID userID = UserGetID(peer);

	// === 기본(원본) 포인터/사이즈를 "일반 처리용"으로 보관 ===
	const char* parseData = pData;
	DHOST_TYPE_INT32 parseSize = size;

	// ===== 정책: 레거시 MAGIC 포맷 허용 여부 (과도기면 true, 완전 전환이면 false) =====
	constexpr bool ACCEPT_LEGACY_MAGIC = true;

	// === 1) 헤더 판별 & 워터마크 검사 ===
	// 기본(신규): [DYN 4][payload...]
	// 레거시:   [MAGIC 2][DYN 4][payload...]

	bool headerOk = false;

	if (size >= 4)
	{
		// 1-A) 레거시 MAGIC 포맷인지 먼저 확인 (옵션)
		if (ACCEPT_LEGACY_MAGIC && size >= 6)
		{
			uint16_t magic = ReadBE16(u);
			if (magic == MAGIC)
			{
				uint32_t dynData = ReadBE32(u + 2); // MAGIC 뒤 4바이트

				if ((dynData & WATERMARK_MASK) == WATERMARK_RESULT)
				{
					headerOk = true;
					parseData = pData + 6;
					parseSize = size - 6;
				}
				else
				{
					ToLog("[PACKET_ERROR] invalid dynamic watermark (legacy magic header)");
					KickToTheLobby(userID, F4PACKET::EKICK_TYPE::version_kick);
					return;
				}
			}
		}

		// 1-B) 신규 워터마크-only 포맷 처리
		if (!headerOk)
		{
			uint32_t dynData = ReadBE32(u); // 맨 앞 4바이트

			if ((dynData & WATERMARK_MASK) == WATERMARK_RESULT)
			{
				headerOk = true;
				parseData = pData + 4;
				parseSize = size - 4;
			}
			else
			{
				ToLog("[PACKET_ERROR] invalid dynamic watermark (watermark-only header)");
				// 필요하면 킥, 아니면 조용히 drop
				//KickToTheLobby(userID, F4PACKET::EKICK_TYPE::version_kick);
				return;
			}
		}
	}
	else
	{
		// 최소 헤더(4B)도 없음
		ToLog("[PACKET_ERROR] packet too small for watermark header");
		return;
	}

	// 여기까지 왔으면 parseData/parseSize는 FlatBuffers payload 시작을 가리킴

	// === 2) 아래부터는 "parseData/parseSize"로 기존 코드 그대로 ===
	FlatBufProtocol::Header header_info;

	if (!FlatBufProtocol::GetHeadInfo(header_info, parseData, parseSize))
	{
		ToLog("[PACKET_ERROR] packet_header parse fail");
		return;
	}

	CPacketImplement<F4PACKET::PACKET_ID>* pPacket = nullptr;

	auto result = FlatBufProtocol::GetPacket(pPacket, parseData, parseSize);
	if (result != FlatBufProtocol::ERESULT::SUCCESS)
	{
		std::string str_log = "[PACKET_ERROR] result : " + std::to_string((int)result) +
			", PacketID : " + std::to_string(header_info.packetID) +
			", UserID : " + std::to_string(userID);
		ToLog(str_log.c_str());
		return;
	}

	if (nullptr == pPacket)
	{
		ToLog("[PACKET_ERROR] pPacket is null");
		return;
	}

	pPacket->SetUserID(userID);

	CHostUserInfo* pUser = FindUser(userID);
	if (pUser != nullptr && pUser->GetUserType() == EUSER_TYPE::OBSERVER)
	{
		OnPacketObserverUser(pPacket, peer, GetRoomElapsedTime());
		SAFE_DELETE(pPacket);
		return;
	}

	IncreasePacketReceiveCount(header_info.packetID, parseSize);
	m_TimeManager->SetLastPacketReceiveTime(GetRoomElapsedTime());

	m_States[m_CurrentState]->OnPacket(pPacket, peer, GetRoomElapsedTime());
	m_States[m_CurrentState]->OnPacket(pPacket, parseData, parseSize, peer, GetRoomElapsedTime());

	SAFE_DELETE(pPacket);
}

void CHost::ProcessPacket(const char* pData, DHOST_TYPE_INT32 size, void* peer)
{
	FlatBufProtocol::Header header_info;

	DHOST_TYPE_USER_ID userID = UserGetID(peer);

	if (!FlatBufProtocol::GetHeadInfo(header_info, pData, size))
	{
		string str_log = "[PACKET_ERROR] packet_header parse fail";
		ToLog(str_log.c_str());

		return;
	}

	CPacketImplement<F4PACKET::PACKET_ID>* pPacket = nullptr;

	auto result = FlatBufProtocol::GetPacket(pPacket, pData, size);

	if (result != FlatBufProtocol::ERESULT::SUCCESS)
	{
		string str_log = "[PACKET_ERROR] result : " + std::to_string((int)result) + ", PacketID : " + std::to_string(header_info.packetID) + ", UserID : " + std::to_string(userID);
		ToLog(str_log.c_str());

		return;
	}

	if (nullptr == pPacket)
	{
		string str_log = "[PACKET_ERROR] pPacket is null";
		ToLog(str_log.c_str());

		return;
	}

	pPacket->SetUserID(userID);

	CHostUserInfo* pUser = FindUser(userID);

	if (pUser != nullptr && pUser->GetUserType() == EUSER_TYPE::OBSERVER)
	{
		OnPacketObserverUser(pPacket, peer, GetRoomElapsedTime());
		SAFE_DELETE(pPacket);
		return;
	}
		
	IncreasePacketReceiveCount(header_info.packetID, size);

//	 디도스 공격으로 초당 패킷을 얼마나 보내는지 알아보기 위해 만들었던 코드, 이거 일랩스드 타임을 이용해서 만들수도 있는데. 
//	if ((F4PACKET::PACKET_ID)pPacket->GetPacketID() == F4PACKET::PACKET_ID::play_c2s_playerMove || 
//		(F4PACKET::PACKET_ID)pPacket->GetPacketID() == F4PACKET::PACKET_ID::play_c2s_playerPositionCorrect )
//	{
//		string str = "[play_c2s_playerMove] RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) 
//			+ ", UserID : " + std::to_string(userID) + 
//			+",  PacketSum : " + std::to_string(count) +
//			", PacketID : " + std::string(F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)pPacket->GetPacketID()));
//		ToLog(str.c_str());

//		count++;
//	}
	
	
	
//	if ((F4PACKET::PACKET_ID)pPacket->GetPacketID() != F4PACKET::PACKET_ID::system_s2s_clientAliveCheck && (F4PACKET::PACKET_ID)pPacket->GetPacketID() != F4PACKET::PACKET_ID::system_c2c_ping)
//	{
//		string str = "[PACKET_RECEIVE] RoomElapsedTime : " + std::to_string(GetRoomElapsedTime())+ ", UserID : " + std::to_string(userID) + ", PacketID : " + std::string(F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)pPacket->GetPacketID()));
//		ToLog(str.c_str());
//	}
	

//#ifdef TEXT_LOG_FILE
//	LOGGER->Log("UserID : %s, PacketID : %d", GetHostID().c_str(), userID, pPacket->GetPacketID());
//#endif

	m_TimeManager->SetLastPacketReceiveTime(GetRoomElapsedTime());

	//string str = "[MESSAGE_LOOP_CHECK] GetLastPacketReceiveTime : " + std::to_string(m_TimeManager->GetLastPacketReceiveTime()) + ", PacketID : " + std::string(F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)pPacket->GetPacketID())) + ", UserID : " + std::to_string(userID);
	//ToLog(str.c_str());

	m_States[m_CurrentState]->OnPacket(pPacket, peer, GetRoomElapsedTime());		// play_c2s_ballRebound 여기서는 안함 
	m_States[m_CurrentState]->OnPacket(pPacket, pData, size, peer, GetRoomElapsedTime()); // 여기에서만 함 

	SAFE_DELETE(pPacket);
}


DHOST_TYPE_INT32 CHost::SendPacket(CPacketBase& packet, const DHOST_TYPE_USER_ID UserID)
{
	char* pSendBuffer = m_PacketDataBuffer;

	int buff_size = 0;

	packet.SetPacketType(kPACKET_TYPE_ALL_WORK);

	{
		FlatBufProtocol::Encode(&packet , &pSendBuffer, buff_size, malloc, free);
	}

	/*if ((F4PACKET::PACKET_ID)packet.GetPacketID() != F4PACKET::PACKET_ID::system_s2s_clientAliveCheck && (F4PACKET::PACKET_ID)packet.GetPacketID() != F4PACKET::PACKET_ID::system_c2c_ping)
	{
		string str = "[SEND Packet] RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", PacketID : " + std::string(F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packet.GetPacketID())) + ", UserID : " + std::to_string(UserID);
		ToLog(str.c_str());
	}*/

	// InGameServer
	if (m_CallbackCpp != nullptr)
	{
		if (UserID != kUSER_ID_INIT)	// Personal Packet
		{
			
			m_CallbackCpp(pSendBuffer, buff_size, UserID);
		}

		return 0;
	}

	if (m_CallbackBroadcast != nullptr)
	{
		for (auto& it : m_HostUserMap)
		{
			auto pUser = it.second;
			if (nullptr == pUser)
			{
				continue;
			}

			if (UserID == it.first)
			{
				HostMessage hostMessage;

				hostMessage.data = pSendBuffer;
				hostMessage.size = buff_size;
				hostMessage.userID = it.first;
				hostMessage.peer = it.second->GetPeer();

				m_CallbackBroadcast(&hostMessage);

				return 0;
			}
		}
	}

	string log_message = "[SEND_PACKET_ERROR] PakcetID : " + to_string(packet.GetPacketID()) + ", UserID : " + to_string(UserID);
	ToLog(log_message.c_str());

	return 0;
}

DHOST_TYPE_INT32 CHost::BroadcastPacket(CPacketBase& packet, const DHOST_TYPE_USER_ID UserID)
{
	char* pSendBuffer = m_PacketDataBuffer;

	int buff_size = 0;

	packet.SetPacketType(kPACKET_TYPE_ALL_WORK);

	{
		FlatBufProtocol::ERESULT result = FlatBufProtocol::Encode(&packet, &pSendBuffer, buff_size, malloc, free);
		if (FlatBufProtocol::ERESULT::SUCCESS != result)
		{
			string log_message = "[SendPacket Encode Fail] Result : " + (string)ResultName(result);
			ToLog(log_message.c_str());
			return 0;
		}
	}

	/*if ((F4PACKET::PACKET_ID)packet.GetPacketID() != F4PACKET::PACKET_ID::system_s2s_clientAliveCheck && (F4PACKET::PACKET_ID)packet.GetPacketID() != F4PACKET::PACKET_ID::system_c2c_ping)
	{
		string str = "[BROARD_CAST_PACKET] RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", PacketID : " + std::string(F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packet.GetPacketID())) + ", UserID : " + std::to_string(UserID);
		ToLog(str.c_str());
	}*/


	// InGameServer
	if (m_CallbackCpp != nullptr)
	{
		for (auto& it : m_HostUserMap)	// Broadcast Packet
		{
			auto pUser = it.second;
			if (nullptr == pUser)
			{
				continue;
			}

			if (UserID == it.first)
			{
				continue;
			}

			if (pUser->GetConnectState() != ECONNECT_STATE::CONNECT)
			{
				continue;
			}

			if (pUser->GetUserType() == EUSER_TYPE::OBSERVER && pUser->GetSpectatorLoadComplete() == false)
			{
				continue;
			}

			m_CallbackCpp(pSendBuffer, buff_size, it.first);
		}
		return 0;
	}

	// HostTestServer
	if (m_CallbackBroadcast != nullptr)
	{
		for (auto& it : m_HostUserMap)
		{
			auto pUser = it.second;
			if (nullptr == pUser)
			{
				continue;
			}

			if (UserID == it.first)
			{
				continue;
			}

			if (pUser->GetConnectState() != ECONNECT_STATE::CONNECT)
			{
				continue;
			}

			HostMessage hostMessage;

			hostMessage.data = pSendBuffer;
			hostMessage.size = buff_size;
			hostMessage.userID = it.first;
			hostMessage.peer = it.second->GetPeer();

			m_CallbackBroadcast(&hostMessage);
		}
	}

	return 0;
}

void CHost::RegistCallbackFunc(ProcessPacketCallbackToCpp pFunc)
{
	m_CallbackCpp = std::move(pFunc);
}

void CHost::RegistCallbackFuncRedis(RedisCallback pFunc)
{
	m_CallbackRedis = std::move(pFunc);
}

void CHost::RegistCallbackFuncRedisSend(RedisSendCallback pFunc)
{
	m_CallbackRedisSend = std::move(pFunc);
}

void CHost::ToLog(const char* msg)
{
    std::string safeMsg = (msg == nullptr) ? "<null>" : msg;
    std::string line = "[RoomElapsed=" + std::to_string(GetRoomElapsedTime()) + "] " + safeMsg;
    AppendHostFileLog(line);

	if (m_CallbackLog != nullptr)
	{
		m_CallbackLog(msg);
	}
}

void CHost::ToLog(const char* msg, LOG_TYPE logType)
{
	switch (logType)
	{
		case LOG_TYPE::LOG_POTENTIAL:
			return;
			//break;
		case LOG_TYPE::LOG_SPEEDHACK:

		default:

			break;
	}

	if (m_CallbackLog != nullptr)
	{
		m_CallbackLog(msg);
	}

    std::string safeMsg = (msg == nullptr) ? "<null>" : msg;
    std::string line = "[RoomElapsed=" + std::to_string(GetRoomElapsedTime()) + "] " + safeMsg;
    AppendHostFileLog(line);
}

void CHost::ToLogInvalidAnimation(F4PACKET::SPlayerInformationT* pInfo, DHOST_TYPE_INT32 AniIndex, DHOST_TYPE_INT32 Result, F4PACKET::PACKET_ID PacketID)
{
	if (pInfo != nullptr)
	{
		std::string str = "RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", CharName : " + pInfo->name + ", AniIndex : " + std::to_string(AniIndex) + ", result : " + std::to_string(Result) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID(PacketID);
		ToLog(str.c_str());
	}
}

// 게임상태를 진행시키기전에 로드완료된 유저중에 연결불량 유저가 있는지 확인해서 로드완료 리스트에서 제거해준다.
void CHost::CheckGameProcessWithBadConnectUser()
{
	if (m_CurrentState == EHOST_STATE::LOAD && m_ListBadConnectUser.size() > 0)
	{
		HostBadConnectUserListType::iterator iterBadConnectUserID;

		for (iterBadConnectUserID = m_ListBadConnectUser.begin(); iterBadConnectUserID != m_ListBadConnectUser.end(); iterBadConnectUserID++)
		{
			DHOST_TYPE_USER_ID BadConnectUserID = *iterBadConnectUserID;

			std::vector<DHOST_TYPE_USER_ID>::iterator iter;
			for (iter = m_SendLoadCompleteUser.begin(); iter != m_SendLoadCompleteUser.end(); iter++)
			{
				if (BadConnectUserID == *iter)
				{
					string str = "CheckGameProcessWithBadConnectUser RoomElapsedTime : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(BadConnectUserID);
					ToLog(str.c_str());

					m_SendLoadCompleteUser.erase(iter);
					break;
				}
			}
		}
	}
}

void CHost::GenerateKey()
{
	if (m_Key == kINT32_INIT)
	{
		/*std::vector<F4PACKET::SPlayerInformationT*> vecCharacters;
		vecCharacters.clear();

		GetCharacterInformationVec(vecCharacters);

		if (vecCharacters.empty() == false)
		{
			for (int i = 0; i < vecCharacters.size(); ++i)
			{
				SPlayerInformationT* pInfo = vecCharacters[i];
				if (pInfo != nullptr)
				{
					m_Key += pInfo->characterid;
				}
			}
		}	*/

		m_Key = ParseHostIdToInt(m_HostID);

	}
}

void CHost::GenerateKeyStr()
{
	if (m_KeyStr == "")
	{

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
		m_KeyStr = m_HostID; // 로컬모드는 방번호로만 

		return;
#endif

		int roomid = kINT32_INIT;

		try
		{
			roomid = ParseHostIdToInt(m_HostID);
		}
		catch (std::exception& e)
		{
			ToLog(e.what());
		}

		if (roomid > kINT32_INIT)
		{
			int step_one = roomid % 2;
			int step_two = roomid % 100;

			if (step_one == 1)	//! 방 번호가 홀수면 밸런스 데이터
			{
				m_pBalance->GetIndexName(step_two, m_KeyStr);
			}
			else	//! 방 번호가 짝수면 애니메이션 데이터
			{
				m_AnimationController->GetIndexName(step_two, m_KeyStr);
			}

			string key_log = "roomid : " + std::to_string(roomid) + ", idx : " + std::to_string(step_two) + ", Name : " + m_KeyStr;
			ToLog(key_log.c_str());
		}
	}
}

DHOST_TYPE_INT32 CHost::GetKey()
{
	return m_Key;
}

DHOST_TYPE_STR CHost::GetKeyStr()
{
	return m_KeyStr;
}

DHOST_TYPE_BOOL	CHost::VerifyHMAC(DHOST_TYPE_INT32 value, const flatbuffers::Vector<int8_t>* hmac)
{
	DHOST_TYPE_BOOL result = false;

//#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
	string str_client_hmac = "";
	string str_server_hmac = "";

	unsigned char recv_hmac[32];

	if (hmac != nullptr)
	{
		for (int i = 0; i < (int)hmac->size(); ++i)
		{
			recv_hmac[i] = (unsigned char)hmac->Get(i);
			str_client_hmac += std::to_string(recv_hmac[i]);
		}

		ToLog(str_client_hmac.c_str());

		unsigned char host_key[sizeof(int)];
		unsigned int host_key_len = sizeof(int);
		int host_key_data = GetKey();
		memcpy(host_key, (unsigned char*)&host_key_data, host_key_len);

		unsigned char origin[sizeof(int)];
		unsigned int origin_len = sizeof(int);
		int origin_data = value;
		memcpy(origin, (unsigned char*)&origin_data, origin_len);

		string check_hmac = "key : " + std::to_string(GetKey()) + ", data : " + std::to_string(value);
		ToLog(check_hmac.c_str());

		auto digest = BuildDigest(host_key, host_key_len, origin, origin_len);

		for (size_t i = 0; i < digest.size(); ++i)
		{
			str_server_hmac += std::to_string(digest[i]);
		}

		ToLog(str_server_hmac.c_str());

		if (str_client_hmac.compare(str_server_hmac) == 0)
		{
			result = true;
		}
	}
//#endif
	
	return result;
}

void CHost::GenerateHMAC(DHOST_TYPE_INT32 value, std::vector<int8_t>& hmac)
{
//#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
	hmac.clear();

	unsigned char host_key[sizeof(int)];
	unsigned int host_key_len = sizeof(int);
	int host_key_data = GetKey();
	memcpy(host_key, (unsigned char*)&host_key_data, host_key_len);

	unsigned char send_origin[sizeof(int)];
	unsigned int send_origin_len = sizeof(int);
	int send_origin_data = value;
	memcpy(send_origin, (unsigned char*)&send_origin_data, send_origin_len);

	auto digest = BuildDigest(host_key, host_key_len, send_origin, send_origin_len);

	for (size_t i = 0; i < digest.size(); ++i)
	{
		hmac.push_back(digest[i]);
	}
//#endif
	
}

void CHost::GenerateHMAC(string& data, string& hmac)
{
	const char* pData = data.c_str();
	unsigned int pDataSize = strlen(data.c_str());

	auto digest = BuildDigest(
		reinterpret_cast<const uint8_t*>(GetKeyStr().c_str()),
		GetKeyStr().length(),
		reinterpret_cast<const uint8_t*>(pData),
		pDataSize);

	for (size_t i = 0; i < digest.size(); ++i)
	{
		hmac += std::to_string(digest[i]);
	}
}

void CHost::VerifyAction(const SPlayerAction* pInfo, string& str)
{
	DHOST_TYPE_FLOAT calc_truncf = kFLOAT_INIT;

	DHOST_TYPE_INT32 calc_bool = 0;

	//! 1. id
	str += std::to_string(pInfo->id());

	//! 2. keys
	str += std::to_string(pInfo->keys());

	//! 3. number
	str += std::to_string(pInfo->number());

	//! 4. directioninput
	calc_truncf = truncf(pInfo->directioninput());
	str += std::to_string((int)calc_truncf);

	//! 5. positionlogic
	calc_truncf = truncf(pInfo->positionlogic().x());
	str += std::to_string((int)calc_truncf);

	calc_truncf = truncf(pInfo->positionlogic().y());
	str += std::to_string((int)calc_truncf);

	calc_truncf = truncf(pInfo->positionlogic().z());
	str += std::to_string((int)calc_truncf);

	//! 6. yawlogic
	calc_truncf = truncf(pInfo->yawlogic());
	str += std::to_string((int)calc_truncf);

	//! 7. collisionpriority
	str += std::to_string(pInfo->collisionpriority());

	//! 8. skillindex
	str += std::to_string((int)pInfo->skillindex());

	//! 9. signature
	calc_bool = (int)pInfo->signature();
	str += std::to_string(calc_bool);
}


bool CHost::Sha256FromMemory_OpenSSL(const uint8_t* data, size_t length, std::array<uint8_t, 32>& out)
{
	if (!data || length == 0)
		return false;

	out = BuildDigest(nullptr, 0, data, length);

	return true;
}

bool CHost::CompareWithClientHash(const uint8_t clientHash[32], uint32_t clientSize, std::array<uint8_t, 32> severSha256, uint32_t serverSize )
{
	if (!clientHash) return false;

	if (clientSize != serverSize)
		return false;

	return std::memcmp(clientHash, severSha256.data(), 32) == 0;
}

DHOST_TYPE_INT32 CHost::VerifyVersionPacket(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_USER_ID userid)
{
	DHOST_TYPE_INT32 result = 1;
	DHOST_TYPE_INT32 version = -2;

	string convert_data_str = "";
	string client_hmac = "";
	string server_hmac = "";

	GenerateKey();
	GenerateKeyStr();

	switch ((F4PACKET::PACKET_ID)packetID)
	{
		case F4PACKET::PACKET_ID::system_c2s_versionInfo:
		{
			if (pData == nullptr)
			{
				return 0;
			}

			// 캐스팅 전에 FlatBuffer의 최소 사이즈 검증 (선택 사항)
			CFlatBufPacket<F4PACKET::system_c2s_versionInfo_data>* pPacket = nullptr;

			try
			{
				pPacket = static_cast<CFlatBufPacket<F4PACKET::system_c2s_versionInfo_data>*>(pData);

				// GetData 내부에서도 nullptr 체크
				auto* data = pPacket->GetData();
				if (data == nullptr)
				{
					return 0;
				}

				// FlatBuffer 검증
				flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
				if (!data->Verify(packet_verify))
				{
					return 0;
				}

				// 1.밸런스파일 해쉬 값 비교 
				const auto* hashBalanceVec = data->balancedatahash();
				if (!hashBalanceVec || hashBalanceVec->size() != 32)
				{
					return -3;
				}

				const uint8_t* clientBalanceHash = reinterpret_cast<const uint8_t*>(hashBalanceVec->data());

				if (!CompareWithClientHash(
					clientBalanceHash,
					hashBalanceVec->size(),
					m_serverBalanceDataHash,
					32))
				{
					// 무결성 실패
					return -3;
				}


				// 2. 애님데이터 해쉬 값 비교 
				
				const auto* hashAnimDataVec = data->animdatahash();
				if (!hashAnimDataVec || hashAnimDataVec->size() != 32)
				{
					return -4; // 애님데이터 비교 실패 
				}

				const uint8_t* clientAnimDataHash = reinterpret_cast<const uint8_t*>(hashAnimDataVec->data());

				if (!CompareWithClientHash(
					clientAnimDataHash,
					hashAnimDataVec->size(),
					m_serverAnimDataHash,
					32))
				{
					// 무결성 실패
					return -4; // 애님데이터 비교 실패 
				}
				

				if (HOST_VERSIONINFO != data->version())
				{
					return -1;
				}

				/*
				// 이후 HMAC 비교 로직
				GetClientHMAC(data->snrnsisj(), client_hmac);
				convert_data_str += std::to_string(GetTypeConvert(data->userid()));
				convert_data_str += std::to_string(GetTypeConvert(data->version()));
				GenerateHMAC(convert_data_str, server_hmac);
				if (client_hmac.compare(server_hmac) == 0)
				{
					result = 1;
				}

				if (result == -2)
				{
					IncreasePacketTamper(data->userid());
				}
				*/
			}
			catch (...)
			{
				// 잘못된 캐스팅이나 예외 상황을 여기서 처리
				return 0;
			}

			break;
		}
	}

	return result;
}


void CHost::ForceActionStand(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_USER_ID userid, EFORCE_ACTION_STAND_TYPE type)
{
	switch ((F4PACKET::PACKET_ID)packetID)
	{
		case F4PACKET::PACKET_ID::play_c2s_playerPenetrateReady:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPenetrateReady_data>*)pData;
			auto* data = pPacket->GetData();

			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr)
			{
				return;
			}

			{
				string verifyLog = "[HACK_CHECK] FORCE_ACTION_STAND RoomElapsedTime : " + std::to_string(GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
					+ ", VerifyType : " + F4PACKET::EnumNameEFORCE_ACTION_STAND_TYPE(type)
					+ ", SKill : " + F4PACKET::EnumNameSKILL_INDEX(data->playeraction()->skillindex())
					+ ", PosX : " + std::to_string(data->playeraction()->positionlogic().x())
					+ ", PosZ : " + std::to_string(data->playeraction()->positionlogic().z());
				ToLog(verifyLog.c_str());
			}

			F4PACKET::play_c2s_playerPenetrateReady_dataT dataT;
			data->UnPackTo(&dataT);
			F4PACKET::play_c2s_playerPenetrateReady_dataT* pData = &dataT;
			
			pData->playeraction->mutate_number(pData->playeraction->number() + 1);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
			send_data.add_playeraction(pData->playeraction.get());
			send_data.add_runmode(MOVE_MODE::pivot);
			send_data.add_lookid(kINT32_INIT);
			send_data.add_yawdest(kFLOAT_INIT);
			send_data.add_lefthand(false);
			send_data.add_ismirroranim(data->ismirroranim());
			send_data.add_normal(true);
			STORE_FBPACKET(builder, message, send_data)

			BroadcastPacket(message, kUSER_ID_INIT);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPenetrate:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPenetrate_data>*)pData;
			auto* data = pPacket->GetData();

			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr)
			{
				return;
			}

			{
				string verifyLog = "[HACK_CHECK] FORCE_ACTION_STAND RoomElapsedTime : " + std::to_string(GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
					+ ", VerifyType : " + F4PACKET::EnumNameEFORCE_ACTION_STAND_TYPE(type)
					+ ", SKill : " + F4PACKET::EnumNameSKILL_INDEX(data->playeraction()->skillindex())
					+ ", PosX : " + std::to_string(data->playeraction()->positionlogic().x())
					+ ", PosZ : " + std::to_string(data->playeraction()->positionlogic().z());
				ToLog(verifyLog.c_str());
			}

			F4PACKET::play_c2s_playerPenetrate_dataT dataT;
			data->UnPackTo(&dataT);
			F4PACKET::play_c2s_playerPenetrate_dataT* pData = &dataT;

			pData->playeraction->mutate_number(pData->playeraction->number() + 1);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
			send_data.add_playeraction(pData->playeraction.get());
			send_data.add_runmode(MOVE_MODE::pivot);
			send_data.add_lookid(kINT32_INIT);
			send_data.add_yawdest(kFLOAT_INIT);
			send_data.add_lefthand(false);
			send_data.add_ismirroranim(data->ismirroranim());
			send_data.add_normal(true);
			STORE_FBPACKET(builder, message, send_data)

			BroadcastPacket(message, kUSER_ID_INIT);
			
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerJumpBallTapOut:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerJumpBallTapOut_data>*)pData;
			if (pPacket != nullptr)
			{
				auto* data = pPacket->GetData();
				if (data == nullptr)
				{
					return;
				}

				flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
				bool data_check = data->Verify(packet_verify);

				if (data_check == false)
				{
					return;
				}

				if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr || data->snrnsisj() == nullptr)
				{
					return;
				}
			}
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerRebound:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerRebound_data>*)pData;

			auto* data = pPacket->GetData();
			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr || data->snrnsisj() == nullptr)
			{
				return;
			}


			if (pPacket != nullptr)
			{

				F4PACKET::play_c2s_playerRebound_dataT dataT;

				data->UnPackTo(&dataT);

				F4PACKET::play_c2s_playerRebound_dataT* pData = &dataT;

				pData->playeraction->mutate_number(pData->playeraction->number() + 1);

				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
				send_data.add_playeraction(pData->playeraction.get());
				send_data.add_runmode(MOVE_MODE::defense);
				send_data.add_lookid(kINT32_INIT);
				send_data.add_yawdest(kFLOAT_INIT);
				send_data.add_lefthand(false);
				send_data.add_ismirroranim(data->ismirroranim());
				send_data.add_normal(true);
				STORE_FBPACKET(builder, message, send_data)
				BroadcastPacket(message, kUSER_ID_INIT);
			}
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerSteal:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerSteal_data>*)pData;

			auto* data = pPacket->GetData();
			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr || data->positionball() == nullptr || data->snrnsisj() == nullptr)
			{
				return;
			}

			{
				string verifyLog = "[HACK_CHECK] FORCE_ACTION_STAND RoomElapsedTime : " + std::to_string(GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
					+ ", VerifyType : " + F4PACKET::EnumNameEFORCE_ACTION_STAND_TYPE(type)
					+ ", SKill : " + F4PACKET::EnumNameSKILL_INDEX(data->playeraction()->skillindex())
					+ ", PosX : " + std::to_string(data->playeraction()->positionlogic().x())
					+ ", PosZ : " + std::to_string(data->playeraction()->positionlogic().z());
				ToLog(verifyLog.c_str());
			}

			F4PACKET::play_c2s_playerSteal_dataT dataT;
			data->UnPackTo(&dataT);
			F4PACKET::play_c2s_playerSteal_dataT* pData = &dataT;

			pData->playeraction->mutate_number(pData->playeraction->number() + 1);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
			send_data.add_playeraction(pData->playeraction.get());
			send_data.add_runmode(MOVE_MODE::defense);
			send_data.add_lookid(kINT32_INIT);
			send_data.add_yawdest(kFLOAT_INIT);
			send_data.add_lefthand(false);
			send_data.add_ismirroranim(data->ismirroranim());
			send_data.add_normal(true);
			STORE_FBPACKET(builder, message, send_data)

			BroadcastPacket(message, kUSER_ID_INIT);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPassive:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPassive_data>*)pData;

			auto* data = pPacket->GetData();
			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerBlock:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerBlock_data>*)pData;

			auto* data = pPacket->GetData();
			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr || data->positionowner() == nullptr || data->positionball() == nullptr || data->positionballblockevent() == nullptr
				|| data->localpositionball() == nullptr || data->slideposition() == nullptr
				|| data->snrnsisj() == nullptr)
			{
				return;
			}

			{
				string verifyLog = "[HACK_CHECK] FORCE_ACTION_STAND RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) 
					+ ", GameTime : " + std::to_string(GetGameTime())
					+ ", UserID : " + std::to_string(userid) 
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
					+ ", VerifyType : " + F4PACKET::EnumNameEFORCE_ACTION_STAND_TYPE(type)
					+ ", SKill : " + F4PACKET::EnumNameSKILL_INDEX(data->playeraction()->skillindex())
					+ ", PosX : " + std::to_string(data->playeraction()->positionlogic().x())
					+ ", PosZ : " + std::to_string(data->playeraction()->positionlogic().z());
				ToLog(verifyLog.c_str());
			}

			F4PACKET::play_c2s_playerBlock_dataT dataT;

			data->UnPackTo(&dataT);

			F4PACKET::play_c2s_playerBlock_dataT* pData = &dataT;

			pData->playeraction->mutate_number(pData->playeraction->number() + 1);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
			send_data.add_playeraction(pData->playeraction.get());
			send_data.add_runmode(MOVE_MODE::defense);
			send_data.add_lookid(kINT32_INIT);
			send_data.add_yawdest(kFLOAT_INIT);
			send_data.add_lefthand(false);
			send_data.add_ismirroranim(data->ismirroranim());
			send_data.add_normal(true);
			STORE_FBPACKET(builder, message, send_data)

			BroadcastPacket(message, kUSER_ID_INIT);

		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerShot:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerShot_data>*)pData;

			auto* data = pPacket->GetData();
			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr
				|| data->snrnsisj() == nullptr)
			{
				return;
			}

			{
				string verifyLog = "[HACK_CHECK] FORCE_ACTION_STAND RoomElapsedTime : " + std::to_string(GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
					+ ", VerifyType : " + F4PACKET::EnumNameEFORCE_ACTION_STAND_TYPE(type)
					+ ", SKill : " + F4PACKET::EnumNameSKILL_INDEX(data->playeraction()->skillindex())
					+ ", PosX : " + std::to_string(data->playeraction()->positionlogic().x())
					+ ", PosZ : " + std::to_string(data->playeraction()->positionlogic().z());
				ToLog(verifyLog.c_str());
			}

			F4PACKET::play_c2s_playerShot_dataT dataT;

			data->UnPackTo(&dataT);

			F4PACKET::play_c2s_playerShot_dataT* pData = &dataT;

			pData->playeraction->mutate_number(pData->playeraction->number() + 1);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
			send_data.add_playeraction(pData->playeraction.get());
			send_data.add_runmode(MOVE_MODE::pivot);
			send_data.add_lookid(kINT32_INIT);
			send_data.add_yawdest(kFLOAT_INIT);
			send_data.add_lefthand(false);
			send_data.add_ismirroranim(data->ismirroranim());
			send_data.add_normal(true);
			STORE_FBPACKET(builder, message, send_data)

			BroadcastPacket(message, kUSER_ID_INIT);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPick:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPick_data>*)pData;
			if (pPacket != nullptr)
			{
				auto* data = pPacket->GetData();
				if (data == nullptr)
				{
					return;
				}

				flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
				bool data_check = data->Verify(packet_verify);

				if (data_check == false)
				{
					return;
				}

				if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr || data->snrnsisj() == nullptr)
				{
					return;
				}

				{
					string verifyLog = "[HACK_CHECK] FORCE_ACTION_STAND play_c2s_playerPick RoomElapsedTime : " + std::to_string(GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(GetGameTime())
						+ ", UserID : " + std::to_string(userid)
						+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
						+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
						+ ", VerifyType : " + F4PACKET::EnumNameEFORCE_ACTION_STAND_TYPE(type)
						+ ", SKill : " + F4PACKET::EnumNameSKILL_INDEX(data->playeraction()->skillindex())
						+ ", PosX : " + std::to_string(data->playeraction()->positionlogic().x())
						+ ", PosZ : " + std::to_string(data->playeraction()->positionlogic().z());
					ToLog(verifyLog.c_str());
				}

				F4PACKET::play_c2s_playerPick_dataT dataT;
				data->UnPackTo(&dataT);
				F4PACKET::play_c2s_playerPick_dataT* pData = &dataT;


				pData->playeraction->mutate_number(pData->playeraction->number() + 1);


				F4PACKET::play_c2s_playerPick_dataT* pTemp_data = &dataT;
				F4PACKET::SPlayerAction* pAction = nullptr;
				pAction = pTemp_data->playeraction.get();

				CCharacter* pCharacter = GetCharacterManager()->GetCharacter(data->playeraction()->id());
				if (pCharacter)
				{
					pAction->mutable_positionlogic().mutate_x(pCharacter->GetPosition().fX);
					pAction->mutable_positionlogic().mutate_y(pCharacter->GetPosition().fY);
					pAction->mutable_positionlogic().mutate_z(pCharacter->GetPosition().fZ);
				}

				//---------------------------------------------------------
				ServerCharacterPosition position;
				position.packetID = PACKET_ID::play_c2s_playerStand;
				position.positionlogic = pAction->positionlogic();
				position.speed = 0.0f;
				AddServerPostionDeque(pAction->id(), position);
				//-------------------------------------------------------------------

				// 서버 위치로 바꿔줘야지 
				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
				send_data.add_playeraction(pAction);
				send_data.add_runmode(MOVE_MODE::defense);
				send_data.add_lookid(kINT32_INIT);
				send_data.add_yawdest(kFLOAT_INIT);
				send_data.add_lefthand(false);
				send_data.add_ismirroranim(data->ismirroranim());
				send_data.add_normal(false); // 랙표시를 해줘보자 
				STORE_FBPACKET(builder, message, send_data)

				BroadcastPacket(message, kUSER_ID_INIT);

			}
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerFakeShot:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerFakeShot_data>*)pData;
			if (pPacket != nullptr)
			{
				auto* data = pPacket->GetData();
				if (data == nullptr)
				{
					return;
				}

				flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
				bool data_check = data->Verify(packet_verify);

				if (data_check == false)
				{
					return;
				}

				if (data->playeraction() == nullptr || data->snrnsisj() == nullptr)
				{
					return;
				}
			}
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPass:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPass_data>*)pData;

			auto* data = pPacket->GetData();
			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr)
			{
				return;
			}

			{
				string verifyLog = "[HACK_CHECK] FORCE_ACTION_STAND RoomElapsedTime : " + std::to_string(GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
					+ ", VerifyType : " + F4PACKET::EnumNameEFORCE_ACTION_STAND_TYPE(type)
					+ ", SKill : " + F4PACKET::EnumNameSKILL_INDEX(data->playeraction()->skillindex())
					+ ", PosX : " + std::to_string(data->playeraction()->positionlogic().x())
					+ ", PosZ : " + std::to_string(data->playeraction()->positionlogic().z());
				ToLog(verifyLog.c_str());
			}

			F4PACKET::play_c2s_playerPass_dataT dataT;

			data->UnPackTo(&dataT);

			F4PACKET::play_c2s_playerPass_dataT* pData = &dataT;

			pData->playeraction->mutate_number(pData->playeraction->number() + 1);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
			send_data.add_playeraction(pData->playeraction.get());
			send_data.add_runmode(MOVE_MODE::pivot);
			send_data.add_lookid(kINT32_INIT);
			send_data.add_yawdest(kFLOAT_INIT);
			send_data.add_lefthand(false);
			send_data.add_ismirroranim(data->ismirroranim());
			send_data.add_normal(true);
			STORE_FBPACKET(builder, message, send_data)

			BroadcastPacket(message, kUSER_ID_INIT);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerShakeAndBake:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerShakeAndBake_data>*)pData;
			auto* data = pPacket->GetData();

			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr)
			{
				return;
			}

			{
				string verifyLog = "[HACK_CHECK] FORCE_ACTION_STAND RoomElapsedTime : " + std::to_string(GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
					+ ", VerifyType : " + F4PACKET::EnumNameEFORCE_ACTION_STAND_TYPE(type)
					+ ", SKill : " + F4PACKET::EnumNameSKILL_INDEX(data->playeraction()->skillindex())
					+ ", PosX : " + std::to_string(data->playeraction()->positionlogic().x())
					+ ", PosZ : " + std::to_string(data->playeraction()->positionlogic().z());
				ToLog(verifyLog.c_str());
			}

			F4PACKET::play_c2s_playerShakeAndBake_dataT dataT;
			data->UnPackTo(&dataT);
			F4PACKET::play_c2s_playerShakeAndBake_dataT* pData = &dataT;

			pData->playeraction->mutate_number(pData->playeraction->number() + 1);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
			send_data.add_playeraction(pData->playeraction.get());
			send_data.add_runmode(MOVE_MODE::pivot);
			send_data.add_lookid(kINT32_INIT);
			send_data.add_yawdest(kFLOAT_INIT);
			send_data.add_lefthand(false);
			send_data.add_ismirroranim(data->ismirroranim());
			send_data.add_normal(true);
			STORE_FBPACKET(builder, message, send_data)

			BroadcastPacket(message, kUSER_ID_INIT);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerSlipAndSlide:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerSlipAndSlide_data>*)pData;
			auto* data = pPacket->GetData();

			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr)
			{
				return;
			}

			{
				string verifyLog = "[HACK_CHECK] FORCE_ACTION_STAND RoomElapsedTime : " + std::to_string(GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
					+ ", VerifyType : " + F4PACKET::EnumNameEFORCE_ACTION_STAND_TYPE(type)
					+ ", SKill : " + F4PACKET::EnumNameSKILL_INDEX(data->playeraction()->skillindex())
					+ ", PosX : " + std::to_string(data->playeraction()->positionlogic().x())
					+ ", PosZ : " + std::to_string(data->playeraction()->positionlogic().z());
				ToLog(verifyLog.c_str());
			}

			F4PACKET::play_c2s_playerSlipAndSlide_dataT dataT;
			data->UnPackTo(&dataT);
			F4PACKET::play_c2s_playerSlipAndSlide_dataT* pData = &dataT;

			pData->playeraction->mutate_number(pData->playeraction->number() + 1);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
			send_data.add_playeraction(pData->playeraction.get());
			send_data.add_runmode(MOVE_MODE::pivot);
			send_data.add_lookid(kINT32_INIT);
			send_data.add_yawdest(kFLOAT_INIT);
			send_data.add_lefthand(false);
			send_data.add_ismirroranim(data->ismirroranim());
			send_data.add_normal(true);
			STORE_FBPACKET(builder, message, send_data)

			BroadcastPacket(message, kUSER_ID_INIT);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerSpinMove:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerSpinMove_data>*)pData;
			auto* data = pPacket->GetData();

			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr)
			{
				return;
			}

			{
				string verifyLog = "[HACK_CHECK] FORCE_ACTION_STAND RoomElapsedTime : " + std::to_string(GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
					+ ", VerifyType : " + F4PACKET::EnumNameEFORCE_ACTION_STAND_TYPE(type)
					+ ", SKill : " + F4PACKET::EnumNameSKILL_INDEX(data->playeraction()->skillindex())
					+ ", PosX : " + std::to_string(data->playeraction()->positionlogic().x())
					+ ", PosZ : " + std::to_string(data->playeraction()->positionlogic().z());
				ToLog(verifyLog.c_str());
			}

			F4PACKET::play_c2s_playerSpinMove_dataT dataT;
			data->UnPackTo(&dataT);
			F4PACKET::play_c2s_playerSpinMove_dataT* pData = &dataT;

			pData->playeraction->mutate_number(pData->playeraction->number() + 1);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
			send_data.add_playeraction(pData->playeraction.get());
			send_data.add_runmode(MOVE_MODE::pivot);
			send_data.add_lookid(kINT32_INIT);
			send_data.add_yawdest(kFLOAT_INIT);
			send_data.add_lefthand(false);
			send_data.add_ismirroranim(data->ismirroranim());
			send_data.add_normal(true);
			STORE_FBPACKET(builder, message, send_data)

			BroadcastPacket(message, kUSER_ID_INIT);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPostUpPenetrate:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPostUpPenetrate_data>*)pData;
			auto* data = pPacket->GetData();

			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr)
			{
				return;
			}

			{
				string verifyLog = "[HACK_CHECK] FORCE_ACTION_STAND RoomElapsedTime : " + std::to_string(GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
					+ ", VerifyType : " + F4PACKET::EnumNameEFORCE_ACTION_STAND_TYPE(type)
					+ ", SKill : " + F4PACKET::EnumNameSKILL_INDEX(data->playeraction()->skillindex())
					+ ", PosX : " + std::to_string(data->playeraction()->positionlogic().x())
					+ ", PosZ : " + std::to_string(data->playeraction()->positionlogic().z());
				ToLog(verifyLog.c_str());
			}

			F4PACKET::play_c2s_playerPostUpPenetrate_dataT dataT;
			data->UnPackTo(&dataT);
			F4PACKET::play_c2s_playerPostUpPenetrate_dataT* pData = &dataT;

			pData->playeraction->mutate_number(pData->playeraction->number() + 1);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
			send_data.add_playeraction(pData->playeraction.get());
			send_data.add_runmode(MOVE_MODE::pivot);
			send_data.add_lookid(kINT32_INIT);
			send_data.add_yawdest(kFLOAT_INIT);
			send_data.add_lefthand(false);
			send_data.add_ismirroranim(data->ismirroranim());
			send_data.add_normal(true);
			STORE_FBPACKET(builder, message, send_data)

			BroadcastPacket(message, kUSER_ID_INIT);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPostUpReady:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPostUpReady_data>*)pData;
			auto* data = pPacket->GetData();

			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr)
			{
				return;
			}

			{
				string verifyLog = "[HACK_CHECK] FORCE_ACTION_STAND RoomElapsedTime : " + std::to_string(GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
					+ ", VerifyType : " + F4PACKET::EnumNameEFORCE_ACTION_STAND_TYPE(type)
					+ ", SKill : " + F4PACKET::EnumNameSKILL_INDEX(data->playeraction()->skillindex())
					+ ", PosX : " + std::to_string(data->playeraction()->positionlogic().x())
					+ ", PosZ : " + std::to_string(data->playeraction()->positionlogic().z());
				ToLog(verifyLog.c_str());
			}

			F4PACKET::play_c2s_playerPostUpReady_dataT dataT;
			data->UnPackTo(&dataT);
			F4PACKET::play_c2s_playerPostUpReady_dataT* pData = &dataT;

			pData->playeraction->mutate_number(pData->playeraction->number() + 1);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
			send_data.add_playeraction(pData->playeraction.get());
			send_data.add_runmode(MOVE_MODE::pivot);
			send_data.add_lookid(kINT32_INIT);
			send_data.add_yawdest(kFLOAT_INIT);
			send_data.add_lefthand(false);
			send_data.add_ismirroranim(data->ismirroranim());
			send_data.add_normal(true);
			STORE_FBPACKET(builder, message, send_data)

			BroadcastPacket(message, kUSER_ID_INIT);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerEscapeDribble:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerEscapeDribble_data>*)pData;
			auto* data = pPacket->GetData();

			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr)
			{
				return;
			}

			{
				string verifyLog = "[HACK_CHECK] FORCE_ACTION_STAND RoomElapsedTime : " + std::to_string(GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
					+ ", VerifyType : " + F4PACKET::EnumNameEFORCE_ACTION_STAND_TYPE(type)
					+ ", SKill : " + F4PACKET::EnumNameSKILL_INDEX(data->playeraction()->skillindex())
					+ ", PosX : " + std::to_string(data->playeraction()->positionlogic().x())
					+ ", PosZ : " + std::to_string(data->playeraction()->positionlogic().z());
				ToLog(verifyLog.c_str());
			}

			F4PACKET::play_c2s_playerEscapeDribble_dataT dataT;
			data->UnPackTo(&dataT);
			F4PACKET::play_c2s_playerEscapeDribble_dataT* pData = &dataT;

			pData->playeraction->mutate_number(pData->playeraction->number() + 1);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
			send_data.add_playeraction(pData->playeraction.get());
			send_data.add_runmode(MOVE_MODE::pivot);
			send_data.add_lookid(kINT32_INIT);
			send_data.add_yawdest(kFLOAT_INIT);
			send_data.add_lefthand(false);
			send_data.add_ismirroranim(data->ismirroranim());
			send_data.add_normal(true);
			STORE_FBPACKET(builder, message, send_data)

			BroadcastPacket(message, kUSER_ID_INIT);
		}
		break;
		default:
			break;
	}
}


void CHost::ForceActionModifyPositionOfServer(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_USER_ID userid, DHOST_TYPE_FLOAT recvTime)
{
	switch ((F4PACKET::PACKET_ID)packetID)
	{
	case F4PACKET::PACKET_ID::play_c2s_playerShot:
	{
		auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerShot_data>*)pData;
		auto* data = pPacket->GetData();

		if (data == nullptr)
		{
			return;
		}

		flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
		bool data_check = data->Verify(packet_verify);

		if (data_check == false)
		{
			return;
		}

		F4PACKET::play_c2s_playerShot_dataT dataT;
		data->UnPackTo(&dataT);
		F4PACKET::play_c2s_playerShot_dataT* pData = &dataT;
		F4PACKET::SPlayerAction* pAction = nullptr;
		pAction = pData->playeraction.get();

		const SPlayerAction* pFixedAction = data->playeraction();

		CCharacter* pCharacter = GetCharacterManager()->GetCharacter(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			auto server_pos = pCharacter->GetCharacterStateAction()->positionlogic();	// 서버가 알고있는 캐릭터의 위치로 바까줌 
			auto client_pos = pAction->positionlogic();	// 패킷으로 받은 캐릭터의 위치
			float gap = VECTOR3_DISTANCE(server_pos, client_pos);

			string position_log = "[MODIFY_POSITION] play_c2s_playerShot GameTime : " + std::to_string(GetGameTime()) + ", CharacterSN : " + std::to_string(data->playeraction()->id()) +
				", GAP : " + std::to_string(gap) + ", PreAction : " + F4PACKET::EnumNameACTION_TYPE(pCharacter->GetCharacterStateActionType()) +
				", Server X : " + std::to_string(server_pos.x()) + ", Server Z : " + std::to_string(server_pos.z()) +
				", Client X : " + std::to_string(pAction->positionlogic().x()) + ", Client Z : " + std::to_string(pAction->positionlogic().z());

			ToLog(position_log.c_str());

			pAction->mutable_positionlogic().mutate_x(server_pos.x());
			pAction->mutable_positionlogic().mutate_y(server_pos.y());
			pAction->mutable_positionlogic().mutate_z(server_pos.z());

			pFixedAction = pAction;
		}

		pAction->mutate_number(pData->playeraction->number() + 1);

		OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recvTime, GetGameTime(), data->playeraction()->keys());

		SetCharacterStateAction(data->playeraction()->id(), *pAction, ACTION_TYPE::action_stand, recvTime);

		CheckQuestAction(data->playeraction());

		// 실패처리 
		SetCharacterForceBallEventFail(data->playeraction()->id(), data->ballnumber());

		SetCharacterAction(data->playeraction()->id(), GetGameTime(), ACTION_TYPE::action_stand, data->playeraction(), kFLOAT_INIT);
		AddCharacterActionDeque(data->playeraction()->id());

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerShot, message, send_data);
		send_data.add_playeraction(pAction);
		send_data.add_ballnumber(data->ballnumber());
		send_data.add_shottype(data->shottype());
		send_data.add_animid(data->animid());
		send_data.add_positionball(data->positionball());
		send_data.add_speed(data->speed());
		send_data.add_enabledribble(data->enabledribble());
		send_data.add_readyanimation(data->readyanimation());
		send_data.add_islefthanded(data->islefthanded());
		send_data.add_localpositionball(data->localpositionball());
		send_data.add_slideposition(data->slideposition());
		send_data.add_ismirroranim(data->ismirroranim());
		send_data.add_hostpermit(true);	// 클라에서 슛 액션에 대한 실패처리가 없어서 일단 무조건 true
		send_data.add_normal(false);
		send_data.add_potentialfxlevel(0);	// 림어택시에만 이펙트를 미리 보내줌
		STORE_FBPACKET(builder, message, send_data)

		BroadcastPacket(message, kUSER_ID_INIT);

	}

	break;

	default:
		break;
	}
}

void CHost::ForceActionStandExtend(DHOST_TYPE_INT32 packetID, void* pData, CCharacter* pCharacter, DHOST_TYPE_FLOAT recvTime, DHOST_TYPE_BOOL normal)
{
	switch ((F4PACKET::PACKET_ID)packetID)
	{
		case F4PACKET::PACKET_ID::play_c2s_playerPenetrateReady:
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPenetrateReady_data>*)pData;
			auto* data = pPacket->GetData();

			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			F4PACKET::play_c2s_playerPenetrateReady_dataT dataT;
			data->UnPackTo(&dataT);
			F4PACKET::play_c2s_playerPenetrateReady_dataT* pData = &dataT;
			F4PACKET::SPlayerAction* pAction = nullptr;
			pAction = pData->playeraction.get();

			pAction->mutable_positionlogic().mutate_x(pCharacter->GetPosition().fX);
			pAction->mutable_positionlogic().mutate_y(pCharacter->GetPosition().fY);
			pAction->mutable_positionlogic().mutate_z(pCharacter->GetPosition().fZ);

			pAction->mutate_number(pData->playeraction->number() + 1);

			OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recvTime, GetGameTime(), data->playeraction()->keys());
			SetCharacterStateAction(data->playeraction()->id(), *pAction, ACTION_TYPE::action_stand, recvTime);
			CheckQuestAction(data->playeraction());
			SetCharacterAction(data->playeraction()->id(), GetGameTime(), ACTION_TYPE::action_stand, data->playeraction(), kFLOAT_INIT);
			AddCharacterActionDeque(data->playeraction()->id());

			//---------------------------------------------------------
			ServerCharacterPosition position;
			position.packetID = PACKET_ID::play_c2s_playerStand;
			position.positionlogic = data->playeraction()->positionlogic();
			position.speed = 0.0f;
			AddServerPostionDeque(data->playeraction()->id(), position);
			//-------------------------------------------------------------------

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
			send_data.add_playeraction(pAction);
			send_data.add_runmode(MOVE_MODE::dribble);
			send_data.add_lookid(kINT32_INIT);
			send_data.add_yawdest(kFLOAT_INIT);
			send_data.add_lefthand(normal); // 랙 표시 
			send_data.add_ismirroranim(data->ismirroranim());
			STORE_FBPACKET(builder, message, send_data)

			BroadcastPacket(message, kUSER_ID_INIT);
		}

		break;
		default:
			break;
	}
}

void CHost::GetClientHMAC(const flatbuffers::Vector<int8_t>* hmac, string& str)
{
//#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))

	if (hmac == nullptr)
	{
		string check_hmac = "VerifyBallLerp hmac is null";
		ToLog(check_hmac.c_str());

		return;
	}

	unsigned char recv_hmac[32];

	for (int i = 0; i < (int)hmac->size(); ++i)
	{
		recv_hmac[i] = (unsigned char)hmac->Get(i);
		str += std::to_string(recv_hmac[i]);
	}
//#endif
}

template<typename T>
DHOST_TYPE_INT32 CHost::GetTypeConvert(T value)
{
	DHOST_TYPE_INT32 result = kINT32_INIT;

	DHOST_TYPE_FLOAT calc_truncf = kFLOAT_INIT;

	if (typeid(T).name() == "float")
	{
		DHOST_TYPE_FLOAT temp_value = (DHOST_TYPE_FLOAT)value;
		calc_truncf = truncf(temp_value);
		result = (int)calc_truncf;
	}
	else
	{
		result = (int)value;
	}

	return result;
}

DHOST_TYPE_BOOL CHost::CheckShotTypeRimAttack(SHOT_TYPE value)
{
	DHOST_TYPE_BOOL result = false;

	if (m_VerifyManager != nullptr)
	{
		result = m_VerifyManager->CheckShotTypeRimAttack(value);
	}

	return result;
}

DHOST_TYPE_BOOL CHost::CheckHaveSkill(DHOST_TYPE_CHARACTER_SN id, F4PACKET::SKILL_INDEX value)
{
	if (m_VerifyManager != nullptr)
	{
		return m_VerifyManager->CheckHaveSkill(id, value);
	}

	return false;
}

DHOST_TYPE_FLOAT CHost::GetSkillLevel(DHOST_TYPE_CHARACTER_SN id, SKILL_INDEX value)
{
	DHOST_TYPE_FLOAT result = kFLOAT_INIT;

	if (m_VerifyManager != nullptr)
	{
		result = m_VerifyManager->GetSkillLevel(id, value);
	}

	return result;
}

DHOST_TYPE_FLOAT CHost::GetRandomValueAlgorithm(DHOST_TYPE_USER_ID userid, DHOST_TYPE_BOOL xorShift)
{
	DHOST_TYPE_FLOAT result = kFLOAT_INIT;;

	if (m_VerifyManager != nullptr)
	{
		result = m_VerifyManager->GetRandomValueAlgorithm(userid, xorShift);
	}

	return result;
}

EACTION_VERIFY CHost::VerifyPotentialFxLevel(DHOST_TYPE_USER_ID userid, DHOST_TYPE_CHARACTER_SN id, DHOST_TYPE_INT32 fxlevel, POTENTIAL_INDEX value)
{
	EACTION_VERIFY result = EACTION_VERIFY::FAIL;

	if (m_VerifyManager != nullptr)
	{
		if (fxlevel == kINT32_INIT)
		{
			result = EACTION_VERIFY::SUCCESS;
		}
		else
		{
			result = m_VerifyManager->VerifyPotentialFxLevel(userid, id, fxlevel, value);
		}
	}

	return result;
}

DHOST_TYPE_BOOL	CHost::CheckPositionIsNan(const TB::SVector3* position)
{
	if (isnan(position->x()) || isnan(position->y()) || isnan(position->z()))
	{
		return true;
	}

	if (isinf(position->x()) || isinf(position->y()) || isinf(position->z()))
	{
		return true;
	}
	
	return false;
}

void CHost::RegistCallbackFuncSendPacket(HostMessageCallback pFunc)
{
	m_CallbackBroadcast = std::move(pFunc);
}

void CHost::RegistCallbackFuncLog(LogCallback pFunc)
{
	m_CallbackLog = std::move(pFunc);
}

void CHost::OnHostJoin(DHOST_TYPE_INT32 UserID, int spectator)
{
	EUSER_TYPE UserType = EUSER_TYPE::NORMAL;

	// 관전자일 경우
	if (spectator == 1)
	{
		UserType = EUSER_TYPE::OBSERVER;
	}

	CHostUserInfo* pUser = this->FindUser(UserID);

	if (pUser == nullptr)
	{
		pUser = CreateUser(UserID, UserType);

		if (pUser == nullptr)
		{
			string log_message = "OnHostJoin CreateUser is fail RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(UserID) + ", UserType : " + ConvertToStringUserType(UserType) + ", MapSize : " + to_string(m_HostUserMap.size());
			ToLog(log_message.c_str());
		}
	}
	else
	{
		if (UserType == EUSER_TYPE::NORMAL)
		{
			// 끊겼다가 다시 접속했을 경우
			if (CheckCallbackEnetDisConnectUser(pUser->GetUserID()) || pUser->GetConnectState() == ECONNECT_STATE::DISCONNECT)
			{
				string log_message = "OnHostJoin ReConnect RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(UserID);
				ToLog(log_message.c_str());

				RemoveCallbackEnetDisConnectUser(pUser->GetUserID());

				PushReConnectUser(UserID);
			}
		}
	}

	pUser->SetConnectState(ECONNECT_STATE::JOIN);
}

// 정상적인 접속이 아닌 호스트에서 강제로 유저를 생성하는 경우
void CHost::OnHostJoinForce(DHOST_TYPE_INT32 UserID)
{
	EUSER_TYPE UserType = EUSER_TYPE::NORMAL;

	DHOST_TYPE_UINT64 cast_user_id = static_cast<DHOST_TYPE_UINT64>(UserID);
	CHostUserInfo* pUser = new CHostUserInfo(UserID, reinterpret_cast<DHOST_TYPE_UINT64*>(cast_user_id), this, GetRoomElapsedTime(), UserType);

	m_HostUserMap.insert(HostUserMapType::value_type(UserID, pUser));

	string log_message = "OnHostJoinForce UserID : " + to_string(UserID) + ", UserType : " + ConvertToStringUserType(pUser->GetUserType()) + ", MapSize : " + to_string(m_HostUserMap.size());
	ToLog(log_message.c_str());
}

void CHost::OnHostConnect(DHOST_TYPE_INT32 UserID, int spectator)
{
	CHostUserInfo* pUser = this->FindUser(UserID);

	if (pUser == nullptr)
	{
		string log_message = "OnHostConnect pUser is null : " + to_string(UserID);
		ToLog(log_message.c_str());

		return;
	}

	string log_message = "OnHostConnect RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(UserID) + ", UserType : " + ConvertToStringUserType(pUser->GetUserType());
	ToLog(log_message.c_str());

	if (CheckDisconnectUser(UserID) == false)
	{
		pUser->SetConnectState(ECONNECT_STATE::CONNECT);
	}
}

void CHost::OnHostDisconnect(DHOST_TYPE_INT32 UserID, int spectator)
{
	CHostUserInfo* pUser = FindUser(UserID);

	if (nullptr == pUser)
	{
		string log_message = "OnHostDisconnect pUser is null UserID : " + to_string(UserID);
		ToLog(log_message.c_str());

		return;
	}

	string log_message = "OnHostDisconnect RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(UserID) + ", UserType : " + ConvertToStringUserType(pUser->GetUserType());
	ToLog(log_message.c_str());
	
	if (pUser->GetUserType() == EUSER_TYPE::OBSERVER)
	{
		//RemoveObserverUser(UserID);
		return;
	}

	if (pUser->GetConnectState() == ECONNECT_STATE::DISCONNECT)
	{
		string log_message = "OnHostDisconnect pUser already DISCONNECT UserID : " + to_string(UserID);
		ToLog(log_message.c_str());

		return;
	}

	PushCallbackEnetDisConnectUser(UserID);
	BadConnectProcess(UserID, true);

	return;
}

void CHost::OnHostQuit(DHOST_TYPE_INT32 UserID)
{
	return;
}

void CHost::OnHostExit()
{
	return;
}

DHOST_TYPE_BOOL CHost::CheckObserverUser(DHOST_TYPE_INT32 UserID)
{
	DHOST_TYPE_BOOL result = false;

	CHostUserInfo* pUser = FindUser(UserID);

	if (pUser != nullptr && pUser->GetUserType() == EUSER_TYPE::OBSERVER)
	{
		result = true;
	}

	return result;
}

void CHost::OnPacketObserverUser(CPacketBase* pPacket, void* peer, const DHOST_TYPE_FLOAT& recv_time)
{
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(GetRoomElapsedTime());
		ToLog(invalid_buffer_log.c_str());

		return;
	}

	DHOST_TYPE_USER_ID userid = pPacket->GetUserID();
	PACKET_ID packetID = (PACKET_ID)pPacket->GetPacketID();

	switch ((PACKET_ID)pPacket->GetPacketID())
	{
		case F4PACKET::PACKET_ID::system_c2s_login:
		{
			//! 패킷 데이터 검증 (모든 패킷에 추가할 것)
			auto* pPacketData = reinterpret_cast<CFlatBufPacket<F4PACKET::system_c2s_login_data>*>(pPacket);

			if (pPacketData == nullptr)
			{
				string invalid_buffer_log = "[PACKET_ERROR] pPacketData is null func : OnPacketObserverUser RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
				ToLog(invalid_buffer_log.c_str());

				return;
			}

			auto* data = pPacketData->GetData();
			if (data == nullptr)
			{
				string invalid_buffer_log = "[PACKET_ERROR] data is null func : OnPacketObserverUser RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
				ToLog(invalid_buffer_log.c_str());

				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacketData->m_pdata), pPacketData->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS func : OnPacketObserverUser RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
				ToLog(invalid_buffer_log.c_str());

				return;
			}
			//!

			F4PACKET::RESULT result = F4PACKET::RESULT::fail;
			F4PACKET::LOGIN_TYPE type = F4PACKET::LOGIN_TYPE::none;
			F4PACKET::EXIT_TYPE exitType = F4PACKET::EXIT_TYPE::lobby;

			auto& HostUserMap = GetHostUserMap();

			if (HostUserMap.empty() == false)
			{
				auto iter = HostUserMap.find(userid);

				if (iter != HostUserMap.end())
				{
					result = F4PACKET::RESULT::success;
					type = F4PACKET::LOGIN_TYPE::normal;

					iter->second->SetLogin(true);
				}
			}

			{
				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, system_s2c_login, message, send_data);
				send_data.add_result(result);
				send_data.add_logintype(type);
				send_data.add_exittype(exitType);
				send_data.add_remaingametime(GetGameTime());
				STORE_FBPACKET(builder, message, send_data)

				string log_message = "[OBSERVER] USER_LOGIN RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(userid)
					+ ", result : " + F4PACKET::EnumNameRESULT(result) + ", loginType : " + F4PACKET::EnumNameLOGIN_TYPE(type)
					+ ", exitType : " + F4PACKET::EnumNameEXIT_TYPE(exitType) + ", remainTime : " + std::to_string(GetGameTime());
				ToLog(log_message.c_str());

				SendPacket(message, userid);
			}
			
			std::vector<F4PACKET::SPlayerInformationT*> vecCharacters;
			vecCharacters.clear();

			GetCharacterInformationVec(vecCharacters);

			// 플레이어들의 정보 전송.
			if (GetRedisLoadValue())	// Host to InGameServer
			{
				CREATE_BUILDER(builder)

				std::vector<flatbuffers::Offset<F4PACKET::SPlayerInformation>> vecchars;

				for (int i = 0; i < vecCharacters.size(); ++i)
				{
					auto packedcharecter = F4PACKET::SPlayerInformation::Pack(builder, vecCharacters[i]);
					vecchars.push_back(packedcharecter);
				}

				auto offvecchars = builder.CreateVector(vecchars);

				CREATE_FBPACKET(builder, system_s2c_playerInfo, message, databuilder)
				databuilder.add_playerlist(offvecchars);

				STORE_FBPACKET(builder, message, databuilder)
				SendPacket(message, userid);

				string log_message = "[OBSERVER] SEND_PLAYER_INFO RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(userid);
				ToLog(log_message.c_str());
			}

			// 로딩 시작 명령.
			{
				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, system_s2c_startLoad, message, send_data);
				send_data.add_result(F4PACKET::RESULT::success);
				send_data.add_stageid(GetStageID());
				send_data.add_gamemode(GetConvertGameMode());
				send_data.add_isnormalgame(GetIsNormalGame());
				send_data.add_switchtoailevel(GetSwitchToAiLevel());
				send_data.add_handicaprulepoint(GetGameManager()->GetHandicapRulePoint());
				send_data.add_treatdelayoption(bTreatDelayMode);
				send_data.add_serverpossync(bServerSyncPos);
				STORE_FBPACKET(builder, message, send_data)

				SendPacket(message, userid);

				string log_message = "[OBSERVER] START_LOAD RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(userid);
				ToLog(log_message.c_str());
			}
		}
		break;
		case F4PACKET::PACKET_ID::system_c2c_loadingPercentage:
		{
			//! 패킷 데이터 검증 (모든 패킷에 추가할 것)
			auto* pPacketData = reinterpret_cast<CFlatBufPacket<F4PACKET::system_c2c_loadingPercentage_data>*>(pPacket);
			if (pPacketData == nullptr)
			{
				string invalid_buffer_log = "[PACKET_ERROR] pPacketData is null func : OnPacketObserverUser RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
				ToLog(invalid_buffer_log.c_str());

				return;
			}

			auto* data = pPacketData->GetData();
			if (data == nullptr)
			{
				string invalid_buffer_log = "[PACKET_ERROR] data is null func : OnPacketObserverUser RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
				ToLog(invalid_buffer_log.c_str());

				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacketData->m_pdata), pPacketData->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS func : OnPacketObserverUser RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
				ToLog(invalid_buffer_log.c_str());

				return;
			}
			//!

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, system_c2c_loadingPercentage, message, send_data);
			send_data.add_userid(userid);
			send_data.add_percentage(data->percentage());
			STORE_FBPACKET(builder, message, send_data)

			SendPacket(message, userid);

			string log_message = "[OBSERVER] LOADING_PERCENTAGE RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(userid) + ", percentage : " + to_string(data->percentage());
			ToLog(log_message.c_str());
		}
		break;
		case F4PACKET::PACKET_ID::system_c2s_loadComplete:
		{
			//! 패킷 데이터 검증 (모든 패킷에 추가할 것)
			auto* pPacketData = reinterpret_cast<CFlatBufPacket<F4PACKET::system_c2s_loadComplete_data>*>(pPacket);

			if (pPacketData == nullptr)
			{
				string invalid_buffer_log = "[PACKET_ERROR] pPacketData is null func : OnPacketObserverUser RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
				ToLog(invalid_buffer_log.c_str());

				return;
			}

			auto* data = pPacketData->GetData();
			if (data == nullptr)
			{
				string invalid_buffer_log = "[PACKET_ERROR] data is null func : OnPacketObserverUser RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
				ToLog(invalid_buffer_log.c_str());

				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacketData->m_pdata), pPacketData->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS func : OnPacketObserverUser RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
				ToLog(invalid_buffer_log.c_str());

				return;
			}
			//!

			CHostUserInfo* pUser = FindUser(userid);
			if (pUser == nullptr)
			{
				string invalid_buffer_log = "[OBSERVER] pUser is null RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
				ToLog(invalid_buffer_log.c_str());

				return;
			}

			pUser->SetSpectatorLoadComplete(true);

			// 스코어 정보 보내기
			{
				CREATE_BUILDER(builder);

				std::vector< flatbuffers::Offset<F4PACKET::SReconnectGameScoreInfo>> veccounterinfo;

				std::vector<DHOST_TYPE_INT32>& team_scores = GetVecScores();

				for (int i = 0; i < team_scores.size(); ++i)
				{
					F4PACKET::SReconnectGameScoreInfoBuilder rolecountinfobuilder(builder);

					rolecountinfobuilder.add_team(i);
					rolecountinfobuilder.add_score(team_scores[i]);

					veccounterinfo.push_back(rolecountinfobuilder.Finish());
				}

				auto offsetcountinfo = builder.CreateVector(veccounterinfo);

				CREATE_FBPACKET(builder, system_s2c_reconnectGameScore, message, databuilder);

				databuilder.add_sinfo(offsetcountinfo);

				STORE_FBPACKET(builder, message, databuilder);
				SendPacket(message, userid);

				string log_message = "[OBSERVER] SEND_GAME_SCORE RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(userid);
				ToLog(log_message.c_str());
			}


			// 기록 정보 보내기
			{
				std::vector<SCharacterRecord*> vec;
				vec.clear();

				GetCharacterRecordVec(vec);

				CREATE_BUILDER(builder);

				std::vector< flatbuffers::Offset<F4PACKET::SReconnectGameRecordInfo>> reconnect_game_record_info_vector;

				flatbuffers::Offset<F4PACKET::SReconnectGameRecordInfo> reconnect_game_record_info_obj;

				for (int i = 0; i < vec.size(); ++i)
				{
					uint32_t character_sn = vec[i]->CharacterSN;

					std::vector< flatbuffers::Offset<F4PACKET::SRecordInfo>> game_record_info_vector;

					flatbuffers::Offset<F4PACKET::SRecordInfo> game_record_info_obj;

					for (int j = 0; j < (int)F4PACKET::RECORD_TYPE::MAX; ++j)
					{
						F4PACKET::RECORD_TYPE type = static_cast<F4PACKET::RECORD_TYPE>(j);

						if (type == F4PACKET::RECORD_TYPE::None)
						{
							continue;
						}

						uint32_t cnt = m_CharacterManager->GetCharacterRecordCountWithType(character_sn, type);

						game_record_info_obj = F4PACKET::CreateSRecordInfo(builder, type, cnt);

						game_record_info_vector.push_back(game_record_info_obj);
					}

					auto game_record_info_vector_obj = builder.CreateVector(game_record_info_vector);

					reconnect_game_record_info_obj = F4PACKET::CreateSReconnectGameRecordInfo(builder, character_sn, game_record_info_vector_obj);

					reconnect_game_record_info_vector.push_back(reconnect_game_record_info_obj);
				}

				auto offsetcountinfo = builder.CreateVector(reconnect_game_record_info_vector);

				CREATE_FBPACKET(builder, system_s2c_reconnectGameRecord, message, databuilder);

				databuilder.add_sinfo(offsetcountinfo);

				STORE_FBPACKET(builder, message, databuilder);
				SendPacket(message, userid);

				string log_message = "[OBSERVER] SEND_GAME_RECORD RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(userid);
				ToLog(log_message.c_str());
			}
		}
		break;
		case F4PACKET::PACKET_ID::system_c2s_exitGame:
		{
			OnHostDisconnect(userid);
		}
		break;
		default:
			return;
	}

	return;
}

void CHost::RemoveObserverUser(DHOST_TYPE_INT32 UserID)
{
	HostUserMapType::iterator iter = m_HostUserMap.find(UserID);

	if (iter == m_HostUserMap.end())
		return;

	m_HostUserMap.erase(iter);
}

void CHost::RedisSaveMatchResult(int score)
{
	if (m_CallbackRedis != nullptr)
	{
		SRedisMatchResult* pInfo = new SRedisMatchResult();

			pInfo->RoomID = ParseHostIdToInt(GetHostID());
		pInfo->ModeType = GetModeType();
		pInfo->EndTime = std::to_string(Util::systemtime_now());
		pInfo->SkillChallengeScore = score;
		pInfo->TutorialResult = score == 1 ? true : false;
		pInfo->PlayTime = GetRoomElapsedTime() - m_GameManager->GetJumpBallStartTime();
		pInfo->AbnormalExpireRoom = GetAbnormalExpireRoom() == true ? true : false;
		pInfo->OverTimeCount = GetOverTimeCount();

		if (CheckSingleMode() == false && m_GameManager->GetJumpBallStartTime() == kFLOAT_INIT)
		{
			pInfo->PlayTime = kFLOAT_INIT;
		}
		
		// 싱글모드(트레이닝, 스킬챌린지 등)가 아닐 경우만 게임정보를 저장한다.
		if (CheckSingleMode() == false)
		{
			std::vector<DHOST_TYPE_INT32>& team_scores = GetVecScores();

			for (int i = 0; i < team_scores.size(); ++i)
			{
				if (GetTeamCharacterMap(i) == true)
				{
					pInfo->vTeamScore.push_back(GetScore(i));
				}
			}

			pInfo->MVP = GetMVP();

			PushCharacterRecord(pInfo);
		}

		SetRedisSaveMatchResult(true);

		m_CallbackRedis(pInfo);
	}
	else
	{
		string log_message = "RedisSaveMatchResult m_CallbackRedis is null";
		ToLog(log_message.c_str());
	}
}

void CHost::OnRedisMatchInfo(SRedisMatchInfo* pData)
{
	string log_message = "OnRedisMatchInfo RoomElapsedTime : " + to_string(GetRoomElapsedTime()) + ", ModeType : " + to_string((int)pData->ModeType) + ", aiHostUserID : " + to_string(pData->AiHostUserID);
	ToLog(log_message.c_str());

	if (pData != nullptr)
	{
		SetModeType(pData->ModeType);
		SetIsNormalGame(pData->isNormalGame);
		SetStageID(pData->StageID);
		SetExitType(pData->ExitType);
		SetSpeedHackRestrictionStepOne(pData->SpeedHackResStepOne);
		SetSpeedHackRestrictionStepTwo(pData->SpeedHackResStepTwo);
		SetSwitchToAiLevel(pData->SwitchToAiLevel);
		SetSceneIndex(pData->SceneIndex);
		SetDebug(pData->isDebug);
		SetNoTieMode(pData->NoTieMode);
        // 디도스 공격 
		SetPacketGuardRule(pData->packetGuardRule);

		string log_message0 = "Host VersionInfo RoomElapsedTime : " + to_string(GetRoomElapsedTime()) 
			+ ", VersionInfo : " + to_string(HOST_VERSIONINFO)
			+ ", magic : " + to_string(pData->packetGuardRule.magic)
			+ ", watermarkMask : " + to_string(pData->packetGuardRule.watermarkMask)
			+ ", watermarkResult : " + to_string(pData->packetGuardRule.watermarkResult)
			+ ", NoTieMode : " + to_string(pData->NoTieMode);

		ToLog(log_message0.c_str());


		if (pData->ModeType == EMODE_TYPE::TUTORIAL_MINIGAME_CUSTOM)
		{

			// 커스텀 게임 인포메이션 넣기 
			F4PACKET::MinigameCustomInfoT* minigameCustom = new F4PACKET::MinigameCustomInfoT();
			m_GameManager->SetMinigameCustom(minigameCustom);

			minigameCustom->ishomeattackretained = pData->MiniGameCustomInfo.isHomeAttackRetained;
			minigameCustom->addshotbuffrate = pData->MiniGameCustomInfo.addShotBuffRate;

			for (int i = 0; i < pData->MiniGameCustomInfo.vShotZonerTryID.size(); i++)
			{
				DHOST_TYPE_UINT32 id = pData->MiniGameCustomInfo.vShotZonerTryID[i];
				pData->MiniGameCustomInfo.vShotZonerTryID.push_back(id);
			}

			for (int i = 0; i < pData->MiniGameCustomInfo.vActionCountID.size(); i++)
			{
				DHOST_TYPE_UINT32 id = pData->MiniGameCustomInfo.vActionCountID[i];
				pData->MiniGameCustomInfo.vActionCountID.push_back(id);
			}

			for (int i = 0; i < pData->MiniGameCustomInfo.vRecordCountID.size(); i++)
			{
				DHOST_TYPE_UINT32 id = pData->MiniGameCustomInfo.vRecordCountID[i];
				pData->MiniGameCustomInfo.vRecordCountID.push_back(id);
			}

		}

		std::vector<int32_t> vec_ai_character;
		vec_ai_character.clear();

		std::vector<int32_t> vec_trio_home_ai_character;
		vec_trio_home_ai_character.clear();

		std::vector<int32_t> vec_trio_away_ai_character;
		vec_trio_away_ai_character.clear();

		// 레디스에서 넘어온 데이터값이 0 이 아니라면 플레이타임을 셋팅해준다.
		if (pData->PlayTime > kFLOAT_INIT)
		{
			SetGameTimeInit(pData->PlayTime);
		}
		
		// 스킬 챌린지나, 트레이닝과 같은 솔로모드인 경우 목욕탕맵으로 셋팅한다.
		if (CheckSingleMode())
		{
			SetStageID(kSTAGE_TUTORIAL);
		}

		// 스킬 챌린지모드는 클라가 들고있는 데이터를 로드해서 플레이한다. (서버에서 정보를 따로 주지 않는다)
		if (CheckSkillChallengeMode())
		{
			SetRedisLoadValue(true, GetRoomElapsedTime());

			return;
		}

		DHOST_TYPE_USER_ID ai_host_user = pData->AiHostUserID;

		DHOST_TYPE_USER_ID trio_home_ai_host_user = kUSER_ID_INIT;
		DHOST_TYPE_USER_ID trio_away_ai_host_user = kUSER_ID_INIT;

		if(GetModeType() == EMODE_TYPE::TRIO || GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED)
		{
			// 실제 유저를 찾아서 ai 의 host 로 셋팅해준다.
			for (int i = 0; i < pData->vUserInfo.size(); ++i)
			{
				SRedisMatchUserInfo* pUserInfo = pData->vUserInfo[i];
				if (pUserInfo != nullptr)
				{
					if (pUserInfo->AiLevel == kIS_NOT_AI && pUserInfo->Team == "0")
					{
						trio_home_ai_host_user = pUserInfo->UserID;
					}

					if (pUserInfo->AiLevel == kIS_NOT_AI && pUserInfo->Team == "1")
					{
						trio_away_ai_host_user = pUserInfo->UserID;
					}
				}
			}

			if (trio_home_ai_host_user == kUSER_ID_INIT)		// 홈팀에 유저가 없다면 어웨이 유저가 모든 권한을 가져가게
			{
				trio_home_ai_host_user = trio_away_ai_host_user;
			}
			else if (trio_away_ai_host_user == kUSER_ID_INIT)	// 어웨이에 유저가 없다면 홈팀의 유저가 모든 권한을 가져가게
			{
				trio_away_ai_host_user = trio_home_ai_host_user;
			}
		}

		string ai_user_log_message = "OnRedisMatchInfo RoomElapsedTime : " + to_string(GetRoomElapsedTime()) + ", ModeType : " + to_string((int)pData->ModeType) 
			+ ", trio_home_ai_host_user : " + to_string(trio_home_ai_host_user) + ", trio_away_ai_host_user : " + to_string(trio_away_ai_host_user);
		ToLog(ai_user_log_message.c_str());

		for (int i = 0; i < pData->vUserInfo.size(); ++i)
		{
			SRedisMatchUserInfo* pUserInfo = pData->vUserInfo[i];

			F4PACKET::SPlayerAbilityT* pAbility = new F4PACKET::SPlayerAbilityT();
			F4PACKET::SPlayerAbilityT* pModAbility = new F4PACKET::SPlayerAbilityT();

			// 플레이어 정보 세팅 및 추가.
			F4PACKET::SPlayerInformationT* playerInformation = new F4PACKET::SPlayerInformationT();

			playerInformation->userid = pUserInfo->UserID;		// Ai 들은 유저 아이디가 1001, 1002, 뭐 이런식으로 들어온다.
			playerInformation->ailevel = pUserInfo->AiLevel;

			DHOST_TYPE_INT32 team = 0;

			if (pUserInfo->Team == "0")
				team = 0;
			else if (pUserInfo->Team == "1")
				team = 1;
			else
				team = static_cast<int32_t>(std::stoi(pUserInfo->Team));

			ai_user_log_message = "OnRedisMatchInfo RoomElapsedTime : " + to_string(GetRoomElapsedTime()) + ", ModeType : " + to_string((int)pData->ModeType) + ", pUserInfo->UserID : " + to_string(pUserInfo->UserID)
				+ ", playerInformation->userid : " + to_string(playerInformation->userid) + ", playerInformation->ailevel : " + to_string(playerInformation->ailevel) + ", Team : " + pUserInfo->Team.c_str();
			ToLog(ai_user_log_message.c_str());

			playerInformation->team = team;
			playerInformation->id = (GetLastCharacterSN());				// 호스트에서 쓰는 캐릭터 아이디
			playerInformation->characterid = (pUserInfo->CharacterID);	// 캐릭터 모델 ID
			playerInformation->name = (pUserInfo->UserName);
			playerInformation->radius = (0.4f);

			playerInformation->height = pUserInfo->AbilityHeight;
			playerInformation->winspan = pUserInfo->AbilityWinspan;
			playerInformation->gender = pUserInfo->AbilityGender;
			playerInformation->handedness = pUserInfo->AbilityHandedness;

			// 게임모드에 관계없이 일단 후보자들 모아둔다.
			if (pUserInfo->AiLevel == kIS_NOT_AI)
			{
				PushAiHostCandidateUser(playerInformation->userid, playerInformation->id);
			}
			
			playerInformation->role = (ConvertToCharacterRole(pUserInfo->CharacterRole));
			pAbility->speed=(pUserInfo->AbilitySpeed * kTYPE_CAST_TO_FLOAT);
			pAbility->vertical=(pUserInfo->AbilityVertical * kTYPE_CAST_TO_FLOAT);
			pAbility->strength=(pUserInfo->AbilityStrength * kTYPE_CAST_TO_FLOAT);
			pAbility->pass=(pUserInfo->AbilityPass * kTYPE_CAST_TO_FLOAT);
			pAbility->dribble=(pUserInfo->AbilityDribble * kTYPE_CAST_TO_FLOAT);
			pAbility->rebound=(pUserInfo->AbilityRebound * kTYPE_CAST_TO_FLOAT);
			pAbility->block=(pUserInfo->AbilityBlock * kTYPE_CAST_TO_FLOAT);
			pAbility->steal=(pUserInfo->AbilitySteal * kTYPE_CAST_TO_FLOAT);
			pAbility->midrangeshot=(pUserInfo->AbilityMidRangeShot * kTYPE_CAST_TO_FLOAT);
			pAbility->threepointshot=(pUserInfo->AbilityThreePointShot * kTYPE_CAST_TO_FLOAT);
			pAbility->postshot=(pUserInfo->AbilityPostShot * kTYPE_CAST_TO_FLOAT);
			pAbility->dunk=(pUserInfo->AbilityDunk * kTYPE_CAST_TO_FLOAT);
			pAbility->layup=(pUserInfo->AbilityLayUp * kTYPE_CAST_TO_FLOAT);
			pAbility->postdefense=(pUserInfo->AbilityPostDefense * kTYPE_CAST_TO_FLOAT);
			pAbility->perimeterdefense=(pUserInfo->AbilityPerimeterDefense * kTYPE_CAST_TO_FLOAT);
			pAbility->postmove=(pUserInfo->AbilityPostMove * kTYPE_CAST_TO_FLOAT);

			playerInformation->ability = std::unique_ptr<F4PACKET::SPlayerAbilityT>(pAbility);
			playerInformation->abilitymodification = std::unique_ptr<F4PACKET::SPlayerAbilityT>(pModAbility);

			for (int j = 0; j < pUserInfo->vSkillInfo.size(); ++j)
			{
				SRedisMatchCharacterSkillInfo* pInfo = pUserInfo->vSkillInfo[j];

				F4PACKET::SKILL_INDEX index;

				auto skill_sn = pInfo->SkillIndex;
				if (F4PACKET::EnumNameSKILL_INDEX((F4PACKET::SKILL_INDEX)skill_sn))
				{
					index = (F4PACKET::SKILL_INDEX)skill_sn;
					F4PACKET::SSkillInfo skillInfo(pInfo->SkillSlot, index, pInfo->SkillLevel, pInfo->ForceAwaken);
					
					playerInformation->skills.push_back(skillInfo);
				}
				else
				{
					log_message = "[ERROR] Invalid SkillSN CharacterID : " + to_string(pUserInfo->CharacterID) + ", SkillSN : " + to_string(skill_sn);
					ToLog(log_message.c_str());
					playerInformation->skills.clear();
					break;
				}
			}

			for (int j = 0; j < pUserInfo->vTagInfo.size(); ++j)
			{
				SRedisMatchCharacterTagInfo* pInfo = pUserInfo->vTagInfo[j];

				F4PACKET::MEDAL_INDEX index;

				auto sn = pInfo->TagIndex;

				if (F4PACKET::EnumNameMEDAL_INDEX((F4PACKET::MEDAL_INDEX)sn))
				{
					index = (F4PACKET::MEDAL_INDEX)sn;
					F4PACKET::SMedalInfo medalInfo(index, pInfo->TagGrade, pInfo->TagLevel, pInfo->TagValue, pInfo->TagCore);
					
					playerInformation->medals.push_back(medalInfo);
				}
				else
				{
					log_message = "[ERROR] Invalid TagSN CharacterID : " + to_string(pUserInfo->CharacterID) + ", TagSN : " + to_string(sn);
					ToLog(log_message.c_str());
					playerInformation->medals.clear();
					break;
				}
			}

			for (int j = 0; j < pUserInfo->vSignatureInfo.size(); ++j)
			{
				SRedisMatchCharacterSignatureInfo* pInfo = pUserInfo->vSignatureInfo[j];

				playerInformation->signatures.push_back(pInfo->SignatureIndex);
			}

			//! Emoji - by thinkingpig
			for (int j = 0; j < pUserInfo->vEmojiInfo.size(); ++j)
			{
				SRedisMatchCharacterEmojiInfo* pInfo = pUserInfo->vEmojiInfo[j];

				if (pInfo != nullptr)
				{
					F4PACKET::SEmojiInfo emojiInfo(pInfo->EmojiSlot, pInfo->EmojiIndex);

					playerInformation->emojis.push_back(emojiInfo);
				}
			}
			//! end

			//! Potential - by thinkingpig
			for (int j = 0; j < pUserInfo->vPotentialInfo.size(); ++j)
			{
				SRedisMatchCharacterPotentialInfo* pInfo = pUserInfo->vPotentialInfo[j];

				F4PACKET::POTENTIAL_INDEX index;

				uint32_t sn = pInfo->PotentialIndex;

				if (F4PACKET::EnumNamePOTENTIAL_INDEX((F4PACKET::POTENTIAL_INDEX)sn))
				{
					index = (F4PACKET::POTENTIAL_INDEX)sn;
					F4PACKET::SPotentialInfo potentialInfo
					(
						index,
						pInfo->PotentialLevel,
						pInfo->PotentialValue,
						pInfo->PotentialAwaken,
						pInfo->PotentialBloomRateLevel,
						pInfo->PotentialBloomRateValue,
						pInfo->PotentialBloomBuffLevel,
						pInfo->PotentialBloomBuffValue
					);

					playerInformation->potentials.push_back(potentialInfo);
				}
				else
				{
					log_message = "[ERROR] Invalid POTENTIAL_INDEX CharacterID : " + to_string(pUserInfo->CharacterID) + ", POTENTIAL_INDEX : " + to_string(sn);
					ToLog(log_message.c_str());
					playerInformation->potentials.clear();
					break;
				}
			}

			// 팀 능력치 인포 
			//

			F4PACKET::STeamAttributesInfoT* pTeamAttributeInfo = new F4PACKET::STeamAttributesInfoT();
			pTeamAttributeInfo->finishingatpost = pUserInfo->TeamAttributesInfo.FinishingAtPost;
			pTeamAttributeInfo->perimetershooting = pUserInfo->TeamAttributesInfo.PerimeterShooting;
			pTeamAttributeInfo->passing = pUserInfo->TeamAttributesInfo.Passing;
			pTeamAttributeInfo->ballhandling = pUserInfo->TeamAttributesInfo.BallHandling;

			pTeamAttributeInfo->postdefense = pUserInfo->TeamAttributesInfo.PostDefense;
			pTeamAttributeInfo->perimeterdefense = pUserInfo->TeamAttributesInfo.PerimeterDefense;
			pTeamAttributeInfo->deflections = pUserInfo->TeamAttributesInfo.Deflections;
			pTeamAttributeInfo->shotcontesting = pUserInfo->TeamAttributesInfo.ShotContesting;

			pTeamAttributeInfo->clutch = pUserInfo->TeamAttributesInfo.Clutch;
			pTeamAttributeInfo->startingmomentum = pUserInfo->TeamAttributesInfo.StartingMomentum;
			pTeamAttributeInfo->tenacity = pUserInfo->TeamAttributesInfo.Tenacity;
			pTeamAttributeInfo->concentration = pUserInfo->TeamAttributesInfo.Concentration;

			playerInformation->teamattributesinfo = std::unique_ptr<F4PACKET::STeamAttributesInfoT>(pTeamAttributeInfo);

			//! JukeBox - by steven
			for (int j = 0; j < pUserInfo->vJukeBoxInfo.size(); ++j)
			{
				SRedisMatchCharacterJukeBoxInfo* pInfo = pUserInfo->vJukeBoxInfo[j];
				uint32_t musicid = pInfo->musicid;
				playerInformation->jukebox.push_back(musicid);
			}

			//
			//! end
			//! 
			playerInformation->specialcharacterlevel = pUserInfo->specialCharacterLevel;

			playerInformation->ceremony1 = (pUserInfo->Ceremony1);
			playerInformation->ceremony2=(pUserInfo->Ceremony2);
			playerInformation->ceremony3=(pUserInfo->Ceremony3);
			playerInformation->ceremonywin=(pUserInfo->CeremonyWin);
			playerInformation->ceremonylose=(pUserInfo->CeremonyLose);
			playerInformation->ceremonydraw=(pUserInfo->CeremonyDraw);
			playerInformation->ceremonymvp=(pUserInfo->CeremonyMVP);

			playerInformation->passivetype = static_cast<F4PACKET::CHARACTER_PASSIVE_TYPE>(pUserInfo->PassiveType);
			playerInformation->elo = pUserInfo->elo;
			playerInformation->burstvalue = pUserInfo->intBurstValue; // added by steven, 2024-10-24

			playerInformation->teamcontrolluser = pUserInfo->TeamControllUser;
			playerInformation->autoplayuser = pUserInfo->AutoPlayUser;
			playerInformation->effectpassiveid = pUserInfo->effectPassiveID;
			playerInformation->cloakid = pUserInfo->cloakID;

			//! 20230728 접속 프로세스 변경 - by thinkingpig
			// 일반 유저인경우
			if (pUserInfo->AiLevel == kIS_NOT_AI)
			{
				// 매칭해서 들어온 유저는 접속여부에 상관없이 일단 생성
				CHostUserInfo* pUser = CreateUser(pUserInfo->UserID);
				if (pUser == nullptr)
				{
					log_message = "OnRedisMatchInfo CreateUser is fail RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(pUserInfo->UserID);
					ToLog(log_message.c_str());
				}
			}
			else
			{
				playerInformation->userid = ai_host_user;

				if (GetModeType() == EMODE_TYPE::TRIO || GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED)
				{
					if (pUserInfo->Team == "0")
					{
						playerInformation->userid = trio_home_ai_host_user;
					}
					else
					{
						playerInformation->userid = trio_away_ai_host_user;
					}
				}
			}

			DHOST_TYPE_UINT32 player_number = kUINT32_INIT;

			// 트리오 모드일때만 플레이어넘버 활성화 (비매너 기록을 1p에만 몰아주기 위해서 - 중국팀 요청)
			if (GetModeType() == EMODE_TYPE::TRIO || GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED)
			{
				player_number = pUserInfo->playerNumber;
			}


			if (m_CharacterManager->CreateCharacter(playerInformation, player_number, m_VerifyManager->GetCharacterIndex(playerInformation)))
			{
				log_message = "OnRedisMatchInfo CreateCharcter CharacterID : " + to_string(playerInformation->id) 
					+ ", UserID : " + to_string(pUserInfo->UserID) 
					+ ", Name : " + playerInformation->name.c_str() 
					+ ", AiLevel : " + to_string(playerInformation->ailevel) 
					+ ", Team : " + to_string(playerInformation->team) 
					+ ", TeamControllUser : " + to_string(playerInformation->teamcontrolluser)
					+ ", AutoPlayUser : " + to_string(playerInformation->autoplayuser)
					+ ", PlayerNumber : " + to_string(player_number) 
					+ ", CloakID : " + to_string(pUserInfo->cloakID)
					+ ", EffectPassiveID : " + to_string(playerInformation->effectpassiveid);

				ToLog(log_message.c_str());

				m_GameManager->SetScore(playerInformation->team, kINT32_INIT);

				//ai 유저인경우 캐릭터 기록에 ai 유저아이디를 셋팅한다.
				if (playerInformation->ailevel > kIS_NOT_AI)
				{
					if (GetModeType() == EMODE_TYPE::TRIO || GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED)
					{
						if (pUserInfo->Team == "0")
						{
							vec_trio_home_ai_character.push_back(playerInformation->id);
						}
						else
						{
							vec_trio_away_ai_character.push_back(playerInformation->id);
						}

						//playerInformation->ailevel = kAI_DEFAULT_LEVEL; // 레디스에서 준 값을 처리함 
					}
					else
					{
						vec_ai_character.push_back(playerInformation->id);
					}
					
					m_CharacterManager->SetCharacterUserID(playerInformation->id, pUserInfo->UserID);
				}

				AddLastCharacterSN();
			}
			else
			{
				log_message = "OnRedisMatchInfo CreateCharcter fail pCharacter is null UserID : " + to_string(pUserInfo->UserID);
				ToLog(log_message.c_str());
			}

			// HandicapeRule 적용 
			SetScore(pData->HandiCapRule.HandicapTeam, pData->HandiCapRule.HandicapScore);
			GetGameManager()->SetHandicapRulePoint(pData->HandiCapRule.HandicapScore);

		}//! end for

		if (GetModeType() == EMODE_TYPE::TRIO || GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED)
		{
			// 나 말고 전부 AI (혼자하는 경우)
			if (trio_home_ai_host_user == trio_away_ai_host_user)
			{
				CHostUserInfo* pUser = FindUser(trio_home_ai_host_user);
				if (pUser != nullptr)
				{
					for (int i = 0; i < vec_trio_home_ai_character.size(); ++i)
					{
						pUser->PushUserWithAiCharacter(vec_trio_home_ai_character[i]);

						log_message = "OnRedisMatchInfo PushUserWithAiCharacter UserID : " + std::to_string(pUser->GetUserID()) + ", CharacterSN : " + to_string(vec_trio_home_ai_character[i]);
						ToLog(log_message.c_str());
					}

					for (int i = 0; i < vec_trio_away_ai_character.size(); ++i)
					{
						pUser->PushUserWithAiCharacter(vec_trio_away_ai_character[i]);

						log_message = "OnRedisMatchInfo PushUserWithAiCharacter UserID : " + std::to_string(pUser->GetUserID()) + ", CharacterSN : " + to_string(vec_trio_away_ai_character[i]);
						ToLog(log_message.c_str());
					}
				}
				else
				{
					log_message = "OnRedisMatchInfo vec_ai_character not empty pUser is null UserID : " + std::to_string(ai_host_user);
					ToLog(log_message.c_str());
				}
			}
			else
			{	// 트리오 모드로 유저수가 1:1 인 경우
				{
					CHostUserInfo* pTrioHomeAiHostUser = FindUser(trio_home_ai_host_user);
					if (pTrioHomeAiHostUser != nullptr)
					{
						for (int i = 0; i < vec_trio_home_ai_character.size(); ++i)
						{
							pTrioHomeAiHostUser->PushUserWithAiCharacter(vec_trio_home_ai_character[i]);

							log_message = "OnRedisMatchInfo PushUserWithAiCharacter UserID : " + std::to_string(pTrioHomeAiHostUser->GetUserID()) + ", CharacterSN : " + to_string(vec_trio_home_ai_character[i]);
							ToLog(log_message.c_str());
						}
					}
				}
				
				{
					CHostUserInfo* pTrioAwayAiHostUser = FindUser(trio_away_ai_host_user);
					if (pTrioAwayAiHostUser != nullptr)
					{
						for (int i = 0; i < vec_trio_away_ai_character.size(); ++i)
						{
							pTrioAwayAiHostUser->PushUserWithAiCharacter(vec_trio_away_ai_character[i]);

							log_message = "OnRedisMatchInfo PushUserWithAiCharacter UserID : " + std::to_string(pTrioAwayAiHostUser->GetUserID()) + ", CharacterSN : " + to_string(vec_trio_away_ai_character[i]);
							ToLog(log_message.c_str());
						}
					}
				}
			}
		}
		else
		{
			if (!vec_ai_character.empty())
			{
				CHostUserInfo* pUser = FindUser(ai_host_user);
				if (pUser != nullptr)
				{
					for (int i = 0; i < vec_ai_character.size(); ++i)
					{
						pUser->PushUserWithAiCharacter(vec_ai_character[i]);

						log_message = "OnRedisMatchInfo PushUserWithAiCharacter UserID : " + std::to_string(pUser->GetUserID()) + ", CharacterSN : " + to_string(vec_ai_character[i]);
						ToLog(log_message.c_str());
					}
				}
				else
				{
					log_message = "OnRedisMatchInfo vec_ai_character not empty pUser is null UserID : " + std::to_string(ai_host_user);
					ToLog(log_message.c_str());
				}
			}
		}

		SetRedisLoadValue(true, GetRoomElapsedTime());

		log_message = "OnRedisMatchInfo load complete...";
		ToLog(log_message.c_str());
	}
	else
	{
		log_message = "OnRedisMatchInfo is null";
		ToLog(log_message.c_str());
	}
}

void CHost::RedisKickReasonSend(DHOST_TYPE_USER_ID UserID, ECHARACTER_KICK_TYPE KickType)
{
	if (m_CallbackRedisSend != nullptr)
	{
		if (GetRedisSaveMatchResult() == false && m_GameManager->GetKickReasonSaveMode())
		{
			DHOST_TYPE_UINT32 kick_reason = static_cast<uint32_t>(KickType);
			{
				string log_message = "RedisKickReasonSend UserID : " + to_string(UserID) + ", KickType : " + to_string(kick_reason);
				ToLog(log_message.c_str());
			}

			m_CallbackRedisSend(UserID, kick_reason);
		}	
	}
	else
	{
		string log_message = "RedisKickReasonSend m_CallbackRedisSend is null";
		ToLog(log_message.c_str());
	}
}

DHOST_TYPE_CHARACTER_SN CHost::GetLastCharacterSN()
{
	return m_CharacterManager->GetLastCharacterSN();
}

DHOST_TYPE_FLOAT CHost::GetBalanceValue(const char* strkey)
{
	if (strkey == nullptr)
	{
		string str_log = "[FILE_ERROR] GetBalanceValue key is null";
		ToLog(str_log.c_str());
		return 0.0f;
	}

	CDataManagerBalance* pTable = GetBalanceTable();
	if (pTable == nullptr)
	{
		string str_log = "[FILE_ERROR] GetBalanceValue table is null";
		ToLog(str_log.c_str());
		return 0.0f;
	}

	return pTable->GetValue(strkey);
}

CCharacterManager* CHost::GetCharacterManager()
{
	return m_CharacterManager;
}

CVerifyManager* CHost::GetVerifyManager()
{
	return m_VerifyManager;
}

void CHost::AddLastCharacterSN()
{
	m_CharacterManager->AddLastCharacterSN();
}

void CHost::SortCharacterInformation()
{
	m_CharacterManager->SortCharacters();
}

void CHost::SortCharacterInformationWithRole()
{
	m_CharacterManager->SortCharactersWithRole();
}

void CHost::MixCharacterTeam()
{
	m_CharacterManager->MixCharacterTeamInRole();
}

F4PACKET::SPlayerInformationT* CHost::FindCharacterInformation(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetCharacterInformation(characterSN);
}

void CHost::GetCharacterInformationVec(std::vector<F4PACKET::SPlayerInformationT*>& vec)
{
	m_CharacterManager->GetCharacterInformationVec(vec);
}

void CHost::GetHomeCharacterInformationVec(std::vector<F4PACKET::SPlayerInformationT*>& vec)
{
	m_CharacterManager->GetHomeCharacterInformationVec(vec);
}

void CHost::GetAwayCharacterInformationVec(std::vector<F4PACKET::SPlayerInformationT*>& vec)
{
	m_CharacterManager->GetAwayCharacterInformationVec(vec);
}

const TB::SVector3 CHost::GetCharacterPosition(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetCharacterPosition(characterSN);
}

F4PACKET::SPlayerAction* CHost::GetCharacterAction(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetCharacterAction(characterSN);
}

F4PACKET::SPlayerAction* CHost::GetCharacterStateAction(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetCharacterStateAction(characterSN);
}

F4PACKET::SHOT_TYPE CHost::GetCharacterActionShotType(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetCharacterActionShotType(characterSN);
}

DHOST_TYPE_BALL_NUMBER CHost::GetCharacterActionPotentialBloom(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetCharacterActionPotentialBloom(characterSN);
}

DHOST_TYPE_INT32 CHost::GetCharacterTeam(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetCharacterTeam(characterSN);
}

SCharacterAction* CHost::GetCharacterActionInfo(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetCharacterActionInfo(characterSN);
}

void CHost::SetCharacterAction(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_GAME_TIME_F actionTime, F4PACKET::ACTION_TYPE actionType,const F4PACKET::SPlayerAction* pInfo, DHOST_TYPE_FLOAT speed, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_BALL_NUMBER potentialBloom)
{
	if (pInfo != nullptr)
	{
		F4PACKET::SPlayerAction fbPlayerAction;
		fbPlayerAction.mutate_id(pInfo->id());
		fbPlayerAction.mutate_keys(pInfo->keys());
		fbPlayerAction.mutate_number(pInfo->number());
		fbPlayerAction.mutate_directioninput(pInfo->directioninput());
		fbPlayerAction.mutable_positionlogic() = TB::SVector3(pInfo->positionlogic().x(), pInfo->positionlogic().y(), pInfo->positionlogic().z());
		fbPlayerAction.mutate_yawlogic(pInfo->yawlogic());
		fbPlayerAction.mutate_collisionpriority(pInfo->collisionpriority());
		fbPlayerAction.mutate_skillindex((F4PACKET::SKILL_INDEX)pInfo->skillindex());

		m_CharacterManager->SetCharacterAction(characterSN, actionTime, (F4PACKET::ACTION_TYPE)actionType, &fbPlayerAction, speed, shotType, potentialBloom);
	}
	else
	{
		m_CharacterManager->SetCharacterAction(characterSN, actionTime, (F4PACKET::ACTION_TYPE)actionType, nullptr, speed, shotType, potentialBloom);
	}
}

void CHost::SetCharacterActionOverlap(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::ACTION_TYPE actionType, F4PACKET::SKILL_INDEX value)
{
	m_CharacterManager->SetCharacterActionOverlap(characterSN, (F4PACKET::ACTION_TYPE)actionType, (F4PACKET::SKILL_INDEX)value);
}

void CHost::SetCharacterActionOverlapDestYaw(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::ACTION_TYPE actionType, DHOST_TYPE_FLOAT DestYaw)
{
	m_CharacterManager->SetCharacterActionOverlapDestYaw(characterSN, actionType, DestYaw);
}

void CHost::AddCharacterActionDeque(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->AddCharacterActionDeque(characterSN);
}

void CHost::AddServerPostionDeque(DHOST_TYPE_CHARACTER_SN characterSN, ServerCharacterPosition position)
{
	m_CharacterManager->AddServerPostionDeque(characterSN, position);
}

void CHost::GenerateCharacterBuff(DHOST_TYPE_CHARACTER_SN characterSN, SCharacterBuff buff)
{
	m_CharacterManager->GenerateCharacterBuff(characterSN, buff);
}

std::deque<SCharacterAction> CHost::GetCharacterActionDeque(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetCharacterActionDeque(characterSN);
}

std::deque<ServerCharacterPosition> CHost::GetServerPostionDeque(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetServerPostionDeque(characterSN);
}

DHOST_TYPE_CHARACTER_SN	CHost::GetCharacterSN(DHOST_TYPE_INT32 indexTeam, DHOST_TYPE_INT32 indexCharacter)
{
	return m_CharacterManager->GetCharacterSN(indexTeam, indexCharacter);
}

size_t CHost::GetCharacterTeamSize(DHOST_TYPE_INT32 teamIndex)
{
	return m_CharacterManager->GetTeamCharacterSize(teamIndex);
}

void CHost::SetTeamsActive(DHOST_TYPE_INT32 team1, DHOST_TYPE_INT32 team2)
{
	return m_CharacterManager->SetTeamsActive(team1, team2);
}

DHOST_TYPE_INT32 CHost::GetIndexTeamWait()
{
	return m_CharacterManager->GetIndexTeamWait();
}

DHOST_TYPE_INT32 CHost::GetIndexOpponentTeam(DHOST_TYPE_INT32 teamIndex)
{
	return m_CharacterManager->GetIndexOpponentTeam(teamIndex);
}

void CHost::SetCharacterReadyPosition(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::ACTION_TYPE actionType, TB::SVector3 position)
{
	m_CharacterManager->SetCharacterReadyPosition(characterSN, (F4PACKET::ACTION_TYPE)actionType, TB::SVector3(position.x(), position.y(), position.z()));
}

void CHost::SetCharacterMoveLock(DHOST_TYPE_CHARACTER_SN characterSN, EMOVEMENT_RESTRICTION value)
{
	m_CharacterManager->SetCharacterMoveLock(characterSN, value);
}

void CHost::SetCharacterMoveUnLock()
{
	m_CharacterManager->SetCharacterMoveUnLock();
}

DHOST_TYPE_UINT32 CHost::GetTeamRoleCount(DHOST_TYPE_INT32 team, ROLE role)
{
	return m_CharacterManager->GetRoleCount((DHOST_TYPE_INT32)team, (F4PACKET::ROLE)role);
}

void CHost::CalcFocusPoint()
{
	m_CharacterManager->CalcFocusPoint();
}

DHOST_TYPE_CHARACTER_SN CHost::GetMaxFocusPointCharacterInTeam(DHOST_TYPE_INT32 team)
{
	return m_CharacterManager->GetMaxFocusPointCharacterInTeam((DHOST_TYPE_INT32)team);
}

DHOST_TYPE_CHARACTER_SN CHost::GetMaxFocusPointCharacterInEnemyTeam(DHOST_TYPE_INT32 Scoreteam)
{
	return m_CharacterManager->GetMaxFocusPointCharacterInEnemyTeam((DHOST_TYPE_INT32)Scoreteam);
}

void CHost::PushCharacterRecord(SRedisMatchResult* pInfo)
{
	m_CharacterManager->PushCharacterRecord(pInfo, GetRoomElapsedTime());
}

F4PACKET::SPlayerInformationT* CHost::FindCharacterInformationWithUserID(DHOST_TYPE_USER_ID UserID)
{
	return m_CharacterManager->FindCharacterInformationWithUserID(UserID);
}

F4PACKET::SPlayerInformationT* CHost::FindCharacterInformationWithUserIDAndIsNotAi(DHOST_TYPE_USER_ID UserID)
{
	return m_CharacterManager->FindCharacterInformationWithUserIDAndIsNotAi(UserID);
}

void CHost::AddCharacterShootInfo(DHOST_TYPE_CHARACTER_SN characterSN, float x, float z, uint32_t zone, bool goal)
{
	m_CharacterManager->AddCharacterShootInfo(characterSN, x, z, zone, goal);
}

void CHost::GetCharacterRecordVec(std::vector<SCharacterRecord*>& vec)
{
	m_CharacterManager->GetCharacterRecordVec(vec);
}

F4PACKET::RECORD_TYPE CHost::ConvertToProtobufRecordType(ECHARACTER_RECORD_TYPE type)
{
	F4PACKET::RECORD_TYPE record_type = F4PACKET::RECORD_TYPE::None;

	switch (type)
	{
		case ECHARACTER_RECORD_TYPE::ASSIST:
		{
			record_type = F4PACKET::RECORD_TYPE::Assist;
		}
		break;
		case ECHARACTER_RECORD_TYPE::BLOCK:
		{
			record_type = F4PACKET::RECORD_TYPE::Block;
		}
		break;
		case ECHARACTER_RECORD_TYPE::LOOSE_BALL:
		{
			record_type = F4PACKET::RECORD_TYPE::LooseBall;
		}
		break;
		case ECHARACTER_RECORD_TYPE::REBOUND:
		{
			record_type = F4PACKET::RECORD_TYPE::Rebound;
		}
		break;
		case ECHARACTER_RECORD_TYPE::STEAL:
		{
			record_type = F4PACKET::RECORD_TYPE::Steal;
		}
		break;
		case ECHARACTER_RECORD_TYPE::TRY_2POINT:
		{
			record_type = F4PACKET::RECORD_TYPE::TryPointTwo;
		}
		break;
		case ECHARACTER_RECORD_TYPE::SUC_2POINT:
		{
			record_type = F4PACKET::RECORD_TYPE::PointTwo;
		}
		break;
		case ECHARACTER_RECORD_TYPE::TRY_3POINT:
		{
			record_type = F4PACKET::RECORD_TYPE::TryPointThree;
		}
		break;
		case ECHARACTER_RECORD_TYPE::SUC_3POINT:
		{
			record_type = F4PACKET::RECORD_TYPE::PointThree;
		}
		break;
		case ECHARACTER_RECORD_TYPE::TURN_OVER:
		{
			record_type = F4PACKET::RECORD_TYPE::TurnOver;
		}
		break;
		case ECHARACTER_RECORD_TYPE::PASS:
		{
			record_type = F4PACKET::RECORD_TYPE::Pass;
		}
		break;
		case ECHARACTER_RECORD_TYPE::NICE_PASS:
		{
			record_type = F4PACKET::RECORD_TYPE::NicePass;
		}
		break;
		case ECHARACTER_RECORD_TYPE::DIVING_CATCH:
		{
			record_type = F4PACKET::RECORD_TYPE::DivingCatch;
		}
		break;
		case ECHARACTER_RECORD_TYPE::OVER_PASS:
		{
			record_type = F4PACKET::RECORD_TYPE::OverPass;
		}
		break;
		default:
		{
			string log_message = "ConvertToProtobufRecordType type is invalid : " + to_string((int)type);
			ToLog(log_message.c_str());
		}
		break;
	}

	return record_type;
}

DHOST_TYPE_STR CHost::ConvertToStringTeam(DHOST_TYPE_INT32 team)
{
	DHOST_TYPE_STR str_team = "NONE";

	switch (team)
	{
		case 0:
		{
			str_team = "HOME";
		}
		break;
		case 1:
		{
			str_team = "AWAY";
		}
		break;
		case 2:
		{
			str_team = "WAIT";
		}
		break;
		default:
		{
			string log_message = "ConvertToEnumTeam team is invalid : " + to_string((int)team);
			ToLog(log_message.c_str());
		}
		break;
	}

	return str_team;
}

CHostUserInfo* CHost::FindUser(DHOST_TYPE_INT32 UserID)
{
	auto iter = m_HostUserMap.find(UserID);

	if (iter == m_HostUserMap.end())
		return nullptr;

	return iter->second;
}

CHostUserInfo* CHost::FindUserWithConnectState(ECONNECT_STATE value)
{
	CHostUserInfo* pUser = nullptr;

	for (auto& it : m_HostUserMap)
	{
		pUser = it.second;

		if (nullptr == pUser)
		{
			continue;
		}

		if (pUser->GetUserType() == EUSER_TYPE::OBSERVER)
		{
			continue;
		}

		if (pUser->GetConnectState() == value)
		{
			break;
		}
	}

	return pUser;
}

//! 20230728 접속 프로세스 변경 - by thinkingpig
CHostUserInfo* CHost::CreateUser(DHOST_TYPE_INT32 UserID, EUSER_TYPE UserType)
{
	CHostUserInfo* pUser = FindUser(UserID);

	if (pUser == nullptr)
	{
		DHOST_TYPE_UINT64 cast_user_id = static_cast<DHOST_TYPE_UINT64>(UserID);

		pUser = new CHostUserInfo(UserID, reinterpret_cast<DHOST_TYPE_UINT64*>(cast_user_id), this, GetRoomElapsedTime(), UserType);

		m_HostUserMap.insert(HostUserMapType::value_type(UserID, pUser));

		// AI가 아닌 실제 유저만 남기는 로그 
		string log_message = "CreateHostUser RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(UserID) + ", UserType : " + ConvertToStringUserType(UserType) + ", MapSize : " + to_string(m_HostUserMap.size());
		ToLog(log_message.c_str());
	}

	return pUser;
}

void CHost::SetGameCap(DHOST_TYPE_INT32 gameCap)
{
	//! game_handler 에서 아웃게임에서 받아온 데이터로 셋팅함 (방이 생성될 때 참가하는 유저 수)
	string log_message = "SetGameCap gameCap : " + to_string(gameCap);
	ToLog(log_message.c_str());

	m_GameCap = gameCap;
}

DHOST_TYPE_INT32 CHost::GetGameCap()
{
	return m_GameCap;
}

DHOST_TYPE_USER_ID CHost::FindUserIDWithCharacterSN(DHOST_TYPE_CHARACTER_SN characterSN)
{
	F4PACKET::SPlayerInformationT* pInfo = FindCharacterInformation(characterSN);
	if (pInfo != nullptr)
	{
		return static_cast<DHOST_TYPE_USER_ID>(pInfo->userid);
	}

	return kUINT32_INIT;
}

DHOST_TYPE_INT32 CHost::FindUserTeam(DHOST_TYPE_USER_ID UserID)
{
	F4PACKET::SPlayerInformationT* pInfo = FindCharacterInformationWithUserIDAndIsNotAi(UserID);

	if (pInfo != nullptr)
	{
		return pInfo->team;
	}

	return -1;
}

DHOST_TYPE_CHARACTER_SN CHost::CheckAssist(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER ballNumber)
{
	DHOST_TYPE_CHARACTER_SN assistCharacterSN = kCHARACTER_SN_INIT;

	DHOST_TYPE_BALL_NUMBER target_ball = ballNumber - 2;

	SBallActionInfo prevBallInfo;

	if (FindTargetBallActionInfo(target_ball, prevBallInfo))
	{
		if (prevBallInfo.BallNumber == target_ball)
		{
			DHOST_TYPE_INT32 PreBallOwnerTeam = GetCharacterTeam(prevBallInfo.BallOwner);
			DHOST_TYPE_INT32 ScorerTeam = GetCharacterTeam(characterSN);

			//! 상해팀 요청에 따라 셀프앨리웁은 시도가 아니라 슛까지 성공해야 된다고해서 simulation 에서 처리하지 않고, 골인 쪽에서 처리함
			if (characterSN == prevBallInfo.BallOwner && prevBallInfo.SkillIndex == (DHOST_TYPE_UINT32)SKILL_INDEX::skill_selfAlleyOop)
			{
				m_CharacterManager->IncreaseSkillSucCount(characterSN, (DHOST_TYPE_UINT32)SKILL_INDEX::skill_selfAlleyOop);
			}

			if (characterSN != prevBallInfo.BallOwner && PreBallOwnerTeam == ScorerTeam && (((int)(ConvertToProtobufBallState(prevBallInfo.Ballstate)) & (int)F4PACKET::BALL_STATE::ballState_pass) == (int)F4PACKET::BALL_STATE::ballState_pass ||
				(int)ConvertToProtobufBallState(prevBallInfo.Ballstate) == (int)F4PACKET::BALL_STATE::ballState_alleyOop))
			{
				m_CharacterManager->UpdateRecord(prevBallInfo.BallOwner, ECHARACTER_RECORD_TYPE::ASSIST, GetOverTime());
				assistCharacterSN = prevBallInfo.BallOwner;
			}
		}
	}

	return assistCharacterSN;
}

DHOST_TYPE_BOOL CHost::CheckAssistAtTime(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER ballNumber, DHOST_TYPE_GAME_TIME_F gameTime)
{
	
	float assistTime = GetBalanceTable()->GetValue("VTimeForAssist");

	DHOST_TYPE_BALL_NUMBER target_ball = ballNumber - 1;

	SBallActionInfo prevBallInfo;

	if (FindTargetBallActionInfo(target_ball, prevBallInfo))
	{
		if (target_ball == prevBallInfo.BallNumber && prevBallInfo.Ballstate == EBALL_STATE::ALLEYOOP || (prevBallInfo.Ballstate == EBALL_STATE::PICK_PASS && (prevBallInfo.fGameTime - gameTime) < assistTime))
		{
			return true;
		}
	}

	return false;
}

DHOST_TYPE_BOOL CHost::CheckIfPrevBallIsPass(DHOST_TYPE_BALL_NUMBER ballNumber)
{
	DHOST_TYPE_BALL_NUMBER target_ball = ballNumber - 1;

	SBallActionInfo prevBallInfo;
	if (FindTargetBallActionInfo(target_ball, prevBallInfo))
	{
		if (prevBallInfo.BallNumber == target_ball)
		{
			if (prevBallInfo.Ballstate == EBALL_STATE::PICK_PASS || prevBallInfo.Ballstate == EBALL_STATE::PASS_LOOSE)
			{
				return true;
			}
		}
	}
	
	return false;
}

DHOST_TYPE_BOOL CHost::GetMostCharacterRerecordWithType(DHOST_TYPE_CHARACTER_SN characterSN, RECORD_TYPE Type, DHOST_TYPE_BOOL allowEqual)
{
	DHOST_TYPE_BOOL result = false;

	if (GetCharacterManager() != nullptr)
	{
		result = GetCharacterManager()->GetMostCharacterRerecordWithType(characterSN, Type, allowEqual);
	}

	return result;
}

void CHost::SetCharacterStateAction(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::SPlayerAction sInfo, F4PACKET::ACTION_TYPE actionType, DHOST_TYPE_GAME_TIME_F time)
{
	if (GetCharacterManager() != nullptr)
	{
		GetCharacterManager()->SetCharacterStateAction(characterSN, sInfo, actionType, time);
	}
}

F4PACKET::ROLE CHost::ConvertToCharacterRole(ECHARACTER_ROLE value)
{
	F4PACKET::ROLE result = F4PACKET::ROLE::MIN;

	switch (value)
	{
		case ECHARACTER_ROLE::ROLE_CENTER:
			result = F4PACKET::ROLE::role_center;
			break;
		case ECHARACTER_ROLE::ROLE_FOWARD_POWER:
			result = F4PACKET::ROLE::role_foward_power;
			break;
		case ECHARACTER_ROLE::ROLE_FOWARD_SMALL:
			result = F4PACKET::ROLE::role_foward_small;
			break;
		case ECHARACTER_ROLE::ROLE_GUARD_POINT:
			result = F4PACKET::ROLE::role_guard_point;
			break;
		case ECHARACTER_ROLE::ROLE_GUARD_SHOOTING:
			result = F4PACKET::ROLE::role_guard_shooting;
			break;
		default:
		{
			string log_message = "ConvertToCharacterRole Invalid character role : " + std::to_string((int)value);
			ToLog(log_message.c_str());
		}
		break;
	}

	return result;
}

DHOST_TYPE_UINT32 CHost::GetSpeedHackLevel(DHOST_TYPE_USER_ID UserID)
{
	F4PACKET::SPlayerInformationT* pCharacter = FindCharacterInformationWithUserIDAndIsNotAi(UserID);

	if (pCharacter != nullptr)
	{
		GetCharacterManager()->GetSpeedHackLevel(pCharacter->id);
	}

	return kUINT32_INIT;
}

void CHost::SetSpeedHackLevel(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_UINT32 value)
{
	F4PACKET::SPlayerInformationT* pCharacter = FindCharacterInformationWithUserIDAndIsNotAi(UserID);

	if (pCharacter != nullptr)
	{
		GetCharacterManager()->SetSpeedHackLevel(pCharacter->id, value);
	}
}

DHOST_TYPE_FLOAT CHost::GetSpeedHackTimeAccumulate(DHOST_TYPE_USER_ID UserID)
{
	F4PACKET::SPlayerInformationT* pCharacter = FindCharacterInformationWithUserIDAndIsNotAi(UserID);

	if (pCharacter != nullptr)
	{
		GetCharacterManager()->GetSpeedHackTimeAccumulate(pCharacter->id);
	}

	return kFLOAT_INIT;
}

void CHost::SetSpeedHackTimeAccumulate(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_FLOAT value)
{
	F4PACKET::SPlayerInformationT* pCharacter = FindCharacterInformationWithUserIDAndIsNotAi(UserID);

	if (pCharacter != nullptr)
	{
		GetCharacterManager()->SetSpeedHackTimeAccumulate(pCharacter->id, value);
	}
}

DHOST_TYPE_BOOL CHost::CheckSpeedHackBothCase(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_USER_ID CharID, DHOST_TYPE_FLOAT ServerElapsedTime,
					   DHOST_TYPE_FLOAT ClientElapsedTime, 
					   TB::SVector3 position,
					   DHOST_TYPE_FLOAT speed,
					   TB::SVector3 &rePosition,
	                   DHOST_TYPE_BOOL& normal)
{
	//! 클라의 시간이 서버보다 400ms 이상 느린 경우 
	if (ServerElapsedTime > ClientElapsedTime + 0.4f)
	{
		/*
		string log_message = "[HACK_CHECK] DETECT_SPEED_HACK_SLOW RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", GameTime : " + std::to_string(GetGameTime())
			+ ", UserID : " + std::to_string(UserID)
			+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID)
			+ ", ServerElapsedTime : " + std::to_string(ServerElapsedTime)
			+ ", ClientElapsedTime : " + std::to_string(ClientElapsedTime);
		ToLog(log_message.c_str()); */

		CCharacter* pCharacter = GetCharacterManager()->GetCharacter(CharID);
		if (!pCharacter) return false;

		rePosition = CommonFunction::ConvertJoVectorToTBVector(pCharacter->GetPosition());

		normal = false;
		return true;
	}
	else
	{
		CBallController* pBallController = BallControllerGet();
		if (pBallController)
		{
			if (pBallController->isOwnerChanaging())
			{
				return false;
			}
		}

		CCharacter* pCharacter = GetCharacterManager()->GetCharacter(CharID);
		if (!pCharacter) return false;

		TB::SVector3 newPosition;
		// 캐릭터 이동 체크 
		if (false) // pCharacter->IsPositionWrong(newPosition))
		{
			SPlayerInformationT* pUser = pCharacter->GetCharacterInformation();
			CHostUserInfo* pHostUser = FindUser(pUser->userid);

			
			string log_message = "[HACK_CHECK] CheckSpeedHackBothCase RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", GameTime : " + std::to_string(GetGameTime())
				+ ", UserID : " + std::to_string(UserID)
				+ ", ServerElapsedTime : " + std::to_string(ServerElapsedTime)
				+ ", ClientElapsedTime : " + std::to_string(ClientElapsedTime);

			ToLog(log_message.c_str()); 


			pHostUser->SpeedHackDetected(SPEEDHACK_CHECK::MOVE);

			if (pHostUser)
			{

				pHostUser->SpeedHackDetected(SPEEDHACK_CHECK::MOVE);

				if (pHostUser->IsSpeedhackDectected_Low())
				{
					// 아직 보류 
					// return true; // 스피드핵 감지 로직에 걸렸다면 이동 제약 걸기 
				}

			}
		}
	}

	return false;
}

ECHARACTER_KICK_TYPE CHost::GetCharacterKickType(DHOST_TYPE_USER_ID UserID)
{
	F4PACKET::SPlayerInformationT* pCharacter = FindCharacterInformationWithUserIDAndIsNotAi(UserID);

	if (pCharacter != nullptr)
	{
		GetCharacterManager()->GetCharacterKickType(pCharacter->id);
	}

	return ECHARACTER_KICK_TYPE::NONE;
}

void CHost::SetCharacterKickType(DHOST_TYPE_USER_ID UserID, ECHARACTER_KICK_TYPE value)
{
	F4PACKET::SPlayerInformationT* pCharacter = FindCharacterInformationWithUserIDAndIsNotAi(UserID);

	if (pCharacter != nullptr)
	{
		// 이미 스피드핵 판정으로 쫒아냈다면 네트워크 불량으로 인해 값이 덮어써지면 안된다.
		if (GetCharacterManager()->GetCharacterKickType(pCharacter->id) == ECHARACTER_KICK_TYPE::SPEED_HACK || GetCharacterManager()->GetCharacterKickType(pCharacter->id) == ECHARACTER_KICK_TYPE::CHEAT_HACK)
		{
			return;
		}

		GetCharacterManager()->SetCharacterKickType(pCharacter->id, value);
	}

	string log_message = "SetCharacterKickType RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + std::to_string(UserID) + ", KickType : " + std::to_string((int)value);
	ToLog(log_message.c_str());

	RedisKickReasonSend(UserID, value);
}

void CHost::IncreaseLatencyCount(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_FLOAT value)
{
	F4PACKET::SPlayerInformationT* pCharacter = FindCharacterInformationWithUserIDAndIsNotAi(UserID);

	if (pCharacter != nullptr)
	{
		GetCharacterManager()->IncreaseLatencyCount(pCharacter->id, value);
	}
}

void CHost::SetAvgPingLatency(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_FLOAT value)
{
	F4PACKET::SPlayerInformationT* pCharacter = FindCharacterInformationWithUserIDAndIsNotAi(UserID);

	if (pCharacter != nullptr)
	{
		GetCharacterManager()->SetAvgPingLatency(pCharacter->id, value);
	}
}

DHOST_TYPE_CHARACTER_SN CHost::GetMVP()
{
	return m_CharacterManager->GetMVP();
}

void CHost::SetMVP(DHOST_TYPE_CHARACTER_SN value)
{
	m_CharacterManager->SetMVP(value);
}

DHOST_TYPE_BOOL CHost::GetNoTableCharacterSend()
{
	return m_CharacterManager->GetNoTableCharacterSend();
}

void CHost::SetNoTableCharacterSend(DHOST_TYPE_BOOL value)
{
	m_CharacterManager->SetNoTableCharacterSend(value);
}

// 주목할만한 선수 있는지 확인
DHOST_TYPE_CHARACTER_SN CHost::CheckNoTableCharacterSoloCondition(DHOST_TYPE_CHARACTER_SN ScorerCharacterSN)
{
	DHOST_TYPE_CHARACTER_SN MaxFocusPointCharacterSN = kCHARACTER_SN_INIT;

	DHOST_TYPE_INT32 ScorerTeam = GetCharacterTeam(ScorerCharacterSN);

	if (ScorerTeam == -1)
	{
		string log_message = "[CheckNoTableCharacterSoloCondition] Invalid team CharacterSN : " + std::to_string(ScorerCharacterSN);
		ToLog(log_message.c_str());

		return kCHARACTER_SN_INIT;
	}

	// FocusPoint 는 팀 구분하지않고 전부 계산하고 (두번째 주목할 선수 뽑아야 될수도 있으니깐)
	CalcFocusPoint();

	MaxFocusPointCharacterSN = GetMaxFocusPointCharacterInTeam(ScorerTeam);

	return MaxFocusPointCharacterSN;
}

// 상대팀에 주목할만한 선수가 있는지 확인
DHOST_TYPE_CHARACTER_SN CHost::CheckNoTableCharacterDuoCondition(DHOST_TYPE_CHARACTER_SN enemyTeamMaxFocusCharacterSN)
{
	return GetMaxFocusPointCharacterInEnemyTeam(GetCharacterTeam(enemyTeamMaxFocusCharacterSN));
}

DHOST_TYPE_CHARACTER_SN CHost::UpdateRecord(DHOST_TYPE_CHARACTER_SN CharacterSN, DHOST_TYPE_BALL_NUMBER ballNumber, ECHARACTER_RECORD_TYPE recordType, DHOST_TYPE_BOOL assist)
{
	DHOST_TYPE_CHARACTER_SN assistCharacterSN = kCHARACTER_SN_INIT;

	//! 연장전부터는 기록 갱신을 하지 않는다 (중국팀 요청) 20240325 - by thinkingpig
	m_CharacterManager->UpdateRecord(CharacterSN, recordType, GetOverTime());

	if ((recordType == ECHARACTER_RECORD_TYPE::SUC_2POINT || recordType == ECHARACTER_RECORD_TYPE::SUC_3POINT) && assist)
	{
		assistCharacterSN = CheckAssist(CharacterSN, ballNumber);
	}

	return assistCharacterSN;
}

void CHost::UpdateRecordBasicTrainingSystem(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER ballNumber, F4PACKET::SKILL_INDEX skill, F4PACKET::SHOT_TYPE shotType)
{
	//! 2점 점프샷인지 확인
	switch (shotType)
	{
		case F4PACKET::SHOT_TYPE::shotType_closePostUnstready:
		case F4PACKET::SHOT_TYPE::shotType_hook:
		case F4PACKET::SHOT_TYPE::shotType_middle:
		case F4PACKET::SHOT_TYPE::shotType_middleUnstready:
		case F4PACKET::SHOT_TYPE::shotType_post:
		case F4PACKET::SHOT_TYPE::shotType_postUnstready:
		{
			m_CharacterManager->UpdateRecord(characterSN, ECHARACTER_RECORD_TYPE::JUMP_SHOT_2_POINT_COUNT, GetOverTime());
		}
		break;
		default:
		{

		}
		break;
	}

	switch (skill)
	{
		case F4PACKET::SKILL_INDEX::skill_hookShot:
		case F4PACKET::SKILL_INDEX::skill_hookShotPost:
		case F4PACKET::SKILL_INDEX::skill_hookShotPostSky:
		case F4PACKET::SKILL_INDEX::skill_hookShotSky:
		case F4PACKET::SKILL_INDEX::skill_spotUpShot:
		case F4PACKET::SKILL_INDEX::skill_spotUpShot3:
		case F4PACKET::SKILL_INDEX::skill_pullUpJumper:
		case F4PACKET::SKILL_INDEX::skill_tipIn:
		case F4PACKET::SKILL_INDEX::skill_hopStep:
		case F4PACKET::SKILL_INDEX::skill_euroStepLayUp:
		case F4PACKET::SKILL_INDEX::euroStepDunk:
		case F4PACKET::SKILL_INDEX::skill_tapDunk:
		case F4PACKET::SKILL_INDEX::skill_oneLegedFadeAway:
		case F4PACKET::SKILL_INDEX::skill_turnAroundFadeAway:
		case F4PACKET::SKILL_INDEX::skill_stepBackThreePointShot:
		case F4PACKET::SKILL_INDEX::skill_divingCatch:
		case F4PACKET::SKILL_INDEX::skill_tapOut:
		case F4PACKET::SKILL_INDEX::skill_blockCatch:
		case F4PACKET::SKILL_INDEX::skill_powerBlock:
		case F4PACKET::SKILL_INDEX::skill_chasedownBlock:
		case F4PACKET::SKILL_INDEX::skill_intercept:
		case F4PACKET::SKILL_INDEX::skill_upperCutSteal:
		case F4PACKET::SKILL_INDEX::skill_stretchBlock:
		case F4PACKET::SKILL_INDEX::skill_pokeSteal:
		case F4PACKET::SKILL_INDEX::skill_snatchBlock:
		{
			m_CharacterManager->IncreaseSkillSucCount(characterSN, (DHOST_TYPE_UINT32)skill);
		}
		break;
		case F4PACKET::SKILL_INDEX::catchAndShot:
		{
			if (shotType != SHOT_TYPE::shotType_none)
			{
				//! 캐치앤샷의 경우 패스 리시브받을 때랑 슛할 때 두번 오기때문에 슛에서만 처리
				m_CharacterManager->IncreaseSkillSucCount(characterSN, (DHOST_TYPE_UINT32)skill);
			}
		}
		case F4PACKET::SKILL_INDEX::skill_divingCatchPass:
		case F4PACKET::SKILL_INDEX::skill_selfAlleyOop:
		{
			//! 상해팀 요청에 따라 셀프앨리웁은 시도가 아니라 슛까지 성공해야 된다고해서 simulation 에서 처리하지 않고, 골인 쪽에서 처리함
			//! 다이빙캐치패스의 경우 패스가 이루어지지 않아도 스킬은 다캐패스로 오기 때문에 타겟이 있는지 확인해야 한다. simulation 에서 처리
		}
		break;
		default:
		{

		}
		break;
	}

	//DevBasicTrainingSystemLog(characterSN);
}

DHOST_TYPE_UINT16 CHost::GetCharacterRecordCountWithType(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::RECORD_TYPE Type)
{
	return m_CharacterManager->GetCharacterRecordCountWithType(characterSN, Type);
}

void CHost::CalcMVPScore()
{
	DHOST_TYPE_CHARACTER_SN mvpCharacterSN = kLAST_CHARACTER_SN;
	
	mvpCharacterSN = m_CharacterManager->CalcMVPScore(GetWinnerTeam());

	SetMVP(mvpCharacterSN);
}

void CHost::ChangeState(EHOST_STATE state, void* pData)
{
	// 현재 게임상태가 END 일 때 LOAD 로 돌아가는걸 방지 (연장전이라면 점프볼로 가기때문)
	if (m_CurrentState == EHOST_STATE::END && state == EHOST_STATE::LOAD)
	{
		return;
	}

	/*
	if (m_CurrentState == EHOST_STATE::JUMP_BALL && state == EHOST_STATE::PLAY) // 나머지 모두는 모두 10초 체크 
	{
		m_AlivePingMaxCnt = kMAX_PING_LATENCY_DEQUE; // 플레이부터는 5초

		// 옵저버를 제외한 실제 유저만 해당
		for (auto& it : GetHostUserMap())
		{
			if (it.second != nullptr)
			{
				CHostUserInfo* pUser = it.second;
				pUser->ClearPingData();
			}
		}
	}
	*/

	if (m_CurrentState == EHOST_STATE::READY && state == EHOST_STATE::PLAY) // 
	{
		//m_CharacterManager->ClearCharacterMoveCnt();
	}

	string log_message = "ChangeState RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", CurrentState : " + ConvertHostStateToStr(m_CurrentState) + ", ChangeState : " + ConvertHostStateToStr(state);
	ToLog(log_message.c_str());

	if (state != m_CurrentState)
	{
		auto iter = m_States.find(state);
		if (iter == m_States.end())
		{
			string log_message = "ChangeState m_States is null RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", CurrentState : " + ConvertHostStateToStr(m_CurrentState) + ", ChangeState : " + ConvertHostStateToStr(state);
			ToLog(log_message.c_str());

			if (CheckSkillChallengeMode())
			{
				state = EHOST_STATE::CHALLENGE_END;
			}
			else
			{
				state = EHOST_STATE::END;
			}

			SetAbnormalExpireRoom(true);
		}

		m_States[m_CurrentState]->OnExit();
		m_CurrentState = state;
		m_States[m_CurrentState]->OnEnter(pData);
	}
}

DHOST_TYPE_STR CHost::ConvertHostStateToStr(EHOST_STATE state)
{
	DHOST_TYPE_STR str_state = "NONE";

	switch (state)
	{
		case EHOST_STATE::ARRANGE:
		{
			str_state = "ARRANGE";
		}
		break;
		case EHOST_STATE::BREAK_TIME:
		{
			str_state = "BREAK_TIME";
		}
		break;
		case EHOST_STATE::CHALLENGE_BREAKTIME:
		{
			str_state = "CHALLENGE_BREAKTIME";
		}
		break;
		case EHOST_STATE::CHALLENGE_END:
		{
			str_state = "CHALLENGE_END";
		}
		break;
		case EHOST_STATE::CHALLENGE_INIT:
		{
			str_state = "CHALLENGE_INIT";
		}
		break;
		case EHOST_STATE::CHALLENGE_PLAY:
		{
			str_state = "CHALLENGE_PLAY";
		}
		break;
		case EHOST_STATE::CHALLENGE_READY:
		{
			str_state = "CHALLENGE_READY";
		}
		break;
		case EHOST_STATE::END:
		{
			str_state = "END";
		}
		break;
		case EHOST_STATE::INIT:
		{
			str_state = "INIT";
		}
		break;
		case EHOST_STATE::JUMP_BALL:
		{
			str_state = "JUMP_BALL";
		}
		break;
		case EHOST_STATE::LINE_UP:
		{
			str_state = "LINE_UP";
		}
		break;
		case EHOST_STATE::LOAD:
		{
			str_state = "LOAD";
		}
		break;
		case EHOST_STATE::PLAY:
		{
			str_state = "PLAY";
		}
		break;
		case EHOST_STATE::READY:
		{
			str_state = "READY";
		}
		break;
		case EHOST_STATE::RESULT:
		{
			str_state = "RESULT";
		}
		break;
		case EHOST_STATE::SCENE_START:
		{
			str_state = "SCENE_START";
		}
		break;
		case EHOST_STATE::SCORE:
		{
			str_state = "SCORE";
		}
		break;
		case EHOST_STATE::TRAINING:
		{
			str_state = "TRAINING";
		}
		break;
		case EHOST_STATE::TRAINING_REBOUND:
		{
			str_state = "TRAINING_REBOUND";
		}
		break;
		case EHOST_STATE::TRAINING_JUMPSHOTBLOCK:
		{
			str_state = "TRAINING_JUMPSHOTBLOCK";
		}
		break;
		case EHOST_STATE::TRAINING_RIMATTACKBLOCK:
		{
			str_state = "TRAINING_RIMATTACKBLOCK";
		}
		break;
		case EHOST_STATE::TRAINING_OFFBALLMOVE:
		{
			str_state = "TRAINING_OFFBALLMOVE";
		}
		break;
		case EHOST_STATE::TUTORIAL_BASIC:
		{
			str_state = "TUTORIAL_BASIC";
		}
		break;
		case EHOST_STATE::TUTORIAL_BOXOUT:
		{
			str_state = "TUTORIAL_BOXOUT";
		}
		break;
		case EHOST_STATE::TUTORIAL_DIVINGCATCH:
		{
			str_state = "TUTORIAL_DIVINGCATCH";
		}
		break;
		case EHOST_STATE::TUTORIAL_JUMPSHOT:
		{
			str_state = "TUTORIAL_JUMPSHOT";
		}
		break;
		case EHOST_STATE::TUTORIAL_JUMPSHOTBLOCK:
		{
			str_state = "TUTORIAL_JUMPSHOTBLOCK";
		}
		break;
		case EHOST_STATE::TUTORIAL_PASS:
		{
			str_state = "TUTORIAL_PASS";
		}
		break;
		case EHOST_STATE::TUTORIAL_REBOUND:
		{
			str_state = "TUTORIAL_REBOUND";
		}
		break;
		case EHOST_STATE::TUTORIAL_RIMATTACK:
		{
			str_state = "TUTORIAL_RIMATTACK";
		}
		break;
		case EHOST_STATE::TUTORIAL_RIMATTACKBLOCK:
		{
			str_state = "TUTORIAL_RIMATTACKBLOCK";
		}
		break;
		case EHOST_STATE::TUTORIAL_STEAL:
		{
			str_state = "TUTORIAL_STEAL";
		}
		break;
		case EHOST_STATE::TUTORIAL_PENETRATE:
		{
			str_state = "TUTORIAL_PENETRATE";
		}
		break;
		case EHOST_STATE::TUTORIAL_SHOOTINGDISTURB:
		{
			str_state = "TUTORIAL_SHOOTINGDISTURB";
		}
		break;
		default:
		{
			str_state = "INVALID_STATE";
		}
		break;
	}

	return str_state;
}

void CHost::CheckUserPing()
{
	// 옵저버를 제외한 실제 유저만 해당
	for (auto& it : GetHostUserMap())
	{
		if (it.second != nullptr)
		{
			CHostUserInfo* pUser = it.second;

			if (pUser->GetUserType() == EUSER_TYPE::OBSERVER)
			{
				continue;
			}

			// 끊긴 유저라면 핑 체크를 하지 않는다.
			if (pUser->GetConnectState() == ECONNECT_STATE::DISCONNECT)
			{
				continue;
			}

			// 재접속해서 대기상태(WAIT)인 유저라면 핑 체크를 하지 않는다.
			if (pUser->GetConnectState() == ECONNECT_STATE::WAIT)
			{
				continue;
			}

			DHOST_TYPE_BOOL bCheckPing = CheckUserPingAvg(pUser);

			if (bCheckPing)
			{
				BadConnectProcess(pUser->GetUserID(), false);
			}
		}
	}
}

void CHost::CheckExpiredRoom()
{
	// 경기 결과가 이미 레디스에 전달됐으면 스킵
	if (GetRedisSaveMatchResult())
	{
		return;
	}

	// 강제로 방을 폭파시켜야 하는지 확인
	if (CheckForciblyExpireRoom())
	{
		ForciblyExpireRoom();
	}

	// 접속 불량인 유저 확인
	CheckUserPing();

	//! 트리오 모드는 AI 전환이 거지같아서 혼자하는 경우는 빠르게 방 폭파 시켜준다.
	if (GetModeType() == EMODE_TYPE::TRIO || GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED)
	{
		if (GetBadConnectUserSize() > kUINT64_INIT)
		{
			// 접속 불량인 유저가 현재 플레이하고 있는 유저수와 같다면 방폭
			if (CheckBadConnectUserIsEqualUserCount())
			{
				ForciblyExpireRoom();
			}
		}
	}

	// 접속 불량인 유저 다른 유저에게 권한 위임
	if (m_CurrentState == EHOST_STATE::PLAY)
	{
		SendDelegateControlToUser();
	}

	if (GetBadConnectUserSize() > kUINT64_INIT)
	{
		// 접속 불량인 유저가 현재 플레이하고 있는 유저수와 같다면 방폭
		if (CheckBadConnectUserIsEqualUserCount())
		{
			ForciblyExpireRoom();
		}
	}
}

DHOST_TYPE_BOOL CHost::CheckForciblyExpireRoom()
{
	//// 호스트 생성 후 유저가 접속도 못하는 경우에 일정시간이 지나면 방이 파괴되어야 한다.
	//if (GetRoomElapsedTime() > kTIME_AFTER_HOST_CREATION && UserCount() == kINT32_INIT)
	//{
	//	string log_message = "CheckExpiredRoom because the user cannot connect";
	//	ToLog(log_message.c_str());

	//	return true;
	//}

	//// 모든 유저가 Enet이 끊긴경우 방이 파괴되어야 한다.
	//if (GetGameCap() > kINT32_INIT && GetGameCap() == m_VecCallbackEnetDisConnectUser.size())
	//{
	//	string log_message = "CheckExpiredRoom because the all user enet disconnect GameCap : " + std::to_string(GetGameCap()) + ", m_VecCallbackEnetDisConnectUser.size : " + std::to_string(m_VecCallbackEnetDisConnectUser.size());
	//	ToLog(log_message.c_str());

	//	return true;
	//}

	//! 클라가 정상적인 경우라면 어떤식으로든 패킷을 보내게되는데 (핑 패킷이든, 플레이 패킷이든) 3분동안 아무 패킷도 안보낸다면 유저가 없는것으로 판단한다
	if (GetRoomElapsedTime() - m_TimeManager->GetLastPacketReceiveTime() > kLAST_PACKET_RECEIVE_TIME)
	{
		string log_message = "CheckExpiredRoom because user's can't send packet GetRoomElapsedTime : " + std::to_string(GetRoomElapsedTime());
		ToLog(log_message.c_str());

		return true;
	}

	return false;
}

DHOST_TYPE_BOOL CHost::CheckBadConnectUserIsEqualUserCount()
{
	if (GetBadConnectUserSize() == static_cast<DHOST_TYPE_UINT64>(UserCount()))
	{
		string log_message = "CheckBadConnectUserIsEqualUserCount Mode : " + to_string((int)GetModeType()) + ", BadConnectUser : " + std::to_string(GetBadConnectUserSize()) + ", UserCount : " + std::to_string(UserCount());
		ToLog(log_message.c_str());

		SetAbnormalExpireRoom(true);

		// 혼자 플레이 하는 경우 경기결과에 패배 셋팅을 해줘야한다.
		if (UserCount() == 1)
		{
			for (auto& it : GetHostUserMap())
			{
				if (it.second != nullptr)
				{
					auto pUser = it.second;

					if (pUser->GetUserType() == EUSER_TYPE::OBSERVER)
					{
						continue;
					}

					SetAbNormalEndUserTeam(FindUserTeam(pUser->GetUserID()));
				}
			}
		}
		return true;
	}

	return false;
}

void CHost::ForciblyExpireRoom()
{
	SetAbnormalExpireRoom(true);

	EMODE_TYPE game_mode = EMODE_TYPE::NONE;

	game_mode = GetModeType();

	switch (game_mode)
	{
		case EMODE_TYPE::SKILL_CHALLENGE_JUMP_SHOT_BLOCK:
		case EMODE_TYPE::SKILL_CHALLENGE_OFF_BALL_MOVE:
		case EMODE_TYPE::SKILL_CHALLENGE_PASS:
		case EMODE_TYPE::SKILL_CHALLENGE_REBOUND:
		{
			ChangeState(EHOST_STATE::CHALLENGE_END);
		}
		break;
		case EMODE_TYPE::AI:
		case EMODE_TYPE::ONE_ON_ONE:
		case EMODE_TYPE::THREE_ON_THREE:
		case EMODE_TYPE::TWO_ON_TWO:
		case EMODE_TYPE::TRAINING:
		case EMODE_TYPE::TRAINING_REBOUND:
		case EMODE_TYPE::TRAINING_JUMPSHOTBLOCK:
		case EMODE_TYPE::TRAINING_RIMATTACKBLOCK:
		case EMODE_TYPE::TRAINING_OFFBALLMOVE:
		case EMODE_TYPE::CUSTOM:
		case EMODE_TYPE::TRIO:
		case EMODE_TYPE::TRIO_PVP_MIXED:
		case EMODE_TYPE::CONTINUOUS:

		case EMODE_TYPE::TUTORIAL_BASIC:
		case EMODE_TYPE::TUTORIAL_BOXOUT:
		case EMODE_TYPE::TUTORIAL_DIVINGCATCH:
		case EMODE_TYPE::TUTORIAL_JUMPSHOT:
		case EMODE_TYPE::TUTORIAL_JUMPSHOTBLOCK:
		case EMODE_TYPE::TUTORIAL_PASS:
		case EMODE_TYPE::TUTORIAL_REBOUND:
		case EMODE_TYPE::TUTORIAL_RIMATTACK:
		case EMODE_TYPE::TUTORIAL_RIMATTACKBLOCK:
		case EMODE_TYPE::TUTORIAL_STEAL:
		case EMODE_TYPE::TUTORIAL_PENETRATE:
		case EMODE_TYPE::TUTORIAL_SHOOTINGDISTURB:
		case EMODE_TYPE::TUTORIAL_BLOCK_BEGINNER:
		case EMODE_TYPE::TUTORIAL_REBOUND_BEGINNER:
		{
			ChangeState(EHOST_STATE::END);
		}
		break;
		default:
		{
			string log_message = "Invalid GameModeType";
			ToLog(log_message.c_str());
		}
		break;
	}
}

CHostUserInfo* CHost::UserFind(DHOST_TYPE_INT32 UserID)
{
	auto iter = m_HostUserMap.find(UserID);

	if (iter == m_HostUserMap.end())
		return nullptr;

	return iter->second;
}

// 실제 유저만 카운트
int CHost::UserCount()
{
	int userCount = kINT32_INIT;

	for (auto& it : m_HostUserMap)
	{
		auto pUser = it.second;
		if (nullptr == pUser)
		{
			continue;
		}

		if (pUser->GetUserType() == EUSER_TYPE::OBSERVER)
		{
			continue;
		}

		++userCount;
	}

	return userCount;
}

void CHost::UserAdd(DHOST_TYPE_USER_ID userid, void* peer)
{
	CHostUserInfo* pUser = this->FindUser(userid);

	if (pUser == nullptr)
	{
		pUser = new CHostUserInfo(userid, peer, this, GetRoomElapsedTime());

		m_HostUserMap.insert(HostUserMapType::value_type(userid, pUser));

		if (CheckOption(HOST_OPTION_NO_REDIS))
		{
			++m_GameCap;
		}

		string log_message = "UserAdd m_HostUserMap insert UserID : " + to_string(userid) + ", UserType : " + ConvertToStringUserType(pUser->GetUserType()) + ", MapSize : " + to_string(m_HostUserMap.size());
		ToLog(log_message.c_str());

		// HostAddUser is used by local/offline mode where ENet connect callbacks are absent.
		// Mark user as CONNECT immediately so BroadcastPacket can deliver login responses.
		pUser->SetConnectState(ECONNECT_STATE::CONNECT);

		if (pUser != nullptr)
		{
#ifdef _DEBUG
			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, local_system_s2c_roomInfo, message, send_data);
					send_data.add_roomid(ParseHostIdToInt(m_HostID));
			STORE_FBPACKET(builder, message, send_data)
			SendPacket(message, userid);
#endif
		}
	}
	else
	{
		string log_message = "UserAdd pUser is not null UserID : " + to_string(userid) + ", UserType : " + ConvertToStringUserType(pUser->GetUserType());
		ToLog(log_message.c_str());

		if (pUser->GetConnectState() != ECONNECT_STATE::CONNECT)
		{
			pUser->SetConnectState(ECONNECT_STATE::CONNECT);
		}

		return;
	}
}

DHOST_TYPE_USER_ID CHost::UserGetID(void* peer)
{
	for (std::map<DHOST_TYPE_USER_ID, CHostUserInfo*>::iterator it = m_HostUserMap.begin(); it != m_HostUserMap.end(); ++it)
	{
		if (peer == it->second->GetPeer())
		{
			return it->first;
		}
	}

	return 0;
}

HostUserMapType& CHost::GetHostUserMap()
{
	return m_HostUserMap;
}

TB::SVector3 CHost::GetBallPosition()
{
	TB::SVector3 svector;// = new SVector3();
	auto pos = m_pBallController->GetBallPosition();
	svector.mutate_x(pos.fX);
	svector.mutate_y(pos.fY);
	svector.mutate_z(pos.fZ);
	return svector;
}

void CHost::BallOnEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ)
{
	if (m_pEnvironmentObject != nullptr)
	{
		switch (shotType)
		{
		case F4PACKET::SHOT_TYPE::shotType_dunkDriveIn:
		case F4PACKET::SHOT_TYPE::shotType_dunkFingerRoll:
		case F4PACKET::SHOT_TYPE::shotType_dunkPost:
			{
				if (m_pEnvironmentObject->PigeonSetAppear(false) == true && GetGameTime() < m_pEnvironmentObject->TimePigeonAppear - 3.0f)
				{
					m_pEnvironmentObject->PigeonReserveAppear();
				}
			}
			break;
		}
	}

	m_States[m_CurrentState]->OnBallEvent(ballNumber, eventName, ownerID, ballState, shotType, team, point, ballPositionX, ballPositionZ);
}


void CHost::ClearBallActionInfoDeque()
{
	if (!m_BallMessageRecord.empty())
	{
		m_BallMessageRecord.clear();
	}
}

void CHost::PushBallActionInfoDeque(SBallActionInfo ballInfo)
{
	if (kMAX_BALL_DEQUE <= m_BallMessageRecord.size())
	{
		m_BallMessageRecord.pop_front();
	}

	m_BallMessageRecord.push_back(ballInfo);
}

void CHost::DevBallActionInfoLog()
{
	for (auto ball : m_BallMessageRecord)
	{
		string log_message = "ballNumber : " + to_string(ball.BallNumber) + ", ballOwner : " + to_string(ball.BallOwner) + ", ballState : " + F4PACKET::EnumNameBALL_STATE(ConvertToProtobufBallState(ball.Ballstate));
		ToLog(log_message.c_str());
	}
}

DHOST_TYPE_BOOL CHost::FindTargetBallActionInfo(DHOST_TYPE_BALL_NUMBER ballNumber, SBallActionInfo& sInfo)
{
	if (m_BallMessageRecord.empty())
	{
		return false;
	}

	DHOST_TYPE_BOOL result = false;

	for (auto ball : m_BallMessageRecord)
	{
		if (ball.BallNumber == ballNumber)
		{
			sInfo = ball;

			result = true;

			break;
		}
	}

	if (result == false)
	{
		sInfo = m_BallMessageRecord.back();
	}
	
	return true;
}

SBallActionInfo CHost::FindTargetBallActionInfoWithIndex(int idx)
{
	return m_BallMessageRecord[idx];
}

F4PACKET::BALL_STATE CHost::ConvertToProtobufBallState(EBALL_STATE state)
{
	F4PACKET::BALL_STATE ball_state = F4PACKET::BALL_STATE::ballState_none;

	switch (state)
	{
		case EBALL_STATE::ALLEYOOP:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_alleyOop;
		}
		break;
		case EBALL_STATE::JUMP:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_jump;
		}
		break;
		case EBALL_STATE::LOOSE:
		{

			ball_state = F4PACKET::BALL_STATE::ballState_loose;
		}
		break;
		case EBALL_STATE::LOOSE_BLOCK:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_looseBlock;
		}
		break;
		case EBALL_STATE::LOOSE_COLLISION:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_looseCollision;
		}
		break;
		case EBALL_STATE::LOOSE_LAY:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_looseLay;
		}
		break;
		case EBALL_STATE::LOOSE_PASS:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_loosePass;
		}
		break;
		case EBALL_STATE::LOOSE_REBOUND:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_looseRebound;
		}
		break;
		case EBALL_STATE::LOOSE_STEAL:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_looseSteal;
		}
		break;
		case EBALL_STATE::PASS:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_pass;
		}
		break;
		case EBALL_STATE::PASS_NORMAL:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_passNormal;
		}
		break;
		case EBALL_STATE::PASS_BLOCK:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_passBlock;
		}
		break;
		case EBALL_STATE::PASS_REBOUND:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_passRebound;
		}
		break;
		case EBALL_STATE::PASS_LOOSE:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_passLoose;
		}
		break;
		case EBALL_STATE::PICK:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_pick;
		}
		break;
		case EBALL_STATE::PICK_BLOCK:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_pickBlock;
		}
		break;
		case EBALL_STATE::PICK_LOOSE:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_pickLoose;
		}
		break;
		case EBALL_STATE::PICK_PASS:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_pickPass;
		}
		break;
		case EBALL_STATE::PICK_REBOUND:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_pickRebound;
		}
		break;
		case EBALL_STATE::PICK_STEAL:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_pickSteal;
		}
		break;
		case EBALL_STATE::READY:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_ready;
		}
		break;
		case EBALL_STATE::SHOT:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_shot;
		}
		break;
		case EBALL_STATE::SHOT_DUNK:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_shotDunk;
		}
		break;
		case EBALL_STATE::SHOT_JUMP:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_shotJump;
		}
		break;
		case EBALL_STATE::SHOT_LAYUP:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_shotLayUp;
		}
		break;
		case EBALL_STATE::SHOT_POST:
		{
			ball_state = F4PACKET::BALL_STATE::ballState_shotPost;
		}
		break;
		default:
		{
			string log_message = "ConvertToEnumBallState type is invalid : " + to_string((int)state);
			ToLog(log_message.c_str());
		}
		break;
	}

	return ball_state;
}

EBALL_STATE CHost::ConvertToEnumBallState(F4PACKET::BALL_STATE state)
{
	EBALL_STATE ball_state = EBALL_STATE::NONE;

	switch ((F4PACKET::BALL_STATE)state)
	{
		case F4PACKET::BALL_STATE::ballState_alleyOop:
		{
			ball_state = EBALL_STATE::ALLEYOOP;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_jump:
		{
			ball_state = EBALL_STATE::JUMP;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_loose:
		{
			ball_state = EBALL_STATE::LOOSE;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_looseBlock:
		{
			ball_state = EBALL_STATE::LOOSE_BLOCK;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_looseCollision:
		{
			ball_state = EBALL_STATE::LOOSE_COLLISION;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_looseLay:
		{
			ball_state = EBALL_STATE::LOOSE_LAY;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_loosePass:
		{
			ball_state = EBALL_STATE::LOOSE_PASS;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_looseRebound:
		{
			ball_state = EBALL_STATE::LOOSE_REBOUND;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_looseSteal:
		{
			ball_state = EBALL_STATE::LOOSE_STEAL;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_pass:
		{
			ball_state = EBALL_STATE::PASS;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_passNormal:
		{
			ball_state = EBALL_STATE::PASS_NORMAL;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_passBlock:
		{
			ball_state = EBALL_STATE::PASS_BLOCK;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_passRebound:
		{
			ball_state = EBALL_STATE::PASS_REBOUND;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_passLoose:
		{
			ball_state = EBALL_STATE::PASS_LOOSE;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_pick:
		{
			ball_state = EBALL_STATE::PICK;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_pickBlock:
		{
			ball_state = EBALL_STATE::PICK_BLOCK;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_pickLoose:
		{
			ball_state = EBALL_STATE::PICK_LOOSE;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_pickPass:
		{
			ball_state = EBALL_STATE::PICK_PASS;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_pickRebound:
		{
			ball_state = EBALL_STATE::PICK_REBOUND;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_pickSteal:
		{
			ball_state = EBALL_STATE::PICK_STEAL;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_ready:
		{
			ball_state = EBALL_STATE::READY;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_shot:
		{
			ball_state = EBALL_STATE::SHOT;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_shotDunk:
		{
			ball_state = EBALL_STATE::SHOT_DUNK;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_shotJump:
		{
			ball_state = EBALL_STATE::SHOT_JUMP;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_shotLayUp:
		{
			ball_state = EBALL_STATE::SHOT_LAYUP;
		}
		break;
		case F4PACKET::BALL_STATE::ballState_shotPost:
		{
			ball_state = EBALL_STATE::SHOT_POST;
		}
		break;
		default:
		{
			string nameBallState = F4PACKET::EnumNameBALL_STATE((F4PACKET::BALL_STATE)state);
			string log_message = "ConvertToEnumBallState type is invalid : " + nameBallState;
			ToLog(log_message.c_str());
		}
		break;
	}

	return ball_state;
}

DHOST_TYPE_CHARACTER_SN CHost::CheckTurnOver(DHOST_TYPE_BALL_NUMBER ballNumber, F4PACKET::BALL_STATE ballState, DHOST_TYPE_INT32 team)
{
	DHOST_TYPE_CHARACTER_SN TurnOverCharacterSN = kCHARACTER_SN_INIT;

	EBALL_STATE current_ball_state = ConvertToEnumBallState(ballState);

	auto deque_size = m_BallMessageRecord.size();

	int deque_idx = static_cast<int>(deque_size) - 2;

	if (deque_idx < 0)
	{
		return TurnOverCharacterSN;
	}

	SBallActionInfo sInfo = FindTargetBallActionInfoWithIndex(deque_idx);

	if (EBALL_STATE::PICK_STEAL == current_ball_state)
	{
		TurnOverCharacterSN = sInfo.BallOwner;
	}
	else if (EBALL_STATE::PICK_LOOSE == current_ball_state)
	{
		switch (sInfo.Ballstate)
		{
			case EBALL_STATE::LOOSE_PASS:
			{
				if (sInfo.BallOwnerTeam != ConvertToStringTeam((DHOST_TYPE_INT32)team))
				{
					auto pre_idx = deque_idx - 1;

					sInfo = FindTargetBallActionInfoWithIndex(pre_idx);
					TurnOverCharacterSN = sInfo.BallOwner;
				}
			}
			break;
			case EBALL_STATE::LOOSE_STEAL:
			{
				auto pre_idx = deque_idx - 1;

				sInfo = FindTargetBallActionInfoWithIndex(pre_idx);
				if (sInfo.BallOwnerTeam != ConvertToStringTeam((DHOST_TYPE_INT32)team))
				{
					TurnOverCharacterSN = sInfo.BallOwner;
				}
			}
			break;
			case EBALL_STATE::LOOSE_COLLISION:
			{
				if (sInfo.BallOwnerTeam != ConvertToStringTeam((DHOST_TYPE_INT32)team))
				{
					sInfo = FindTargetBallActionInfoWithIndex(deque_idx);
					TurnOverCharacterSN = sInfo.BallOwner;
				}
			}
			break;
			default:
				break;
		}
	}

	return TurnOverCharacterSN;
}

DHOST_TYPE_CHARACTER_SN CHost::GetNeariestTeamCharacterFromRim(DHOST_TYPE_INT32 indexTeam)
{
	auto character_count = m_CharacterManager->GetTeamCharacterSize(indexTeam);
	float min_value = 10000;
	DHOST_TYPE_CHARACTER_SN characterID = kCHARACTER_SN_INIT;
	for (size_t i = 0; i < character_count; i++)
	{
		
		int sn = m_CharacterManager->GetCharacterSN(indexTeam, i);
		
		F4PACKET::SPlayerAction* pInfo = GetCharacterStateAction(sn);

		float x = pInfo->positionlogic().x();
		float z = pInfo->positionlogic().z();
		float distance = GetShotPositionDistanceFromRim(x, z);
		if (distance < min_value)
		{
			min_value = distance;
			characterID = m_CharacterManager->GetCharacter(sn)->GetCharacterInformation()->id;
		}
		
		//yaw = pInfo->yawlogic();
		
	}
	return characterID;
}

DHOST_TYPE_BOOL CHost::CheckCurrentBallStateShot()
{
	DHOST_TYPE_BOOL result = false;

	int ball_number = BallNumberGet();

	SBallActionInfo last_ball_data;

	if (FindTargetBallActionInfo(ball_number, last_ball_data))
	{
		if (last_ball_data.BallNumber == ball_number)
		{
			switch (last_ball_data.Ballstate)
			{
				case EBALL_STATE::SHOT:
				case EBALL_STATE::SHOT_DUNK:
				case EBALL_STATE::SHOT_JUMP:
				case EBALL_STATE::SHOT_LAYUP:
				case EBALL_STATE::SHOT_POST:
					result = true;
					break;
				default:
					break;
			}
		}
	}

	return result;
}

void CHost::PushBallShotHindrance(std::vector<DHOST_TYPE_CHARACTER_SN> vCharacterSN)
{
	if (m_BallShotHindrance.empty() == false)
	{
		m_BallShotHindrance.clear();
	}

	for (int i = 0; i < vCharacterSN.size(); ++i)
	{
		m_BallShotHindrance.push_back(vCharacterSN[i]);
	}
}

std::vector<DHOST_TYPE_CHARACTER_SN>& CHost::GetBallShotHindrance()
{
	return m_BallShotHindrance;
}

void CHost::PushMatchInfo(int id, int target)
{
	m_CharacterManager->PushMatchInfo(id, target);
}

void CHost::ModifyMatchInfo(int id, int target)
{
	m_CharacterManager->ModifyMatchInfo(id, target);
}

int CHost::GetMatchInfoID(int target)
{
	return m_CharacterManager->GetMatchInfoID(target);
}

int CHost::GetMatchInfoTarget(int id)
{
	return m_CharacterManager->GetMatchInfoTarget(id);
}

int CHost::GetSwitchID(DHOST_TYPE_INT32 team)
{
	return m_CharacterManager->GetSwitchID(team);
}

void CHost::SetSwitchID(DHOST_TYPE_INT32 team, int id)
{
	m_CharacterManager->SetSwitchID(team, id);
}

void CHost::OnCharacterMessage(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time, DHOST_TYPE_GAME_TIME_F gameTime, DHOST_TYPE_UINT32 keys, F4PACKET::MOVE_MODE moveMode)
{
	m_CharacterManager->OnCharacterMessage(characterSN, packetID, pData, time, gameTime, keys, moveMode);
}

DHOST_TYPE_BOOL CHost::GetSecondAnimationReceivePacket(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetSecondAnimationReceivePacket(characterSN);
}

DHOST_TYPE_BOOL CHost::GetShotIsLeftHanded(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetShotIsLeftHanded(characterSN);
}

void CHost::SetShotIsLeftHanded(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value)
{
	m_CharacterManager->SetShotIsLeftHanded(characterSN, value);
}

void CHost::CheckCharacterBurstGauge(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_GAME_TIME_F time, uint64_t actionType, std::vector<DHOST_TYPE_INT32>& teamScores)
{
	m_CharacterManager->CheckCharacterBurstGauge(characterSN, time, actionType, teamScores, CheckGameElapsedTimeOneMinute());

	/*std::string dev_log = "[BURST] GameTime : " + std::to_string(time) + ", CharacterName : " + m_CharacterManager->GetCharacterInformation(characterSN)->name + ", BurstGauge : " + std::to_string(GetCharacterBurstGauge(characterSN));
	SendDebugMessage(dev_log);*/


#ifdef _DEBUG
	SendCharacterBurstGauge();
#endif // DEBUG

}

DHOST_TYPE_UINT64 CHost::GetBurstActionTypeWithRecordType(ECHARACTER_RECORD_TYPE recordType)
{
	return m_CharacterManager->GetBurstActionTypeWithRecordType(recordType);
}

DHOST_TYPE_BOOL CHost::CheckCharacterActionPenetrateType(F4PACKET::ACTION_TYPE type)
{
	return m_CharacterManager->CheckCharacterActionPenetrateType(type);
}

DHOST_TYPE_FLOAT CHost::GetCharacterBurstGauge(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetCharacterBurstGauge(characterSN);
}

EBURST_ON_FIRE_MODE_STEP CHost::GetCharacterBurstOnFireModeStep(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetCharacterBurstOnFireModeStep(characterSN);
}

void CHost::UpdateCharacterBurstGaugeWithPassiveActivation(DHOST_TYPE_CHARACTER_SN characterSN, BURST_REQUEST burstRequest)
{
	m_CharacterManager->UpdateCharacterBurstGaugeWithPassiveActivation(characterSN, burstRequest);
}

void CHost::UpdateCharacterBurstGaugeWithMedalActivation(DHOST_TYPE_CHARACTER_SN characterSN, BURST_REQUEST burstRequest)
{
	m_CharacterManager->UpdateCharacterBurstGaugeWithMedalActivation(characterSN, burstRequest);
}

void CHost::UpdateCharacterOnFireModeStatBuff(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacterManager()->GetCharacter(characterSN);
	if (pCharacter != nullptr)
	{
		std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT> mapData = GetCharacterManager()->GetSendOnFireModeCharacterReservation();

		std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT>::iterator iter = mapData.find(characterSN);
		if (iter != mapData.end())
		{
			GetCharacterManager()->UpdateCharacterOnFireModeStatBuff(pCharacter, iter->second, true);
		}
	}
}

DHOST_TYPE_BOOL CHost::GetTeamCharacterMap(DHOST_TYPE_INT32 teamIndex)
{
	return m_CharacterManager->GetTeamCharacterMap(teamIndex);
}

std::vector<MatchInfo>& CHost::GetMatchInfo()
{
	return m_CharacterManager->GetMatchInfo();
}

DHOST_TYPE_BOOL CHost::GetTeamIllegalScreen(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetTeamIllegalScreen(characterSN);
}

DHOST_TYPE_BOOL CHost::SetTeamIllegalScreen(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value)
{
	return m_CharacterManager->SetTeamIllegalScreen(characterSN, value, GetOffenseTeam());
}

void CHost::InitMannerManagerData(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_BALL_NUMBER ballNo, DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->InitMannerManagerData(gameTime, ballNo, characterSN);
}

void CHost::ComebackAwayFromKeyboard(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_USER_ID UserID, DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->ComebackAwayFromKeyboard(gameTime, UserID, characterSN);
}

void CHost::UpdateIntentionalBallHogPlay(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_BALL_NUMBER ballNo)
{
	m_CharacterManager->UpdateIntentionalBallHogPlay(characterSN, gameTime, ballNo);
}

void CHost::BadMannerDetectImpossibleShooting(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT shotClock, DHOST_TYPE_FLOAT value)
{
	m_CharacterManager->BadMannerDetectImpossibleShooting(characterSN, shotClock, value);
}

void CHost::UpdateAwayFromKeyboard(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT gameTime)
{
	m_CharacterManager->UpdateAwayFromKeyboard(characterSN, gameTime);
}

void CHost::IncreaseEmojiCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value)
{
	m_CharacterManager->IncreaseEmojiCount(characterSN, value);
}

void CHost::IncreaseCeremonyCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value)
{
	m_CharacterManager->IncreaseCeremonyCount(characterSN, value);
}

void CHost::IncreaseSkillSucCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_UINT32 value)
{
	m_CharacterManager->IncreaseSkillSucCount(characterSN, value);

	//DevBasicTrainingSystemLog(characterSN);
}

void CHost::IncreaseUseSkillCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->IncreaseUseSkillCount(characterSN);
}

void CHost::IncreaseUseSignatureCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->IncreaseUseSignatureCount(characterSN);
}

void CHost::IncreaseUseQuickChatCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->IncreaseUseQuickChatCount(characterSN);
}

void CHost::IncreaseUseSwitchCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->IncreaseUseSwitchCount(characterSN);
}

void CHost::IncreaseActionGreatDefenseCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->IncreaseActionGreatDefenseCount(characterSN);
}

void CHost::IncreaseActionLayUpGoalCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->IncreaseActionLayUpGoalCount(characterSN);
}

void CHost::IncreaseActionDunkGoalCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->IncreaseActionDunkGoalCount(characterSN);
}

void CHost::IncreaseActionAnkleBreakeCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->IncreaseActionAnkleBreakeCount(characterSN);
}

void CHost::IncreaseIllegalScreenCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->IncreaseIllegalScreenCount(characterSN);
}

void CHost::IncreaseNotSamePositionCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->IncreaseNotSamePositionCount(characterSN);
}

void CHost::SetSpeedHackClientjudgmentCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value)
{
	m_CharacterManager->SetSpeedHackClientjudgmentCount(characterSN, value);
}

void CHost::IncreaseHackImpossibleAction(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->IncreaseHackImpossibleAction(characterSN);
}

void CHost::SetMemoryTamperClientjudgmentCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value)
{
	m_CharacterManager->SetMemoryTamperClientjudgmentCount(characterSN, value);
}

void CHost::IncreasePacketTamper(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->IncreasePacketTamper(characterSN);
}

void CHost::CheckQuestAction(const SPlayerAction* pInfo)
{
	if (pInfo != nullptr)
	{
		if (pInfo->skillindex() > F4PACKET::SKILL_INDEX::MIN && pInfo->skillindex() <= F4PACKET::SKILL_INDEX::MAX)
		{
			IncreaseUseSkillCount(pInfo->id());
		}
		
		if (pInfo->signature())
		{
			IncreaseUseSignatureCount(pInfo->id());
		}
	}
}

void CHost::UseJoystick(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_CharacterManager->UseJoystick(characterSN);
}

void CHost::CheckTeamSurrenderActivation(DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT gameTime)
{
	if(m_CurrentState == EHOST_STATE::END || m_CurrentState == EHOST_STATE::RESULT)
	{
		return;
	}

	// 기권하기 활성화 또는 비활성화 시키기
	if (m_CharacterManager != nullptr)
	{
		//! 조건1. 일반전, 랭킹전, 트리오모드 에서만 발동되게
		if (GetModeType() == EMODE_TYPE::THREE_ON_THREE || GetModeType() == EMODE_TYPE::TRIO || GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED)
		{
			//! 조건2. 경기 시간이 1분 30초 이상 지났는지
			if (GetGameElapsedTimeSinceGameStart() > kTEAM_SURRENDER_CHECK_ACTIVATION_GAME_TIME)
			{
				std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT> TeamSurrenderCheck = m_CharacterManager->GetTeamSurrenderCheck();

				for (std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT>::iterator iter = TeamSurrenderCheck.begin(); iter != TeamSurrenderCheck.end(); ++iter)
				{
					//! 활성화 -> 비활성화 (활성화 된적이 있었다면 비활성화 조건 체크)

					//! 점수차이
					DHOST_TYPE_UINT32 gap = std::labs(GetScore(iter->first) - GetScore(GetIndexOpponentTeam(iter->first)));

					//! 투표 실패처리가 안되었다면
					if (m_CharacterManager->GetTeamSurrenderCheckVoteFail(iter->first) == false)
					{
						// 기권이 활성화중인지 확인
						if (iter->second > kFLOAT_INIT)
						{
							DHOST_TYPE_INT32 vote_yes = m_CharacterManager->GetTeamSurrenderVoteCount(iter->first, true);
							DHOST_TYPE_INT32 vote_no = m_CharacterManager->GetTeamSurrenderVoteCount(iter->first, false);

							// 두명 이상이 찬성했는지 확인해서 바로 종료시키기
							if (vote_yes >= 2)
							{
								DHOST_TYPE_FLOAT endTime = m_CharacterManager->GetTeamSurrenderGameEndTime(iter->first);

								if (endTime > kFLOAT_INIT && (endTime - GetGameTime() >= kTEAM_SURRENDER_CHECK_GAME_END_DELAY_TIME))
								{
									std::string str_log = "SurrenderVote GameTime : " + std::to_string(GetGameTime()) + ", SurrenderGameEndTime : " + std::to_string(m_CharacterManager->GetTeamSurrenderGameEndTime(iter->first)) + ", TeamIndex : " + std::to_string(iter->first) + ", YES : " + std::to_string(vote_yes) + ", NO : " + std::to_string(vote_no);
									ToLog(str_log.c_str());

									m_CharacterManager->SaveSurrenderVoteInfo(iter->first);

									ChangeState(EHOST_STATE::END);
								}

								break;
							}

							//! 경기 시간이 30초 이하이거나, 점수차가 3점이하, 기권하기 투표가 2분이 지났으면, 기권하기에서 2명이상이 반대표를 던졌을 때
							if (GetGameTime() <= kTEAM_SURRENDER_CHECK_INACTIVATION_GAME_TIME || gap <= kTEAM_SURRENDER_CHECK_INACTIVATION_SCORE_GAP || (GetRoomElapsedTime() - iter->second >= kTEAM_SURRENDER_CHECK_ACTIVATION_ELAPSED_TIME) || vote_no >= 2)
							{
								SendSurrenderActivationInfo(iter->first, false);
								m_CharacterManager->SetTeamSurrenderCheckVoteFail(iter->first, true);
								m_CharacterManager->SetTeamSurrenderCheckTime(iter->first, kTEAM_SURRENDER_CHECK_EXPIRE);
								break;
							}
						}
					}
					
					//! 비활성화 -> 활성화 (활성화 된적이 없었다면)
					if (iter->second < kFLOAT_INIT && gap >= kTEAM_SURRENDER_CHECK_ACTIVATION_SCORE_GAP && GetGameTime() > kTEAM_SURRENDER_CHECK_INACTIVATION_GAME_TIME)
					{
						if (GetScore(iter->first) < GetScore(GetIndexOpponentTeam(iter->first)))
						{
							SendSurrenderActivationInfo(iter->first, true);
							m_CharacterManager->SetTeamSurrenderCheckTime(iter->first, GetRoomElapsedTime());
							break;
						}
					}
				}
			}
		}
	}
	

}

void CHost::SendSurrenderActivationInfo(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_BOOL activation)
{
	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_s2c_surrenderActivation, message, send_data);
	send_data.add_team(teamIndex);
	send_data.add_activation(activation);
	STORE_FBPACKET(builder, message, send_data)

	BroadcastPacket(message, kUSER_ID_INIT);

	std::string str_log = "RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", TeamIndex : " + std::to_string(teamIndex) + ", Activation : " + std::to_string(activation);
	ToLog(str_log.c_str());
}

void CHost::AddTeamSurrenderVote(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value)
{
	if (m_CharacterManager != nullptr)
	{
		m_CharacterManager->AddTeamSurrenderVote(characterSN, value);
	}
}

DHOST_TYPE_BOOL CHost::GetTeamSurrenderActivation(DHOST_TYPE_CHARACTER_SN characterSN)
{
	if (m_CharacterManager != nullptr)
	{
		return m_CharacterManager->GetTeamSurrenderActivation(characterSN);
	}

	return false;
}

DHOST_TYPE_INT32 CHost::GetTeamSurrenderVoteCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value)
{
	DHOST_TYPE_INT32 result = kINT32_INIT;

	if (m_CharacterManager != nullptr)
	{
		F4PACKET::SPlayerInformationT* pInfo = m_CharacterManager->GetCharacterInformation(characterSN);

		if (pInfo != nullptr)
		{
			result = m_CharacterManager->GetTeamSurrenderVoteCount(pInfo->team, value);
		}
	}

	return result;
}

void CHost::AddTeamSurrenderGameEndTime(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value)
{
	if (m_CharacterManager != nullptr)
	{
		F4PACKET::SPlayerInformationT* pInfo = m_CharacterManager->GetCharacterInformation(characterSN);

		if (pInfo != nullptr)
		{
			m_CharacterManager->AddTeamSurrenderGameEndTime(pInfo->team, value);
		}
	}
}

DHOST_TYPE_UINT16 CHost::GetObserverUserCount()
{
	DHOST_TYPE_UINT16 result = kUINT16_INIT;

	for (auto& it : m_HostUserMap)
	{
		auto pUser = it.second;

		if (nullptr == pUser)
		{
			continue;
		}

		if(pUser->GetUserType() == EUSER_TYPE::OBSERVER && pUser->GetConnectState() == ECONNECT_STATE::CONNECT)
		{
			++result;
		}
	}

	return result;
}

void CHost::SendObserverUserCountInfo()
{
	DHOST_TYPE_UINT16 observer_user_count = kUINT16_INIT;

	observer_user_count = GetObserverUserCount();

	if (observer_user_count >= kUINT16_INIT)
	{
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, system_s2c_observerUserCountInfo, message, send_data);
		send_data.add_count(observer_user_count);
		STORE_FBPACKET(builder, message, send_data)

		BroadcastPacket(message, kUSER_ID_INIT);
	}
}

void CHost::CheckCharacterPositionSync(const F4PACKET::SPlayerAction* pInfo, DHOST_TYPE_INT32 packetID)
{
	if (GetCharacterManager() != nullptr && pInfo != nullptr)
	{
		//! 서버에서 알고있는 캐릭터의 위치와 클라에서 알고있는 위치
		CCharacter* pCharacter = GetCharacterManager()->GetCharacter(pInfo->id());
		if (pCharacter != nullptr)
		{
			if (GetCurrentState() == EHOST_STATE::PLAY && pCharacter->GetCharacterStateActionType() != F4PACKET::ACTION_TYPE::action_none)
			{
				auto server_pos = pCharacter->GetCharacterStateAction()->positionlogic();
				auto client_pos = pInfo->positionlogic();
				auto gap = VECTOR3_DISTANCE(server_pos, client_pos);
				if (gap > kCHARACTER_NOT_SAME_POSITION)
				{
					pCharacter->IncreaseNotSamePositionCount();
				}
			}
		}
	}	
}

DHOST_TYPE_BALL_NUMBER CHost::GetCharacterForceBallEventFail(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetCharacterForceBallEventFail(characterSN);
}

void CHost::SetCharacterForceBallEventFail(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER value)
{
	m_CharacterManager->SetCharacterForceBallEventFail(characterSN, value);
}

DHOST_TYPE_BALL_NUMBER CHost::GetCharacterBallEventSuccess(DHOST_TYPE_CHARACTER_SN characterSN)
{
	return m_CharacterManager->GetCharacterBallEventSuccess(characterSN);
}

void CHost::SetCharacterBallEventSuccess(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER value)
{
	m_CharacterManager->SetCharacterBallEventSuccess(characterSN, value);
}

DHOST_TYPE_BOOL CHost::CheckDoYouHaveMedal(SPlayerInformationT* pInfo, std::vector<F4PACKET::MEDAL_INDEX>& vInfo)
{
	DHOST_TYPE_BOOL result = false;

	if (vInfo.size() == 0)
	{
		result = true;
	}

	if (pInfo != nullptr && result == false)
	{
		for (int i = 0; i < vInfo.size(); ++i)
		{
			F4PACKET::MEDAL_INDEX param_index = vInfo[i];

			DHOST_TYPE_BOOL bCheck = false;

			for (int j = 0; j < pInfo->medals.size(); ++j)
			{
				F4PACKET::MEDAL_INDEX own_index = pInfo->medals[i].index();

				if (own_index == param_index)
				{
					bCheck = true;
					break;
				}
			}

			if (bCheck == false)
			{
				break;
			}
		}
	}

	return result;
}

DHOST_TYPE_INT32 CHost::CheckDoYouHaveMedal(SPlayerInformationT* pInfo, F4PACKET::MEDAL_INDEX value)
{
	DHOST_TYPE_INT32 result = kINT32_INIT;

	if (pInfo != nullptr)
	{
		for (int i = 0; i < pInfo->medals.size(); ++i)
		{
			F4PACKET::MEDAL_INDEX own_index = pInfo->medals[i].index();

			if (own_index == value)
			{
				result = pInfo->medals[i].value();
				break;
			}
		}
	}

	return result;
}


DHOST_TYPE_BOOL CHost::CheckDoYouHaveSignature(SPlayerInformationT* pInfo, DHOST_TYPE_INT32 value)
{
	DHOST_TYPE_BOOL result = false;

	if (value == kINT32_INIT)
	{
		result = true;
	}

	if (pInfo != nullptr && result == false)
	{
		for (int i = 0; i < pInfo->signatures.size(); ++i)
		{
			if (pInfo->signatures[i] == value)
			{
				result = true;
				break;
			}
		}
	}

	return result;
}

DHOST_TYPE_BOOL CHost::CheckNicePassBonus(F4PACKET::SBallShotT* pBallShot, SPlayerInformationT* pInfo)
{
	DHOST_TYPE_BOOL result = false;

	//! 나이스패스 보너스값 검수
	if (pBallShot != nullptr)
	{
		CCharacter* pCharacter = GetCharacterManager()->GetCharacter(pBallShot->owner);
		if (pCharacter != nullptr)
		{
			CAbility* pOwnerAbility = pCharacter->GetAbility();
			if (pOwnerAbility != nullptr)
			{
				if (pBallShot->bonusnicepass > kFLOAT_INIT)
				{
					DHOST_TYPE_FLOAT check_nice_pass = kFLOAT_INIT;

					DHOST_TYPE_FLOAT a = kFLOAT_INIT;
					DHOST_TYPE_FLOAT b = kFLOAT_INIT;
					DHOST_TYPE_FLOAT c = kFLOAT_INIT;

					switch (pBallShot->shottype)
					{
						case F4PACKET::SHOT_TYPE::shotType_dunkDriveIn:
						case F4PACKET::SHOT_TYPE::shotType_dunkFingerRoll:
						case F4PACKET::SHOT_TYPE::shotType_dunkPost:
						{
							a = pOwnerAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_dunk);
							b = GetBalanceTable()->GetValue("VNicePassBonus_DunkA");
							c = GetBalanceTable()->GetValue("VNicePassBonus_DunkB");
						}
						break;
						case F4PACKET::SHOT_TYPE::shotType_layUpDriveIn:
						case F4PACKET::SHOT_TYPE::shotType_layUpFingerRoll:
						case F4PACKET::SHOT_TYPE::shotType_layUpPost:
						{
							a = pOwnerAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_layUp);
							b = GetBalanceTable()->GetValue("VNicePassBonus_LayUpA");
							c = GetBalanceTable()->GetValue("VNicePassBonus_LayUpB");
						}
						break;
						case F4PACKET::SHOT_TYPE::shotType_middle:
						{
							a = pOwnerAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_midRangeShot);
							b = GetBalanceTable()->GetValue("VNicePassBonus_MidRangeShotA");
							c = GetBalanceTable()->GetValue("VNicePassBonus_MidRangeShotB");
						}
						break;
						case F4PACKET::SHOT_TYPE::shotType_post:
						case F4PACKET::SHOT_TYPE::shotType_hook:
						{
							a = pOwnerAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_postShot);
							b = GetBalanceTable()->GetValue("VNicePassBonus_PostShotA");
							c = GetBalanceTable()->GetValue("VNicePassBonus_PostShotB");
						}
						break;
						case F4PACKET::SHOT_TYPE::shotType_threePoint:
						{
							a = pOwnerAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_threePointShot);
							b = GetBalanceTable()->GetValue("VNicePassBonus_ThreePointShotA");
							c = GetBalanceTable()->GetValue("VNicePassBonus_ThreePointShotB");
						}
						break;
						default:
							break;
					}

					check_nice_pass = a * b + c;

					/*{
						string nicepass_log = "[NICE_PASS] first : " + std::to_string(check_nice_pass)
							+ ", a : " + std::to_string(a) + ", b : " + std::to_string(b)
							+ ", c : " + std::to_string(c)
							+ ", shotType : " + F4PACKET::EnumNameSHOT_TYPE(pBallShot->shottype);
						ToLog(nicepass_log.c_str());
					}*/

					if (pBallShot->nicepasshindrance)
					{
						check_nice_pass *= GetBalanceTable()->GetValue("VNicePassBonus_Hindrance");
					}

					/*{
						string nicepass_log = "[NICE_PASS] Hindrance : " + std::to_string(check_nice_pass);
						ToLog(nicepass_log.c_str());
					}*/

					CCharacter* pPasser = GetCharacterManager()->GetCharacter(pBallShot->nicepasspasser);
					if (pPasser != nullptr)
					{
						F4PACKET::SPlayerInformationT* pPasserInformation = pPasser->GetCharacterInformation();
						if (pPasserInformation != nullptr)
						{
							DHOST_TYPE_INT32 check_medal_data = CheckDoYouHaveMedal(pPasserInformation, F4PACKET::MEDAL_INDEX::medal_commander);
							if (check_medal_data > kINT32_INIT)
							{
								check_nice_pass += GetBalanceTable()->GetValue("VNicePassBonus_Medal_Commander") * (check_medal_data * 1.0f);

								/*{
									string nicepass_log = "[NICE_PASS] medal_commander : " + std::to_string(check_nice_pass);
									ToLog(nicepass_log.c_str());
								}*/
							}

							check_medal_data = CheckDoYouHaveMedal(pPasserInformation, F4PACKET::MEDAL_INDEX::commander);
							if (check_medal_data > kINT32_INIT)
							{
								check_nice_pass += GetBalanceTable()->GetValue("VNicePassBonus_Medal_Commander") * (check_medal_data * 1.0f);

								/*{
									string nicepass_log = "[NICE_PASS] commander : " + std::to_string(check_nice_pass)
										+ ", F4PACKET::MEDAL_INDEX::commander : " + std::to_string(check_medal_data)
										+ ", VNicePassBonus_Medal_Commander : " + std::to_string(GetBalanceTable()->GetValue("VNicePassBonus_Medal_Commander"));
									ToLog(nicepass_log.c_str());
								}*/
							}

							check_medal_data = CheckDoYouHaveMedal(pPasserInformation, F4PACKET::MEDAL_INDEX::sacrifice);
							if (check_medal_data > kINT32_INIT)
							{
								check_nice_pass += GetBalanceTable()->GetValue("VNicePassBonus_Medal_Sacrifice") * (check_medal_data * 1.0f);
							}
						}
					}

					DHOST_TYPE_INT32 trunc_bonusnicepass = (DHOST_TYPE_INT32)truncf(pBallShot->bonusnicepass * 100.0f);
					DHOST_TYPE_INT32 trunc_check_nice_pass = (DHOST_TYPE_INT32)truncf(check_nice_pass * 100.0f);

					DHOST_TYPE_INT32 diff = abs(trunc_bonusnicepass - trunc_check_nice_pass);

					if (diff <= 1)
					{
						result = true;
					}
					else
					{
						string nicepass_log = "[BALL_SHOT] NICE_PASS_BONUS_CHECK RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", GameTime : " + std::to_string(GetGameTime())
							+ ", UserID : " + std::to_string(pInfo->userid) + ", result : " + std::to_string(result)
							+ ", diff : " + std::to_string(diff)
							+ ", nicepasshindrance : " + std::to_string(pBallShot->nicepasshindrance)
							+ ", pBallShot->bonusnicepass : " + std::to_string(pBallShot->bonusnicepass)
							+ ", check_nice_pass : " + std::to_string(check_nice_pass);
						ToLog(nicepass_log.c_str());
					}
				}
			}
		}
	}

	return result;
}

DHOST_TYPE_BOOL CHost::InitialPassiveDataSet(DHOST_TYPE_CHARACTER_SN characterSN, const F4PACKET::play_c2s_playerPassiveDataSet_data* pInfo)
{
	if (GetCharacterManager() != nullptr)
	{
		return GetCharacterManager()->InitialPassiveDataSet(characterSN, pInfo);
	}
	return false;
}


DHOST_TYPE_BOOL  CHost::SetPacketCharacterPassive(DHOST_TYPE_CHARACTER_SN characterSN, const F4PACKET::play_c2s_playerPassive_data* pInfo)
{
	if (GetCharacterManager() != nullptr)
	{
		return GetCharacterManager()->SetPacketCharacterPassive(characterSN, pInfo);
	}
	return false;
}


DHOST_TYPE_BOOL CHost::GetPotentialInfo(DHOST_TYPE_CHARACTER_SN id, ACTION_TYPE actionType, SHOT_TYPE shotType, SKILL_INDEX skillIndex, SPotentialInfo& sInfo)
{
	DHOST_TYPE_BOOL result = false;

	if (GetCharacterManager() != nullptr)
	{
		result = GetCharacterManager()->GetPotentialInfo(id, actionType, shotType, skillIndex, sInfo);
	}

	return result;
}

DHOST_TYPE_BOOL CHost::GetCharacterPotentialInfo(DHOST_TYPE_CHARACTER_SN id, F4PACKET::POTENTIAL_INDEX value, SPotentialInfo& sInfo)
{
	DHOST_TYPE_BOOL result = false;

	if (GetCharacterManager() != nullptr)
	{
		result = GetCharacterManager()->GetCharacterPotentialInfo(id, value, sInfo);
	}

	return result;
}

DHOST_TYPE_FLOAT CHost::GetCharacterPotentialValueSum(DHOST_TYPE_CHARACTER_SN id)
{
	DHOST_TYPE_FLOAT result = kFLOAT_INIT;

	if (GetCharacterManager() != nullptr)
	{
		result = GetCharacterManager()->GetCharacterPotentialValueSum(id);
	}

	return result;
}

void CHost::BadMannerNotice(uint32_t CharacterSN, uint16_t BadMannerType, uint16_t Count)
{
	std::string sendMessage = "";

	if (m_CharacterManager != nullptr)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_CharacterManager->GetCharacterInformation(CharacterSN);

		if (pCharacter != nullptr)
		{
			sendMessage = "[BAD_MANNER] RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + std::to_string(pCharacter->userid) + ", Name : " + pCharacter->name + ", Type : ";

			switch (BadMannerType)
			{
				case (uint16_t)EBAD_MANNER_TYPE::INTENTIONAL_BALL_HOG_PLAY:
					sendMessage += "INTENTIONAL_BALL_HOG_PLAY ";
					break;
				case (uint16_t)EBAD_MANNER_TYPE::AWAY_FROM_KEYBOARD:
					sendMessage += "AWAY_FROM_KEYBOARD ";
					AwayFromKeyboard(pCharacter->userid, CharacterSN);
					break;
				case (uint16_t)EBAD_MANNER_TYPE::IMPOSSIBLE_SHOOTING:
					sendMessage += "IMPOSSIBLE_SHOOTING ";
					break;
				default:
					sendMessage += "NONE ";
					break;
			}
			sendMessage += ", Count : " + std::to_string(Count) + ", CurrentGameTime : " + std::to_string(GetGameTime());

			//ToLog(sendMessage.c_str());

			SendDebugMessage(sendMessage);
		}
	}
}

void CHost::SendDebugMessage(string str)
{
#ifdef _DEBUG
	CREATE_BUILDER(builder)
	auto stringoffset = builder.CreateString(str);
	CREATE_FBPACKET(builder, system_s2c_debugString, debug_message, databuilder);
	databuilder.add_str(stringoffset);
	STORE_FBPACKET(builder, debug_message, databuilder);
	BroadcastPacket(debug_message, kUSER_ID_INIT); 				
#endif
}

void CHost::SendDebugMessageUserOnly(string str, int UserID)
{
#ifdef _DEBUG
	CREATE_BUILDER(builder)
	auto stringoffset = builder.CreateString(str);
	CREATE_FBPACKET(builder, system_s2c_debugString, debug_message, databuilder);
	databuilder.add_str(stringoffset);
	STORE_FBPACKET(builder, debug_message, databuilder);
	SendPacket(debug_message, UserID);
#endif
}


void CHost::CreateEnvironment()
{
	if (m_pEnvironmentObject == nullptr)
	{
		m_pEnvironmentObject = new CEnvironmentObject(this);
	}
}

bool CHost::EnvironmentCatSurprised(int moveNumber, TB::SVector3 position)
{
	if (m_pEnvironmentObject != nullptr)
	{
		return m_pEnvironmentObject->CatSetMove(moveNumber, position, true);
	}

	return false;
}


DHOST_TYPE_BOOL CHost::CheckReplay()
{
	if (!CheckReplayCooldownTime())
	{
		return false;
	}

	if (!CheckReplayRequireAssist())
	{
		return false;
	}

	if (!CheckReplayQualifyingPoint())
	{
		return false;
	}

	return true;
}

DHOST_TYPE_BOOL CHost::CheckReplayCooldownTime()
{
	if (GetReplayTime() == GetGameTimeInit() || (GetReplayTime() - GetGameTime()) >= GetBalanceTable()->GetValue("VReplayCoolTime"))
	{
		return true;
	}

	return false;
}

DHOST_TYPE_BOOL CHost::CheckReplayRequireAssist()
{
	if (GetBalanceTable()->GetValue("VReplayRequiresAssist") > kFLOAT_INIT)
	{
		return GetReplayAssistCheck();
	}

	return true;
}

DHOST_TYPE_BOOL CHost::CheckReplayQualifyingPoint()
{
	DHOST_TYPE_FLOAT point = m_CharacterManager->CalcReplayPoint(GetScoredTime());

	SetReplayPoint(point);

	if (GetReplayPoint() >= GetBalanceTable()->GetValue("VReplayQualifyingPoint"))
	{
		return true;
	}

	return false;
}

void CHost::CheckBuzzerBeater(DHOST_TYPE_INT32 ScoreTeam, DHOST_TYPE_INT32 Point, DHOST_TYPE_FLOAT PositionX, DHOST_TYPE_FLOAT PositionZ)
{
	//! 버저비터 판단조건 1 (게임시간이 1초 미만인가)
	if (GetGameTime() < kBUZZER_BEATER_TIME)
	{
		//! 버저비터 판단조건 2 (득점으로 팀이 승리했는가)
		DHOST_TYPE_INT32 MyTeamScore = GetScore(ScoreTeam);
		DHOST_TYPE_INT32 OpponentTeamScore = GetScore(GetIndexOpponentTeam(ScoreTeam));

		if (MyTeamScore > OpponentTeamScore  && MyTeamScore - Point <= OpponentTeamScore)
		{
			//! 버저비터 판단조건 3 (골대와의 거리가 충분한가 5m)
			SVector3 ball_pos(PositionX, 0.f, PositionZ);
			SVector3 rim_pos(GetPhysicsHandler()->GetRimPosition().x(), 0.f, GetPhysicsHandler()->GetRimPosition().z());

			float distance = VECTOR3_DISTANCE(ball_pos, rim_pos);

			if (distance >= kBUZZER_BEATER_DISTANCE)
			{
				m_GameManager->SetBuzzerBeater(true);
			}
		}
	}
}

DHOST_TYPE_BOOL CHost::GetBuzzerBeater()
{
	return m_GameManager->GetBuzzerBeater();
}

DHOST_TYPE_FLOAT CHost::GetShotRoomElapsedTime()
{
	return m_GameManager->GetShotRoomElapsedTime();
}

void CHost::SetShotRoomElapsedTime(DHOST_TYPE_FLOAT value)
{
	m_GameManager->SetShotRoomElapsedTime(value);
}

DHOST_TYPE_INT32 CHost::GetOffenseTeam()
{
	return m_GameManager->GetOffenseTeam();
}

void CHost::SetOffenseTeam(DHOST_TYPE_INT32  value)
{
	m_GameManager->SetOffenseTeam(value);
}

DHOST_TYPE_INT32 CHost::GetWinnerTeam()
{
	return m_GameManager->GetWinnerTeam();
}

void CHost::SetWinnerTeam()
{
	m_GameManager->SetWinnerTeam();
}

DHOST_TYPE_INT32 CHost::GetAbNormalEndUserTeam()
{
	return m_GameManager->GetAbNormalEndUserTeam();
}

void CHost::SetAbNormalEndUserTeam(DHOST_TYPE_INT32 team)
{
	m_GameManager->SetAbNormalEndUserTeam(team);
}

DHOST_TYPE_BOOL CHost::GetAbnormalExpireRoom()
{
	return m_GameManager->GetAbnormalExpireRoom();
}

void CHost::SetAbnormalExpireRoom(DHOST_TYPE_BOOL value)
{
	m_GameManager->SetAbnormalExpireRoom(value);
}

DHOST_TYPE_BOOL CHost::GetRedisLoadValue()
{
	return m_GameManager->GetRedisLoadValue();
}

void CHost::SetRedisLoadValue(DHOST_TYPE_BOOL value, DHOST_TYPE_FLOAT time)
{
	m_GameManager->SetRedisLoadValue(value, time);
}

DHOST_TYPE_FLOAT CHost::GetRedisLoadRoomElapsedTime()
{
	return m_GameManager->GetRedisLoadRoomElapsedTime();
}

void CHost::SetRedisLoadRoomElapsedTime(DHOST_TYPE_FLOAT value)
{
	m_GameManager->SetRedisLoadRoomElapsedTime(value);
}

DHOST_TYPE_BOOL CHost::GetRedisSaveMatchResult()
{
	return m_GameManager->GetRedisSaveMatchResult();
}

void CHost::SetRedisSaveMatchResult(DHOST_TYPE_BOOL value)
{
	m_GameManager->SetRedisSaveMatchResult(value);
}

DHOST_TYPE_INT32 CHost::GetOption()
{
	return m_GameManager->GetOption();
}

void CHost::SetOption(DHOST_TYPE_UINT64 value)
{
	m_GameManager->SetOption(value);
}

DHOST_TYPE_BOOL CHost::CheckOption(DHOST_TYPE_UINT64 flag)
{
	return m_GameManager->CheckOption(flag);
}

std::vector<DHOST_TYPE_INT32>& CHost::GetVecScores()
{
	return m_GameManager->GetVecScores();
}

DHOST_TYPE_FLOAT CHost::GetSpeedHackRestrictionStepOne()
{
	return m_GameManager->GetSpeedHackRestrictionStepOne();
}

void CHost::SetSpeedHackRestrictionStepOne(DHOST_TYPE_FLOAT value)
{
	m_GameManager->SetSpeedHackRestrictionStepOne(value);
}

DHOST_TYPE_FLOAT CHost::GetSpeedHackRestrictionStepTwo()
{
	return m_GameManager->GetSpeedHackRestrictionStepTwo();
}

void CHost::SetSpeedHackRestrictionStepTwo(DHOST_TYPE_FLOAT value)
{
	m_GameManager->SetSpeedHackRestrictionStepTwo(value);
}

DHOST_TYPE_UINT32 CHost::GetSwitchToAiLevel()
{
	return m_GameManager->GetSwitchToAiLevel();
}

void CHost::SetSwitchToAiLevel(DHOST_TYPE_UINT32 value)
{
	m_GameManager->SetSwitchToAiLevel(value);
}

DHOST_TYPE_BOOL CHost::GetLogShotRate()
{
	return m_GameManager->GetLogShotRate();
}

void CHost::SetLogShotRate(DHOST_TYPE_BOOL value)
{
	m_GameManager->SetLogShotRate(value);
}

DHOST_TYPE_INT32 CHost::GetOverTimeCount()
{
	return m_GameManager->GetOverTimeCount();
}

void CHost::IncreaseOverTimeCount()
{
	m_GameManager->IncreaseOverTimeCount();
}
//! end

void CHost::BadConnectProcess(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_BOOL ByEnet)
{
	CHostUserInfo* pUser = FindUser(UserID);

	if (pUser != nullptr)
	{
		pUser->SetConnectState(ECONNECT_STATE::DISCONNECT);
		pUser->ClearPingData();

		PushBadConnectUser(pUser->GetUserID());

		// 호스트 핑 체크로 불량유저 판단일 때만 해당 유저에게 나가라고 한다. 이넷에서 끊어졌다면 이미 패킷을 받을 수 없는 상태이기 때문에
		if (ByEnet == true)
		{
			string log_message = "DISCONNECT_USER DisConnected by Enet BadConnectUser : " + std::to_string(pUser->GetUserID());
			ToLog(log_message.c_str());

			SetCharacterKickType(pUser->GetUserID(), ECHARACTER_KICK_TYPE::LONG_LATENCY);
		}
		else
		{
			string log_message = "DISCONNECT_USER DisConnected by Host BadConnectUser : " + std::to_string(pUser->GetUserID());
			ToLog(log_message.c_str());

			KickToTheLobby(UserID, F4PACKET::EKICK_TYPE::long_latency);
		}
	}
}

void CHost::PushBadConnectUser(DHOST_TYPE_USER_ID UserID)
{
	DHOST_TYPE_BOOL bInsert = true;

	F4PACKET::SPlayerInformationT* pCharacter = FindCharacterInformationWithUserIDAndIsNotAi(UserID);

	// 트리오 모드인 경우 유저 아이디가 같기 때문에 캐릭터를 구분할 필요가 없다.
	if (pCharacter == nullptr && GetModeType() == EMODE_TYPE::TRIO || GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED)
	{
		pCharacter = FindCharacterInformationWithUserID(UserID);
	}

	if (pCharacter != nullptr)
	{
		HostBadConnectUserListType::iterator iter;

		for (iter = m_ListBadConnectUser.begin(); iter != m_ListBadConnectUser.end(); iter++)
		{
			if (*iter == UserID)
			{
				bInsert = false;

				break;
			}
		}

		if (bInsert)
		{
			string log_message = "PushBadConnectUser UserID : " + to_string(UserID) + ", CharacterSN : " + to_string(pCharacter->id);
			ToLog(log_message.c_str());

			m_ListBadConnectUser.push_back(UserID);

			GetCharacterManager()->SetSwitchToAi(pCharacter->id, true);
			GetCharacterManager()->SetCharacterDisconnectTime(pCharacter->id, GetRoomElapsedTime());

			RemoveLoginCompleteUser(UserID);
			RemoveLoadCompleteUser(UserID);
			RemoveHostJoinUser(UserID);
		}
	}
}

void CHost::PushAiHostCandidateUser(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_CHARACTER_SN CharacterSN, DHOST_TYPE_UINT32 AiLevel)
{
	DHOST_TYPE_BOOL insert = true;

	for (int i = 0; i < m_VecAiHostCandidateUser.size(); ++i)
	{
		if (m_VecAiHostCandidateUser[i] == UserID)
		{
			insert = false;
			break;
		}
	}

	// 후보자 등록
	if (insert)
	{
		m_VecAiHostCandidateUser.push_back(UserID);

#ifdef _DEBUG
		string log_message = "PushAiHostCandidateUser UserID : " + std::to_string(UserID) + ", m_VecAiHostCandidateUserSize : " + std::to_string(m_VecAiHostCandidateUser.size());
		ToLog(log_message.c_str());
#endif // DEBUG
	}
}


void CHost::PushCallbackEnetDisConnectUser(DHOST_TYPE_USER_ID UserID)
{
	m_VecCallbackEnetDisConnectUser.push_back(UserID);

	RemoveLoginCompleteUser(UserID);
	RemoveLoadCompleteUser(UserID);
	RemoveHostJoinUser(UserID);
}

void CHost::RemoveCallbackEnetDisConnectUser(DHOST_TYPE_USER_ID UserID)
{
	std::vector<DHOST_TYPE_USER_ID>::iterator iter;

	for (iter = m_VecCallbackEnetDisConnectUser.begin(); iter != m_VecCallbackEnetDisConnectUser.end(); ++iter)
	{
		if (*iter == UserID)
		{
			string log_message = "RemoveCallbackEnetDisConnectUser UserID : " + std::to_string(UserID);
			ToLog(log_message.c_str());

			m_VecCallbackEnetDisConnectUser.erase(iter);

			break;
		}
	}
}

DHOST_TYPE_BOOL CHost::CheckCallbackEnetDisConnectUser(DHOST_TYPE_USER_ID UserID)
{
	for (int i = 0; i < m_VecCallbackEnetDisConnectUser.size(); ++i)
	{
		if (UserID == m_VecCallbackEnetDisConnectUser[i])
		{
			return true;
		}
	}

	return false;
}

DHOST_TYPE_BOOL CHost::CheckUserPingAvg(CHostUserInfo* pUser)
{
	if (GetOption() & HOST_OPTION_NO_KICK)
	{
		return false;
	}

#ifdef _DEBUG
	if (GetDevNoKick())
	{
		return false;
	}
#endif // _DEBUG

	//! 20221207 신버전 핑체크 - by thinkingpig
	{
		if (pUser->GetPingAverage() >= kPING_EXPIRED_AVG_TIME)
		{
			if (m_CurrentState == EHOST_STATE::LOAD)
			{ 
				string log_message = "CheckUserPingAvg Kick During Loading, RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", UserID : " + std::to_string(pUser->GetUserID()) + ", Avg : " + std::to_string(pUser->GetPingAverage());
				ToLog(log_message.c_str());
			}
			else
			{
				string log_message = "CheckUserPingAvg Kick,  RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) 
					+ ", UserID : " + std::to_string(pUser->GetUserID()) 
					+ ", Avg : " + std::to_string(pUser->GetPingAverage())
					+ ", Kick_CurrentSate : " + std::to_string((int)m_CurrentState);

				ToLog(log_message.c_str());
			}

			return true;
		}
	}
	
	return false;
}

DHOST_TYPE_BOOL CHost::GetUserConnectState(CHostUserInfo* pUser)
{
	if (pUser != nullptr)
	{
		if (pUser->GetConnectState() == ECONNECT_STATE::JOIN || pUser->GetConnectState() == ECONNECT_STATE::CONNECT)
		{
			return true;
		}
	}

	return false;
}

void CHost::CheckAiBeingControlled()
{
	if (CheckBadConnectUserCheckList() == false)
	{
		return;
	}

	HostBadConnectUserListType::iterator iter;

	for (iter = m_ListBadConnectUser.begin(); iter != m_ListBadConnectUser.end(); iter++)
	{
		DHOST_TYPE_BOOL bCheckDataExists = FindBadConnectUserCheckList(*iter);

		// 접속 불량 유저가 이미 다른 유저에게 컨트롤 권한이 넘어간 경우 스킵
		if (bCheckDataExists)
		{
			continue;
		}

		CHostUserInfo* pDisConnectUser = FindUser(*iter);

		if (pDisConnectUser != nullptr)
		{
			// 접속 불량인 유저가 다른 캐릭터의 컨트롤을 가지고 있는 경우라면 해당 유저가 가지고 있던 컨트롤 권한을 초기화해준다.
			if (pDisConnectUser->GetUserWithAiCharacter().empty() == false)
			{
				auto vec = pDisConnectUser->GetUserWithAiCharacter();

				for (int k = 0; k < vec.size(); ++k)
				{
					DHOST_TYPE_USER_ID RemoveUserID = FindUserIDWithCharacterSN(vec[k]);

					if (FindBadConnectUserCheckList(RemoveUserID))
					{
						m_ListBadConnectUserCheck.remove(RemoveUserID);
					}
				}
			}
		}
	}
}

// ai 컨트롤을 가장 적게하고 있는 정상 유저 찾기
DHOST_TYPE_USER_ID CHost::FindUserWithLeastAiControl()
{
	DHOST_TYPE_USER_ID AiOwnerUserID = kUINT32_INIT;
	DHOST_TYPE_UINT64 AiControlCount = kUINT64_INIT;

	for (int i = 0; i < m_VecAiHostCandidateUser.size(); ++i)
	{
		CHostUserInfo* pUser = FindUser(m_VecAiHostCandidateUser[i]);

		if (pUser != nullptr && CheckBadConnectUser(pUser->GetUserID()) == false && pUser->GetConnectState() == ECONNECT_STATE::CONNECT)
		{
			if (AiOwnerUserID == kUSER_ID_INIT || pUser->GetUserWithAiCharacter().size() < AiControlCount)
			{
				AiControlCount = pUser->GetUserWithAiCharacter().size();
				AiOwnerUserID = pUser->GetUserID();
			}
		}
	}

	return AiOwnerUserID;
}

// 해당 캐릭터의 컨트롤 권한을 가지고 있는 유저를 찾는다.
DHOST_TYPE_USER_ID CHost::FindAiControlUser(DHOST_TYPE_CHARACTER_SN CharacterSN)
{
	for (auto& it : m_HostUserMap)
	{
		auto pUser = it.second;

		if (nullptr == pUser)
		{
			continue;
		}

		auto vec_control_character = pUser->GetUserWithAiCharacter();

		for (int j = 0; j < vec_control_character.size(); ++j)
		{
			if (vec_control_character[j] == CharacterSN)
			{
				return pUser->GetUserID();
			}
		}
	}

	return kUSER_ID_INIT;
}

void CHost::RemoveBadConnectUser(DHOST_TYPE_USER_ID UserID)
{
	HostBadConnectUserListType::iterator iter;

	for (iter = m_ListBadConnectUser.begin(); iter != m_ListBadConnectUser.end(); iter++)
	{
		if (*iter == UserID)
		{
			m_ListBadConnectUser.remove(*iter);

			string log_message = "[REMOVE_BAD_CONNECT_USER] RemoveUserID : " + std::to_string(UserID) + ", BadConnectListSize : " + std::to_string(m_ListBadConnectUser.size());
			ToLog(log_message.c_str());

			break;
		}
	}
}

void CHost::RemoveBadConnectUserCheck(DHOST_TYPE_USER_ID UserID)
{
	HostBadConnectUserListType::iterator iter;

	for (iter = m_ListBadConnectUserCheck.begin(); iter != m_ListBadConnectUserCheck.end(); iter++)
	{
		if (*iter == UserID)
		{
			m_ListBadConnectUserCheck.remove(*iter);

			string log_message = "[REMOVE_BAD_CONNECT_USER_CHECK] RemoveUserID : " + std::to_string(UserID) + ", BadConnectCheckListSize : " + std::to_string(m_ListBadConnectUserCheck.size());
			ToLog(log_message.c_str());

			break;
		}
	}
}

DHOST_TYPE_BOOL CHost::FindBadConnectUserCheckList(DHOST_TYPE_USER_ID UserID)
{
	HostBadConnectUserListType::iterator iter;

	for (iter = m_ListBadConnectUserCheck.begin(); iter != m_ListBadConnectUserCheck.end(); iter++)
	{
		if (iter == m_ListBadConnectUserCheck.end())
		{
			return false;
		}

		if (*iter == UserID)
		{
			return true;
		}
	}

	return false;
}

DHOST_TYPE_BOOL CHost::CheckBadConnectUserCheckList()
{
	if (m_ListBadConnectUser.empty())
	{
		return false;
	}

	if (m_ListBadConnectUser.size() == m_ListBadConnectUserCheck.size())
	{
		return false;
	}

	return true;
}

void CHost::SendDelegateControlToUser()
{
	if (CheckBadConnectUserCheckList() == false)
	{
		return;
	}

	for (HostBadConnectUserListType::iterator bad = m_ListBadConnectUser.begin(); bad != m_ListBadConnectUser.end(); bad++)
	{
		string log_message = "m_ListBadConnectUser DisConnectUserID : " + std::to_string(*bad);
		ToLog(log_message.c_str());
	}

	for (HostBadConnectUserListType::iterator check = m_ListBadConnectUserCheck.begin(); check != m_ListBadConnectUserCheck.end(); check++)
	{
		string log_message = "m_ListBadConnectUserCheck RemainUserID : " + std::to_string(*check);
		ToLog(log_message.c_str());
	}

	HostBadConnectUserListType::iterator iter;

	for (iter = m_ListBadConnectUser.begin(); iter != m_ListBadConnectUser.end(); iter++)
	{
		if (iter == m_ListBadConnectUser.end())
		{
			return;
		}

		DHOST_TYPE_BOOL bCheckDataExists = FindBadConnectUserCheckList(*iter);

		// 접속 불량 유저가 이미 다른 유저에게 컨트롤 권한이 넘어간 경우 스킵
		if (bCheckDataExists)
		{
			continue;
		}

		// Ai를 가장 적게 컨트롤 하고있는 유저를 찾아온다.
		DHOST_TYPE_USER_ID takeAwayUser = FindUserWithLeastAiControl();

		CHostUserInfo* pTakeAwayUser = FindUser(takeAwayUser);
		if (pTakeAwayUser == nullptr)
		{
			return;
		}

		CHostUserInfo* pDisConnectUser = FindUser(*iter);
		if (pDisConnectUser == nullptr)
		{
			return;
		}

		CREATE_BUILDER(builder);

		//! 접속불량자
		int userid = *iter;

		F4PACKET::SPlayerInformationT* pPlayerInfo = FindCharacterInformationWithUserIDAndIsNotAi(userid);

		if (pPlayerInfo == nullptr)
		{
			return;
		}

		int characterid = pPlayerInfo->id;

		std::vector< flatbuffers::Offset<F4PACKET::SDisConnectUserInfo>> veccounterinfo;

		// 접속 불량인 유저데이터도 넘기자
		{
			F4PACKET::SDisConnectUserInfoBuilder rolecountinfobuilder(builder);

			rolecountinfobuilder.add_userid(pTakeAwayUser->GetUserID());
			rolecountinfobuilder.add_characterid(characterid);
			veccounterinfo.push_back(rolecountinfobuilder.Finish());

			string log_message = "SYSTEM_S2C_CHANGE_OF_CONTROL_DATA takeAwayUserID : " + to_string(pTakeAwayUser->GetUserID()) + ", to-Ai-UserSN  : " + to_string(*iter) + ", to-Ai-CharacterSN : " + to_string(characterid);
			ToLog(log_message.c_str());

			pTakeAwayUser->PushUserWithAiCharacter(characterid);

			{
				F4PACKET::SPlayerInformationT* pAiHostCharacter = FindCharacterInformationWithUserIDAndIsNotAi(pTakeAwayUser->GetUserID());

				F4PACKET::SPlayerInformationT* pAiCharacter = FindCharacterInformation(characterid);

				// 해당 캐릭터는 AI라고 알려준다. 핵 체크를 안하기 위해 ( 2025-04-23 )
				CCharacter*  pCharacter = GetCharacterManager()->GetCharacter(characterid);
				if (pCharacter)
				{
					pCharacter->SetAI(true);
				}

				if (pAiHostCharacter != nullptr && pAiCharacter != nullptr)
				{
					string log_message = "CHANGE_OF_CONTROL Character : " + pAiCharacter->name + " >>>>>> Character : " + pAiHostCharacter->name;
					SendDebugMessage(log_message);
				}
			}
		}

		auto vDisConnectCharacter = pDisConnectUser->GetUserWithAiCharacter();

		if (vDisConnectCharacter.empty() == false)
		{
			// 접속 불량인 유저가 컨트롤하고 있던 캐릭터
			for (int i = 0; i < vDisConnectCharacter.size(); ++i)
			{
				F4PACKET::SDisConnectUserInfoBuilder rolecountinfobuilder(builder);

				rolecountinfobuilder.add_userid(pTakeAwayUser->GetUserID());
				rolecountinfobuilder.add_characterid(vDisConnectCharacter[i]);
				veccounterinfo.push_back(rolecountinfobuilder.Finish());

				string log_message = "SYSTEM_S2C_CHANGE_OF_CONTROL_DATA with ai takeAwayUserID : " + to_string(pTakeAwayUser->GetUserID()) + ", AiUserSN  : " + to_string(*iter) + ", AiCharacterSN : " + to_string(vDisConnectCharacter[i]);
				ToLog(log_message.c_str());

				pTakeAwayUser->PushUserWithAiCharacter(vDisConnectCharacter[i]);

				{
					F4PACKET::SPlayerInformationT* pAiHostCharacter = FindCharacterInformationWithUserIDAndIsNotAi(pTakeAwayUser->GetUserID());
					F4PACKET::SPlayerInformationT* pAiCharacter = FindCharacterInformation(vDisConnectCharacter[i]);
					if (pAiHostCharacter != nullptr && pAiCharacter != nullptr)
					{
						// 기존에 가지고 있던 AiHostUserID 를 권한을 가져가는 유저한테 준다.
						pAiCharacter->userid = pTakeAwayUser->GetUserID();

						string log_message = "CHANGE_OF_CONTROL Character : " + pAiCharacter->name + " >>>>>> Character : " + pAiHostCharacter->name;
						SendDebugMessage(log_message);
					}
				}
			}
		}

		auto offsetcountinfo = builder.CreateVector(veccounterinfo);

		CREATE_FBPACKET(builder, system_s2c_changeOfControl, message, databuilder);

		databuilder.add_sinfo(offsetcountinfo);

		m_ListBadConnectUserCheck.push_back(userid);

		STORE_FBPACKET(builder, message, databuilder);
		SendPacket(message, pTakeAwayUser->GetUserID());

		pDisConnectUser->InitUserWithAiCharacter();
	}
}

void CHost::SendOnFireModeCharacterInfo()
{
	std::vector<DHOST_TYPE_CHARACTER_SN> vecInfo;
	vecInfo.clear();

	vecInfo = m_CharacterManager->GetSendOnFireModeCharacter();

	if (vecInfo.empty() == false)
	{
		CREATE_BUILDER(builder);

		std::vector<flatbuffers::Offset<F4PACKET::SCharacterOnFireModeInfo>> vecSendInfo;


		for (int i = 0; i < vecInfo.size(); ++i)
		{
			F4PACKET::SPlayerInformationT* sInformation = FindCharacterInformation(vecInfo[i]);
			F4PACKET::SPlayerAbilityT* sAbility = GetCharacterManager()->GetCharacter(vecInfo[i])->GetAbility()->GetModificationAbility();

			if (sAbility->block < 0.0f)
			{
				string str = "[HACK_CHECK] [BURST] Ability is wrong RoomElapsedTime : "
					+ std::to_string(m_TimeManager->GetRoomElapsedTime())
					+ "RoomID: " + GetHostID()
					+ ", UserID : " + std::to_string(vecInfo[i])
					+ ", sAbility->block : " + std::to_string(sAbility->block);

				ToLog(str.c_str());
			}

			F4PACKET::SPlayerAbilityT sData;
			sData.block = sAbility->block;
			sData.dribble = sAbility->dribble;
			sData.dunk = sAbility->dunk;
			sData.layup = sAbility->layup;
			sData.midrangeshot = sAbility->midrangeshot;
			sData.pass = sAbility->pass;
			sData.perimeterdefense = sAbility->perimeterdefense;
			sData.postdefense = sAbility->postdefense;
			sData.postmove = sAbility->postmove;
			sData.postshot = sAbility->postshot;
			sData.rebound = sAbility->rebound;
			sData.speed = sAbility->speed;
			sData.steal = sAbility->steal;
			sData.strength = sAbility->strength;
			sData.threepointshot = sAbility->threepointshot;
			sData.vertical = sAbility->vertical;

			DHOST_TYPE_FLOAT stat_mod = GetCharacterManager()->FindSendOnFireModeCharacterReservation(vecInfo[i]);

			if (sAbility->block <= 0.0f)
			{
				if (stat_mod < 0)
				{
					string str = "[HACK_CHECK] [BURST] Ability is wrong RoomElapsedTime : "
						+ std::to_string(m_TimeManager->GetRoomElapsedTime())
						+ "RoomID: " + GetHostID()
						+ ", UserID : " + std::to_string(vecInfo[i])
						+ ", sAbility->block : " + std::to_string(sAbility->block)
						+ ", stat_mod : " + std::to_string(stat_mod);

					ToLog(str.c_str());
				}
			}
			

			sData.block += stat_mod;
			sData.dribble += stat_mod;
			sData.dunk += stat_mod;
			sData.layup += stat_mod;
			sData.midrangeshot += stat_mod;
			sData.pass += stat_mod;
			sData.perimeterdefense += stat_mod;
			sData.postdefense += stat_mod;
			sData.postmove += stat_mod;
			sData.postshot += stat_mod;
			sData.rebound += stat_mod;
			sData.speed += stat_mod;
			sData.steal += stat_mod;
			sData.strength += stat_mod;
			sData.threepointshot += stat_mod;
			sData.vertical += stat_mod;

			if( sData.block < 0.0f || sData.dribble < 0.0f || sData.dunk < 0.0f || sData.layup < 0.0f || sData.midrangeshot < 0.0f || sData.pass < 0.0f || sData.perimeterdefense < 0.0f || 
				sData.postdefense < 0.0f || sData.postmove < 0.0f || sData.postshot < 0.0f || sData.rebound < 0.0f || sData.speed < 0.0f || sData.steal < 0.0f || sData.strength < 0.0f || 
				sData.threepointshot < 0.0f || sData.vertical < 0.0f )
			{

				string str = "[HACK_CHECK] [BURST] Ability is wrong RoomElapsedTime : "
					+ std::to_string(m_TimeManager->GetRoomElapsedTime())
					+ "RoomID: " + GetHostID()
					+ ", UserID : " + std::to_string(vecInfo[i])
					+ ", block : " + std::to_string(sData.block); 

				/*
					+ ", dribble : " + std::to_string(sData.dribble)
					+ ", dunk : " + std::to_string(sData.dunk)
					+ ", layup : " + std::to_string(sData.layup)
					+ ", midrangeshot : " + std::to_string(sData.midrangeshot)
					+ ", pass : " + std::to_string(sData.pass)
					+ ", perimeterdefense : " + std::to_string(sData.perimeterdefense)
					+ ", postdefense : " + std::to_string(sData.postdefense)
					+ ", postmove : " + std::to_string(sData.postmove)
					+ ", postshot : " + std::to_string(sData.postshot)
					+ ", rebound : " + std::to_string(sData.rebound)
					+ ", pass : " + std::to_string(sData.pass)
					+ ", speed : " + std::to_string(sData.speed)
					+ ", steal : " + std::to_string(sData.steal)
					+ ", strength : " + std::to_string(sData.strength)
					+ ", threepointshot : " + std::to_string(sData.threepointshot)
					+ ", vertical : " + std::to_string(sData.vertical);
							
				
				ToLog(str.c_str()); */


				// 예외처리 
				sData.block = 0.0f;
				sData.dribble = 0.0f;
				sData.dunk = 0.0f;
				sData.layup = 0.0f;
				sData.midrangeshot = 0.0f;
				sData.pass = 0.0f;
				sData.perimeterdefense = 0.0f;
				sData.postdefense = 0.0f;
				sData.postmove = 0.0f;
				sData.postshot = 0.0f;
				sData.rebound = 0.0f;
				sData.speed = 0.0f; 
				sData.steal = 0.0f;
				sData.strength = 0.0f;
				sData.threepointshot = 0.0f;
				sData.vertical = 0.0f;


				// 서버에서 먼저 버스트를 보내면서 예약한 후, 클라이언트가 응답을 하면 서버에서 값을 바꾸는 구조. 
				// 1. 여기서 0 이하가 안되게 계산해서 보내야, 클라와 서버가 동기화가 됨 
				// 2. 한 캐릭터가 여러개의 버스트를 발생 시켜도  map 으로 관리하기 때문에 이미 값이 있다면 최신으로 바꾸지 않는다.
				
				// 0보다 작은 경우, 이경우가 나오면 안되지만 서버 버그를 찾아야 한다.  
				GetCharacterManager()->GetCharacter(vecInfo[i])->GetAbility()->SetModificationAbilityAll(0.0f);

				// 이경우에는 예약된 것을 실행하지 말아야 한다.
				GetCharacterManager()->RemoveSendOnFireModeCharacterReservation(vecInfo[i]);

				
			}

			auto data = F4PACKET::SPlayerAbility::Pack(builder, &sData);

			F4PACKET::SCharacterOnFireModeInfoBuilder pushData(builder);			

			DHOST_TYPE_FLOAT burst_gauge = GetCharacterBurstGauge(sInformation->id);

			DHOST_TYPE_INT32 step = kINT32_INIT;

			if (burst_gauge >= GetBalanceTable()->GetValue("VBurst_OnFire2_Standard"))
			{
				step = 2;
			}
			else if (burst_gauge < GetBalanceTable()->GetValue("VBurst_OnFire2_Standard") && burst_gauge >= GetBalanceTable()->GetValue("VBurst_OnFire1_Standard"))
			{
				step = 1;
			}

			pushData.add_id(sInformation->id);
			pushData.add_step(step);
			pushData.add_ability(data);

			vecSendInfo.push_back(pushData.Finish());			
		}

		auto offsetInfo = builder.CreateVector(vecSendInfo);

		CREATE_FBPACKET(builder, play_s2c_onFireModeInfo, message, databuilder);
		databuilder.add_sinfo(offsetInfo);

		STORE_FBPACKET(builder, message, databuilder);

		BroadcastPacket(message, kUSER_ID_INIT);

		m_CharacterManager->ClearSendOnFireModeCharacter();
	}
}

DHOST_TYPE_BOOL CHost::CheckDisconnectUser(DHOST_TYPE_USER_ID UserID)
{
	HostBadConnectUserListType::iterator iter;

	for (iter = m_ListBadConnectUser.begin(); iter != m_ListBadConnectUser.end(); iter++)
	{
		if (*iter == UserID)
		{
			return true;
		}
	}

	return false;
}

// 재접속한 유저에게만 보낸다 
void CHost::SendReconnectUserWithPassiveInfo(DHOST_TYPE_USER_ID UserID)
{

	F4PACKET::SPlayerInformationT* pPlayerInfo = FindCharacterInformationWithUserIDAndIsNotAi(UserID);
	if (pPlayerInfo != nullptr)
	{
		std::vector<F4PACKET::SPlayerInformationT*> vecCharacters;
		vecCharacters.clear();

		GetCharacterInformationVec(vecCharacters);
		for (int i = 0; i < vecCharacters.size(); ++i)
		{
			SCharacterPassive* scharacterPassive = GetCharacterManager()->GetCharacterPassive(vecCharacters[i]->id);

			if (scharacterPassive != nullptr)
			{
				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);
				send_data.add_idplayer(vecCharacters[i]->id);
				send_data.add_type(scharacterPassive->type);
				send_data.add_valuecurr(scharacterPassive->valueCurr);
				send_data.add_valuemax(scharacterPassive->valueMax);
				send_data.add_activate(scharacterPassive->activate);
				send_data.add_effecttrigger(scharacterPassive->effectTrigger);
				send_data.add_starttime(scharacterPassive->startTime);
				send_data.add_hostpermit(true);
				STORE_FBPACKET(builder, message, send_data)

				SendPacket(message, UserID);

				//BroadcastPacket(message, kUSER_ID_INIT);
			}

		}


	}

}

void CHost::SendGameScore()
{
	// 스코어 정보 보내기
	{
		CREATE_BUILDER(builder);

		std::vector< flatbuffers::Offset<F4PACKET::SReconnectGameScoreInfo>> veccounterinfo;

		std::vector<DHOST_TYPE_INT32>& team_scores = GetVecScores();

		for (int i = 0; i < team_scores.size(); ++i)
		{
			F4PACKET::SReconnectGameScoreInfoBuilder rolecountinfobuilder(builder);

			rolecountinfobuilder.add_team(i);
			rolecountinfobuilder.add_score(team_scores[i]);

			veccounterinfo.push_back(rolecountinfobuilder.Finish());
		}

		auto offsetcountinfo = builder.CreateVector(veccounterinfo);

		CREATE_FBPACKET(builder, system_s2c_reconnectGameScore, message, databuilder);
		databuilder.add_sinfo(offsetcountinfo);
		STORE_FBPACKET(builder, message, databuilder);

		BroadcastPacket(message, kUSER_ID_INIT);
		
		string log_message = "Send GameScore RoomElapsedTime : " + std::to_string(GetRoomElapsedTime());
		ToLog(log_message.c_str());
	}
}

// 재접속시 게임 정보를 보내준다.
void CHost::SendReconnectUserWithGameInfo(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_CHARACTER_SN CharacterSN)
{
	// 스코어 정보 보내기
	{
		CREATE_BUILDER(builder);

		std::vector< flatbuffers::Offset<F4PACKET::SReconnectGameScoreInfo>> veccounterinfo;

		std::vector<DHOST_TYPE_INT32>& team_scores = GetVecScores();

		for (int i = 0; i < team_scores.size(); ++i)
		{
			F4PACKET::SReconnectGameScoreInfoBuilder rolecountinfobuilder(builder);

			rolecountinfobuilder.add_team(i);
			rolecountinfobuilder.add_score(team_scores[i]);

			veccounterinfo.push_back(rolecountinfobuilder.Finish());
		}

		auto offsetcountinfo = builder.CreateVector(veccounterinfo);

		CREATE_FBPACKET(builder, system_s2c_reconnectGameScore, message, databuilder);

		databuilder.add_sinfo(offsetcountinfo);

		STORE_FBPACKET(builder, message, databuilder);
		SendPacket(message, UserID);

		string log_message = "SendReconnectUserWithGameInfo GameScore RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) +", ReConnectUserID : " + to_string(UserID);
		ToLog(log_message.c_str());
	}
	

	// 기록 정보 보내기
	{
		std::vector<SCharacterRecord*> vec;
		vec.clear();

		GetCharacterRecordVec(vec);

		CREATE_BUILDER(builder);

		std::vector< flatbuffers::Offset<F4PACKET::SReconnectGameRecordInfo>> reconnect_game_record_info_vector;

		flatbuffers::Offset<F4PACKET::SReconnectGameRecordInfo> reconnect_game_record_info_obj;
		
		for (int i = 0; i < vec.size(); ++i)
		{
			uint32_t character_sn = vec[i]->CharacterSN;

			std::vector< flatbuffers::Offset<F4PACKET::SRecordInfo>> game_record_info_vector;

			flatbuffers::Offset<F4PACKET::SRecordInfo> game_record_info_obj;

			for (int j = 0; j < (int)F4PACKET::RECORD_TYPE::MAX; ++j)
			{
				F4PACKET::RECORD_TYPE type = static_cast<F4PACKET::RECORD_TYPE>(j);
				
				if (type == F4PACKET::RECORD_TYPE::None)
				{
					continue;
				}

				uint32_t cnt = m_CharacterManager->GetCharacterRecordCountWithType(character_sn, type);

				game_record_info_obj = F4PACKET::CreateSRecordInfo(builder, type, cnt);

				game_record_info_vector.push_back(game_record_info_obj);
			}

			auto game_record_info_vector_obj = builder.CreateVector(game_record_info_vector);

			reconnect_game_record_info_obj = F4PACKET::CreateSReconnectGameRecordInfo(builder, character_sn, game_record_info_vector_obj);

			reconnect_game_record_info_vector.push_back(reconnect_game_record_info_obj);
		}

		auto offsetcountinfo = builder.CreateVector(reconnect_game_record_info_vector);

		CREATE_FBPACKET(builder, system_s2c_reconnectGameRecord, message, databuilder);

		databuilder.add_sinfo(offsetcountinfo);

		STORE_FBPACKET(builder, message, databuilder);
		SendPacket(message, UserID);

		string log_message = "SendReconnectUserWithGameInfo CharacterRecord RoomElapsedTime : " + std::to_string(GetRoomElapsedTime()) + ", ReConnectUserID : " + to_string(UserID);
		ToLog(log_message.c_str());
	}

	// 캐릭터의 마지막 액션 정보 보내주기
	{
		F4PACKET::SPlayerAction* pInfo = GetCharacterStateAction(CharacterSN);

		if (pInfo != nullptr)
		{
			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, system_s2c_reconnectCharacterLastActionInfo, message, send_data);
			send_data.add_userid(UserID);
			send_data.add_characterid(CharacterSN);
			send_data.add_number(pInfo->number());
			STORE_FBPACKET(builder, message, send_data)

			SendPacket(message, UserID);

			string log_message = "SendReconnectUserWithGameInfo CharacterLastActionInfo RoomElapsedTime : " + to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(UserID) + ", CharacterSN  : " + to_string(CharacterSN) + ", ActionNum : " + to_string(pInfo->number());
			ToLog(log_message.c_str());
		}
	}

	// 패시브 정보 보내기 
	SendReconnectUserWithPassiveInfo(UserID);
}

// 이미 게임이 진행된 상태라면 공수전환에서 유저를 게임에 진입시킨다.
void CHost::ReconnectUserOperation()
{
	std::list<DHOST_TYPE_USER_ID> vData;
	vData.clear();

	GetBadConnectUser(vData);

	if (vData.empty() == false)
	{
		std::list<DHOST_TYPE_USER_ID>::iterator iter;

		for (iter = vData.begin(); iter != vData.end(); ++iter)
		{
			DHOST_TYPE_USER_ID UserID = *iter;

			CHostUserInfo* pReConnnectUser = FindUser(UserID);

			if (pReConnnectUser != nullptr)
			{
				if (pReConnnectUser->GetConnectState() == ECONNECT_STATE::WAIT)
				{
					ReconnectUserOperationChangeOfControl(UserID);
					ReconnectUserOperationProcess(UserID);
				}
			}
		}
	}
}

// 게임이 진행하기 전이라면
void CHost::ReconnectUserOperationBeforeLoading(DHOST_TYPE_USER_ID UserID)
{
	if (CheckDisconnectUser(UserID))
	{
		// 현재 게임상태가 플레이 전 이라면
		if (m_CurrentState == EHOST_STATE::LOAD || m_CurrentState == EHOST_STATE::INIT)
		{
			ReconnectUserOperationProcess(UserID);
		}
	}
}

void CHost::ReconnectUserOperationProcess(DHOST_TYPE_USER_ID UserID)
{
	CHostUserInfo* pUser = FindUser(UserID);

	if (pUser != nullptr)
	{
		F4PACKET::SPlayerInformationT* pPlayerInfo = FindCharacterInformationWithUserIDAndIsNotAi(pUser->GetUserID());

		if (pPlayerInfo != nullptr)
		{
			// 재접속 유저한테 현재 게임정보를 보내준다.
			SendReconnectUserWithGameInfo(UserID, pPlayerInfo->id);

			// 나갔다가 들어왔으니 Ai 가 아닌 유저가 플레이 했다고 저장한다
			GetCharacterManager()->SetSwitchToAi(pPlayerInfo->id, false);

			GetCharacterManager()->SetCharacterDiconnectComebackTime(pPlayerInfo->id, GetRoomElapsedTime());


			CCharacter* pCharacter = GetCharacterManager()->GetCharacter(pPlayerInfo->id);




			// 재접속 유저의 핑데이터 및 접속상태 변경					
			pUser->ClearPingData();
			pUser->SetConnectState(ECONNECT_STATE::CONNECT);

			RemoveBadConnectUser(UserID);
			RemoveBadConnectUserCheck(UserID);

			// 재접속 유저의 킥 타입값을 초기화 해준다. (네트워크 불량으로 재접속하는 경우만)
			if (GetCharacterKickType(pPlayerInfo->id) == ECHARACTER_KICK_TYPE::LONG_LATENCY)
			{
				SetCharacterKickType(pPlayerInfo->id, ECHARACTER_KICK_TYPE::NONE);
			}
		}
	}
}

void CHost::ReconnectUserOperationChangeOfControl(DHOST_TYPE_USER_ID UserID)
{
	F4PACKET::SPlayerInformationT* pReConnnectPlayerInfo = FindCharacterInformationWithUserIDAndIsNotAi(UserID);

	if (pReConnnectPlayerInfo == nullptr)
	{
		string log_message = "ReconnectUserOperationChangeOfControl pReConnnectPlayerInfo is null RoomElapsedTime : " + to_string(GetRoomElapsedTime()) + ", UserID : " + to_string(UserID);
		ToLog(log_message.c_str());

		return;
	}

	// 기존에 ai 권한을 가진애한테 뺏어야 되는 캐릭터들
	std::vector<DHOST_TYPE_INT32> vecTakeOffCharacterSN;
	vecTakeOffCharacterSN.clear();

	// ai 권한을 줘야하는 캐릭터들 (트리오 모드에만 해당)
	std::vector<DHOST_TYPE_INT32> vecTakeAwayCharacterSN;
	vecTakeAwayCharacterSN.clear();

	std::vector<F4PACKET::SPlayerInformationT*> vecCharacters;
	vecCharacters.clear();

	if (GetModeType() == EMODE_TYPE::TRIO || ( GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED && pReConnnectPlayerInfo->teamcontrolluser ))
	{
		GetCharacterInformationVec(vecCharacters);

		for (int i = 0; i < vecCharacters.size(); ++i)
		{
			if (vecCharacters[i] != nullptr && pReConnnectPlayerInfo->team == vecCharacters[i]->team)
			{
				vecTakeOffCharacterSN.push_back(vecCharacters[i]->id);

				if (vecCharacters[i]->ailevel > kIS_NOT_AI)
				{
					vecTakeAwayCharacterSN.push_back(vecCharacters[i]->id);
				}
			}
		}
	}

	// ai 권한을 가져간애한테서 권한을 빼는작업
	if (pReConnnectPlayerInfo != nullptr)
	{
		// 다시 유저가 됨 
		CCharacter* pCharacter = GetCharacterManager()->GetCharacter(pReConnnectPlayerInfo->id);
		if (pCharacter)
		{
			pCharacter->SetAI(false);
		}

		// 해당 유저의 캐릭터를 누가 컨트롤 하는지 정보 가져오기
		DHOST_TYPE_USER_ID ai_control_user = FindAiControlUser(pReConnnectPlayerInfo->id);

		CHostUserInfo* pTakeOffUser = FindUser(ai_control_user);

		if (pTakeOffUser != nullptr)
		{
			// 재접속 유저의 캐릭터를 컨트롤하고 있던 유저한테 컨트롤 권한을 뺏는다.
			CREATE_BUILDER(builder);

			std::vector< flatbuffers::Offset<F4PACKET::SDisConnectUserInfo>> veccounterinfo;

			if (GetModeType() == EMODE_TYPE::TRIO || (GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED && pReConnnectPlayerInfo->teamcontrolluser))
			{
				for (int i = 0; i < vecTakeOffCharacterSN.size(); ++i)
				{
					F4PACKET::SDisConnectUserInfoBuilder rolecountinfobuilder(builder);
					rolecountinfobuilder.add_userid(UserID);
					rolecountinfobuilder.add_characterid(vecTakeOffCharacterSN[i]);
					veccounterinfo.push_back(rolecountinfobuilder.Finish());

					string log_message = "ChangeOfControl RoomElapsedTime : " + to_string(GetRoomElapsedTime()) + ", TakeOffUserUserID : " + to_string(pTakeOffUser->GetUserID()) + ", AiUserSN  : " + to_string(UserID) + ", AiCharacterSN : " + to_string(vecTakeOffCharacterSN[i]);
					ToLog(log_message.c_str());
				}
			}
			else
			{
				F4PACKET::SDisConnectUserInfoBuilder rolecountinfobuilder(builder);
				rolecountinfobuilder.add_userid(UserID);
				rolecountinfobuilder.add_characterid(pReConnnectPlayerInfo->id);
				veccounterinfo.push_back(rolecountinfobuilder.Finish());

				string log_message = "ChangeOfControl RoomElapsedTime : " + to_string(GetRoomElapsedTime()) + ", TakeOffUserUserID : " + to_string(pTakeOffUser->GetUserID()) + ", AiUserSN  : " + to_string(UserID) + ", AiCharacterSN : " + to_string(pReConnnectPlayerInfo->id);
				ToLog(log_message.c_str());
			}

			auto offsetcountinfo = builder.CreateVector(veccounterinfo);

			CREATE_FBPACKET(builder, system_s2c_changeOfControl, message, databuilder);

			databuilder.add_sinfo(offsetcountinfo);

			STORE_FBPACKET(builder, message, databuilder);
			SendPacket(message, pTakeOffUser->GetUserID());

			if (GetModeType() == EMODE_TYPE::TRIO || (GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED && pReConnnectPlayerInfo->teamcontrolluser))
			{
				for (int i = 0; i < vecTakeOffCharacterSN.size(); ++i)
				{
					pTakeOffUser->RemoveUserWithAiCharacter(vecTakeOffCharacterSN[i]);
				}
			}
			else
			{
				pTakeOffUser->RemoveUserWithAiCharacter(pReConnnectPlayerInfo->id);
			}
			
			{
				F4PACKET::SPlayerInformationT* pAiHostCharacter = FindCharacterInformationWithUserIDAndIsNotAi(pTakeOffUser->GetUserID());
				F4PACKET::SPlayerInformationT* pAiCharacter = FindCharacterInformation(pReConnnectPlayerInfo->id);
				if (pAiHostCharacter != nullptr && pAiCharacter != nullptr)
				{
					string log_message = "CHANGE_OF_CONTROL Character : " + pAiCharacter->name + " <<<<<< Character : " + pAiHostCharacter->name;
					SendDebugMessage(log_message);
				}
			}
		}
	}

	// 트리오 모드인경우 상대한테 권한을 뺏어서 재접속 유저한테 넘겨줘야 한다.
	if (vecTakeAwayCharacterSN.empty() == false)
	{
		CREATE_BUILDER(builder);
		std::vector< flatbuffers::Offset<F4PACKET::SDisConnectUserInfo>> veccounterinfo;

		// 접속 불량인 유저가 컨트롤하고 있던 캐릭터
		for (int i = 0; i < vecTakeAwayCharacterSN.size(); ++i)
		{
			F4PACKET::SPlayerInformationT* pAiCharacter = FindCharacterInformation(vecTakeAwayCharacterSN[i]);

			if (pAiCharacter != nullptr)
			{
				F4PACKET::SDisConnectUserInfoBuilder rolecountinfobuilder(builder);

				rolecountinfobuilder.add_userid(pReConnnectPlayerInfo->userid);
				rolecountinfobuilder.add_characterid(vecTakeAwayCharacterSN[i]);
				veccounterinfo.push_back(rolecountinfobuilder.Finish());

				string log_message = "SYSTEM_S2C_CHANGE_OF_CONTROL_DATA with ai takeAwayUserID : " + to_string(pReConnnectPlayerInfo->userid) + ", AiCharacterSN : " + to_string(vecTakeAwayCharacterSN[i]);
				ToLog(log_message.c_str());

				// 기존에 가지고 있던 AiHostUserID 를 권한을 가져가는 유저한테 준다.
				pAiCharacter->userid = pReConnnectPlayerInfo->userid;

				auto offsetcountinfo = builder.CreateVector(veccounterinfo);

				CREATE_FBPACKET(builder, system_s2c_changeOfControl, message, databuilder);

				databuilder.add_sinfo(offsetcountinfo);

				STORE_FBPACKET(builder, message, databuilder);
				SendPacket(message, pReConnnectPlayerInfo->userid);
			}
		}
	}
}

void CHost::KickToTheLobby(DHOST_TYPE_USER_ID UserID, F4PACKET::EKICK_TYPE type)
{
	CHostUserInfo* pUser = FindUser(UserID);
	if (pUser == nullptr)
	{
		return;
	}

	if (GetOption() & HOST_OPTION_NO_KICK)
	{
		return;
	}

#ifdef _DEBUG
	if (GetDevNoKick())
	{
		return;
	}
#endif // _DEBUG

	switch (type)
	{
		case F4PACKET::EKICK_TYPE::long_latency:
		{
			SetCharacterKickType(UserID, ECHARACTER_KICK_TYPE::LONG_LATENCY);
		}
		break;
		case F4PACKET::EKICK_TYPE::speed_hack:
		{
			SetCharacterKickType(UserID, ECHARACTER_KICK_TYPE::SPEED_HACK);
		}
		break;
		case F4PACKET::EKICK_TYPE::cheat_kick:
		{
			SetCharacterKickType(UserID, ECHARACTER_KICK_TYPE::CHEAT_HACK);
		}
		break;
		default:
		{

		}
		break;
	}

	CREATE_BUILDER(builder);

	CREATE_FBPACKET(builder, system_s2c_kickToTheLobby, message, databuilder);

	databuilder.add_kicktype(type);

	STORE_FBPACKET(builder, message, databuilder);


	if (type == F4PACKET::EKICK_TYPE::speed_hack)
	{
		string str = "[HACK_CHECK] KickToTheLobby RoomElapsedTime : "
			+ std::to_string(m_TimeManager->GetRoomElapsedTime())
			+ ", UserID : " + std::to_string(UserID)
			+ ", CurrentState : " + ConvertHostStateToStr(m_CurrentState)
			+ ", kickType : "
			+ F4PACKET::EnumNameEKICK_TYPE(type);

		ToLog(str.c_str());
	}
	else
	{
		string str = "KickToTheLobby RoomElapsedTime : "
			+ std::to_string(m_TimeManager->GetRoomElapsedTime())
			+ ", UserID : " + std::to_string(UserID)
			+ ", CurrentState : " + ConvertHostStateToStr(m_CurrentState)
			+ ", kickType : "
			+ F4PACKET::EnumNameEKICK_TYPE(type);

		ToLog(str.c_str());
	}


	SendPacket(message, UserID);
}

void CHost::AwayFromKeyboard(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_CHARACTER_SN CharacterSN)
{
	if (GetOption() & HOST_OPTION_PREVENT_AFK)
	{
		return;
	}
	if (GetModeType() == EMODE_TYPE::AI || GetModeType() == EMODE_TYPE::THREE_ON_THREE || GetModeType() == EMODE_TYPE::CONTINUOUS || GetModeType() == EMODE_TYPE::TRIO || GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED)
	{

		CCharacter* pCharacter = GetCharacterManager()->GetCharacter(CharacterSN);
		if (pCharacter) pCharacter->SetAI(true);

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, system_s2c_awayFromKeyboard, message, send_data);
		send_data.add_characterid(CharacterSN);
		send_data.add_gamemode(GetConvertGameMode());
		STORE_FBPACKET(builder, message, send_data)

		SendPacket(message, UserID);

		string str = "[BAD_MANNER] AwayFromKeyboard RoomElapsedTime : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(UserID) + ", CharacterID : " + std::to_string(CharacterSN);
		ToLog(str.c_str());
	}
}

void CHost::PushLoginCompleteUser(DHOST_TYPE_USER_ID UserID)
{
	DHOST_TYPE_BOOL bInsert = true;

	// 이미 패킷을 보내온 유저인지 확인한다.
	for (int i = 0; i < m_LoginCompleteUser.size(); ++i)
	{
		if (m_LoginCompleteUser[i] == UserID)
		{
			bInsert = false;
			break;
		}
	}

	if (bInsert)
	{
		string str = "PushLoginCompleteUser RoomElapsedTime : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(UserID);
		ToLog(str.c_str());

		m_LoginCompleteUser.push_back(UserID);
	}
}

DHOST_TYPE_UINT64 CHost::GetLoginCompleteUserSize()
{
	return m_LoginCompleteUser.size();
}

void CHost::RemoveLoginCompleteUser(DHOST_TYPE_USER_ID UserID)
{
	std::vector<DHOST_TYPE_USER_ID>::iterator iter;
	for (iter = m_LoginCompleteUser.begin(); iter != m_LoginCompleteUser.end(); iter++)
	{
		if (UserID == *iter)
		{
			string str = "RemoveLoginCompleteUser RoomElapsedTime : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(UserID);
			ToLog(str.c_str());

			m_LoginCompleteUser.erase(iter);
			break;
		}
	}
}


DHOST_TYPE_BOOL CHost::CheckLoginCompleteUser(DHOST_TYPE_USER_ID UserID)
{
	for (int i = 0; i < m_LoginCompleteUser.size(); ++i)
	{
		if (m_LoginCompleteUser[i] == UserID)
		{
			return true;
		}
	}

	return false;
}

void CHost::PushSendLoadCompleteUser(DHOST_TYPE_USER_ID UserID)
{
	DHOST_TYPE_BOOL bInsert = true;

	// 이미 패킷을 보내온 유저인지 확인한다.
	for (int i = 0; i < m_SendLoadCompleteUser.size(); ++i)
	{
		if (m_SendLoadCompleteUser[i] == UserID)
		{
			bInsert = false;
			break;
		}
	}

	if (bInsert)
	{
		string str = "PushSendLoadCompleteUser RoomElapsedTime : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(UserID);
		ToLog(str.c_str());

		m_SendLoadCompleteUser.push_back(UserID);
	}
}

DHOST_TYPE_UINT64 CHost::GetSendLoadCompleteUserSize()
{
	return m_SendLoadCompleteUser.size();
}

// 입장신까지 클라가 로드 컴플릿 패킷을 보내지 않은 경우 해당 유저를 ai 처리를 해야한다.
void CHost::CheckSendLoadCompleteUser()
{
	for (auto& it : m_HostUserMap)
	{
		CHostUserInfo* pUser = it.second;

		DHOST_TYPE_BOOL bBadConnect = true;

		if (pUser != nullptr)
		{
			if (pUser->GetUserType() == EUSER_TYPE::NORMAL)
			{
				for (int i = 0; i < m_SendLoadCompleteUser.size(); ++i)
				{
					if (m_SendLoadCompleteUser[i] == pUser->GetUserID())
					{
						bBadConnect = false;
						break;
					}
				}

				if (bBadConnect)
				{
					string str = "CheckSendLoadCompleteUser RoomElapsedTime : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(pUser->GetUserID());
					ToLog(str.c_str());

					pUser->SetConnectState(ECONNECT_STATE::DISCONNECT);

					PushBadConnectUser(pUser->GetUserID());

					KickToTheLobby(pUser->GetUserID());
				}
			}
		}
	}	
}

DHOST_TYPE_BOOL CHost::IsLoadCompleteUser(DHOST_TYPE_USER_ID UserID)
{
	std::vector<DHOST_TYPE_USER_ID>::iterator iter;
	for (iter = m_SendLoadCompleteUser.begin(); iter != m_SendLoadCompleteUser.end(); iter++)
	{
		if (UserID == *iter)
		{
			return true;
		}
	}

	return false;
}

void CHost::ClearSendLoadCompleteUser()
{
	m_SendLoadCompleteUser.clear();
}


void CHost::RemoveLoadCompleteUser(DHOST_TYPE_USER_ID UserID)
{
	std::vector<DHOST_TYPE_USER_ID>::iterator iter;
	for (iter = m_SendLoadCompleteUser.begin(); iter != m_SendLoadCompleteUser.end(); iter++)
	{
		if (UserID == *iter)
		{
			string str = "RemoveLoadCompleteUser RoomElapsedTime : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(UserID);
			ToLog(str.c_str());

			m_SendLoadCompleteUser.erase(iter);
			break;
		}
	}
}

void CHost::PushReConnectUser(DHOST_TYPE_USER_ID UserID)
{
	DHOST_TYPE_BOOL bInsert = true;

	// 이미 패킷을 보내온 유저인지 확인한다.
	for (int i = 0; i < m_ReConnectUser.size(); ++i)
	{
		if (m_ReConnectUser[i] == UserID)
		{
			bInsert = false;
			break;
		}
	}

	if (bInsert)
	{
		string str = "PushReConnectUser RoomElapsedTime : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(UserID);
		ToLog(str.c_str());

		m_ReConnectUser.push_back(UserID);
	}
}

DHOST_TYPE_BOOL CHost::FindReConnectUser(DHOST_TYPE_USER_ID UserID)
{
	DHOST_TYPE_BOOL result = false;

	std::vector<DHOST_TYPE_USER_ID>::iterator iter;

	for (iter = m_ReConnectUser.begin(); iter != m_ReConnectUser.end(); iter++)
	{
		if (UserID == *iter)
		{
			string str = "FindReConnectUser RoomElapsedTime : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(UserID);
			ToLog(str.c_str());

			result = true;

			break;
		}
	}

	return result;
}

void CHost::RemoveReConnectUser(DHOST_TYPE_USER_ID UserID)
{
	std::vector<DHOST_TYPE_USER_ID>::iterator iter;

	for (iter = m_ReConnectUser.begin(); iter != m_ReConnectUser.end(); iter++)
	{
		if (UserID == *iter)
		{
			string str = "RemoveReConnectUser RoomElapsedTime : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(UserID);
			ToLog(str.c_str());

			m_ReConnectUser.erase(iter);
			break;
		}
	}
}

void CHost::SendClientAliveCheck()
{
	for (auto& it : m_HostUserMap)
	{
		CHostUserInfo* pUser = it.second;
		if (nullptr == pUser)
		{
			continue;
		}

		if (pUser->GetUserType() == EUSER_TYPE::OBSERVER)
		{
			continue;
		}

		if (pUser->GetConnectState() == ECONNECT_STATE::DISCONNECT)
		{
			continue;
		}
		if (pUser->GetConnectState() == ECONNECT_STATE::NONE  && GetRoomElapsedTime() - pUser->GetCreateTime() < kTIME_AFTER_USER_CREATION)
		{
			continue;
		}

		/* 이렇게 하면 컨넥트하고 바로 나간 유저들 무한 로딩으로 기다린다. 
		if (!CheckLoginCompleteUser(pUser->GetUserID()) )
		{
			if(!CheckSingleMode())
				continue;
		}
		*/
		

		DHOST_TYPE_INT32 ping_index = pUser->GetCheckPingIndexEx();
		DHOST_TYPE_FLOAT elapsed_time = GetRoomElapsedTime();
		pUser->PushUserGamePingDeque(elapsed_time);


		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, system_s2s_clientAliveCheck, message, send_data);
		send_data.add_idx(ping_index);
		send_data.add_elapsedtime(elapsed_time);
		send_data.add_clienttime(kFLOAT_INIT);
		STORE_FBPACKET(builder, message, send_data)

		SendPacket(message, pUser->GetUserID());

		SendSystemTimeCheck(pUser);
	}
}

// 여기서 랜덤 시드를 보내고 , 받은후 랜덤 시드 교체 
void CHost::SendSystemTimeCheck(CHostUserInfo* pUser)
{
	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_s2s_systemTimeCheck, message, send_data);

	uint32_t sendSeed = Util::GetRandomUINT32();

	//pUser

	pUser->SetSendRandomSeed(sendSeed); // 보낸것과 받은 것을 비교해야 한다. 

	send_data.add_idx(pUser->GetRandomSeedIndex());
	send_data.add_systemtime(sendSeed);
	STORE_FBPACKET(builder, message, send_data)

	SendPacket(message, pUser->GetUserID());
}

void CHost::SendSyncInfo(SYNCINFO_TYPE type, DHOST_TYPE_CHARACTER_SN sn, DHOST_TYPE_CHARACTER_SN targetid)
{
	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerSyncInfo, message, send_data);

	send_data.add_id(sn); // ID 는 랜덤 인덱스 
	send_data.add_type((int)type);
	send_data.add_targetid((int)targetid);
	send_data.add_correct(0); // 랜덤시드 보내는 곳 , 서로 다른 랜덤 시드 

	send_data.add_position(0);
	send_data.add_synctime(0);
	send_data.add_snrnsisj(0);
	send_data.add_synctime(0.1f);

	STORE_FBPACKET(builder, message, send_data)

	BroadcastPacket(message, kUSER_ID_INIT);
}

void CHost::SendSyncInfo(SYNCINFO_TYPE type)
{

	if (!bTreatDelayMode) return; // 리바운드, 픽의 서버 판단 모드가 아니면, 패킷 보내지 않음

	for (auto& it : m_HostUserMap)
	{
		CHostUserInfo* pUser = it.second;
		if (nullptr == pUser)
		{
			continue;
		}

		if (pUser->GetUserType() == EUSER_TYPE::OBSERVER)
		{
			continue;
		}

		if (pUser->GetConnectState() == ECONNECT_STATE::DISCONNECT)
		{
			continue;
		}

		if (pUser->GetConnectState() == ECONNECT_STATE::NONE && GetRoomElapsedTime() - pUser->GetCreateTime() < kTIME_AFTER_USER_CREATION)
		{
			continue;
		}

		if (pUser->GetConnectState() == ECONNECT_STATE::WAIT) // 최초 접속 상태면 CONNECT 이기 때문에 이부분이 안먹히는 것 같다. 
		{
			continue;
		}



		//DHOST_TYPE_INT32 ping_index = pUser->GetCheckPingIndexEx();
		//DHOST_TYPE_FLOAT elapsed_time = GetRoomElapsedTime();
		//pUser->PushUserGamePingDeque(elapsed_time);

		pUser->SetSendActionKey(pUser->GetSendRandomSeed()); // 돌아오는 키가 클라이언트와 동일하게 하기 위해, 이전 것을 보내야 한다.

		CREATE_BUILDER(builder)	
		CREATE_FBPACKET(builder, play_s2c_playerSyncInfo, message, send_data);

		send_data.add_id(pUser->GetRandomSeedIndex()); // ID 는 랜덤 인덱스 
		send_data.add_type((int)type);
		send_data.add_correct(pUser->GetRandomSeed()); // 랜덤시드 보내는 곳 , 서로 다른 랜덤 시드 
		send_data.add_targetid(0);
		send_data.add_position(0);
		send_data.add_synctime(0);
		send_data.add_snrnsisj(0);
		send_data.add_synctime(0.1f);

		STORE_FBPACKET(builder, message, send_data)

		SendPacket(message, pUser->GetUserID());


		/*
		if ( type == SYNCPOSITION_TYPE::REBOUND)
		{

			string str = "@@@@@@@@@@@@@@@ SendRebound : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(pUser->GetUserID());
			ToLog(str.c_str());

		}
		else
		if (type == SYNCPOSITION_TYPE::PICK)
		{

			string str = "@@@@@@@@@@@@@@@ SendPick : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(pUser->GetUserID());
			ToLog(str.c_str());

		}
		else
		if (type == SYNCPOSITION_TYPE::READY)
		{
			string str = "@@@@@@@@@@@@@@@ Ready : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(pUser->GetUserID());
			ToLog(str.c_str());
		}
		*/
		

	}

	
	
}

void CHost::PushHostJoinUser(DHOST_TYPE_USER_ID UserID)
{
	DHOST_TYPE_BOOL bInsert = true;

	for (int i = 0; i < m_HostJoinUser.size(); ++i)
	{
		if (m_HostJoinUser[i] == UserID)
		{
			bInsert = false;
			break;
		}
	}

	if (bInsert)
	{
		string str = "PushHostJoinUser RoomElapsedTime : " + std::to_string(m_TimeManager->GetRoomElapsedTime()) + ", UserID : " + std::to_string(UserID);
		ToLog(str.c_str());

		m_HostJoinUser.push_back(UserID);
	}
}

void CHost::RemoveHostJoinUser(DHOST_TYPE_USER_ID UserID)
{
	std::vector<DHOST_TYPE_USER_ID>::iterator iter;

	for (iter = m_HostJoinUser.begin(); iter != m_HostJoinUser.end(); iter++)
	{
		if (*iter == UserID)
		{
			m_HostJoinUser.erase(iter);

			string log_message = "RemoveHostJoinUserID : " + std::to_string(UserID) + ", HostJoinUserSize : " + std::to_string(m_HostJoinUser.size());
			ToLog(log_message.c_str());

			break;
		}
	}
}

int CHost::GetHostJoinUserCount()
{
	return m_HostJoinUser.size();
}

DHOST_TYPE_UINT32 CHost::GetRandomSeed(DHOST_TYPE_USER_ID UserID)
{
	DHOST_TYPE_UINT32 result = kUINT32_INIT;

	CHostUserInfo* pUser = FindUser(UserID);

	if(pUser != nullptr)
	{
		result = pUser->GetRandomSeed();
	}

	return result;
}

DHOST_TYPE_BOOL CHost::CheckBadConnectUser(DHOST_TYPE_USER_ID UserID)
{
	F4PACKET::SPlayerInformationT* pCharacter = FindCharacterInformationWithUserIDAndIsNotAi(UserID);

	if (pCharacter != nullptr)
	{
		HostBadConnectUserListType::iterator iter;

		for (iter = m_ListBadConnectUser.begin(); iter != m_ListBadConnectUser.end(); iter++)
		{
			if (*iter == UserID)
			{
				return true;
			}
		}
	}

	return false;
}

DHOST_TYPE_UINT64 CHost::GetBadConnectUserSize()
{
	return m_ListBadConnectUser.size();
}

void CHost::GetBadConnectUser(std::list<DHOST_TYPE_USER_ID>& vData)
{
	HostBadConnectUserListType::iterator iter;

	for (iter = m_ListBadConnectUser.begin(); iter != m_ListBadConnectUser.end(); iter++)
	{
		vData.push_back(*iter);
	}
}

DHOST_TYPE_STR CHost::ConvertToStringUserType(EUSER_TYPE value)
{
	DHOST_TYPE_STR str = "NORMAL";

	switch (value)
	{
		case EUSER_TYPE::OBSERVER:
		{
			str = "OBSERVER";
		}
		break;
	}

	return str;
}

//! GameManager Area
DHOST_TYPE_STAGE_ID CHost::GetStageID()
{
	return m_GameManager->GetStageID();
}

F4PACKET::EGAME_MODE CHost::GetConvertGameMode()
{
	return m_GameManager->GetConvertGameMode();
}

void CHost::SetStageID(DHOST_TYPE_STAGE_ID value)
{
	m_GameManager->SetStageID(value);
}

EEXIT_TYPE CHost::GetExitType()
{
	return m_GameManager->GetExitType();
}

F4PACKET::EXIT_TYPE CHost::GetConvertExitType()
{
	return m_GameManager->GetConvertExitType();
}

void CHost::SetExitType(EEXIT_TYPE value)
{
	m_GameManager->SetExitType(value);
}

F4PACKET::BB_STATE CHost::GetBackBoardState()
{
	return m_GameManager->GetBackBoardState();
}

void CHost::SetBackBoardState(F4PACKET::BB_STATE value)
{
	m_GameManager->SetBackBoardState(value);
}

void CHost::CheckBackBoardState()
{
	m_GameManager->CheckBackBoardState(GetBalanceValue("VBackBoardDamage"));
}

EMODE_TYPE CHost::GetModeType()
{
	return m_GameManager->GetModeType();
}

void CHost::SetModeType(EMODE_TYPE value)
{
	m_GameManager->SetModeType(value);
}

DHOST_TYPE_BOOL CHost::GetIsNormalGame()
{
	return m_GameManager->GetIsNormalGame();
}

void CHost::SetIsNormalGame(DHOST_TYPE_BOOL value)
{
	m_GameManager->SetIsNormalGame(value);
}

DHOST_TYPE_BOOL CHost::GetNoTieMode()
{
	return m_GameManager->GetNoTieMode();
}

void CHost::SetNoTieMode(DHOST_TYPE_BOOL value)
{
	return m_GameManager->SetNoTieMode(value);
}

DHOST_TYPE_BOOL CHost::GetDebug()
{
	return m_GameManager->GetDebug();
}

void CHost::SetDebug(DHOST_TYPE_BOOL value)
{
	m_GameManager->SetDebug(value);
}

DHOST_TYPE_BOOL CHost::CheckSingleMode()
{
	return m_GameManager->CheckSingleMode();
}

DHOST_TYPE_BOOL CHost::CheckSkillChallengeMode()
{
	return m_GameManager->CheckSkillChallengeMode();
}

DHOST_TYPE_INT32 CHost::GetScore(DHOST_TYPE_INT32 teamIndex)
{
	return m_GameManager->GetScore(teamIndex);
}

void CHost::SetScore(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_INT32 score)
{
	m_GameManager->SetScore(teamIndex, score);
}

DHOST_TYPE_FLOAT CHost::GetScoredTime()
{
	return m_GameManager->GetScoredTime();
}

void CHost::SetScoredTime(DHOST_TYPE_FLOAT value)
{
	m_GameManager->SetScoredTime(value);
}

DHOST_TYPE_FLOAT CHost::GetScoreRescue(DHOST_TYPE_INT32 teamIndex, F4PACKET::SBallShotT* pBallShot)
{
	DHOST_TYPE_FLOAT rescue = kFLOAT_INIT;

	// 팀간의 점수차이
	int scoreDelta = kINT32_INIT;

	if (teamIndex == 0)
	{
		scoreDelta = GetScore(1) - GetScore(0);
	}
	else
	{
		scoreDelta = GetScore(0) - GetScore(1);
	}

	// teamIndex의 팀이 지고있는지 확인
	if (scoreDelta > kINT32_INIT)
	{
		if (scoreDelta >= GetGameManager()->GetRescue2ScoreExtend())
		{
			if (pBallShot->shottype == SHOT_TYPE::shotType_threePoint)
			{
				if (pBallShot->wideopen == true)
					rescue = GetBalanceTable()->GetValue("VShoot_3pointProbability_Open_Rescue2");
				else
					rescue = GetBalanceTable()->GetValue("VShoot_3pointProbability_Rescue2");
			}
			else
				rescue = GetBalanceTable()->GetValue("VShootProbability_Rescue2");
		}
		else if (scoreDelta >= GetGameManager()->GetRescue1ScoreExtend())
		{
			if (pBallShot->shottype == SHOT_TYPE::shotType_threePoint)
			{
				if (pBallShot->wideopen == true)
					rescue = GetBalanceTable()->GetValue("VShoot_3pointProbability_Open_Rescue1");
				else
					rescue = GetBalanceTable()->GetValue("VShoot_3pointProbability_Rescue1");
			}
				
			else
				rescue = GetBalanceTable()->GetValue("VShootProbability_Rescue1");
		}
	}

	return rescue;
}

DHOST_TYPE_FLOAT CHost::GetScoreRescue_CorrectionEarlyInTheGame(DHOST_TYPE_INT32 teamIndex, F4PACKET::SHOT_TYPE shottype)
{
	DHOST_TYPE_FLOAT rescue = kFLOAT_INIT;

	// 팀간의 점수차이
	int scoreDelta = kINT32_INIT;

	if (teamIndex == 0)
		scoreDelta = GetScore(1) - GetScore(0);
	else
		scoreDelta = GetScore(0) - GetScore(1);

	// teamIndex의 팀이 지고있는지 확인
	if (scoreDelta > kINT32_INIT)
	{
		if (shottype == SHOT_TYPE::shotType_threePoint)
		{
			if (scoreDelta >= GetBalanceTable()->GetValue("VEarlyRescue2Score"))
				rescue = GetBalanceTable()->GetValue("VWideOpenBonus_ThreePointShot_EarlyRescue_Benefit2");
			else if (scoreDelta >= GetBalanceTable()->GetValue("VEarlyRescue1Score"))
				rescue = GetBalanceTable()->GetValue("VWideOpenBonus_ThreePointShot_EarlyRescue_Benefit1");
			else
				rescue = kFLOAT_INIT;
		}
		else if (shottype == SHOT_TYPE::shotType_middle)
		{
			if (scoreDelta >= GetBalanceTable()->GetValue("VEarlyRescue2Score"))
				rescue = GetBalanceTable()->GetValue("VWideOpenBonus_MidRangeShot_EarlyRescue_Benefit2");
			else if (scoreDelta >= GetBalanceTable()->GetValue("VEarlyRescue1Score"))
				rescue = GetBalanceTable()->GetValue("VWideOpenBonus_MidRangeShot_EarlyRescue_Benefit1");
			else
				rescue = kFLOAT_INIT;
		}
		else
			rescue = kFLOAT_INIT;
	}

	return rescue;
}

DHOST_TYPE_BOOL CHost::GetEnableRescue_CorrectionEarlyInTheGame(F4PACKET::SBallShotT* pBallShot, CAbility* pInfo)
{
	DHOST_TYPE_BOOL enableRescue = false;

	// 모드 조건
	if (GetModeType() != EMODE_TYPE::THREE_ON_THREE)
		return enableRescue;

	// 게임 초반 시간 조건
	if (GetOverTime() == true)
		return enableRescue;

	if (GetGameTimeInit() - GetGameTime() >= GetBalanceTable()->GetValue("VEarlyRescue_Time"))
		return enableRescue;

	// 와이드오픈 조건
	if (pBallShot->wideopen == false)
		return enableRescue;


	switch (pBallShot->shottype)
	{
	case SHOT_TYPE::shotType_middle:
		if (pInfo->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_midRangeShot) > 40.0f)
			enableRescue = true;
		break;
	case SHOT_TYPE::shotType_threePoint:
		if (pInfo->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_threePointShot) > 40.0f)
			enableRescue = true;
		break;
	default:
	{

	}
	break;
	}

	return enableRescue;
}

// 지고 있는 상황인지 확인
DHOST_TYPE_BOOL CHost::GetScoreLoosing(DHOST_TYPE_INT32 teamIndex)
{
	DHOST_TYPE_BOOL result = false;

	// 팀간의 점수차이
	int scoreDelta = kINT32_INIT;

	if (teamIndex == 0)
	{
		scoreDelta = GetScore(1) - GetScore(0);
	}
	else
	{
		scoreDelta = GetScore(0) - GetScore(1);
	}

	// teamIndex의 팀이 지고있는지 확인
	if (scoreDelta > kINT32_INIT)
	{
		result = true;
	}

	return result;
}

DHOST_TYPE_UINT32 CHost::GetScoreDifference()
{
	DHOST_TYPE_UINT32 result = kUINT32_INIT;

	// 팀간의 점수차이
	int scoreDelta = GetScore(0) - GetScore(1);

	result = abs(scoreDelta);

	return result;
}

DHOST_TYPE_FLOAT CHost::GetJumpBallStartTime()
{
	return m_GameManager->GetJumpBallStartTime();
}

void CHost::SetJumpBallStartTime(DHOST_TYPE_FLOAT value)
{
	m_GameManager->SetJumpBallStartTime(value);
}

DHOST_TYPE_BOOL CHost::GetLoadComplete()
{
	return m_GameManager->GetLoadComplete();
}

void CHost::SetLoadComplete(DHOST_TYPE_BOOL value)
{
	m_GameManager->SetLoadComplete(value);
}

DHOST_TYPE_FLOAT CHost::GetReplayPoint()
{
	return m_GameManager->GetReplayPoint();
}

void CHost::SetReplayPoint(DHOST_TYPE_FLOAT value)
{
	m_GameManager->SetReplayPoint(value);
}

DHOST_TYPE_BOOL CHost::GetReplayAssistCheck()
{
	return m_GameManager->GetReplayAssistCheck();
}

void CHost::SetReplayAssistCheck(DHOST_TYPE_BOOL value)
{
	m_GameManager->SetReplayAssistCheck(value);
}
//! end


//! TimeManager Area
DHOST_TYPE_GAME_TIME_F CHost::GetGameTime()
{
	return m_TimeManager->GetGameTime();
}

void CHost::SetGameTime(DHOST_TYPE_GAME_TIME_F value)
{
	m_TimeManager->SetGameTime(value);
}

DHOST_TYPE_BOOL CHost::CheckGameElapsedTimeOneMinute()
{
	DHOST_TYPE_BOOL bCheck = false;

	if (m_TimeManager->GetGameTimeInit() - GetGameTime() <= kHOST_TIME_MINUTE_1)
	{
		bCheck = true;
	}

	if (GetOverTime())
	{
		bCheck = false;
	}

	return bCheck;
}

DHOST_TYPE_FLOAT CHost::GetGameElapsedTimeSinceGameStart()
{
	return m_TimeManager->GetGameTimeInit() - GetGameTime();
}

DHOST_TYPE_GAME_TIME_F CHost::GetGameTimeInit()
{
	return m_TimeManager->GetGameTimeInit();
}

void CHost::SetGameTimeInit(DHOST_TYPE_GAME_TIME_F value)
{
	string str = "SetGameTimeInit CurrentGameTimeInit : " + std::to_string(GetGameTimeInit()) + ", ChagneGameTimeInit : " + std::to_string(value);
	ToLog(str.c_str());

	m_TimeManager->SetGameTimeInit(value);
}

DHOST_TYPE_GAME_TIME_F CHost::GetRoomElapsedTime()
{
	return m_TimeManager->GetRoomElapsedTime();
}

DHOST_TYPE_BOOL CHost::GetOverTime()
{
	return m_TimeManager->GetOverTime();
}

void CHost::SetOverTime(DHOST_TYPE_BOOL value)
{
	m_TimeManager->SetOverTime(value);

	if (value)
	{
		//! 첫 연장전일 때 정규시간 기록을 저장
		if (GetOverTimeCount() == kINT32_INIT)
		{
			if (GetCharacterManager() != nullptr)
			{
				GetCharacterManager()->SaveRegularTimeCharacterRecord();
			}
		}

		IncreaseOverTimeCount();
	}
}

DHOST_TYPE_GAME_TIME_F CHost::GetShotClock()
{
	return m_TimeManager->GetShotClock();
}

void CHost::SetShotClock(DHOST_TYPE_GAME_TIME_F value)
{
	m_TimeManager->SetShotClock(value);
}

void CHost::ResetShotClock()
{
	m_TimeManager->ResetShotClock();
	ResetShotClockPacketSend();
}

// 샷클락이 리셋될 때 클라에 보내주는 패킷
void CHost::ResetShotClockPacketSend()
{
	//! 일리걸 스크린 활성화 되어있으면 초기화
	if (m_CharacterManager->CheckTeamIllegalScreen())
	{
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerIllegalScreen, message, send_data);
		send_data.add_id(kCHARACTER_ID_INIT);
		STORE_FBPACKET(builder, message, send_data)

		BroadcastPacket(message, kUSER_ID_INIT);

		m_CharacterManager->InitTeamIllegalScreen();
	}
}

DHOST_TYPE_GAME_TIME_F CHost::GetReplayTime()
{
	return m_TimeManager->GetReplayTime();
}

void CHost::SetReplayTime(DHOST_TYPE_GAME_TIME_F value)
{
	m_TimeManager->SetReplayTime(value);
}
//! end

//! DevManager Area
void CHost::DevPlayPacketSave(DHOST_TYPE_BOOL value)
{
	if (m_DevManager != nullptr)
	{
		m_DevManager->DevPlayPacketSave(value);
	}
}

DHOST_TYPE_BOOL CHost::GetPlayPacketSave()
{
	if (m_DevManager != nullptr)
	{
		return m_DevManager->GetPlayPacketSave();
	}
	return false;
}

void CHost::SetPlayPacketSave(DHOST_TYPE_BOOL value)
{
	if (m_DevManager != nullptr)
	{
		m_DevManager->SetPlayPacketSave(value);
	}
}

void CHost::SaveBinary(const char* pData, DHOST_TYPE_UINT32 size, DHOST_TYPE_USER_ID UserID, DHOST_TYPE_GAME_TIME_F time)
{
	if (m_DevManager != nullptr)
	{
		m_DevManager->SaveBinary(pData, size, UserID, time);
	}
}

void CHost::LoadBinary(DHOST_TYPE_USER_ID UserID)
{
	if (m_DevManager != nullptr)
	{
		m_DevManager->LoadBinary(UserID);
	}
}

void CHost::InitPacketCount()
{
	if (m_DevManager != nullptr)
	{
		m_DevManager->InitPacketCount();
	}
}

void CHost::IncreasePacketReceiveCount(DHOST_TYPE_UINT16 packet_id, DHOST_TYPE_UINT64 packet_size)
{
	if (m_DevManager != nullptr)
	{
		m_DevManager->IncreasePacketReceiveCount(packet_id, packet_size);
	}
}

void CHost::PrintPacketCountReport()
{
	if (m_DevManager != nullptr)
	{
		std::map<DHOST_TYPE_UINT16, DHOST_TYPE_INT32> mapInfo;
		mapInfo.clear();

		m_DevManager->GetPacketCountReport(mapInfo);

		int count_sum = 0;
		for (auto element : mapInfo)
		{
			count_sum += element.second;

#ifdef _DEBUG
			string log_message = "PacketID : " + std::to_string(element.first) + " [" + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)element.first) + "], Count : " + std::to_string(element.second);
			ToLog(log_message.c_str());
#endif // DEBUG

//#ifdef TEXT_LOG_FILE
//			LOGGER->Log("Packet Id : %d, Count : %d", element.first, element.second);
//#endif
		}
//#ifdef TEXT_LOG_FILE
//		LOGGER->Log("Total Received packet count : %u", count_sum);
//		LOGGER->Log("Total Received packet size : %u", m_DevManager->GetPacketTotalSize());
//		LOGGER->Log("Total Received PB packet size : %u", m_DevManager->m_MovePB_PacketSize);
//		LOGGER->Log("Total Received FB packet size : %u", m_DevManager->m_MoveFB_PacketSize);
//#endif

#ifdef _DEBUG
		string log_message = "Total Received packet count : " + std::to_string(count_sum);
		ToLog(log_message.c_str());



		log_message = "Total Received packet size : " + std::to_string(m_DevManager->GetPacketTotalSize());
		ToLog(log_message.c_str());

		log_message = "MinimunPacketID : " + std::to_string(m_DevManager->GetPacketMinimumID()) + ", MinimunPacketSize : " + std::to_string(m_DevManager->GetPacketMinimumSize()) + ", MaximunPacketID : " + std::to_string(m_DevManager->GetPacketMaximumID()) + ", MaximunPacketSize : " + std::to_string(m_DevManager->GetPacketMaximumSize());
		ToLog(log_message.c_str());
#endif // DEBUG
	}
}


void CHost::DevConsole()
{
	if (m_DevManager != nullptr)
	{
		m_DevManager->DevConsole();
	}
}

void CHost::DevAiLevelSetting(uint32_t team, uint32_t aiLevel)
{
	if (m_DevManager != nullptr)
	{
		m_DevManager->DevAiLevelSetting(team, aiLevel);
	}
}

void CHost::DevStageSetting(uint32_t stageId)
{
	if (m_GameManager != nullptr)
	{
		m_GameManager->SetStageID(stageId);
	}
}

void CHost::DevSceneSetting(int32_t value)
{
	if (m_GameManager != nullptr)
	{
		m_GameManager->SetSceneIndex(value);
	}
}

DHOST_TYPE_INT32 CHost::GetSceneIndex()
{
	if (m_GameManager != nullptr)
	{
		return m_GameManager->GetSceneIndex();
	}

	return kDEFAULT_SCENE_INDEX;
}

void CHost::SetSceneIndex(DHOST_TYPE_INT32 value)
{
	if (m_GameManager != nullptr)
	{
		return m_GameManager->SetSceneIndex(value);
	}
}

DHOST_TYPE_BOOL CHost::GetDevNoKick()
{
	if (m_DevManager != nullptr)
	{
		return m_DevManager->GetDevNoKick();
	}

	return false;
}

void CHost::SetDevNoKick(DHOST_TYPE_BOOL value)
{
	if (m_DevManager != nullptr)
	{
		m_DevManager->SetDevNoKick(value);
	}
}

DHOST_TYPE_UINT32 CHost::GetHomeAiLevel()
{
	if (m_DevManager != nullptr)
	{
		return m_DevManager->GetHomeAiLevel();
	}
	return kUINT32_INIT;
}

void CHost::SetHomeAiLevel(DHOST_TYPE_UINT32 value)
{
	if (m_DevManager != nullptr)
	{
		m_DevManager->SetHomeAiLevel(value);
	}
}

DHOST_TYPE_UINT32 CHost::GetAwayAiLevel()
{
	if (m_DevManager != nullptr)
	{
		return m_DevManager->GetAwayAiLevel();
	}
	return kUINT32_INIT;
}

void CHost::SetAwayAiLevel(DHOST_TYPE_UINT32 value)
{
	if (m_DevManager != nullptr)
	{
		m_DevManager->SetAwayAiLevel(value);
	}
}

void CHost::SendCharacterBurstGauge()
{
	std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT> send_map;
	send_map.clear();

	m_CharacterManager->GetAllCharacterBurstGauge(send_map);

	if (send_map.empty() == false && m_CurrentState == EHOST_STATE::PLAY)
	{
		CREATE_BUILDER(builder);

		vector<F4PACKET::SDevCharacterBurstGauge> vecSendInfo;

		for (std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT>::iterator iter = send_map.begin(); iter != send_map.end(); ++iter)
		{
			F4PACKET::SDevCharacterBurstGauge pushData(iter->first, iter->second);

			F4PACKET::SPlayerInformationT* pInfo = m_CharacterManager->GetCharacterInformation(iter->first);

//			string str = "[SendCharacterBurstGauge] gameTime : " + std::to_string(GetGameTime()) +", name : " + pInfo->name.c_str() + ", BurstGauge : " + std::to_string(iter->second);
//			ToLog(str.c_str());

			vecSendInfo.push_back(pushData);
		}

		auto offsetInfo = builder.CreateVectorOfStructs(vecSendInfo);

		CREATE_FBPACKET(builder, system_s2c_devBurstGauge, message, databuilder);
		databuilder.add_sinfo(offsetInfo);

		STORE_FBPACKET(builder, message, databuilder);

		BroadcastPacket(message, kUSER_ID_INIT);
	}
}

void CHost::DevUpdateEverySecond()
{
	//SendCharacterBurstGauge();
}

void CHost::DevBasicTrainingSystemLog(DHOST_TYPE_CHARACTER_SN value)
{
	CCharacter* pCharacter = GetCharacterManager()->GetCharacter(value);
	if (pCharacter != nullptr)
	{
		SCharacterRecord* pInfo = pCharacter->GetCharacterRecord();
		if (pInfo != nullptr)
		{
			for (std::map<DHOST_TYPE_UINT32, DHOST_TYPE_INT32>::iterator it = pInfo->mapSKillSucInfo.begin(); it != pInfo->mapSKillSucInfo.end(); ++it)
			{
				if (it->second > kINT32_INIT)
				{
					std::string str = "CharacterSN : " + std::to_string(pInfo->CharacterSN) + ", SkillName : " + F4PACKET::EnumNameSKILL_INDEX((SKILL_INDEX)it->first) + ", Count : " + std::to_string(it->second);
					ToLog(str.c_str());
				}
			}
		}
	}
}
//! end

DHOST_TYPE_FLOAT CHost::GetShotPositionDegree(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z)
{
	return m_PhysicsHandler->GetShotPositionDegree(x, z);
}

DHOST_TYPE_FLOAT CHost::GetShotPositionDistanceFromRim(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z)
{
	return m_PhysicsHandler->GetShotPositionDistanceFromRim(x, z);
}

DHOST_TYPE_UINT32 CHost::GetShotPositionZone(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z)
{
	return m_PhysicsHandler->GetShotPositionZone(x, z);
}

void CHost::OnPacketGuardRule(SPacketGuardRule* pGiftInfo)
{
	/*
	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_s2c_versionInfo, message, send_data);
	send_data.add_userid(userid);
	send_data.add_version(HOST_VERSIONINFO);
	STORE_FBPACKET(builder, message, send_data)
	SendPacket(message, data->userid());*/
}


void CHost::OnGiveGift(SGiveGiftInfo* pGiftInfo)
{
	if (pGiftInfo)
	{
		for (std::map<std::string, SAddAbility>::iterator it = pGiftInfo->Attrs.begin(); it != pGiftInfo->Attrs.end(); ++it)
		{
			const std::string& abilityName = it->first;
			const SAddAbility& abilityInfo = it->second;

			std::string str = "abilityName:" + abilityName +
				" addValue:" + std::to_string(abilityInfo.AddValue) +
				" timesType:" + std::to_string(abilityInfo.TimesType) +
				" times:" + std::to_string(abilityInfo.Times);
			ToLog(str.c_str());
		}


		map<DHOST_TYPE_CHARACTER_SN, CCharacter*> characters = GetCharacterManager()->GetCharacters();
		for (auto it = characters.begin(); it != characters.end(); ++it)
		{
			CCharacter* character = it->second;
			if (!character) continue;

			SPlayerInformationT* characterInformation = character->GetCharacterInformation();

			if( characterInformation->userid == pGiftInfo->UserID && characterInformation->ailevel == kIS_NOT_AI ) //&& !character->IsAICharacter())
			{
				character->OnCharacterGiftGive(pGiftInfo);

				std::string str2 = "Give Character_UserID:" + std::to_string(characterInformation->userid) +
					"ID" + std::to_string(characterInformation->id);
				ToLog(str2.c_str());

				//break; // // 테스트 한명만 
			}
		}

		std::string str1 = "userId:" + std::to_string(pGiftInfo->UserID) +
			" fromId" + std::to_string(pGiftInfo->GiftFrom.Id) +
			" fromUserId" + std::to_string(pGiftInfo->GiftFrom.UserID) +
			" fromUserName" + pGiftInfo->GiftFrom.UserName;
		ToLog(str1.c_str());
	}
}



string CHost::TestPacket()
{
	int TestSize = 100000;
	int count = 100;
	clock_t start;
	//clock_t end;
	string strTimeProtobuf, strTimeFlatbuf;

	char* buffer = (char*)malloc(10000);
	int buff_size = 0;

	float scoreid = 0;
	float point = 0;
	DHOST_TYPE_INT32 team = 0;
	float scorehome = 0;
	float scoreaway = 0;
	SHOT_TYPE shot_type = SHOT_TYPE::shotType_closePostUnstready;
	float remaintime = 0;

	//typedef F4PACKET::protobuf_test_data PROTOBUFTEST;
	//typedef F4PACKET::flatbuf_test_data FLATBUFTEST;
	//typedef F4PACKET::flatbuf_test_dataT FLATBUFTESTT;


	vector<float> tt;//int da = sizeof(F4PACKET::SPlayerActionT);
	flatbuffers::FlatBufferBuilder builder(1);
	builder.Clear();
	
	
	
	TB::SVector3 datavec(1,1,1);
	F4PACKET::SPlayerAction data(1,1, 1, 1.f, datavec, 1.f, 5, F4PACKET::SKILL_INDEX::skill_alleyOopLayUpPass, false);
	auto da = sizeof(F4PACKET::play_c2s_playerMove_data);
	auto datacreated = builder.CreateStruct(data);
	builder.Finish(datacreated);
	auto* pnt = builder.GetBufferPointer();
	int daa = builder.GetSize();
	
	//PACKET_ID::SVector3 vec;
	//vec.set_x(1);
	//vec.set_y(1);
	//vec.set_z(1);

	//SPlayerAction datasvec3;
	//datasvec3.set_id(1);
	//datasvec3.set_keys(1);
	//datasvec3.set_number(1);
	//datasvec3.set_directioninput(1);
	//datasvec3.set_allocated_positionlogic(&vec);
	//datasvec3.set_yawlogic(1);
	//datasvec3.set_collisionpriority(COLLISION_PRIORITY::collisionPriority_high);
	//datasvec3.set_skillindex(SKILL_INDEX::skill_alleyOopPass);
	//
	//





	//int daaaa = sizeof(datasvec3);
	//google::uint8* encode_data = reinterpret_cast<google::uint8*>(buffer);
	//
	//int bbysa = datasvec3.ByteSizeLong();
	//datasvec3.SerializeToArray(encode_data,bbysa);
	//

	start = clock();
	//float t = 0;
	//for (size_t i = 0; i < TestSize; i++)
	//{
	//	/*CProtoBufPacket<protobuf_test3_data> data(protobuf_test3);
	//	auto& dataprotobuf = data.GetData();
	//	dataprotobuf.set_arrdatas(1.f);
	//	dataprotobuf.set_idata(1);
	//	dataprotobuf.set_strdata("test");
	//	int size = data.GetEncodeStreamSize();
	//	buff_size += size;
	//	data.Encode(buffer,size);
	//	data.Decode(buffer, size);*/
	//}
	//strTimeProtobuf = to_string(clock() - start);



	//int size = 0;



	//start = clock();
	////flatbuffers::FlatBufferBuilder builder;
	//
	//for (size_t i = 0; i < TestSize; i++)
	//{
	//	flatbuffers::FlatBufferBuilder __builder;
	//	CFlatBufPacket<F4PACKET::flatbuf_test3_data> message(F4PACKET::PACKET_ID::flatbuf_test3);
	//	
	//		auto packetbuilder = F4PACKET::Createflatbuf_test3_dataDirect(__builder);
	//		
	//		__builder.Finish(packetbuilder);
	//		auto datare = __builder.GetBufferPointer();
	//		int sizecur = __builder.GetSize();
	//		size += sizecur;
	//		message.Decode((const char*)datare, sizecur);

	//	//builder.Clear();

	//	//FLATBUFTEST* sampledata = new FLATBUFTEST);// (scoreid, point, point, scorehome, scoreaway, point, remaintime);

	//	//auto data = builder.CreateVector(tta);
	//	//F4PACKET::flatbuf_test_dataBuilder fbdatabuilder(builder);
	//	
	//	//auto* dataresul = dataresult->UnPack();
	//	//delete(sampledata);
	//}
	strTimeFlatbuf = to_string(clock() - start);
	free(buffer);

	string n = "Protobuf duration: ";// +strTimeProtobuf + "ms   Size: " + to_string(buff_size) + "Byte\nFlatBuffer duration: " + strTimeFlatbuf + "ms   Size: " + to_string(size) + "Byte";
	return n;
}