#include "State_Challenge_JumpShotBlock_Ready.h"
#include "BallController.h"

void CState_Challenge_JumpShotBlock_Ready::OnEnter(void* pData)
{
	pChallengeData = (ChallengeData*)pData;
	m_timeRemain = 3.0f;
	m_timeInt = static_cast<int>(m_timeRemain);

	SVector3 positionBall = positionBalls[rand() % 5];

	CREATE_BUILDER(__builder)

	CFlatBufPacket<F4PACKET::system_s2c_challengeStart_data> message(F4PACKET::PACKET_ID::system_s2c_challengeStart);

	std::vector<TB::SVector3> vectordata;
	TB::SVector3 pPosition1;// = send_data.add_positioncharacter();
	pPosition1.mutate_x(2.0f * side);
	pPosition1.mutate_y(0.0f);
	pPosition1.mutate_z(9.0f);
	vectordata.push_back(pPosition1);

	TB::SVector3 pPosition2;// = send_data.add_positioncharacter();
	pPosition2.mutate_x(positionBall.x());
	pPosition2.mutate_y(positionBall.y());
	pPosition2.mutate_z(positionBall.z());
	vectordata.push_back(pPosition2);

	TB::SVector3 pPosition3;// = send_data.add_positioncharacter();
	pPosition3.mutate_x(4.0f * side);
	pPosition3.mutate_y(0.0f);
	pPosition3.mutate_z(7.0f);
	vectordata.push_back(pPosition3);

	auto vectoroffset = __builder.CreateVectorOfStructs(vectordata);

	F4PACKET::system_s2c_challengeStart_dataBuilder databuilder(__builder);
	
	TB::SVector3 positionBalldata(positionBall.x(), positionBall.y(), positionBall.z());
	
	positionBalldata.mutate_y(0.15f);
	positionBalldata.mutate_z(4.0f);

	databuilder.add_ballnumber(m_pHost->BallNumberGet() + 1);
	databuilder.add_ownerid(pChallengeData->m_startPlayerID);
	databuilder.add_time(pChallengeData->m_tryCount);
	databuilder.add_positionball((const TB::SVector3*)&positionBalldata);
	databuilder.add_velocityball((const TB::SVector3*)&positionBalldata);
	databuilder.add_positioncharacter(vectoroffset);

	__builder.Finish(databuilder.Finish());
	auto* pdata = (char*)__builder.GetBufferPointer();
	message.Decode(pdata, __builder.GetSize());
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	side *= -1;
}

void CState_Challenge_JumpShotBlock_Ready::OnUpdate(float timeDelta)
{
	m_timeRemain -= timeDelta;

	int timeInt = (int)m_timeRemain;

	if (timeInt != m_timeInt)
	{
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, system_s2c_count, message, send_data);
		send_data.add_value(m_timeInt);
		STORE_FBPACKET(builder, message, send_data)
		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		m_timeInt = timeInt;
	}

	if (m_timeRemain < 0.0f)
	{
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, challenge_s2c_start, message, send_data);
		send_data.add_userid(0);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		m_pHost->ChangeState(EHOST_STATE::CHALLENGE_PLAY, pChallengeData);
	}
}