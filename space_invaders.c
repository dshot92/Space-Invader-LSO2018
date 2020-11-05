#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define DIM_QUEUE 10000
#define DELAY 10000
#define ENEMIES 50
#define STEP 1
#define MAXX COLS
#define MAXY LINES
#define BULLETS 2
#define SPRITE 3
#define SPRITE_X 3
#define SPRITE_Y 3
#define SPRITE_BOSS_X 6
#define SPRITE_BOSS_Y 5
#define LIFE 6
#define BOSS_LIFE 10
#define MAX_STRING 20
#define EMP_DELAY 2
#define EASY 6
#define MEDIUM 5
#define HARD 3

#define RED 1
#define GREEN 2
#define YELLOW  3
#define BLUE  4
#define CYAN  5

typedef struct {
  int x, oldX;
  int y, oldY;
  int life;
  char lifes[LIFE][LIFE];
  char sprite[SPRITE_X*SPRITE_Y];
  char sprite_boss[SPRITE_BOSS_X*SPRITE_BOSS_Y];
  _Bool dir;
  char c;
  char c2;
  _Bool cooldown[2];
  _Bool bomb;
  _Bool boss;
  _Bool upgrade;
  int missile;
  _Bool missile_shot;
  _Bool emp;
  _Bool shield;
}Object;

Object Ship;
Object Boss;
Object Block[4];
Object Upgrade;
Object Upgrade_missile;
Object Upgrade_emp;
Object Upgrade_shield;
Object Missile;
Object Boss_bomb[ENEMIES];
Object Invader[ENEMIES];
Object Invader_2_Level[ENEMIES];
Object Bullet[BULLETS];
Object Bomb[ENEMIES];
Object Bomb_2_Level[ENEMIES];
pthread_t thread_invader[ENEMIES];
pthread_t thread_invader_2_level[ENEMIES];
pthread_t thread_bomb[ENEMIES];
pthread_t thread_bomb_2_level[ENEMIES];
pthread_t boss;
pthread_t thread_bomb_Boss[ENEMIES];

void *player(void *n);
void *invader(void *n);
void *invader_2_level(void *n);
void *control(void *n);
void *getCommand(void *n);
void *bullet(void *n);
void *invader_bomb(void *n);
void *invader_bomb_2_level(void *n);
void *boss_object(void *n);
void *boss_bomb(void *n);
void *upgrade_life(void *n);
void *upgrade_missile(void *n);
void *upgrade_emp(void *n);
void *upgrade_shield(void *n);
void *missile(void *n);
void *timer(void *n);
void printSprite(Object obj);
void printShip(Object ship);
void printScore();
void viewScore();

void pushQueue(Object obj);
Object popQueue();
void initializeBuffer();
void bounce(int self);
void bounce_2_level(int self);
void collisions_invaders();
void collisions_bombs();
void collisions_bombs_2_level();
void collisions_bombs_boss();
void gameScore();
void intro();
void printLife();
void choosedifficulty();
void printLogo();
void block_print();
void block_init();

void debug(){
  mvprintw(MAXY/2, MAXX/2,"debug");
  refresh();
  sleep(1);
}

Object buffer[DIM_QUEUE];
int counter;
char command;
int num_bombs = 0;
int *thread;
int score=0;
int difficulty;
_Bool gameover = false;
_Bool second_level_active = false;
int emp = 1;

sem_t semaphore;
sem_t semaphore_objects;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_invaders = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_bomb = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_ship = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_writing = PTHREAD_MUTEX_INITIALIZER;

int main(){

  pthread_t tid_player, tid_control, tid_getCommand;
  Ship.life = 1;
  Boss.x = Boss.y = -10;
  Boss.life = 100000;
  Missile.life = 0;
  Upgrade.life = 0;
  srand(time(NULL));

  initscr();
  noecho();
  curs_set(0);
  if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }

  start_color();			/* Start color 			*/

  init_pair(YELLOW, COLOR_YELLOW ,COLOR_BLACK);
  init_pair(GREEN, COLOR_GREEN ,COLOR_BLACK);
  init_pair(RED, COLOR_RED ,COLOR_BLACK);
  init_pair(BLUE, COLOR_BLUE ,COLOR_BLACK);
  init_pair(CYAN, COLOR_CYAN ,COLOR_BLACK);

  choosedifficulty();
  block_init();

  pthread_create(&tid_getCommand, NULL, getCommand, NULL);

  //intro();

  initializeBuffer();

  pthread_create(&tid_player, NULL, player, NULL);
  pthread_create(&tid_control, NULL, control, NULL);
  thread = (int *) malloc(sizeof(int));
  for(int i = 0; i < ENEMIES; i++){
    *thread = i;
    pthread_create(&thread_invader[i], NULL, invader, (void *)thread);
    sleep(2);
  }

  while(1);

  endwin();


  return 0;
}

