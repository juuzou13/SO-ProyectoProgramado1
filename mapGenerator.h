#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define true 1
#define false 0

#define RED "\x1b[31m"
#define CYAN "\x1b[36m"
#define GREEN "\x1b[32m"

#define YELLOW "\x1b[33m"
#define MAGENTA "\x1b[35m"
#define BLUE "\x1b[34m"

#define DEFAULT "\x1b[0m"

#define EASY 10
#define MEDIUM 20
#define HARD 30

enum CardinalPoint
{
    North,
    South,
    East,
    West
};
enum DoorState
{
    Closed,
    Open
};
enum RoomType
{
    Normal,
    Treasure,
    Trap,
    Start,
    Goal,
    Wall
};

struct door
{
    enum CardinalPoint cardinal;
    enum DoorState state;
};

struct room
{
    int id;
    int type;
    struct door doors[4];
    int openDoorsLeft;

    int treasure;
    int trap;

    int occupiedByMonster;
    int monsterInRoomID;

    int isHeroInRoom;

};

short N;
struct room **game_map;

int getNeighbour(int currID, int direction)
{
    switch (direction)
    {
    case North:
        return currID - N > 0 ? currID - N : -1;
    case South:
        return currID + N <= N * N ? currID + N : -1;
    case East:
        return currID % N != 0 ? currID + 1 : -1;
    case West:
        return currID % N != 1 ? currID - 1 : -1;
    }
}

void createDoors(struct door doors[4])
{
    for (int i = 0; i < 4; i++)
    {
        doors[i].cardinal = i;
        doors[i].state = Closed;
    }
}

char *getCardinalName(int cardinal)
{
    switch (cardinal)
    {
    case 0:
        return "Norte";
        break;
    case 1:
        return "Sur";
        break;
    case 2:
        return "Este";
        break;
    case 3:
        return "Oeste";
        break;
    }
}

void getRoomDetails(struct room *room)
{
    printf("\n");
    printf("Room id: %d\n", room->id);
    printf("Room north neighbour: %d\n", getNeighbour(room->id, North));
    printf("Room south neighbour: %d\n", getNeighbour(room->id, South));
    printf("Room east neighbour: %d\n", getNeighbour(room->id, East));
    printf("Room west neighbour: %d\n", getNeighbour(room->id, West));

    char tipo[10];

    switch (room->type)
    {
    case Normal:
        strcpy(tipo, "Normal");
        break;
    case Treasure:
        strcpy(tipo, "Treasure");
        break;
    case Trap:
        strcpy(tipo, "Trap");
        break;
    case Start:
        strcpy(tipo, "Start");
        break;
    case Goal:
        strcpy(tipo, "Goal");
        break;
    case Wall:
        strcpy(tipo, "Wall");
        break;
    }

    printf("Room type: %s\n", tipo);
    printf("Room doors: \n");

    for (int k = 0; k < 4; k++)
    {
        printf("  Door %d) Cardinal: %s, State: %d\n", k + 1, getCardinalName(room->doors[k].cardinal), room->doors[k].state);
    }

    printf("Room open doors left: %d\n", room->openDoorsLeft);
    printf("Room isHeroInRoom: %d\n", room->isHeroInRoom);
    printf("Room occupiedByMonster: %d\n", room->occupiedByMonster);
    printf("Room treasure: %d\n", room->treasure);
    printf("Room trap: %d\n", room->trap);
    printf("Room monsterInRoomID: %d\n", room->monsterInRoomID);
    printf("\n");
}

void getMapDetails()
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            printf("Matriz en posicion [%d][%d]\n", i, j);
            getRoomDetails(&game_map[i][j]);
        }
    }
}

void getRoomDetailsByID(int id)
{
    if (id > N * N || id < 1)
    {
        printf("No existe habitacion con dicho id.\n");
    }
    else
    {
        int i = (id - 1) / N;
        int j = (id - 1) % N;
        getRoomDetails(&game_map[i][j]);
    }
}

