#include "IsaacGamePub.h"
#include "IsaacRoom.h"

IsaacRoom::IsaacRoom(DWORD roomAdd)
{
    // 加载房间类型 宽高
    dwRoomType = *(DWORD *)(roomAdd + 0x0008);
    dwRoomWidth = *(DWORD *)(roomAdd + 0x000C);
    dwRoomHeight = *(DWORD *)(roomAdd + 0x0010);

    // 加载房间地形
    isaacTerrainList = new IsaacTerrain[dwRoomHeight * dwRoomWidth]();
    for (DWORD i = 0; i < dwRoomHeight * dwRoomWidth; i++)
    {
        isaacTerrainList[i].dwProp = *(DWORD *)(roomAdd + 0x76C + i * 4);
        DWORD dwTerrainAdd = *(DWORD *)(roomAdd + 0x24 + i * 4);
        if (dwTerrainAdd != NULL)
        {
            isaacTerrainList[i].dwType = *(DWORD *)(dwTerrainAdd + 0x003C);
            isaacTerrainList[i].dwStyle = *(DWORD *)(dwTerrainAdd + 0x0008);
        }
    }

    // 加载房间内对象
    dwObjectCount = *(DWORD *)(roomAdd + 0x1264);
    if (dwObjectCount > 0)
    {
        isaacObjectList = new IIsaacObject *[dwObjectCount]();
        for (DWORD i = 0; i < dwObjectCount; i++)
        {
            // 对象实例：[[roomAdd+00018190]+0000125C]+objectNum*4]
            DWORD dwObjectAdd = *(*(DWORD **)(roomAdd + 0x125C) + i);
            DWORD dwObjectType = IsaacObjectBase::getObjectType(dwObjectAdd);
            if (dwObjectType == 0x0001)
            {
                isaacObjectList[i] = new IsaacCharactor(dwObjectAdd);
            }
            else if (dwObjectType == 0x0002)
            {
                isaacObjectList[i] = new IsaacTears(dwObjectAdd);
            }
            else if (dwObjectType == 0x0003)
            {
                isaacObjectList[i] = new IsaacFamiliar(dwObjectAdd);
            }
            else if (dwObjectType == 0x0005)
            {
                isaacObjectList[i] = new IsaacItem(dwObjectAdd);
            }
            else
            {
                isaacObjectList[i] = new IsaacEnemy(dwObjectAdd);
            }
        }
    }
    return;
}

IsaacRoom::~IsaacRoom()
{
    delete[] isaacTerrainList;
    if (dwObjectCount > 0 && isaacObjectList != NULL)
    {
        for (DWORD i = 0; i < dwObjectCount; i++)
        {
            delete isaacObjectList[i];
        }
        delete[] isaacObjectList;
    }
}

/**
 * 获取房间名称（字符串为32位）.
 *
 * \param roomName：存放房间名称字符串
 */
void IsaacRoom::GetRoomName(CHAR *roomName)
{
    ::strcpy_s(roomName, 32, ROOMNAME[dwRoomType]);
    return;
}

void IsaacRoom::show()
{
    printf_s("当前房间\n");
    CHAR roomName[32] = "";
    GetRoomName(roomName);
    printf_s("%s  W：%2d  H：%2d\n", roomName, dwRoomWidth, dwRoomHeight);
    CHAR buffer[MAX_PATH] = {0};
    DWORD dwOffset = 1;
    memset(buffer, ' ', MAX_PATH);
    for (DWORD i = 0; i < dwRoomWidth; i++)
    {
        buffer[dwOffset] = Num2SingleChar(i);
        dwOffset++;
    }
    buffer[dwOffset] = '\n';
    buffer[dwOffset + 1] = '\0';
    printf_s(buffer);

    for (DWORD i = 0; i < dwRoomHeight; i++)
    {
        memset(buffer, ' ', MAX_PATH);
        buffer[0] = Num2SingleChar(i);
        dwOffset = 1;
        for (DWORD j = 0; j < dwRoomWidth; j++)
        {
            if (isaacTerrainList[i * dwRoomWidth + j].dwProp != 0)
            {
                buffer[dwOffset] = '#';
            }
            dwOffset++;
        }
        buffer[dwOffset] = '\n';
        buffer[dwOffset + 1] = '\0';
        printf_s(buffer);
    }
    return;
}

void IsaacRoom::showObjects()
{
    printf_s("房间内对象数量：%d\n", dwObjectCount);
    for (DWORD i = 0; i < dwObjectCount; i++)
    {
        isaacObjectList[i]->show();
    }
    return;
}

DWORD IsaacTerrain::getTerrainType()
{
    DWORD dwTerrainType = IsaacRoomStaticInfo::TERRNONE;
    switch (dwProp)
    {
    case PROPBUTTON:
        // 按钮
        dwTerrainType = IsaacRoomStaticInfo::TERRBUTTON;
        break;
    case PROPSPIKE:
        // 地刺
        dwTerrainType = IsaacRoomStaticInfo::TERRSPIKE;
        break;
    case PROPOBSTACLE:
        // 障碍物
        dwTerrainType = IsaacRoomStaticInfo::TERRDOOR;
        break;
    case PROPGULLY:
        // 沟壑
        dwTerrainType = IsaacRoomStaticInfo::TERRGULLY;
        break;
    default:
        break;
    }
    return dwTerrainType;
}