void *player(void *n){
  Ship.y = MAXY-SPRITE;
  Ship.x = MAXX/2;
  Ship.life = LIFE;
  Ship.cooldown[0] = true;
  Ship.cooldown[1] = true;
  Ship.missile = difficulty/2;
  Ship.missile_shot = false;
  Ship.emp = false;
  Ship.shield = true;
  strcpy(Ship.sprite, " A /T\\---");
  Ship.c = 'A';



  int i;
  int *bul, *miss;
  pthread_t thread_bullet[BULLETS], upgrade_thread, missile_thread, prejectile_thread, emp_thread, timer_thread, shield_thread;

  while(1) {

    Ship.oldX = Ship.x;
    Ship.oldY = Ship.y;

    while(1) {
      pthread_mutex_lock(&mutex_invaders);
      pthread_mutex_lock(&mutex_ship);

      Ship.oldX = Ship.x;
      Ship.oldY = Ship.y;

      if(command == 'd' && Ship.x + 3 < MAXX-1){
        strcpy(Ship.sprite, " /|/||---");
        Ship.x++;
      }
      if(command == 'a' && Ship.x  > 0){
        strcpy(Ship.sprite, "|\\ ||\\---");
        Ship.x--;
      }

      if(command == 'w' && Ship.y > MAXY/2){
        Ship.y--;
      }
      if(command == 'e' && Ship.y > MAXY/2 && Ship.y - 3 < MAXY-1){
        Ship.y--;
        Ship.x++;
      }
      if(command == 'q' && Ship.y > MAXY/2 && Ship.x > 0){
        Ship.y--;
        Ship.x--;
      }
      if(command == 's' && Ship.y + 3 < MAXY){
        Ship.y++;
      }

      if(command == ' ' && Ship.cooldown[0] && Ship.cooldown[1] ){
        Ship.cooldown[0] = false;
        Ship.cooldown[1] = false;
        for(i = 0; i < BULLETS; i++){
          bul = (int *) malloc(sizeof(int));
          *bul = i;
          pthread_create(&thread_bullet[i], NULL, bullet, (void *)bul);
        }
      }

      if(command == 'l'){
        Ship.life--;
      }
      if(command == 'p'){
        Ship.life++;
      }
      if(command == 'k'){
        Ship.missile++;
      }
      if(command == 'u'){
        Ship.emp = true;
      }
      if(command == 'j'){
        Ship.shield = true;
      }
      if(command == 'i'){
        for(i = 0; i<ENEMIES;++i){
          Invader[i].life--;
          Invader_2_Level[i].life--;
        }
        if(Boss.life > 0) Boss.life--;
      }

      if(Ship.emp && command == 'f'){
        Ship.emp = false;
        pthread_create(&timer_thread, NULL, timer, NULL);
        for(i = 0; i<ENEMIES;++i){
          Invader[i].life--;
          Invader_2_Level[i].life--;
        }
        if(Boss.life > 1) Boss.life--;
      }

      if(rand()%10000 < 10*difficulty && !Upgrade.life && Ship.life >= difficulty){

        pthread_create(&upgrade_thread, NULL, upgrade_life, NULL);
      }

      if(rand()%10000 < 10*difficulty && Ship.missile < 3 && Upgrade_missile.life == 0 ){
        pthread_create(&missile_thread, NULL, upgrade_missile, NULL);
      }

      if(rand()%10000 < 10*difficulty && !Ship.emp && Upgrade_emp.life == 0 && second_level_active){
        pthread_create(&emp_thread, NULL, upgrade_emp, NULL);
      }
      if(rand()%10000 < 10*difficulty && !Ship.shield && Upgrade_shield.life == 0){
        pthread_create(&shield_thread, NULL, upgrade_shield, NULL);
      }

      if( command == 'm' && Ship.missile && !Ship.missile_shot){
        Ship.missile_shot = true;
        Ship.missile--;
        pthread_create(&prejectile_thread, NULL, missile, NULL);
      }

      pushQueue(Ship);

      command = '+';
      pthread_mutex_unlock(&mutex_ship);
      pthread_mutex_unlock(&mutex_invaders);
      usleep(DELAY*difficulty);
    }
  }
}

void *bullet(void *n){
  pthread_mutex_lock(&mutex_invaders);
  int index = *((int *)n);
  pthread_mutex_unlock(&mutex_invaders);


  Bullet[index].y = Ship.y-1;
  Bullet[index].c = '*';
  Bullet[index].life = 1;
  Bullet[index].x = Ship.x+1;
  int step;
  if (index){
    step = -1;
  }else{
    step = 1;
  }

  while(Bullet[index].life != 0){
    sem_wait(&semaphore_objects);
    pthread_mutex_lock(&mutex_invaders);

    Bullet[index].oldX = Bullet[index].x;
    Bullet[index].oldY = Bullet[index].y;
    Bullet[index].y--;
    Bullet[index].x += step;
    if(Bullet[index].x == MAXX || Bullet[index].x == 0){
      step *= -1;
    }
    if(Bullet[index].y <= -1){
      Bullet[index].life--;
    }
    pushQueue(Bullet[index]);

    pthread_mutex_unlock(&mutex_invaders);
    usleep(DELAY*difficulty);
  }
  Ship.cooldown[index] = true;
  pthread_mutex_lock(&mutex_writing);
  mvaddch(Bullet[index].y, Bullet[index].x, ' ');
  Bullet[index].y = -1;
  Bullet[index].x = -1;
  pthread_mutex_unlock(&mutex_writing);

  pthread_exit(NULL);
}

