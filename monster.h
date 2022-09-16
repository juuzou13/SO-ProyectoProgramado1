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
    //printf("Waiting for %f seconds\n", rTime);
    sleep(rTime);
    return 0;
}

int changeMonsterState(struct monster *m, int state) {
    m->state = state;
    if(state == IDLE) {
        randomWaitTime();
    }
    return m->state;
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

int monsterAttack(struct monster *m, struct hero *h) {
    changeMonsterState(m, ATTACK);
    h->hp -= m->atk;
    printf("Monster %d attacked hero for %d damage", m->id, m->atk);
    changeMonsterState(m, IDLE);
}