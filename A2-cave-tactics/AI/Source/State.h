#ifndef STATE_H
#define STATE_H

#include <string>

class GameObjectV2;

class State
{
	const std::string m_stateID;
	GameObjectV2* m_go;
protected:
	State(const std::string& stateID);
public:
	virtual ~State();
	const std::string& GetStateID();
	void Init(GameObjectV2* &go);
	GameObjectV2* GetGO();


	//To be implemented by concrete states
	virtual void Enter() = 0;
	virtual void Update(double dt) = 0;
	virtual void Exit() = 0;
};

#endif