void *invader(void *n){
  pthread_mutex_lock(&mutex_invaders);
  int index = *((int *)n);
  pthread_mutex_unlock(&mutex_invaders);

  Invader[index].x =MAXX/2;
  Invader[index].y = 3;
  Invader[index].life = 2;
  Invader[index].c = 'X';
  // Invader[index].c2 = index+48; // numbers from 0 to ENEMIES
  Invader[index].c2 = index+97; //Lettern from a to ENEMIES
  Invader[index].dir = ((rand()%2) < 1);
  Invader[index].bomb = true;

  int dx = STEP;

  int *bomb;

  while(Invader[index].life != 0) {
    sem_wait(&semaphore_objects);
    pthread_mutex_lock(&mutex_invaders);

    bounce(index);
    Invader[index].oldX = Invader[index].x;
    Invader[index].oldY = Invader[index].y;


    if(Invader[index].x + dx > MAXX-1 || Invader[index].x +dx < 1){
      Invader[index].y++;
      Invader[index].dir = !Invader[index].dir;
    }
    if(Invader[index].dir){
      Invader[index].x -= dx;
    }
    else{
      Invader[index].x += dx;
    }

    if((rand()%100) < 10 && Invader[index].bomb && num_bombs < ENEMIES){
      bomb = (int *) malloc(sizeof(int));
      *bomb = index;
      num_bombs++;
      Invader[index].bomb = false;
      pthread_create(&thread_bomb[index], NULL, invader_bomb, (void *)bomb);
    }

    if(Invader[index].y > MAXY - SPRITE_Y - 1){
      Invader[index].life--;
      Ship.life = 0;
    }

    pushQueue(Invader[index]);

    pthread_mutex_unlock(&mutex_invaders);
    //collisions_invaders();
    usleep(DELAY*difficulty*difficulty*emp);
  }
  score+= (10 - difficulty);
  pthread_mutex_lock(&mutex_writing);
  mvaddch(Invader[index].y, Invader[index].x, ' ');
  Invader[index].x = -1;
  Invader[index].y = -1;
  pthread_mutex_unlock(&mutex_writing);
  usleep((rand() % DELAY) * index * index);
  pthread_mutex_lock(&mutex_invaders);
  second_level_active = true;
  *thread = index;
  pthread_create(&thread_invader_2_level[index], NULL, invader_2_level, (void *)thread);
  pthread_mutex_unlock(&mutex_invaders);
}

void *invader_2_level(void *n){
  pthread_mutex_lock(&mutex_invaders);
  int index = *((int *)n);
  usleep((rand()%30000) * index);
  pthread_mutex_unlock(&mutex_invaders);

  usleep((rand()%30000) * index);
  Invader_2_Level[index].x = (rand()%MAXX - 1) ;
  Invader_2_Level[index].y = (rand()%2 + 1);
  Invader_2_Level[index].life = 2;
  Invader_2_Level[index].c = 'X';
  // Invader[index].c2 = index+48; // numbers from 0 to ENEMIES
  Invader_2_Level[index].c2 = index+65; //Lettern from A to ENEMIES
  Invader_2_Level[index].dir = ((rand()%2) < 1);
  Invader_2_Level[index].bomb = true;
  Invader_2_Level[index].boss = true;

  int dx = STEP;
  _Bool spawn_boss = true;
  int *bomb;

  while(Invader_2_Level[index].life != 0) {
    pthread_mutex_lock(&mutex_invaders);
    sem_wait(&semaphore_objects);

    bounce_2_level(index);
    Invader_2_Level[index].oldX = Invader_2_Level[index].x;
    Invader_2_Level[index].oldY = Invader_2_Level[index].y;

    if(Invader_2_Level[index].x + dx > MAXX-1 || Invader_2_Level[index].x +dx < 1){
      Invader_2_Level[index].y++;
      Invader_2_Level[index].dir = !Invader_2_Level[index].dir;
    }
    if(Invader_2_Level[index].dir){
      Invader_2_Level[index].x -= dx;
    }
    else{
      Invader_2_Level[index].x += dx;
    }

    if((rand()%100) < 10 && Invader_2_Level[index].bomb && num_bombs < ENEMIES){
      bomb = (int *) malloc(sizeof(int));
      *bomb = index;
      num_bombs++;
      Invader_2_Level[index].bomb = false;
      pthread_create(&thread_bomb_2_level[index], NULL, invader_bomb_2_level, (void *)bomb);
    }

    if(Invader_2_Level[index].y > MAXY - SPRITE_Y - 1){
      Invader_2_Level[index].life--;
      Ship.life = 0;
    }
    pushQueue(Invader_2_Level[index]);
    for(int i = 0; i < ENEMIES; ++i){
      if(!Invader_2_Level[i].boss) spawn_boss = false;
    }

    pthread_mutex_unlock(&mutex_invaders);
    usleep(DELAY*difficulty*difficulty*emp);
  }
  if(spawn_boss){
    pthread_create(&boss, NULL, boss_object, (void *)boss);
  }
  score+= (10 - difficulty)*2;
  pthread_mutex_lock(&mutex_writing);
  mvaddch(Invader_2_Level[index].y, Invader_2_Level[index].x, ' ');
  Invader_2_Level[index].x = -1;
  Invader_2_Level[index].y = -1;
  pthread_mutex_unlock(&mutex_writing);
}
void *boss_object(void *n){
  Boss.x = MAXX/2 - 2;
  Boss.y = 1;
  Boss.life = BOSS_LIFE;
  Boss.c = '[';
  Boss.bomb = true;
  Boss.dir = ((rand()%2) < 1);
  strcpy(Boss.sprite_boss, "  #   # # # # ###### # # "); // 5X5
  strcpy(Boss.sprite_boss, "  ##   /  \\ /    \\###### v  v "); //	6X5


  int *bomb;
  int index=0;
  int dx = STEP;
  while(Boss.life >= 1){
    Boss.oldX = Boss.x;
    Boss.oldY = Boss.y;
    if(Boss.x+SPRITE_BOSS_X + dx > MAXX-1 || Boss.x + dx < 1){
      Boss.y++;
      Boss.dir = !Boss.dir;
    }
    if(Boss.dir){
      Boss.x -= dx;
    }
    else{
      Boss.x += dx;
    }

    if((rand()%100) < 10 && num_bombs < ENEMIES){
      bomb = (int *) malloc(sizeof(int));
      *bomb = index;
      index++;
      index %= ENEMIES;
      Boss.bomb = false;
      pthread_create(&thread_bomb_Boss[index], NULL, boss_bomb, (void *)bomb);
      num_bombs++;
    }

    if(Boss.y+SPRITE_BOSS_Y > MAXY - SPRITE_Y - 1){
      Boss.life--;
      Ship.life = 0;
    }
    pushQueue(Boss);
    usleep(DELAY*difficulty*difficulty);
  }
  viewScore();

}

