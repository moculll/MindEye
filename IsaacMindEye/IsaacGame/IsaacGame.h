#pragma once
#include "../Common.h"
#include "IsaacRoom.h"
#include "IsaacObject.h"
#include "IsaacGamePub.h"

class IsaacGame
{
private:
	static const DWORD GAMEOFFSET = 0x007FD65C;
	static const DWORD LEVELOFFSET = 0x00018190;
	static const DWORD MAPWIDTH = 13;
	static const DWORD MAPHEIGHT = 13;

	static IsaacGame *pIsaacGame; // 保存的实例指针
	static DWORD dwCount;		  // 引用计数

	CRITICAL_SECTION cirticalSection; // 用于同步的临界区

	/** 游戏内变量 */
	DWORD dwModuleBaseAdd; // isaac-ng.exe模块基址
	DWORD *pGame;		   // 游戏实例指针
	DWORD *pLevel;
	DWORD dwStatus;
	DWORD map[MAPHEIGHT][MAPWIDTH] = {0};
	DWORD *pRoomEntries[MAPHEIGHT][MAPWIDTH] = {0};

	/** 保存的类变量 */
	IsaacRoom *pIsaacRoom;

	IsaacGame();
	~IsaacGame();

	/** 加载游戏地图 */
	void loadMap();

	/** 加载当前房间 */
	void loadRoom();

	/** 调试用变量·函数 */
	DWORD dwShowMode; // 显示模式

	void showInConsole(); // 在控制台中显示（调试）
	void showMap();		  // 显示地图（调试）

public:
	/**
	 * 开始访问游戏对象.
	 *
	 * \return 游戏对象指针
	 */
	static IsaacGame *begin();

	/** 结束访问游戏对象 */
	void end();

	/** 获取最新的游戏状态（线程安全） */
	void refresh();

	/**
	 * 获取人物信息（线程安全）.
	 *
	 * \param pIsaacCharactorInfo（输出）：用于保存人物信息的地址
	 * \return TRUE：获取人物信息成功
	 */
	BOOL getCharactorInfo(IsaacCharactorInfo *pIsaacCharactorInfo);

	/**
	 * 获取地形信息（线程安全）.
	 *
	 * \param pIsaacTerrianInfo（输出）：用于保存地形信息的地址
	 * \return TRUE：获取地形信息成功
	 */
	BOOL getRoomStaticInfo(IsaacRoomStaticInfo *pIsaacRoomStaticInfo);

	/** 调试用变量·函数 */
	static const DWORD SHOWNONE = 0x0000;	  // 不显示游戏信息
	static const DWORD SHOWMAP = 0x0001;	  // 显示地图
	static const DWORD SHOWROOM = 0x0002;	  // 显示当前房间
	static const DWORD SHOWOBJECTS = 0x0004;  // 显示当前房间
	static const DWORD SHOWDEFAULT = SHOWMAP; // 默认显示信息

	/** 切换下一个显示类型 */
	void preShowMode();

	/** 切换上一个显示类型 */
	void nextShowMode();
};
