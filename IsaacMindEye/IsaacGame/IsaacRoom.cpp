#include "IsaacRoom.h"

IsaacRoom::IsaacRoom(DWORD *pRoom)
{
    dwRoomType = pRoom[2];
    dwRoomWidth = pRoom[3];
    dwRoomHeight = pRoom[4];
    IsaacTerrainList = new IsaacTerrain[dwRoomHeight * dwRoomWidth]();
    for (DWORD i = 0; i < dwRoomHeight * dwRoomWidth; i++)
    {
        IsaacTerrainList[i].dwProp = pRoom[0x01DB + i];
        if (pRoom[0x0009 + i] != 0)
        {
            IsaacTerrainList[i].dwType = *(DWORD *)(pRoom[0x0009 + i] + 0x003C);
            IsaacTerrainList[i].dwStyle = *(DWORD *)(pRoom[0x0009 + i] + 0x0008);
            if (IsaacTerrainList[i].dwType != 0x0004)
            {
                printf_s("%8X  %8X  ", IsaacTerrainList[i].dwType, IsaacTerrainList[i].dwStyle);
            }
        }
    }
    printf_s("\n");
    return;
}

IsaacRoom::~IsaacRoom()
{
    delete[] IsaacTerrainList;
}

/**
 * 获取房间名称（字符串至少为12位）.
 *
 * \param roomName：存放房间名称字符串
 */
void IsaacRoom::GetRoomName(CHAR *roomName)
{
    ::strcpy_s(roomName, 12, ROOMNAME[dwRoomType]);
    return;
}
