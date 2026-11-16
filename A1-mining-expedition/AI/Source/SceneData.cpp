#include "SceneData.h"

SceneData::SceneData()
{

}
SceneData::~SceneData()
{

}

int SceneData::GetObjectCount()
{
	return m_objectCount;
}
void SceneData::SetObjectCount(int objCount)
{
	m_objectCount = objCount;
}
int SceneData::GetGridNo()
{
	return m_noGrid;
}
void SceneData::SetGridNo(int gridNum)
{
	m_noGrid = gridNum;
}
float SceneData::GetGridSize()
{
	return m_gridSize;
}
void SceneData::SetGridSize(float gridSize) 
{
	m_gridSize = gridSize;
}
float SceneData::GetGridOffset()
{
	return m_gridOffset;
}
void SceneData::SetGridOffset(float gridOffset)
{
	m_gridOffset = gridOffset;
}

int SceneData::GetFishCount()
{
	return m_fishCount;
}

void SceneData::SetFishCount(int fCount)
{
	m_fishCount = fCount;
}