#include "CAnimationController.h"
#include "AnimationInformation_generated.h"

#ifdef _WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

std::string CAnimationController::GetCurrentDirectoryStr()
{
	char buff[FILENAME_MAX]; //create string buffer to hold path
	GetCurrentDir(buff, sizeof(buff));
	std::string currentpath(buff);
	return currentpath;
}

bool CAnimationController::ReadFile(std::string filePath, unsigned char** _data, int* datalen)
{
	std::ifstream is(filePath, std::ifstream::binary);
	if (is) {
		// seekg�� �̿��� ���� ũ�� ����
		is.seekg(0, is.end);
		int length = (int)is.tellg();
		is.seekg(0, is.beg);

		// malloc���� �޸� �Ҵ�
		unsigned char* buffer = (unsigned char*)malloc(length);

		// read data as a block:
		is.read((char*)buffer, length);
		is.close();
		*_data = buffer;
		*datalen = length;
		return true;
	}

	return false;
}
void CAnimationController::ReadBinary()
{
	
	int datalen = 0;
	string str = GetCurrentDirectoryStr();
	std::vector<string> candidates;
	candidates.push_back(str + "/game/cservice/Resource/AnimData.bin");
	candidates.push_back(str + "/Assets/InGame/Res/AnimData.bin");
	candidates.push_back(str + "/../Assets/InGame/Res/AnimData.bin");
	candidates.push_back(str + "/../BasketMan.00.Client/Assets/InGame/Res/AnimData.bin");
	candidates.push_back(str + "/../../BasketMan.00.Client/Assets/InGame/Res/AnimData.bin");
	candidates.push_back(str + "/../../../BasketMan.00.Client/Assets/InGame/Res/AnimData.bin");

	for (size_t i = 0; i < candidates.size(); ++i)
	{
		if (ReadFile(candidates[i], &m_data, &datalen))
		{
			break;
		}
	}
	if (!m_data)
		return;
	auto monster = flatbuffers::GetRoot<TBAnimationInformationList>(m_data);
	if (!monster)
		return;
	auto datas = monster->datas();
	if (!datas)
		return;
	
	for (int i = 0; i < datas->size(); i++)
	{
		auto data = datas->Get(i);
		if (!data)
			continue;

		// ����׸�� �� ���� ��� �����͸� map �� �־ ������ �ִ� �����͸� ã��, �������� �� ���� ������ �ִ� �����͸� ���� map �� �����.
#ifdef _DEBUG
		m_Animinformation->insert(pair<int, const TBAnimationInformation*>(i, data));
#else
		if (CheckInsertAnimationData(data) >= 0)
		{
			m_Animinformation->insert(pair<int, const TBAnimationInformation*>(i, data));
		}
#endif
	}

	m_DataLength = datalen;
}

inline const TBAnimationInformation* CAnimationController::GetAnimationInformation(int _nameanim)
{
	auto iter = m_Animinformation->find(_nameanim);
	if (iter != m_Animinformation->end())
		return iter->second;
	return NULL;
}

CAnimationController::CAnimationController()
{
	m_data = NULL;
	m_DataLength = 0;
	m_Animinformation = new std::map<int, const TBAnimationInformation*>();
}

CAnimationController::~CAnimationController()
{
	if (m_Animinformation != NULL)
	{
		m_Animinformation->clear();

		delete(m_Animinformation);
	}

	if (m_data)
		free(m_data);

	m_data = NULL;
}

float CAnimationController::GetAnimationDuration(int _nameanim)
{
	const TBAnimationInformation* pData = GetAnimationInformation(_nameanim);
	if (pData != nullptr)
	{
		auto length = pData->length();
		return length;
	}
	return 0;
}

bool CAnimationController::IsEventExist(int _nameanim, string namekey)
{
	const TBAnimationInformation* pData = GetAnimationInformation(_nameanim);
	if (pData != nullptr)
	{
		auto animeventlist = pData->eventlist();
		for (size_t i = 0; i < animeventlist->size(); i++)
		{
			return animeventlist->Get(i)->key()->str().compare(namekey) == 0;
		}
	}

	return false;
}

