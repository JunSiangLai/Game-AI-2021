#pragma once
#include "SingletonTemplate.h"

class SceneData : public Singleton<SceneData>
{
	friend Singleton<SceneData>;

private:
	int m_objectCount;
	int m_fishCount;
	int m_noGrid;
	float m_gridSize;
	float m_gridOffset;

	SceneData();
	~SceneData();
public:
	int GetObjectCount();
	int GetFishCount();
	int GetGridNo();
	float GetGridSize();
	float GetGridOffset();

	void SetObjectCount(int objCount);
	void SetFishCount(int fCount);
	void SetGridNo(int gridNum);
	void SetGridSize(float gridSize);
	void SetGridOffset(float gridOffset);
};