void *boss_bomb(void *n){
  pthread_mutex_lock(&mutex_invaders);
  int index = *((int *)n);
  pthread_mutex_unlock(&mutex_invaders);

  Boss_bomb[index].y = Boss.y+5;
  Boss_bomb[index].x = Boss.x+2;
  Boss_bomb[index].c = '@';
  Boss_bomb[index].life = 7;

  while(Boss_bomb[index].life){
    pthread_mutex_lock(&mutex_bomb);
    pthread_mutex_lock(&mutex_invaders);

    Boss_bomb[index].oldX = Boss_bomb[index].x;
    Boss_bomb[index].oldY = Boss_bomb[index].y;

    if(Boss_bomb[index].y > MAXY){
      Boss_bomb[index].life--;
    }else{
      Boss_bomb[index].y++;
    }
    pushQueue(Boss_bomb[index]);
    pthread_mutex_unlock(&mutex_invaders);
    pthread_mutex_unlock(&mutex_bomb);
    usleep(DELAY*difficulty*difficulty);
  }
  num_bombs--;
}

void *invader_bomb(void *n){
  pthread_mutex_lock(&mutex_invaders);
  int index = *((int *)n);
  pthread_mutex_unlock(&mutex_invaders);

  Bomb[index].y = Invader[index].y-1;
  Bomb[index].x = Invader[index].x;
  Bomb[index].c = 'o';
  Bomb[index].life = 1;

  while(Bomb[index].life){
    pthread_mutex_lock(&mutex_bomb);
    pthread_mutex_lock(&mutex_invaders);

    Bomb[index].oldX = Bomb[index].x;
    Bomb[index].oldY = Bomb[index].y;

    if(Bomb[index].y > MAXY){
      Bomb[index].life--;
    }else{
      Bomb[index].y++;
    }
    pushQueue(Bomb[index]);
    pthread_mutex_unlock(&mutex_invaders);
    pthread_mutex_unlock(&mutex_bomb);
    usleep(DELAY*difficulty*difficulty);
  }
  Invader[index].bomb = true;
  num_bombs--;
}

void *invader_bomb_2_level(void *n){
  pthread_mutex_lock(&mutex_invaders);
  int index = *((int *)n);
  pthread_mutex_unlock(&mutex_invaders);

  Bomb_2_Level[index].y = Invader_2_Level[index].y-1;
  Bomb_2_Level[index].x = Invader_2_Level[index].x;
  Bomb_2_Level[index].c = '#';
  Bomb_2_Level[index].life = 1;

  while(Bomb_2_Level[index].life){
    pthread_mutex_lock(&mutex_bomb);
    pthread_mutex_lock(&mutex_invaders);

    Bomb_2_Level[index].oldX = Bomb_2_Level[index].x;
    Bomb_2_Level[index].oldY = Bomb_2_Level[index].y;

    if(Bomb_2_Level[index].y > MAXY){
      Bomb_2_Level[index].life--;
    }else{
      Bomb_2_Level[index].y++;
    }
    pushQueue(Bomb_2_Level[index]);
    pthread_mutex_unlock(&mutex_invaders);
    pthread_mutex_unlock(&mutex_bomb);
    usleep(DELAY*difficulty*difficulty);
  }
  Invader_2_Level[index].bomb = true;
  num_bombs--;
}