float CAnimationController::GetEventTime(int nameAnim, string nameKey)
{
	const TBAnimationInformation* pData = GetAnimationInformation(nameAnim);
	if (pData != nullptr)
	{
		auto animeventlist = pData->eventlist();
		for (size_t i = 0; i < animeventlist->size(); i++)
		{
			if (animeventlist->Get(i)->key()->str() == nameKey)
				return animeventlist->Get(i)->time();
		}
	}
	
	return 0;
}

bool CAnimationController::GetBallLocalPosition(int nameAnim, float time, bool mirror, bool calculateRoot, JOVECTOR3& position)
{
	bool retValue = false;

	auto animationInformation = GetAnimationInformation(nameAnim);


	if (animationInformation != NULL)
	{
		position = CalculateLocalPosition(animationInformation->curvepositionball(), time);
		position += CalculateLocalPosition(animationInformation->curvepositiondummy001(), time);
		if (mirror)
		{
			position.fX *= -1.0f;
		}
		retValue = true;
	}
	else
	{
		position = JOVECTOR3::zero;
	}

	return retValue;
}

JOVECTOR3 CAnimationController::CalculateSlide(int nameAnim, float timeEvent, float timeoffset, bool mirror, const JOVECTOR3& positionStart, const JOVECTOR3& positionTarget, float yaw)
{
	JOVECTOR3 retValue = JOVECTOR3::zero;

	JOVECTOR3 ballPosition;
	if (GetBallLocalPosition(nameAnim, timeEvent, mirror, true, ballPosition) == true)
	{
		JOQUATERNION rotation = JOQUATERNION::Rotate(JOVECTOR3::up, yaw);
		JOVECTOR3 positionDeltaBall = positionTarget - (rotation * ballPosition + positionStart);
		retValue = positionDeltaBall;
		retValue.fY = 0.0f;
		//Debug.DrawLine(positionTarget - positionDeltaBall, positionTarget, Color.red, 3f);

	}

	return retValue;
}

JOVECTOR3 CAnimationController::CalculateSlideWithLocalPosition(const JOVECTOR3& positionStart, const JOVECTOR3& positionTarget, float yaw, JOVECTOR3 localPosition)
{
	JOVECTOR3 retValue = JOVECTOR3::zero;

	JOQUATERNION rotation = JOQUATERNION::Rotate(JOVECTOR3::up, yaw);
	JOVECTOR3 positionDeltaBall = positionTarget - (rotation * localPosition + positionStart);
	retValue = positionDeltaBall;
	retValue.fY = 0.0f;

	return retValue;
}

JOQUATERNION CAnimationController::GetBallLocalRotation(int nameAnim, float time, bool mirror)
{
	{
		JOQUATERNION rotation;

		const TBAnimationInformation* animationInformation = GetAnimationInformation(nameAnim);

		if (animationInformation != NULL)
		{
			rotation = CAnimationController::CalculateLocalRotation(animationInformation->curverotationball(), time);
			rotation.v.fY *= -1.f;
		}
		else
		{
			rotation = JOQUATERNION::identity;
		}

		return rotation;
	}
}

const bool CAnimationController::LoadData(const char* filename)
{
	int datalen = 0;
	const char* c_string = filename;
	
	string *tmp_string = new string(c_string);
	if (ReadFile(*tmp_string, &m_data, &datalen))
	{
		delete(tmp_string);
		
		auto monster = flatbuffers::GetRoot<TB::TBAnimationInformationList>(m_data);
		if (monster == NULL)
			return false;
		auto datas = monster->datas();
		for (size_t i = 0; i < datas->Length(); i++)
		{
			auto data = datas->Get(i);
			m_Animinformation->insert(pair<int, const TB::TBAnimationInformation*>(i, data));
		}
	}
	else
		return false;
	return true;
}

