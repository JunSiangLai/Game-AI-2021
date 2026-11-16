#ifndef SCENE_HEX_H
#define SCENE_HEX_H

#include <vector>
#include "SceneBase.h"
#include "GameObjectV2.h"
#include "ObjectBaseV2.h"
#include "PostOfficeV2.h"
#include "Maze.h"
#include <queue>

struct Edge
{
	float sX, sY, eX, eY;
	Edge(float SX, float SY, float EX, float EY) { sX = SX; sY = SY; eX = EX; eY = EY; };
};

class SceneHex : public SceneBase, public ObjectBaseV2
{
public:
	SceneHex();
	~SceneHex();

	enum TURN
	{
		PLAYER,
		COMPUTER,
	};

	static int m_noGrid;
	static float m_gridSize;
	static float m_gridOffset;
	static std::vector<Maze::TILE_CONTENT> m_myGrid; //read maze and store here
	static Maze m_maze;
	static bool m_turnJustEnded;

	virtual bool Handle(MessageV2* message);
	virtual void Read();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go);
	void RenderGO(GameObjectV2* go, float zOffset);
	GameObjectV2* FetchGO(GameObjectV2::GAMEOBJECT_TYPE type);

	void DFS(MazePt curr);
	void DFSGenerateEdgeAndVertices(MazePt curr);

	void UpdateVisibility();
	void VISIBLEBFS(MazePt start, int range);
	void VISIBLEDFS(MazePt start, Maze::DIRECTION dir);
	void CALCVISIBILITY(MazePt start, float range);
	void CALCVISIBLEHEX();

protected:
	bool debugRay;

	// Game
	double flare;
	std::vector<GameObject*> m_flareList;
	int playerControlled;

	// Turn
	TURN m_currTurn;
	int m_turnNo;
	bool m_turnOver;

	// Hex
	std::vector<GameObject *> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	int m_objectCount;

	MazePt m_start;
	MazePt m_end;
	MazePt m_target;

	std::vector<Edge> m_myEdges; //Edges
	std::vector<bool> m_render; // for rendering visibility
	std::vector<bool> m_visited; //visited set for DFS/BFS
	std::queue<MazePt> m_queue; //queue for BFS
	std::vector< std::vector<std::pair<float, float>> > m_myMazePt; //read maze and store here
	std::vector<std::pair<int, int>> m_myGridIndex;
	std::vector< std::tuple<float, float, float> > LOStriangles;
	Vector3 origin;
	unsigned m_mazeKey;
	float m_wallLoad;

	void RenderLine(Vector3 origin, Vector3 destination);

	// A1
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

	float zOffset;
	int m_waveTimer;
	int enemies;
	int crystals;
	bool wave;
	bool end;
	bool win;
	//GameObjectV2* m_ghost;

	PostOfficeV2* m_PostOffice;
	MessageV2* msg;

	StateMachineV2* GruntState;
	StateMachineV2* DetonatorState;
	StateMachineV2* EngineerState;
	StateMachineV2* DrillerState;

	// Camera
	float m_cameraZoom;
};
#endif