int setOccupied(int id, int state)
{
    if (id > N * N || id < 1)
    {
        printf("No existe habitacion con dicho id.\n");
        return -1;
    }
    else
    {
        int i = (id - 1) / N;
        int j = (id - 1) % N;
        game_map[i][j].occupiedByMonster = state;
        return 0;
    }
}

int setMonsterID(int roomID, int monsterID)
{
    if (roomID > N * N || roomID < 1)
    {
        printf("No existe habitacion con dicho id.\n");
        return -1;
    }
    else
    {
        int i = (roomID - 1) / N;
        int j = (roomID - 1) % N;
        game_map[i][j].monsterInRoomID = monsterID;
        return 0;
    }
}

int setHeroInRoom(int id, int state)
{
    if (id > N * N || id < 1)
    {
        printf("No existe habitacion con dicho id.\n");
        return -1;
    }
    else
    {
        int i = (id - 1) / N;
        int j = (id - 1) % N;
        game_map[i][j].isHeroInRoom = state;
        return 0;
    }
}

int isHeroInRoom(int roomID){
    if (roomID > N * N || roomID < 1)
    {
        printf("No existe habitacion con dicho id.\n");
        return -1;
    }
    else
    {
        int i = (roomID - 1) / N;
        int j = (roomID - 1) % N;
        return game_map[i][j].isHeroInRoom;
    }
}

int getFreeRooms()
{
    int k = 0;

    int found = 0;

    while (!found)
    {
        int i = rand() % N;
        int j = rand() % N;

        if (game_map[i][j].type != Wall && game_map[i][j].occupiedByMonster == 0 && game_map[i][j].type != Start && game_map[i][j].type != Goal)
        {
            found = 1;
            return game_map[i][j].id;
        }
    }
}

struct room *getRoomPointerByID(int id)
{
    if (id > N * N || id < 1)
    {
        printf("No existe habitacion con dicho id.\n");
    }
    else
    {
        int i = (id - 1) / N;
        int j = (id - 1) % N;
        return &game_map[i][j];
    }
}

void printArray(int *array, int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}