bool CAnimationController::GetRootLocalInformation(int nameAnim, float time, float timeoffset, bool mirror, JOVECTOR3& position, JOQUATERNION& rotation)
{
	const TBAnimationInformation* animationInformation = GetAnimationInformation(nameAnim);

	if (animationInformation != NULL)
	{
		position = CAnimationController::CalculateLocalPosition(animationInformation->curvepositiondummy001(), time);
		rotation = CAnimationController::CalculateLocalRotation(animationInformation->curverotationdummy001(), time);
		if (mirror)
		{
			position.fX *= -1.f;
			rotation.v.fY *= -1.f;
		}
		return true;
	}

	position = JOVECTOR3::zero;
	rotation = JOQUATERNION::identity;

	return false;
}

bool CAnimationController::GetRootLocalPosition(int nameAnim, float time, float timeoffset, bool mirror, JOVECTOR3& position)
{
	const TBAnimationInformation* animationInformation = GetAnimationInformation(nameAnim);

	if (animationInformation != NULL)
	{
		position = CAnimationController::CalculateLocalPosition(animationInformation->curvepositiondummy001(), time);
		
		if (mirror)
		{
			position.fX *= -1.f;
		}
		return true;
	}
	position = JOVECTOR3::zero;
	return false;
}

int CAnimationController::CheckAnimationData(int nameAnim)
{
	int result = 0;

	const TBAnimationInformation* animationInformation = GetAnimationInformation(nameAnim);

	if (animationInformation != NULL)
	{
		const TBVector3Curve* curve_position = animationInformation->curvepositiondummy001();
		
		if (curve_position != NULL)
		{
			const TBAnimationCurve* curve_x = curve_position->x();
			const TBAnimationCurve* curve_y = curve_position->y();
			const TBAnimationCurve* curve_z = curve_position->z();

			if (curve_x != NULL && curve_y != NULL && curve_z != NULL)
			{
				auto x_keys = curve_x->keys();
				auto x_length = x_keys->size();

				auto y_keys = curve_y->keys();
				auto y_length = y_keys->size();

				auto z_keys = curve_z->keys();
				auto z_length = z_keys->size();

				if (x_keys == NULL || y_keys == NULL || z_keys == NULL)
				{
					result = -4;
				}
				else
				{
					if (x_length < 1 || y_length < 1 || z_length < 1)
					{
						result = -5;
					}
				}
			}
			else
			{
				result = -3;
			}
		}
		else
		{
			result = -2;
		}
	}
	else
	{
		result = -1;
	}

	return result;
}

int CAnimationController::CheckInsertAnimationData(const TBAnimationInformation* pInfo)
{
	int result = 0;

	if (pInfo != nullptr)
	{
		const TBVector3Curve* curve_position = pInfo->curvepositiondummy001();

		if (curve_position != NULL)
		{
			const TBAnimationCurve* curve_x = curve_position->x();
			const TBAnimationCurve* curve_y = curve_position->y();
			const TBAnimationCurve* curve_z = curve_position->z();

			if (curve_x != NULL && curve_y != NULL && curve_z != NULL)
			{
				auto x_keys = curve_x->keys();
				auto x_length = x_keys->size();

				auto y_keys = curve_y->keys();
				auto y_length = y_keys->size();

				auto z_keys = curve_z->keys();
				auto z_length = z_keys->size();

				if (x_keys == NULL || y_keys == NULL || z_keys == NULL)
				{
					result = -4;
				}
				else
				{
					if (x_length < 1 || y_length < 1 || z_length < 1)
					{
						result = -5;
					}
				}
			}
			else
			{
				result = -3;
			}
		}
		else
		{
			result = -2;
		}
	}
	else
	{
		result = -1;
	}

	return result;
}

size_t CAnimationController::GetAnimationInfoSize()
{
	return m_Animinformation->size();
}

const char* CAnimationController::ValidationAnimationInfo(int idx)
{
	const TBAnimationInformation* animationInformation = GetAnimationInformation(idx);

	if (animationInformation != nullptr)
	{
		return animationInformation->name()->c_str();
	}

	return nullptr;
}

void CAnimationController::GetIndexName(int idx, string& str)
{
	auto data = m_Animinformation->find(idx);
	if (data != m_Animinformation->end())
	{
		str = data->second->name()->c_str();
	}
}

