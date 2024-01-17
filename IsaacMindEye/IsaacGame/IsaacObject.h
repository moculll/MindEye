#pragma once
#include "../Common.h"

class IsaacGameObject
{
protected:
	FLOAT fXPos;   // 横坐标
	FLOAT fYPos;   // 纵坐标
	FLOAT fXSpeed; // 横坐标移动速度
	FLOAT fYSpeed; // 纵坐标移动速度

public:
	IsaacGameObject(DWORD *pGameObject);
};

class IssacCharactor : public IsaacGameObject
{
public:
	IssacCharactor(DWORD *pGameObject);
};
