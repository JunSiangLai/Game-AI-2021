#pragma once
#include "GameObjectV2.h"
#include <vector>
#include "SceneBase.h"
#include "SceneData.h"
#include "ObjectBaseV2.h"
#include "PostOfficeV2.h"

class SceneMiningExpedition : public SceneBase, public ObjectBaseV2
{
public:
	SceneMiningExpedition();
	~SceneMiningExpedition();

	virtual bool Handle(MessageV2* message);
	virtual void Read();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderLine(Vector3 origin, Vector3 destination);
	void RenderGO(GameObjectV2* go, float zOffset);
	void SetUp();
	
	GameObjectV2* FetchGO(GameObjectV2::GAMEOBJECT_TYPE type);
protected:
	int numPlayers;
	bool setUp;
	bool nextFrame;

	std::vector<GameObjectV2*> m_gruntList;
	std::vector<GameObjectV2*> m_detonatorList;
	std::vector<GameObjectV2*> m_resourceList;
	std::vector<GameObjectV2*> m_turretList;
	std::vector<GameObjectV2*> m_playerList;
	GameObjectV2* m_Molly;
	GameObjectV2* m_DropPod;

	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	float zOffset;
	float m_waveTimer;
	int enemies;
	int crystals;
	bool wave;
	bool end;
	bool win;
	//GameObjectV2* m_ghost;

	PostOfficeV2* m_PostOffice;
	MessageV2* msg;
	SceneData* m_SceneData;

	StateMachineV2* GruntState;
	StateMachineV2* DetonatorState;
	StateMachineV2* EngineerState;
	StateMachineV2* DrillerState;

	// Camera
	float m_cameraZoom;
};