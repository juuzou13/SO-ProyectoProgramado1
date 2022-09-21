#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define true 1
#define false 0

enum MonsterStates { IDLE, WANDER, ATTACK, DEAD };
pthread_mutex_t lock;

struct monster {
    pthread_mutex_t monsterLock;
    int id;
    int hp;
    int atk;
    int state;
    int location;
};

struct hero {
    pthread_mutex_t heroLock;
    int hp;
    int atk;
    int location;
};

// Monster Functions

int randomWaitTime(){
    float rTime = ((rand() % 5)+1);
    rTime /= 10;
    // printf("Waiting for %f seconds\n", rTime);
    sleep(rTime);
    return 0;
}

void changeMonsterState(struct monster *m, int state) {
    m->state = state;
    if(state == IDLE){
        randomWaitTime();
    }
}

int monsterMove(struct monster *m, int location) {
    changeMonsterState(m, WANDER);
    //printf("Monster %d is moving to %d\n", m->id, location);
    if (m->location == location) {
        return false;
    }
    m->location = location;
    //printf("Monster %d is now in %d\n", m->id, m->location);

    changeMonsterState(m, IDLE);
    return true;
}

int isMonsterInHerosLocation(struct monster *m, struct hero *h) {
    if (m->location == h->location) {
        return true;
    }
    return false;
}

int attackHero(struct monster *m, struct hero *h) {
    changeMonsterState(m, ATTACK);
    printf("Monster %d is attacking hero\n", m->id);
    h->hp -= m->atk;
    printf("Hero has %d hp left\n", h->hp);
    changeMonsterState(m, IDLE);
    return true;
}

// Hero Functions

void* updateTrap(void* h) {
    struct hero *hero = (struct hero*) h;
    int roomID = hero->location;
    printf("roomID: %d\n", roomID);
    struct room *room = getRoomPointerByID(roomID);
    pthread_mutex_t roomLock = room->room_lock;
    printf("Trap in room %d is now active\n", roomID);

    float waitTime = ((rand()%15-5+1)+5);
    waitTime /= 10;

    printf("Trap will activate in %f seconds\n", waitTime);

    sleep(waitTime);
    room->activated_trap = 1;
    
    pthread_mutex_lock(&roomLock);
    if (room->isHeroInRoom){
        hero->hp -= 1;
        printf("Hero has been hit by trap in room %d\n", roomID);
        printf("Hero has %d hp left\n", hero->hp);
    }
    pthread_mutex_unlock(&roomLock);

    printf("Trap in room %d has been triggered\n", roomID);
    room->trap = 0;
    pthread_exit(0);
}

int heroMove(struct hero *h, int location) {
    // changeMonsterState(m, WANDER);
    //printf("Monster %d is moving to %d\n", m->id, location);
    if (h->location == location) {
        return false;
    }
    h->location = location;
    if (getRoomPointerByID(h->location)->trap == 1){
        pthread_t thread;
        pthread_create(&thread, NULL, updateTrap, h);
    }
    //printf("Monster %d is now in %d\n", m->id, m->location);
    
    // changeMonsterState(m, IDLE);
    return true;
}

int isHeroInMonstersLocation(struct hero *h, struct monster *m) {
    if (h->location == m->location) {
        return true;
    }
    return false;
}

int attackMonster(struct hero *h, struct monster *m) {
    // changeMonsterState(m, ATTACK);
    
    printf("Hero is attacking monster %d with atk: %d\n", m->id, h->atk);
    m->hp -= h->atk;
    printf("Monster has %d hp left\n", m->hp);
    printf("Hero has %d atk left\n", h->atk);

    // changeMonsterState(m, IDLE);
    return true;
}

pthread_mutex_t *getMonsterLock(struct monster *m) {
    return &m->monsterLock;
}

pthread_mutex_t *getHeroLock(struct hero *h) {
    return &h->heroLock;
}