bool CAnimationController::GetRootLocalPositionEx(int nameAnim, float time, float Pretime, bool mirror, JOVECTOR3& position)
{
	const TBAnimationInformation* animationInformation = GetAnimationInformation(nameAnim);

	if (animationInformation != NULL)
	{
		position = CAnimationController::CalculateLocalPositionEx(animationInformation->curvepositiondummy001(), time, Pretime);

		if (mirror)
		{
			position.fX *= -1.f;
		}

		return true;
	}

	position = JOVECTOR3::zero;

	return false;
}

bool CAnimationController::GetRootLocalPositionZero(int nameAnim, float time, float Pretime, bool mirror, JOVECTOR3& position)
{
	const TBAnimationInformation* animationInformation = GetAnimationInformation(nameAnim);

	if (animationInformation != NULL)
	{
		position = CAnimationController::CalculateLocalPositionZero(animationInformation->curvepositiondummy001(), time, Pretime);

		if (mirror)
		{
			position.fX *= -1.f;
		}

		return true;
	}

	position = JOVECTOR3::zero;

	return false;
}

JOVECTOR3 CAnimationController::CalculateRootLocalPosition(const TBAnimationInformation* animationInformation, float time, float timeoffset, bool mirror)
{
	JOVECTOR3 position = CalculateLocalPosition(animationInformation->curvepositiondummy001(), time);
	if (mirror == true)
	{
		position.fX *= -1.0f;
	}

	return position;
}

JOVECTOR3 CAnimationController::CalculateRotationRootLogic(JOVECTOR3& movedRoot, float yaw)
{
	JOVECTOR3 retValue;

	JOQUATERNION rotation = JOQUATERNION::Rotate(JOVECTOR3::up, yaw);

	retValue = (rotation * movedRoot) * -1.0f;

	return retValue;
}

double CAnimationController::Evaluate(const TBAnimationCurve* curve, float t)
{
	unsigned int i = 0;
	auto keys = curve->keys();
	auto length = keys->size();
	for (i = 0; i < length; i++)
	{
		if (keys->Get(i)->time() >= t)
		{
			break;
		}
	}
	i--;

	int min = 0;
	int max = static_cast<int>(keys->size() - 2);

	i = CLAMP(i, min, max);

	auto x = CAnimationController::EvaluateFrame(t, keys->Get(i), keys->Get(i+1));
	return x;
}

double CAnimationController::EvaluateFrame(float t,const TBKeyframe* keyframe0,const TBKeyframe* keyframe1)
{
	double p1x = static_cast<double>(keyframe0->time());
	double p1y = static_cast<double>(keyframe0->value());
	double tp1 = static_cast<double>(keyframe0->outtangent());
	double p2x = static_cast<double>(keyframe1->time());
	double p2y = static_cast<double>(keyframe1->value());
	double tp2 = static_cast<double>(keyframe1->intangent());
	double a = (p1x * tp1 + p1x * tp2 - p2x * tp1 - p2x * tp2 - 2 * p1y + 2 * p2y) / (p1x * p1x * p1x - p2x * p2x * p2x + 3 * p1x * p2x * p2x - 3 * p1x * p1x * p2x);
	double b = ((-p1x * p1x * tp1 - 2 * p1x * p1x * tp2 + 2 * p2x * p2x * tp1 + p2x * p2x * tp2 - p1x * p2x * tp1 + p1x * p2x * tp2 + 3 * p1x * p1y - 3 * p1x * p2y + 3 * p1y * p2x - 3 * p2x * p2y) / (p1x * p1x * p1x - p2x * p2x * p2x + 3 * p1x * p2x * p2x - 3 * p1x * p1x * p2x));
	double c = ((p1x * p1x * p1x * tp2 - p2x * p2x * p2x * tp1 - p1x * p2x * p2x * tp1 - 2 * p1x * p2x * p2x * tp2 + 2 * p1x * p1x * p2x * tp1 + p1x * p1x * p2x * tp2 - 6 * p1x * p1y * p2x + 6 * p1x * p2x * p2y) / (p1x * p1x * p1x - p2x * p2x * p2x + 3 * p1x * p2x * p2x - 3 * p1x * p1x * p2x));
	double d = ((p1x * p2x * p2x * p2x * tp1 - p1x * p1x * p2x * p2x * tp1 + p1x * p1x * p2x * p2x * tp2 - p1x * p1x * p1x * p2x * tp2 - p1y * p2x * p2x * p2x + p1x * p1x * p1x * p2y + 3 * p1x * p1y * p2x * p2x - 3 * p1x * p1x * p2x * p2y) / (p1x * p1x * p1x - p2x * p2x * p2x + 3 * p1x * p2x * p2x - 3 * p1x * p1x * p2x));

	return a * t * t * t + b * t * t + c * t + d;
}

