#include "IsaacGame.h"

IsaacGame *IsaacGame::pIsaacGame = NULL;
DWORD IsaacGame::dwCount = 0;

IsaacGame::IsaacGame()
{
    // 获取isaac-ng.exe模块基址
    MODULEENTRY32 me32 = {0};
    me32.dwSize = sizeof(MODULEENTRY32);
    GetModuleInfoByName(NULL, L"isaac-ng.exe", &me32);
    dwModuleBaseAdd = (DWORD)me32.modBaseAddr;

    // 游戏实例：isaac-ng.exe + GAMEOFFSET(7FD65C)
    pGame = (DWORD *)(dwModuleBaseAdd + GAMEOFFSET);

    pLevel = NULL;
    pIsaacRoom = NULL;
    dwStatus = 0;

    // 初始化临界区
    ::InitializeCriticalSection(&cirticalSection);

    // 调试模式
    dwShowMode = SHOWDEFAULT;
}

IsaacGame::~IsaacGame()
{
    // 删除临界区
    ::DeleteCriticalSection(&cirticalSection);
}

void IsaacGame::loadMap()
{
    // 地图对象：[pGame(isaac-ng.exe+7FD65C)]+0001796C
    DWORD dwMapBaseAdd = NULL;
    dwMapBaseAdd = *pGame + 0x0001796C;
    ::memcpy_s(map, MAPWIDTH * MAPHEIGHT * sizeof(dwMapBaseAdd), (DWORD *)dwMapBaseAdd, MAPWIDTH * MAPHEIGHT * sizeof(dwMapBaseAdd));
    ::memset(pRoomEntries, NULL, MAPHEIGHT * MAPWIDTH * sizeof(pRoomEntries[0][0]));
    for (DWORD i = 0; i < MAPHEIGHT; i++)
    {
        for (DWORD j = 0; j < MAPWIDTH; j++)
        {
            if (map[i][j] != 0xFFFFFFFF)
            {
                // 房间对象：[dwMapBaseAdd([pGame(isaac-ng.exe+7FD65C)]+0001796C)+offset*4] * B8 + [pGame(isaac-ng.exe+7FD65C)] + 14
                pRoomEntries[i][j] = (DWORD *)(*((DWORD *)dwMapBaseAdd + i * MAPWIDTH + j) * 0xB8 + *pGame + 0x14);
            }
        }
    }
    return;
}

void IsaacGame::loadRoom()
{
    // 房间对象：[[pGame(isaac-ng.exe+7FD65C)]+00018190]
    DWORD roomAdd = *(DWORD *)(*pGame + 0x00018190);
    if (pIsaacRoom != NULL)
    {
        delete pIsaacRoom;
    }
    pIsaacRoom = new IsaacRoom(roomAdd);
    return;
}

IsaacGame *IsaacGame::begin()
{
    if (pIsaacGame == NULL)
    {
        pIsaacGame = new IsaacGame();
    }
    dwCount++;
    return pIsaacGame;
}

void IsaacGame::end()
{
    dwCount--;
    if (dwCount == 0)
    {
        IsaacGame *pIsaacGameTmp = pIsaacGame;
        pIsaacGame = NULL;
        delete pIsaacGameTmp;
    }
    return;
}

void IsaacGame::refresh()
{
    ::EnterCriticalSection(&cirticalSection);

    if (pGame == NULL)
    {
        ::LeaveCriticalSection(&cirticalSection);
        return;
    }

    // 关卡实例：[isaac-ng.exe+GAMEOFFSET(7FD65C)]+LEVELOFFSET(00018190)
    pLevel = (DWORD *)(*pGame + LEVELOFFSET);

    if (pLevel == NULL)
    {
        ::LeaveCriticalSection(&cirticalSection);
        return;
    }

    // 加载游戏地图
    loadMap();

    // 加载当前房间
    loadRoom();

    // 显示到控制台（调试）
    DEBUG(showInConsole());

    ::LeaveCriticalSection(&cirticalSection);
    return;
}

BOOL IsaacGame::getCharactorInfo(IsaacCharactorInfo *pIsaacCharactorInfo)
{
    ::EnterCriticalSection(&cirticalSection);
    if (pIsaacRoom == NULL || pIsaacRoom->isaacObjectList == NULL || pIsaacRoom->dwObjectCount == 0)
    {
        ::LeaveCriticalSection(&cirticalSection);
        return FALSE;
    }
    pIsaacCharactorInfo->fXPos = ((IsaacCharactor *)pIsaacRoom->isaacObjectList[0])->getXPos();
    pIsaacCharactorInfo->fYPos = ((IsaacCharactor *)pIsaacRoom->isaacObjectList[0])->getYPos();
    pIsaacCharactorInfo->fXSpeed = ((IsaacCharactor *)pIsaacRoom->isaacObjectList[0])->getXSpeed();
    pIsaacCharactorInfo->fYSpeed = ((IsaacCharactor *)pIsaacRoom->isaacObjectList[0])->getYSpeed();
    pIsaacCharactorInfo->dwHealthLimit = ((IsaacCharactor *)pIsaacRoom->isaacObjectList[0])->getHealthLimit();
    pIsaacCharactorInfo->dwHealth = ((IsaacCharactor *)pIsaacRoom->isaacObjectList[0])->getHealth();
    pIsaacCharactorInfo->dwHealthEx = ((IsaacCharactor *)pIsaacRoom->isaacObjectList[0])->getHealthEx();
    pIsaacCharactorInfo->dwSheild = ((IsaacCharactor *)pIsaacRoom->isaacObjectList[0])->getSheild();
    pIsaacCharactorInfo->dwKey = ((IsaacCharactor *)pIsaacRoom->isaacObjectList[0])->getKey();
    pIsaacCharactorInfo->dwBomb = ((IsaacCharactor *)pIsaacRoom->isaacObjectList[0])->getBomb();
    pIsaacCharactorInfo->dwGold = ((IsaacCharactor *)pIsaacRoom->isaacObjectList[0])->getGold();
    pIsaacCharactorInfo->dwCharId = ((IsaacCharactor *)pIsaacRoom->isaacObjectList[0])->getCharId();
    ::LeaveCriticalSection(&cirticalSection);
    return TRUE;
}

