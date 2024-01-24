#pragma once
#include "../Common.h"
#include "IsaacObject.h"

struct IsaacTerrain
{
	DWORD dwProp;  // 地形特性
	DWORD dwType;  // 地形类型
	DWORD dwStyle; // 地形样式
};

class IsaacRoom
{
private:
	constexpr static const CHAR *ROOMNAME[] = {
		"星象房", "普通房间", "商店", "", "宝箱房",
		"BOSS房", "精英房", "隐藏房", "超级隐藏", "赌博房",
		"诅咒房", "挑战房", "图书室", "献祭房", "",
		"恶魔房", "天使房", "", "", "",
		"宝库", "", "", "BOSS RUSH", "",
		"", "", "秘密出口", "", ""};

public:
	DWORD dwRoomType;
	DWORD dwRoomHeight;
	DWORD dwRoomWidth;
	DWORD dwObjectCount;
	IIsaacObject **isaacObjectList;
	IsaacTerrain *isaacTerrainList;

	IsaacRoom(DWORD roomAdd);
	~IsaacRoom();

	void GetRoomName(CHAR *roomName);

	/** 调试用变量·函数 */
	void show();
	void showObjects();
};