void *upgrade_life(void *n){
  Upgrade.x = rand()%MAXX;
  Upgrade.y = MAXY/2;
  Upgrade.c = '+';
  Upgrade.life = 1;

  while(Upgrade.life){
    pthread_mutex_lock(&mutex_invaders);
    Upgrade.oldX = Upgrade.x;
    Upgrade.oldY = Upgrade.y;

    Upgrade.y++;

    if(Upgrade.y >= MAXY) Upgrade.life = 0;

    if(Upgrade.x >= Ship.x && Upgrade.x < Ship.x+SPRITE_X
      && Upgrade.y >= Ship.y && Upgrade.y < Ship.y+SPRITE_Y){
        Ship.life++;
        Upgrade.life = 0;
      }

      pushQueue(Upgrade);

      pthread_mutex_unlock(&mutex_invaders);
      usleep(DELAY*10*difficulty);
    }
    Upgrade.x = -10;
    Upgrade.y = -10;

  }

  void *upgrade_missile(void *n){
    Upgrade_missile.x = rand()%MAXX;
    Upgrade_missile.y = MAXY/2;
    Upgrade_missile.c = '^';
    Upgrade_missile.life = 1;

    while(Upgrade_missile.life){
      if(Upgrade_missile.x >= Ship.x && Upgrade_missile.x < Ship.x+SPRITE_X
        && Upgrade_missile.y >= Ship.y && Upgrade_missile.y < Ship.y+SPRITE_Y){
          Ship.missile++;
          Upgrade_missile.life = 0;
        }
        Upgrade_missile.oldX = Upgrade_missile.x;
        Upgrade_missile.oldY = Upgrade_missile.y;

        Upgrade_missile.y++;

        if(Upgrade_missile.y >= MAXY) Upgrade_missile.life = 0;

        pushQueue(Upgrade_missile);

        usleep(DELAY*10*difficulty);
      }
      Upgrade_missile.x = -10;
      Upgrade_missile.y = -10;
    }

    void *upgrade_emp(void *n){
      Upgrade_emp.x = rand()%MAXX;
      Upgrade_emp.y = MAXY/2;
      Upgrade_emp.c = '?';
      Upgrade_emp.life = 1;

      while(Upgrade_emp.life){
        if(Upgrade_emp.x >= Ship.x && Upgrade_emp.x < Ship.x+SPRITE_X
          && Upgrade_emp.y >= Ship.y && Upgrade_emp.y < Ship.y+SPRITE_Y){
            Ship.emp =true;
            Upgrade_emp.life = 0;
          }
          Upgrade_emp.oldX = Upgrade_emp.x;
          Upgrade_emp.oldY = Upgrade_emp.y;

          Upgrade_emp.y++;

          if(Upgrade_emp.y >= MAXY) Upgrade_emp.life = 0;

          pushQueue(Upgrade_emp);

          usleep(DELAY*10*difficulty);
        }
        Upgrade_emp.x = -10;
        Upgrade_emp.y = -10;
      }
      void *missile(void *n){
        Missile.x = Ship.x+1;
        Missile.y = Ship.y-1;
        Missile.c = '^';
        Missile.life = 1;

        while(Missile.life){
          pthread_mutex_lock(&mutex_invaders);
          Missile.oldX = Missile.x;
          Missile.oldY = Missile.y;

          Missile.y--;

          if(Missile.y < 0){
            Missile.life = 0;
            Ship.missile_shot = false;
          }
          if(command == '.' && Missile.x < MAXX-1){
            Missile.x++;
          }
          if(command == ',' && Missile.x  > 0){
            Missile.x--;
          }
          pushQueue(Missile);
          pthread_mutex_unlock(&mutex_invaders);
          usleep(DELAY*2*difficulty);
        }
        pushQueue(Missile);
        Missile.oldX = -10;
        Missile.oldY = -10;
        Missile.x = -10;
        Missile.y = -10;
        usleep(200);
        Ship.missile_shot = false;
      }

      void *timer(void *n){
        emp = 10;
        sleep(EMP_DELAY);
        emp = 1;
      }

      void *upgrade_shield(void *n){
        Upgrade_shield.x = rand()%MAXX;
        Upgrade_shield.y = MAXY/2;
        Upgrade_shield.c = '0';
        Upgrade_shield.life = 1;

        while(Upgrade_shield.life){
          if(Upgrade_shield.x >= Ship.x && Upgrade_shield.x < Ship.x+SPRITE_X
            && Upgrade_shield.y >= Ship.y && Upgrade_shield.y < Ship.y+SPRITE_Y){
              Ship.shield =true;
              Upgrade_shield.life = 0;
            }
            Upgrade_shield.oldX = Upgrade_shield.x;
            Upgrade_shield.oldY = Upgrade_shield.y;

            Upgrade_shield.y++;

            if(Upgrade_shield.y >= MAXY) Upgrade_shield.life = 0;

            pushQueue(Upgrade_shield);

            usleep(DELAY*10*difficulty);
          }
          Upgrade_shield.x = -10;
          Upgrade_shield.y = -10;
        }

        void *control(void *n){
          Object aux;
          while(Ship.life > 0 && Boss.life > 0){
            aux = popQueue();
            switch(aux.c){
              case 'A':
              attron(COLOR_PAIR(YELLOW));
              printShip(aux);
              attroff(COLOR_PAIR(YELLOW));
              break;
              case 'X':
              attron(COLOR_PAIR(aux.life));
              printSprite(aux);
              attroff(COLOR_PAIR(aux.life));
              sem_post(&semaphore_objects);
              break;
              case '*':
              printSprite(aux);
              sem_post(&semaphore_objects);
              break;
              case 'o':
              printSprite(aux);
              collisions_bombs();
              sem_post(&semaphore_objects);
              break;
              case '#':
              attron(COLOR_PAIR(BLUE));
              printSprite(aux);
              collisions_bombs_2_level();
              attroff(COLOR_PAIR(BLUE));
              sem_post(&semaphore_objects);
              break;
              case '[':
              attron(COLOR_PAIR(BLUE));
              printShip(aux);
              collisions_bombs_2_level();
              attroff(COLOR_PAIR(BLUE));
              sem_post(&semaphore_objects);
              break;
              case '@':
              attron(COLOR_PAIR(RED));
              printSprite(aux);
              collisions_bombs_boss();
              attroff(COLOR_PAIR(RED));
              sem_post(&semaphore_objects);
              break;
              case '+':
              attron(COLOR_PAIR(CYAN));
              printSprite(aux);
              attroff(COLOR_PAIR(CYAN));
              sem_post(&semaphore_objects);
              break;
              case '^':
              pthread_mutex_lock(&mutex_ship);
              attron(COLOR_PAIR(CYAN));
              printSprite(aux);
              attroff(COLOR_PAIR(CYAN));
              pthread_mutex_unlock(&mutex_ship);
              sem_post(&semaphore_objects);
              break;
              case '?':
              pthread_mutex_lock(&mutex_ship);
              attron(COLOR_PAIR(CYAN));
              printSprite(aux);
              attroff(COLOR_PAIR(CYAN));
              pthread_mutex_unlock(&mutex_ship);
              sem_post(&semaphore_objects);
              break;
              case '0':
              pthread_mutex_lock(&mutex_ship);
              attron(COLOR_PAIR(CYAN));
              printSprite(aux);
              attroff(COLOR_PAIR(CYAN));
              pthread_mutex_unlock(&mutex_ship);
              sem_post(&semaphore_objects);
              break;
            }
            printLife();
            collisions_invaders();
            gameScore();
            refresh();
          }
          if(Ship.life == 0){
            mvaddstr(MAXY/2, (MAXX - 5)/2, "GAME-OVER");
          } else{
            mvaddstr(MAXY/2, (MAXX - 3)/2, "YOU-WON");
          }
          refresh();
        }

        void printSprite(Object obj){
          pthread_mutex_lock(&mutex_writing);
          pthread_mutex_lock(&mutex_invaders);
          mvaddch(obj.oldY, obj.oldX, ' ');
          mvaddch(obj.y, obj.x, ' ');
          if(obj.life){
            if(obj.c == 'X') mvaddch(obj.y, obj.x, obj.c2);
            else mvaddch(obj.y, obj.x, obj.c);
          }

          pthread_mutex_unlock(&mutex_invaders);
          pthread_mutex_unlock(&mutex_writing);
        }

        void printShip(Object ship){
          char shield[49] = "         ___   /   \\  |   |  |   |  ~~~~~        ";
          pthread_mutex_lock(&mutex_invaders);
          int i,j, index=0;
          if(ship.c == 'A'){
            for (i = 0; i < SPRITE_Y; i++){
              for (j = 0; j < SPRITE_X; j++){
                mvaddch(Ship.oldY+i, Ship.oldX+j+1, ' ');
                mvaddch(Ship.oldY+i, Ship.oldX+j-1, ' ');
                index++;
              }
            }
            if(Ship.shield){
              index = 0;
              for (i = 0; i < 7; i++){
                for (j = 0; j < 7; j++){
                  mvaddch(Ship.y-2+i, Ship.x-2+j,shield[index]);
                  index++;
                }
              }
            }
            index = 0;
            for (i = 0; i < SPRITE_Y; i++){
              for (j = 0; j < SPRITE_X; j++){
                mvaddch(Ship.y+i, Ship.x+j, Ship.sprite[index]);
                index++;
              }
            }
            strcpy(Ship.sprite, " A /T\\---");
          }
          else{
            index = 0;
            for (i = 0; i < SPRITE_BOSS_Y; i++){
              for (j = 0; j < SPRITE_BOSS_X; j++){
                mvaddch(Boss.oldY+i, Boss.oldX+j+1, ' ');
                mvaddch(Boss.oldY+i, Boss.oldX+j-1, ' ');
                index++;
              }
            }
            index = 0;
            for (i = 0; i < SPRITE_BOSS_Y; i++){
              for (j = 0; j < SPRITE_BOSS_X; j++){
                mvaddch(Boss.y+i, Boss.x+j, Boss.sprite_boss[index]);
                index++;
              }
            }
          }
          pthread_mutex_unlock(&mutex_invaders);
        }


        void pushQueue(Object obj) {
          pthread_mutex_lock(&lock);
          if(counter < DIM_QUEUE){
            counter++;
            buffer[counter] = obj;
            sem_post(&semaphore);
          }
          pthread_mutex_unlock(&lock);
        }

        Object popQueue() {
          sem_wait(&semaphore);
          pthread_mutex_lock(&lock);
          Object val;
          if(counter >= 0){
            val = buffer[counter];
            counter--;
          }
          pthread_mutex_unlock(&lock);
          return val;
        }

        void initializeBuffer(){
          counter = 0;
          sem_init(&semaphore, 0, 0);
          sem_init(&semaphore_objects, 0, 1);
        }

        void *getCommand(void *n){
          command = -1;
          //clear();
          while(1){
            command = getchar();
          }
        }

        void bounce(int self){
          int i;
          pthread_mutex_lock(&lock);
          for( i = 0; i < ENEMIES; ++i){
            if(i != self ){
              if(Invader[i].y == Invader[self].y && Invader[i].x == Invader[self].x){
                Invader[self].dir = !Invader[self].dir;
                Invader[i].dir = !Invader[i].dir;
              }
            }
          }
          pthread_mutex_unlock(&lock);
        }

        void bounce_2_level(int self){
          int i;
          pthread_mutex_lock(&lock);
          for( i = 0; i < ENEMIES; ++i){
            if(i != self ){
              if(Invader_2_Level[i].y == Invader_2_Level[self].y && Invader_2_Level[i].x == Invader_2_Level[self].x){
                Invader_2_Level[self].dir = !Invader_2_Level[self].dir;
                Invader_2_Level[i].dir = !Invader_2_Level[i].dir;
              }
            }
          }
          pthread_mutex_unlock(&lock);
        }
        void collisions_invaders(){
          int i,j, index=0, range = 2;
          pthread_mutex_lock(&mutex_invaders);

          for(j = 0; j < ENEMIES; ++j){
            if(Missile.life != 0 && Invader[j].life != 0 && Invader[j].y == Missile.y && (Invader[j].x > Missile.x - range) && (Invader[j].x <= Missile.x +range)){
              Invader[j].life-=2;
              if(Invader[j].life < 0) Invader[j].life = 0;
              Missile.life--;
            }
            if(Missile.life != 0 && Invader_2_Level[j].life != 0 && Invader_2_Level[j].y == Missile.y && (Invader_2_Level[j].x <= Missile.x + range) && (Invader_2_Level[j].x > Missile.x - range)){
              Invader_2_Level[j].life-=2;
              if(Invader_2_Level[j].life < 0) Invader_2_Level[j].life = 0;
              Missile.life--;
            }
          }
          for(i = 0; i < BULLETS; ++i){

            if(Bullet[i].life){
              for(j = 0; j < ENEMIES; ++j){
                if(Invader[j].life != 0 && Invader[j].y == Bullet[i].y && Invader[j].x == Bullet[i].x){
                  Invader[j].life--;
                  Bullet[i].life--;
                }
              }
            }
          }
          index = 0;
          for(i = 0; i < BULLETS; ++i){
            if(Bullet[i].life){
              for(j = 0; j < ENEMIES; ++j){

                if(Invader_2_Level[j].life != 0 && Invader_2_Level[j].y == Bullet[i].y && Invader_2_Level[j].x == Bullet[i].x){
                  Invader_2_Level[j].life--;
                  Bullet[i].life--;
                }
              }
            }
          }
          index = 0;
          for(i = 0; i < BULLETS; ++i){
            if(Bullet[i].life && Bullet[i].x < Boss.x + SPRITE_BOSS_X && Bullet[i].x >= Boss.x && Bullet[i].y < Boss.y + SPRITE_BOSS_Y &&  Bullet[i].y >= Boss.y ){
              Boss.life--;
              Bullet[i].life--;
            }
          }
          if(Missile.life && Missile.x < Boss.x + SPRITE_BOSS_X && Missile.x >= Boss.x && Missile.y < Boss.y + SPRITE_BOSS_Y &&  Missile.y >= Boss.y ){
            Boss.life-=2;
            Missile.life--;
          }
          pthread_mutex_unlock(&mutex_invaders);
        }

        void collisions_bombs(){
          int i;
          pthread_mutex_lock(&mutex_invaders);

          for(i = 0; i < ENEMIES; ++i){
            if(Bomb[i].life != 0 && Bomb[i].y >= Ship.y && Bomb[i].y <= Ship.y + SPRITE_Y  && Bomb[i].x >= Ship.x && Bomb[i].x < Ship.x+SPRITE_X ){
              Bomb[i].life--;
              if(Ship.shield){
                Ship.shield = false;
                pthread_mutex_lock(&mutex_writing);
                mvaddstr(Ship.y-1, Ship.x-2, "       ");
                mvaddstr(Ship.y+3, Ship.x-2, "       ");
                pthread_mutex_unlock(&mutex_writing);
              }
              else Ship.life--;
            }
          }
          pthread_mutex_unlock(&mutex_invaders);

        }

        void collisions_bombs_2_level(){
          pthread_mutex_lock(&mutex_invaders);
          for(int i = 0; i < ENEMIES; ++i){
            if(Bomb_2_Level[i].life != 0 && Bomb_2_Level[i].y >= Ship.y && Bomb_2_Level[i].y <= Ship.y + SPRITE_Y&& Bomb_2_Level[i].x >= Ship.x && Bomb_2_Level[i].x < Ship.x+SPRITE_X ){
              Bomb_2_Level[i].life--;
              if(Ship.shield){
                Ship.shield = false;
                pthread_mutex_lock(&mutex_writing);
                mvaddstr(Ship.y-1, Ship.x-2, "       ");
                mvaddstr(Ship.y+3, Ship.x-2, "       ");
                pthread_mutex_unlock(&mutex_writing);
              }
              else Ship.life-=2;
            }
          }
          if(Ship.life < 0) Ship.life = 0;
          pthread_mutex_unlock(&mutex_invaders);
        }

        void collisions_bombs_boss(){
          pthread_mutex_lock(&mutex_invaders);
          for(int i = 0; i < ENEMIES; ++i){
            if(Boss_bomb[i].life != 0 && Boss_bomb[i].y >= Ship.y && Boss_bomb[i].y <= Ship.y + SPRITE_Y  && Boss_bomb[i].x >= Ship.x && Boss_bomb[i].x < Ship.x+SPRITE_X ){
              Boss_bomb[i].life--;
              if(Ship.shield){
                Ship.shield = false;
                pthread_mutex_lock(&mutex_writing);
                mvaddstr(Ship.y-1, Ship.x-2, "       ");
                mvaddstr(Ship.y+3, Ship.x-2, "       ");
                pthread_mutex_unlock(&mutex_writing);
              }
              else Ship.life-=3;
            }
          }
          if(Ship.life < 0) Ship.life = 0;
          pthread_mutex_unlock(&mutex_invaders);
        }

        void gameScore(){
          pthread_mutex_lock(&mutex_writing);
          mvaddstr(0, 1, "SCORE:");
          mvprintw(0, 11, "%d", score);
          if(MAXX > 28){
            if(Ship.missile != 0) mvprintw(0, 16, "Missile: %d", Ship.missile);
            else mvaddstr(0, 16, "             ");
            if(Ship.emp != 0) mvaddstr(0, 30, "EMP Available");
            else mvaddstr(0, 30, "              ");
          }else{
            if(Ship.missile != 0) mvprintw(1, 1, "Missile: %d", Ship.missile);
            else mvaddstr(0, 1, "                           ");
            if(Ship.emp != 0) mvaddstr(2, 1, "EMP Available");
            else mvaddstr(0, 1, "                           ");
          }
          mvprintw(Boss.y+2, Boss.x+2, "%d", Boss.life);
          pthread_mutex_unlock(&mutex_writing);
        }

        void intro(){
          int i,j, k=0;
          char intro[100][100] = {{"In a world, where dyslexia reigns"},
          {"undisturbed through the population,"},
          {"long distant neighbours in our"},
          {"Milky Way Galaxy had enough of it"},
          {"and decided to get here and to give"},
          {"us a new knowledge renaissance."},
          {"After some research they decide that"},
          {"the most common way to contact us is"},
          {"through landing in the United States."},
          {"Little did they know that the new "},
          {"president of the USA is Donald J. Trump,"},
          {"the sole reason of this dyslexia "},
          {"spreading amongst human beings."},
          {""},
          {"He immediately decide to counter this"},
          {"new wave of knowledge with all the"},
          {"military power he can muster."},
          {""},
          {"So you find yourself trapped into"},
          {"being an earth-trooper on the frontline"},
          {"to defeat the knowledge of"},
          {"the Alphabet Civilization."},
          {""},
          {"Good Luck to you, trooper."},
          {""},
          {""},
          {"LONG LIVE DYSLEXIA"}};
          attron(COLOR_PAIR(YELLOW));
          for(i = 27; i > 0 ; i--){
            clear();
            for(j = k ; j > 0  ; j--){
              mvaddstr((LINES/2)-j , (COLS/2- strlen(intro[k-j])/2) , intro[k-j]);
            }
            mvaddstr((LINES/2) , (COLS/2- strlen(intro[k])/2) , intro[k]);
            k++;
            refresh();
            usleep(DELAY*100);
          }
          attroff(COLOR_PAIR(YELLOW));
          attron(COLOR_PAIR(GREEN));
          do{
            mvaddstr(((LINES/2) +4) , (COLS/2 - (strlen("--Press SPACE to start--")/2)) , "                              ");
            refresh();
            usleep(DELAY*15);
            mvaddstr(((LINES/2) +4) , (COLS/2 - (strlen("--Press SPACE to start--")/2)) , "--Press SPACE to start--");
            refresh();
            usleep(DELAY*15);
          }while(command != ' ');
          attroff(COLOR_PAIR(GREEN));
          command = '+'; // clear command buffer, so that ship wont fire immediately
          clear();
        }

        void printLife(){
          pthread_mutex_lock(&mutex_writing);
          mvaddstr(0,MAXX-14, "              ");
          mvaddstr(0, MAXX-7, "  LIVES");
          mvprintw(0, MAXX-8, "%d", Ship.life);
          pthread_mutex_unlock(&mutex_writing);
        }

        void choosedifficulty(){
          char difficult[15];
          int i = 0;

          while(1) {
            if(i <= 0){
              mvaddstr(MAXY /2 , MAXX/2-5, "                   ");
              refresh();
              strcpy(difficult, "v  EASY  v");
              difficulty = EASY;
            }
            if(i == 1){
              strcpy(difficult, "v MEDIUM ^");
              mvaddstr(MAXY /2 , MAXX/2-5, "                   ");
              refresh();
              difficulty = MEDIUM;
            }
            if(i >= 2){
              strcpy(difficult, "^  HARD  ^");
              mvaddstr(MAXY /2 , MAXX/2-5, "                   ");
              refresh();
              difficulty = HARD;
            }

            mvaddstr(MAXY /2 -5, MAXX/2-8, "CHOOSE DIFFICULTY");
            mvaddstr(MAXY /2 , MAXX/2-5, difficult);
            mvaddstr(MAXY /2 +5, MAXX/2-10, "Press SPACE to select");
            printLogo();
            refresh();
            switch (getchar()) {
              case 'w':
              if(i>0) {
                i--;
              }
              break;
              case 's':
              if(i<2) {
                i++;
              }
              break;
              case ' ':
              clear();
              refresh();
              return;
            }
            clear();
          }
        }

        void printLogo(){
          int offsetUp = 13, offsetDown = 21;
          mvaddstr(0, MAXX/2-offsetUp, " _____                                    ");
          mvaddstr(1, MAXX/2-offsetUp, "/  ___|                                   ");
          mvaddstr(2, MAXX/2-offsetUp, "\\ `--. _ __   __ _  ___ ___               ");
          mvaddstr(3, MAXX/2-offsetUp, " `--. \\ '_ \\ / _` |/ __/ _ \\              ");
          mvaddstr(4, MAXX/2-offsetUp, "/\\__/ / |_) | (_| | (_|  __/              ");
          mvaddstr(5, MAXX/2-offsetUp, "\\____/| .__/ \\__,_|\\___\\___|              ");
          mvaddstr(6, MAXX/2-offsetUp, "      | |                                 ");
          mvaddstr(7, MAXX/2-offsetUp, "      |_|                                 ");
          mvaddstr(MAXY-7, MAXX/2-offsetDown, " _____                    _               ");
          mvaddstr(MAXY-6, MAXX/2-offsetDown, "|_   _|                  | |              ");
          mvaddstr(MAXY-5, MAXX/2-offsetDown, "  | | _ ____   ____ _  __| | ___ _ __ ___ ");
          mvaddstr(MAXY-4, MAXX/2-offsetDown, "  | || '_ \\ \\ / / _` |/ _` |/ _ \\ '__/ __|");
          mvaddstr(MAXY-3, MAXX/2-offsetDown, " _| || | | \\ V / (_| | (_| |  __/ |  \\__ \\");
          mvaddstr(MAXY-2, MAXX/2-offsetDown, " \\___/_| |_|\\_/ \\__,_|\\__,_|\\___|_|  |___/");
        }

        void block_init(){
          int x = COLS/4;
          for (int i = 0; i < x ; i+= x){
            Block[i].x = x * i ;
            Block[i].y = MAXY - 6;
            Block[i].oldX = Block[i].x;
            Block[i].oldY = Block[i].y;
            Block[i].c = 234;
            Block[i].life = 1;
          }
        }

        void block_print(){
          int x = COLS/4;
          for (int i = 0; i < x ; ++i){
            Block[i].oldX = Block[i].x;
            Block[i].oldY = Block[i].y;
            Block[i].x = x * i ;
            Block[i].y = MAXY - 6;
            Block[i].c = 234;
            mvaddch(Block[i].oldY, Block[i].oldX, ' ');
            mvaddch(Block[i].y, Block[i].x, Block[i].c);
          }
        }

        void viewScore(){

          FILE *f = fopen("scoreLog.txt", "w");
          if (f == NULL)
          {
              printf("Error opening file!\n");
              exit(1);
          }

          fprintf(f, "%d", score);

          fclose(f);

        }

        void printScore(){

          // FILE *fp;
          //  char ch;
          //  /*Open file in read mode*/
          //  fp= fopen ('example.txt', 'r');
          //  while( (ch = getc(fp)) != EOF) {
          //   /*getc() function reads a character and its value is stored in variable 'ch' until EOF is encountered*/
          //   printf('%ch', ch);
          //  }
          //
          //   fclose(fp);

        }