void printArrayFrom(int *array, int index, int size)
{
    for (int i = index; i < size; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}
/*
int isGoal(int id)
{
    int i = (id - 1) / N;
    int j = (id - 1) % N;
    return game_map[i][j].type == Goal;
}
*/

int isInArray(int *array, int size, int element)
{
    for (int i = 0; i < size; i++)
    {
        if (array[i] == element)
        {
            return true;
        }
    }
    return false;
}

isDoorOpen(int id, int cardinal)
{
    int i = (id - 1) / N;
    int j = (id - 1) % N;

    if (game_map[i][j].doors[cardinal].state == Open)
    {
        return true;
    }

    return false;
}

void drawTemporalMap()
{
    printf("\n");
    char openDoors[5] = {' ', ' ', ' ', ' ', '\0'};
    struct room *room;

    char *roomColor;

    for (int i = 0; i < N; i++)
    {

        for (int j = 0; j < N; j++)
        {

            room = &game_map[i][j];

            for (int k = 0; k < 4; k++)
            {
                if (room->type == Wall)
                {
                    for (int k = 0; k < 4; k++)
                    {
                        openDoors[k] = '/';
                    }
                }
                else
                {
                    if (room->doors[k].state == Open)
                    {
                        switch (room->doors[k].cardinal)
                        {
                        case North:
                            openDoors[0] = '^';
                            break;
                        case South:
                            openDoors[1] = 'v';
                            break;
                        case East:
                            openDoors[2] = '>';
                            break;
                        case West:
                            openDoors[3] = '<';
                            break;
                        }
                    }
                    else
                    {
                        openDoors[k] = ' ';
                    }
                }
            }
            char *spaces = "";
            char *spaces2 = "  ";

            int q = room->id;
            if (q < 10)
            {
                spaces2 = "  ";
            }
            else if (q < 100)
            {
                spaces2 = " ";
            }
            else if (q < 1000)
            {
                spaces2 = "";
            }
            if ((room->type == Treasure && room->treasure == 0) || (room->type == Trap && room->trap == 0)) {
                printf("%s" GREEN "|" DEFAULT, spaces);
            }
            else if (room->type == Treasure && room->treasure == 1) {
                printf("%s" YELLOW "|" DEFAULT, spaces);
            }
            else if (room->type == Trap && room->trap == 1) {
                printf("%s" RED "|" DEFAULT, spaces);
            } else {
                printf("%s|", spaces);
            }
            if (room->isHeroInRoom == 1 && room->occupiedByMonster == 1)
            {
                printf(YELLOW "%d" DEFAULT ") %s%s", room->id, openDoors, spaces2);
            }
            else if (room->isHeroInRoom == 1)
            {
                printf(MAGENTA "%d" DEFAULT ") %s%s", room->id, openDoors, spaces2);
            }
            else if (room->type == Start)
            {
                roomColor = GREEN;
                printf(GREEN "%d" DEFAULT ") %s%s", room->id, openDoors, spaces2);
            }
            else if (room->type == Goal)
            {
                roomColor = RED;
                printf(RED "%d" DEFAULT ") %s%s", room->id, openDoors, spaces2);
            }
            else if (room->occupiedByMonster == 1)
            {
                printf(CYAN "%d" DEFAULT ") %s%s", room->id, openDoors, spaces2);
            }
            else
            {
                printf(DEFAULT "%d" DEFAULT ") %s%s", room->id, openDoors, spaces2);
            }


            for (int k = 0; k < 4; k++)
            {
                openDoors[k] = ' ';
            }

            if (room->id % N == 0)
            {
                printf("%s|", spaces);
            }
        }
        printf("\n");
    }
    printf("\n");
}

int getMaxNeighbours(int id)
{

    if (id == 1 || id == N || id == N * N || id == N * N - N + 1)
    {
        return 2;
    }

    return 3;
}

void connectRooms(int *roomsArray, int *directionsArray, int size)
{
    int currentRoomId = 0;
    int nextRoom = 0;
    struct room *currentRoomPtr;

    int oppositeDoor = -1;

    int oppositeTable[4] = {1, 0, 3, 2};

    for (int i = 0; i < size - 1; i++)
    {
        currentRoomId = roomsArray[i];
        struct room *currentRoomPt = getRoomPointerByID(currentRoomId);
        currentRoomPt->type = getRandomRoomType();

        if (currentRoomPt->type == Treasure){
                currentRoomPt->treasure = 1;
        }
        else if (currentRoomPt->type == Trap){
            currentRoomPt->trap = 1;
        }

        openDoor(currentRoomId, directionsArray[i]);
        if (oppositeDoor != -1)
        {
            openDoor(currentRoomId, oppositeDoor);
        }
        oppositeDoor = oppositeTable[directionsArray[i]];
    }

    currentRoomPtr = getRoomPointerByID(roomsArray[size - 1]);
    currentRoomPtr->doors[oppositeDoor].state = Open;
    currentRoomPtr->openDoorsLeft = currentRoomPtr->openDoorsLeft - 1;
}

void getUnvisitedeighbors(int *roomsList, int size, int *unvisitedO, int *unvisitedSizeO, int *unvisitedDirectionsO, int *unvisitedDirectionsSizeO, int goalRoomID)
{
    int neighbor;
    int unvisited[N * N];
    int unvisitedSize = 0;
    int unvisitedDirections[N * N];
    int unvisitedDirectionsSize = 0;
    int oppositeTable[4] = {1, 0, 3, 2};

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            neighbor = getNeighbour(roomsList[i], j);
            if (roomsList[i] != goalRoomID && neighbor != -1 && !isInArray(roomsList, size, neighbor) && !isInArray(unvisited, unvisitedSize, neighbor))
            {
                unvisited[unvisitedSize++] = neighbor;
                unvisitedDirections[unvisitedDirectionsSize++] = oppositeTable[j];
            }
        }
    }

    *unvisitedSizeO = unvisitedSize;
    *unvisitedDirectionsSizeO = unvisitedDirectionsSize;
    memcpy(unvisitedO, unvisited, unvisitedSize * sizeof(int));
    memcpy(unvisitedDirectionsO, unvisitedDirections, unvisitedDirectionsSize * sizeof(int));
}

