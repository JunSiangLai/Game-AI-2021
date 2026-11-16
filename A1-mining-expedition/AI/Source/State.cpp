#include "State.h"
#include "GameObjectV2.h"

State::State(const std::string& stateID)
	: m_stateID(stateID)
{
}

void State::Init(GameObjectV2* go)
{
	m_go = go;
}

State::~State()
{
}

const std::string& State::GetStateID()
{
	return m_stateID;
}

GameObjectV2* State::GetGO()
{
	return m_go;
}
