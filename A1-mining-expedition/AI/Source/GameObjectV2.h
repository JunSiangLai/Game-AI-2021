#pragma once

#include "../../Common/Source/Vector3.h"
#include "StateMachineV2.h"
#include "ObjectBaseV2.h"
#include "PostOfficeV2.h"
#include "SceneData.h"
#include <set>

struct GameObjectV2 : public ObjectBaseV2
{
	enum GAMEOBJECT_TYPE
	{
		GO_NONE = 0,

		GO_DRILLER,
		GO_ENGINEER,
		GO_GRUNT,
		GO_DETONATOR,
		GO_ORE,
		GO_TURRET,
		GO_MOLLY,
		GO_DROPPOD,
		GO_TOTAL, //must be last
	};

	//States in State classes
	StateMachineV2 *sm;

	enum STATE 
	{
		STATE_NONE,
		// Grunt
		STATE_CHASE,
		STATE_ATTACK,
		STATE_FLEE,
		// Detonator
		STATE_POUND,
		STATE_FIREBALL,
		STATE_SUICIDEBOMB,
		// Player
		STATE_SEARCH,
		STATE_MINE,
		STATE_RUN,
		// DAttack
		STATE_D_ATTACK,
		// DDefend
		STATE_D_DEFEND,
		// EDefend
		STATE_E_DEFEND,
		STATE_DEATH,
	};
	float currHealth;
	float maxHealth;
	float baseSpeed;
	float moveSpeed;
	float atkSpd;
	float atkCD;
	float countDown;

	GAMEOBJECT_TYPE type;
	std::string ID;
	Vector3 pos;
	Vector3 vel;
	Vector3 scale;
	Vector3 target;
	Vector3 step;
	bool active;
	float mass;

	State* m_currState;
	State* m_nextState;
	PostOfficeV2* m_PostOffice;
	SceneData* sceneData;
	static std::set<std::string> IDs;

	// MSG
	bool wave;
	bool nearDetonator;
	float closestObjectDist;
	GameObjectV2* closestObject;
	Vector3 closestObjectPos;

	// Players
	GameObjectV2* currOre;
	bool escaped;
	bool EngineerDead;

	// Turrets
	GameObjectV2* Engineer;


	GameObjectV2(GAMEOBJECT_TYPE typeValue = GO_NONE);
	~GameObjectV2();

	void GenerateID();

	//States in GameObject
	void Update(double dt);
	void SetNextState(const std::string& nextStateID);
	const std::string GetCurrentState();

	virtual bool Handle(MessageV2* Message);
	virtual void Read();

};