#ifndef SCENE_MOVEMENT_H
#define SCENE_MOVEMENT_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
#include "ObjectBase.h"
#include <queue>

class SceneMovement : public SceneBase, public ObjectBase
{
public:
	SceneMovement();
	~SceneMovement();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	virtual bool Handle(Message* message);

	void RenderGO(GameObject *go);

	void RenderLine(Vector3 origin, Vector3 destination);

	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE type);
	void Process(Message* message);
protected:

	std::vector<GameObject *> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	GameObject *m_ghost;
	/*int m_objectCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;*/
	float m_hourOfTheDay;
	int m_numGO[GameObject::GO_TOTAL];
	float zOffset;
	std::queue<Message*> m_messageQueue;
};

#endif