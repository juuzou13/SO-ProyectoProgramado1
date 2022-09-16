#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "hero.h"


#define true 1
#define false 0

enum MonsterStates { IDLE, WANDER, ATTACK, DEAD };

struct monster {
    int id;
    int hp;
    int atk;
    int state;
    int location;
};

int randomWaitTime(){
    float rTime = ((rand() % 5)+1);
    rTime /= 10;
    printf("Waiting for %f seconds\n", rTime);
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