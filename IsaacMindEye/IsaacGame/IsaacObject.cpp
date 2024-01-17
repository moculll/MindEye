#include "IsaacObject.h"

IsaacGameObject::IsaacGameObject(DWORD *pGameObject)
{
	fXPos = ((FLOAT *)pGameObject)[0xA5];
	fYPos = ((FLOAT *)pGameObject)[0xA6];
	fXSpeed = ((FLOAT *)pGameObject)[0xAE];
	fYSpeed = ((FLOAT *)pGameObject)[0xAF];
}

IssacCharactor::IssacCharactor(DWORD *pGameObject) : IsaacGameObject(pGameObject)
{
}
