#pragma once
#include <Windows.h>
#include <vector>

/** 人物信息 */
struct IsaacCharactorInfo
{
	FLOAT fXPos;		 // 横坐标
	FLOAT fYPos;		 // 纵坐标
	FLOAT fXSpeed;		 // 横坐标移动速度
	FLOAT fYSpeed;		 // 纵坐标移动速度
	DWORD dwHealthLimit; // 红心上限
	DWORD dwHealth;		 // 红心
	DWORD dwHealthEx;	 // 白心/金心/腐心等
	DWORD dwSheild;		 // 护盾
	DWORD dwKey;		 // 钥匙
	DWORD dwBomb;		 // 炸弹
	DWORD dwGold;		 // 金币
	DWORD dwCharId;		 // 角色id
};

/** 房间信息 */
struct IsaacRoomStaticInfo
{
	static const DWORD TERRNONE = 0x0000;
	static const DWORD TERRWALL = 0x0001;	// 墙壁
	static const DWORD TERRDOOR = 0x0002;	// 门
	static const DWORD TERRSPIKE = 0x0003;	// 地刺
	static const DWORD TERRROCK = 0x0004;	// 岩石
	static const DWORD TERRGULLY = 0x0005;	// 沟壑
	static const DWORD TERRFIRE = 0x0006;	// 篝火
	static const DWORD TERRFECES = 0x0007;	// 大便
	static const DWORD TERRBUTTON = 0x0008; // 按钮

	DWORD dwRoomType;							   // 房间类型
	DWORD dwRoomWidth;							   // 房间宽度
	DWORD dwRoomHeight;							   // 房间高度
	std::vector<std::vector<DWORD>> terrainMatrix; // 地形矩阵
};