int openDoor(int roomID, int doorDirection)
{
    struct room *room = getRoomPointerByID(roomID);

    if (room->openDoorsLeft == 0)
    {
        printf("Cant open any more doors in %d\n", roomID);
        return false;
    }

    room->doors[doorDirection].state = Open;
    room->openDoorsLeft = room->openDoorsLeft - 1;

    return true;
}

/*
int closeDoor(int roomID, int doorDirection)
{
    struct room *room = getRoomPointerByID(roomID);

    if (room->openDoorsLeft == 3)
    {
        printf("Cant close anymore doors in %d\n", roomID);
        return false;
    }

    room->doors[doorDirection].state = Closed;

    return true;
}
*/

/*
void getClosedDoorsFromRoom(int ID)
{
    struct room *currRoom = getRoomPointerByID(ID);

    for (int i = 0; i < 4; i++)
    {
        if (currRoom->doors[i].state == Closed)
        {
            printf("Door %d is closed in %d\n", i, ID);
        }
    }
}
*/

int getRandomRoomType()
{
    int probability = rand() % 10;

    if (probability < 3)
    {
        return Normal;
    }
    else if (probability > 6)
    {
        return Treasure;
    }
    else
    {
        return Trap;
    }
}

void createMap(int startRoomID, int total)
{
    int idList[total];
    int id;

    for (int i = 0; i < N; i++)
    {
        game_map[i] = (int *)malloc(N * sizeof(int));
        for (int j = 0; j < N; j++)
        {
            struct room *room = (struct room *)malloc(sizeof(struct room));

            id = (j + 1) + N * i;

            room->id = (j + 1) + N * i;
            idList[id - 1] = (j + 1) + N * i;

            int probability = rand() % 10;

            if (probability < 3)
            {
                room->type = Normal;
            }
            else if (probability > 6)
            {
                room->type = Treasure;
                room->treasure = 1;
            }
            else
            {
                room->type = Trap;
                room->trap = 1;
            }

            room->type = room->id == startRoomID ? Start : Wall;
            if(room->type == Wall) {
                room->trap = 0;
                room->treasure = 0;
            }
            createDoors(room->doors);
            room->openDoorsLeft = getMaxNeighbours(room->id);
            room->isHeroInRoom = false;
            room->occupiedByMonster = room->type == Start ? true : false;
            room->monsterInRoomID = -1;

            game_map[i][j] = *room;
        }
    }
}

int canThisDoorBeOpened(int currentRoomID, int doorDirection)
{
    int oppositeTable[4] = {1, 0, 3, 2};

    int neighbor = getNeighbour(currentRoomID, doorDirection);
    struct room *currentRoom = getRoomPointerByID(currentRoomID);

    if (neighbor == -1 || currentRoom->openDoorsLeft == 0)
    {
        return false;
    }

    struct room *neighborRoom = getRoomPointerByID(neighbor);

    if (neighborRoom->openDoorsLeft == 0)
    {
        return false;
    }

    return true;
}

int getRoomType(int id)
{
    struct room *room = getRoomPointerByID(id);
    return room->type;
}

int setRoomChestState(int id, int state)
{
    struct room *room = getRoomPointerByID(id);
    room->treasure = state;
}