BOOL IsaacGame::getRoomStaticInfo(IsaacRoomStaticInfo *pIsaacRoomStaticInfo)
{
    ::EnterCriticalSection(&cirticalSection);
    if (pIsaacRoom == NULL)
    {
        ::LeaveCriticalSection(&cirticalSection);
        return FALSE;
    }
    // 获取房间类型与宽高
    pIsaacRoomStaticInfo->dwRoomType = pIsaacRoom->dwRoomType;
    pIsaacRoomStaticInfo->dwRoomWidth = pIsaacRoom->dwRoomWidth;
    pIsaacRoomStaticInfo->dwRoomHeight = pIsaacRoom->dwRoomHeight;

    // 获取房间地形矩阵
    for (DWORD i = 0; i < pIsaacRoom->dwRoomHeight; i++)
    {
        std::vector<DWORD> terrainRow;
        for (DWORD j = 0; j < pIsaacRoom->dwRoomWidth; j++)
        {
            terrainRow.push_back(pIsaacRoom->isaacTerrainList[i * pIsaacRoom->dwRoomWidth + j].getTerrainType());
        }
        pIsaacRoomStaticInfo->terrainMatrix.push_back(terrainRow);
    }
    ::LeaveCriticalSection(&cirticalSection);
    return TRUE;
}

void IsaacGame::preShowMode()
{
    if (dwShowMode == SHOWNONE)
    {
        return;
    }
    else if (dwShowMode == SHOWMAP)
    {
        dwShowMode = SHOWOBJECTS;
    }
    else
    {
        dwShowMode = dwShowMode >> 1;
    }
}

void IsaacGame::nextShowMode()
{
    if (dwShowMode == SHOWNONE)
    {
        return;
    }
    else if (dwShowMode == SHOWOBJECTS)
    {
        dwShowMode = SHOWMAP;
    }
    else
    {
        dwShowMode = dwShowMode << 1;
    }
    return;
}

void IsaacGame::showInConsole()
{
    if (dwShowMode & SHOWMAP)
    {
        showMap();
    }
    if (dwShowMode & SHOWROOM)
    {
        if (pIsaacRoom == NULL)
        {
            printf_s("未处于游戏状态\n");
        }
        else
        {
            pIsaacRoom->show();
        }
    }
    if (dwShowMode & SHOWOBJECTS)
    {
        if (pIsaacRoom == NULL)
        {
            printf_s("未处于游戏状态\n");
        }
        else
        {
            pIsaacRoom->showObjects();
        }
    }
    return;
}

/**
 * 显示地图（调试）.
 *
 */
void IsaacGame::showMap()
{
    printf_s("游戏地图\n");
    printf_s("  0 1 2 3 4 5 6 7 8 9 A B C\n");
    printf_s(" - - - - - - - - - - - - -\n");
    for (DWORD i = 0; i < MAPHEIGHT; i++)
    {
        CHAR buffer1[MAX_PATH] = {0};
        CHAR buffer2[MAX_PATH] = {0};
        memset(buffer1, ' ', MAX_PATH);
        memset(buffer2, ' ', MAX_PATH);
        buffer1[0] = Num2SingleChar(i);
        buffer1[1] = '|';
        DWORD dwBufOffset = 2;
        for (DWORD j = 0; j < MAPWIDTH; j++)
        {
            if (map[i][j] != 0xFFFFFFFF)
            {
                DWORD *pRoomBaseInfo = (DWORD *)*(pRoomEntries[i][j] + 4);
                if (pRoomBaseInfo == NULL)
                {
                    buffer1[dwBufOffset] = '#';
                }
                else
                {
                    DWORD dwRoomType = *(pRoomBaseInfo + 2);
                    buffer1[dwBufOffset] = Num2SingleChar(dwRoomType);
                }
            }
            if (j == MAPWIDTH - 1 || map[i][j] != map[i][j + 1])
            {
                buffer1[dwBufOffset + 1] = '|';
            }
            if (i == MAPHEIGHT - 1 || map[i][j] != map[i + 1][j])
            {
                buffer2[dwBufOffset] = '-';
            }
            dwBufOffset += 2;
        }
        buffer1[dwBufOffset] = '\n';
        buffer1[dwBufOffset + 1] = '\0';
        buffer2[dwBufOffset] = '\n';
        buffer2[dwBufOffset + 1] = '\0';
        printf_s(buffer1);
        printf_s(buffer2);
    }
    printf_s("|0-星象|1-普通|2-商店|3-未知|4-宝箱|\n");
    printf_s("|5-BOSS|6-精英|7-隐藏|8-超隐|9-赌博|\n");
    printf_s("|A-诅咒|B-BS挑|C-挑战|D-献祭|E-恶魔|\n");
    printf_s("|F-天使|G-夹层|H-未知|I-BSRH|J-未知|\n");
    printf_s("|K-宝库|L-未知|M-未知|N-未知|O-秘出|\n");
    printf_s("|P-未知|R-未知|S-未知|T-未知|U-未知|\n");
    return;
}