JOQUATERNION CAnimationController::CalculateLocalRotation(const TBQuaternionCurve* quaternioninfo, float time)
{
	JOQUATERNION rotation = JOQUATERNION(CAnimationController::Evaluate(quaternioninfo->x(), time),
										 CAnimationController::Evaluate(quaternioninfo->y(), time),
										 CAnimationController::Evaluate(quaternioninfo->z(), time),
										 CAnimationController::Evaluate(quaternioninfo->w(), time));
	return rotation;
}

inline JOVECTOR3 CAnimationController::CalculateLocalPosition(const TBVector3Curve* animcurve, float time)
{
	JOVECTOR3 position(CAnimationController::Evaluate(animcurve->x(), time),
		CAnimationController::Evaluate(animcurve->y(), time),
		CAnimationController::Evaluate(animcurve->z(), time));
	
	return position;
}

inline JOVECTOR3 CAnimationController::CalculateLocalPositionEx(const TBVector3Curve* animcurve, float time, float Pretime)
{
	double x = 0.0f;
	double y = 0.0f;
	double z = 0.0f;

	if (Pretime == 0.0f)
	{
		x = CAnimationController::Evaluate(animcurve->x(), time);
		y = CAnimationController::Evaluate(animcurve->y(), time);
		z = CAnimationController::Evaluate(animcurve->z(), time);
	}
	else
	{
		x = CAnimationController::Evaluate(animcurve->x(), time) - CAnimationController::Evaluate(animcurve->x(), Pretime);
		y = CAnimationController::Evaluate(animcurve->y(), time) - CAnimationController::Evaluate(animcurve->y(), Pretime);
		z = CAnimationController::Evaluate(animcurve->z(), time) - CAnimationController::Evaluate(animcurve->z(), Pretime);
	}

	JOVECTOR3 position(x, y, z);

	return position;
}

inline JOVECTOR3 CAnimationController::CalculateLocalPositionZero(const TBVector3Curve* animcurve, float time, float Pretime)
{
	double x = 0.0f;
	double y = 0.0f;
	double z = 0.0f;

	x = CAnimationController::Evaluate(animcurve->x(), time) - CAnimationController::Evaluate(animcurve->x(), Pretime);
	y = CAnimationController::Evaluate(animcurve->y(), time) - CAnimationController::Evaluate(animcurve->y(), Pretime);
	z = CAnimationController::Evaluate(animcurve->z(), time) - CAnimationController::Evaluate(animcurve->z(), Pretime);

	JOVECTOR3 position(x, y, z);

	return position;
}

const TBAnimationInformation* CAnimationController::FindAnimationInfo(int name)
{
	TBAnimationInformation* pData = nullptr;
	pData = (TBAnimationInformation*)m_Animinformation->find(name)->second;

	return pData;
}

const char* CAnimationController::GetAnimationName(int index)
{
	const char* str = nullptr;

	const TBAnimationInformation* animationInformation = GetAnimationInformation(index);
	if (animationInformation != nullptr)
	{
		return animationInformation->name()->c_str();
	}

	return str;
}

int CAnimationController::FindAnimationInfo(string name)
{
	for (std::map<int, const TBAnimationInformation*>::iterator iter = m_Animinformation->begin(); iter != m_Animinformation->end(); ++iter)
	{
		if (iter->second != nullptr && iter->second->name()->c_str() == name)
		{
			return iter->first;
		}
	}

	return -1;
}