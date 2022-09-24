#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define true 1
#define false 0

enum MonsterStates
{
    IDLE,
    WANDER,
    ATTACK,
    DEAD
};
pthread_mutex_t lock;

struct monster
{
    pthread_mutex_t monsterLock;
    int id;
    int hp;
    int atk;
    int state;
    int location;
};

struct hero
{
    pthread_mutex_t heroLock;
    int hp;
    int atk;
    int location;
};

int randomInt(int min, int max)
{
    return min + rand() % (max + 1 - min);
}

int randomWaitTime()
{
    float rTime = randomInt(1, 5);
    rTime /= 10;

    sleep(rTime);
    return 0;
}

void changeMonsterState(struct monster *m, int state)
{
    m->state = state;
}

void monsterWaitAfterAction(int min, int max)
{
    float r = randomInt(min, max);
    r /= 10;

    sleep(r);
}

int monsterMove(struct monster *m, int location)
{
    changeMonsterState(m, WANDER);
    if (m->location == location)
    {
        return false;
    }
    m->location = location;

    changeMonsterState(m, IDLE);
    monsterWaitAfterAction(10, 15);
    return true;
}

int isMonsterInHerosLocation(struct monster *m, struct hero *h)
{
    if (m->location == h->location)
    {
        return true;
    }
    return false;
}

int attackHero(struct monster *m, struct hero *h)
{
    if (h->location == m->location)
    {
        changeMonsterState(m, ATTACK);
        h->hp -= m->atk;
        changeMonsterState(m, IDLE);
        monsterWaitAfterAction(9, 15);
        return true;
    }
    return false;
}

void *updateTrap(void *h)
{
    struct hero *hero = (struct hero *)h;
    int roomID = hero->location;

    struct room *room = getRoomPointerByID(roomID);

    float waitTime = randomInt(8, 15);
    waitTime /= 10;

    sleep(waitTime);
    room->activated_trap = 1;

    if (room->isHeroInRoom)
    {
        hero->hp -= 1;
    }

    room->trap = 0;
    pthread_exit(0);
}

int heroMove(struct hero *h, int location)
{

    if (h->location == location)
    {
        return false;
    }
    h->location = location;
    if (getRoomPointerByID(h->location)->trap == 1)
    {
        pthread_t thread;
        pthread_create(&thread, NULL, updateTrap, h);
    }
    sleep(0.1);

    return true;
}

int isHeroInMonstersLocation(struct hero *h, struct monster *m)
{
    if (h->location == m->location)
    {
        return true;
    }
    return false;
}

int attackMonster(struct hero *h, struct monster *m)
{

    if (m->state == IDLE)
    {
        m->hp -= h->atk;
    }
    else
    {
        printf("Monster is not idle, cannot attack\n");
    }

    sleep(0.1);

    return true;
}

pthread_mutex_t *getMonsterLock(struct monster *m)
{
    return &m->monsterLock;
}

pthread_mutex_t *getHeroLock(struct hero *h)
{
    return &h->heroLock;
}