int getRoomChestState(int id)
{
    struct room *room = getRoomPointerByID(id);
    return room->treasure;
}

int generateMap()
{

    if (N < 2)
    {
        printf("N debe ser mayor o igual a 2.\n");
        return -1;
    }

    int total = N * N;

    game_map = (int **)malloc(N * sizeof(struct room *));

    int startRoomID = N % 2 == 0 ? total / 2 - N / 2 : total / 2 + 1;
    int goalRoomID;

    createMap(startRoomID, total);

    drawTemporalMap();

    int blockedIds[total];
    int blockedIdsSize = 0;

    int touredIds[total];
    int touredIdsSize = 0;

    int connections[total];
    int connectionsSize = 0;

    int possibleConnections[total];
    int possibleConnectionsSize = 0;

    int possibleDestinations[4];
    int possibleDestinationsSize = 0;

    touredIds[touredIdsSize++] = startRoomID;

    int currentNeighbour;
    int currentRoomId;
    int chosenNextRoom;

    int roomCount = N - 1;
    int deadEnds = rand() % (4);

    roomCount = roomCount - deadEnds;

    while (roomCount > 0)
    {
        currentRoomId = touredIds[touredIdsSize - 1];

        for (int i = 0; i < 4; i++)
        {
            currentNeighbour = getNeighbour(currentRoomId, i);

            if (!isInArray(touredIds, touredIdsSize, currentNeighbour) && !isInArray(blockedIds, blockedIdsSize, currentNeighbour))
            {
                if (currentNeighbour != -1)
                {
                    if (currentRoomId == startRoomID)
                    {
                        if (currentNeighbour != goalRoomID)
                        {
                            possibleDestinations[possibleDestinationsSize++] = currentNeighbour;
                            possibleConnections[possibleConnectionsSize++] = i;
                        }
                    }
                    else
                    {
                        possibleDestinations[possibleDestinationsSize++] = currentNeighbour;
                        possibleConnections[possibleConnectionsSize++] = i;
                    }
                }
            }
        }

        if (possibleDestinationsSize == 0)
        {
            blockedIds[blockedIdsSize++] = currentRoomId;
            touredIdsSize--;
            connectionsSize--;
            possibleConnectionsSize = 0;
            possibleDestinationsSize = 0;
        }
        else
        {
            int random = rand() % possibleDestinationsSize;
            chosenNextRoom = possibleDestinations[random];
            touredIds[touredIdsSize++] = chosenNextRoom;
            connections[connectionsSize++] = possibleConnections[random];
            possibleDestinationsSize = 0;
            possibleConnectionsSize = 0;
        }

        struct room *room = getRoomPointerByID(currentRoomId);

        roomCount--;
    }

    struct room *goalRoom = getRoomPointerByID(touredIds[touredIdsSize - 1]);
    goalRoom->type = Goal;
    goalRoomID = goalRoom->id;

    int camino[total];
    int caminoSize = 0;

    printf("Abriendo puertas del laberinto...\n\n");

    memcpy(camino, touredIds, sizeof(touredIds));
    caminoSize = touredIdsSize;

    drawTemporalMap();

    connectRooms(touredIds, connections, touredIdsSize);

    drawTemporalMap();

    int possibleDoors[4];
    int possibleDoorsSize = 0;

    for (int i = 0; i < touredIdsSize; i++)
    {
        struct room *currRoom = getRoomPointerByID(touredIds[i]);
        struct room *neighbour;
        int neighbourID;
        int oppositeTable[4] = {1, 0, 3, 2};

        for (int j = 0; j < 4; j++)
        {
            neighbourID = getNeighbour(currRoom->id, j);
            if (neighbourID != -1)
            {
                neighbour = getRoomPointerByID(neighbourID);
                if (currRoom->doors[j].state == Closed && neighbour->type != Wall && neighbour->openDoorsLeft > 0)
                {
                    if (currRoom->type == Start)
                    {
                        if (neighbour->type != Goal)
                        {
                            possibleDoors[possibleDoorsSize++] = j;
                        }
                    }
                    else if (currRoom->type == Goal)
                    {
                        if (neighbour->type != Start)
                        {
                            possibleDoors[possibleDoorsSize++] = j;
                        }
                    }
                    else
                    {
                        possibleDoors[possibleDoorsSize++] = j;
                    }
                }
            }
        }

        if (rand() % 10 < 8)
        {
            possibleDoorsSize = 0;
        }

        for (int j = 0; j < possibleDoorsSize; j++)
        {
            if (canThisDoorBeOpened(currRoom->id, possibleDoors[j]))
            {
                openDoor(currRoom->id, possibleDoors[j]);
                int nid = getNeighbour(currRoom->id, possibleDoors[j]);
                openDoor(nid, oppositeTable[possibleDoors[j]]);
            }
        }

        possibleDoorsSize = 0;
    }

    printf("Creando callejones sin salida...\n\n");

    int univisted[total];
    int unvisitedSize = 0;

    int unvisitedDirections[total];
    int unvisitedDirectionsSize = 0;

    struct room *currentRoom;
    struct room *roomToConnect;

    getUnvisitedeighbors(touredIds, touredIdsSize, &univisted, &unvisitedSize, &unvisitedDirections, &unvisitedDirectionsSize, goalRoomID);

    int oppositeTable[4] = {1, 0, 3, 2};

    int roomToConnectId;
    int connectedRoomID;
    int directionToConnect;
    int oppositeDirection;

    int tries = unvisitedDirectionsSize;

    int visitedIndexes[unvisitedSize];
    int visitedIndexesSize = 0;

    int actualDeadEnds = 0;

    int opporunities = 500;

    drawTemporalMap();

    while (0 < deadEnds || caminoSize < N)
    {
        int random = rand() % unvisitedSize;

        while (isInArray(visitedIndexes, visitedIndexesSize, random))
        {
            random = rand() % unvisitedSize;
        }

        connectedRoomID = univisted[random];
        directionToConnect = unvisitedDirections[random];

        roomToConnectId = getNeighbour(connectedRoomID, directionToConnect);
        oppositeDirection = oppositeTable[directionToConnect];

        currentRoom = getRoomPointerByID(connectedRoomID);
        roomToConnect = getRoomPointerByID(roomToConnectId);

        int res = openDoor(roomToConnectId, oppositeDirection);
        if (res)
        {
            struct room *room = getRoomPointerByID(connectedRoomID);

            camino[caminoSize++] = connectedRoomID;

            room->type = getRandomRoomType();
            if (room->type == Treasure){
                room->treasure = 1;
            }
            else if (room->type == Trap){
                room->trap = 1;
            }

            openDoor(connectedRoomID, directionToConnect);
            deadEnds--;

            actualDeadEnds++;
        }
        else
        {
            currentRoom->type = Wall;
        }

        visitedIndexes[visitedIndexesSize++] = random;

        if (visitedIndexesSize == unvisitedSize)
        {
            return -1;
        }
    }

    struct room *startRoom = getRoomPointerByID(startRoomID);
    startRoom->type = Start;
    startRoom->trap = 0;
    startRoom->treasure = 0;
    startRoom->occupiedByMonster = 0;

    drawTemporalMap();

    printf("Camino de habitaciones hasta el final (%d):\n", caminoSize - actualDeadEnds);
    printArray(camino, caminoSize - actualDeadEnds);
    printf("Callejones sin salida (%d):\n", actualDeadEnds);
    printArrayFrom(camino, caminoSize - actualDeadEnds, caminoSize);

    printf("Total Rooms: %d\n", caminoSize);

    printf("Start Room: %d\n", startRoomID);
    printf("Goal Room: %d\n", goalRoomID);

    return startRoomID;
}
