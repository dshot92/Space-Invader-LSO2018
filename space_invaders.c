#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define STEP 1
#define MAXX COLS
#define MAXY LINES
#define RIGHT 67
#define LEFT 68
#define DELAY 30000
#define ENEMIES 26
#define SPRITE 3
#define SPRITE_X 13
#define SPRITE_Y 9
#define LIFE 6
#define MAX_STRING 20

#define RED 1
#define GREEN 2
#define YELLOW  3

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_Bul = PTHREAD_MUTEX_INITIALIZER;

int done = 1;
int counter = 0;
char command;
int score=0;

char hearts[LIFE][MAX_STRING] = {"LIVES = @",
"LIVES = @@",
"LIVES = @@@",
"LIVES = @@@@",
"LIVES = @@@@@",
"LIVES = @@@@@@"};

typedef struct {
  int x;
  int y;
  int life;
  char lifes[LIFE][LIFE];
  char sprite[SPRITE_X*SPRITE_Y];
  _Bool dir;
  char c;
  _Bool updated;
  _Bool cooldown;

}Object;

Object Ship;
Object Invader[ENEMIES];
Object Bullet[2];

void *printShip(void *n);

void *printInvader(void *n);

void *control(void *n);

void *getCommand(void *n);

void *bullet(void *m);

void printSprite(Object ship);
void printSpriteAccurate(Object ship);
void deleteSprite(Object ship);
_Bool noLife();
int EnemyWin();
void printLife();
void GameOver();
void removeBullet();
void printBullet();
void bounce(int i);
void remove_Invader();
void print_Invader();
void collisions();
void gameScore();
void intro();

int main(){

  pthread_t tid1, tid2, tid3, tid4;
  int n1 = 1, n2 = 2, n3 = 3, n4 = 4;
  int i = 0;

  srand(time(NULL));

  initscr();
  noecho();
  curs_set(0);

  initscr();			/* Start curses mode 		*/
  if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }

  start_color();			/* Start color 			*/

  init_pair(YELLOW, COLOR_YELLOW ,COLOR_BLACK);
  init_pair(GREEN, COLOR_GREEN ,COLOR_BLACK);
  init_pair(RED, COLOR_RED ,COLOR_BLACK);

  clock_t start, end;
  double cpu_time_used;

  double elapesed_time = 0;

  start = clock();

  pthread_create(&tid4, NULL, getCommand, (void *)&n4);
  intro();

  pthread_create(&tid1, NULL, printShip, (void *)&n1);
  pthread_create(&tid2, NULL, printInvader, (void *)&n2);
  pthread_create(&tid3, NULL, control, (void *)&n3);

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  while(1);
  // while(!noLife()/* && !EnemyWin()*/){}; //TODO REMOVE THIS

  endwin();

  return 0;
}

void *printShip(void *n){
  int i, m1 = 0, m2 = 1;

  pthread_t bul;

  Ship.y = MAXY-SPRITE;
  Ship.x = MAXX/2;
  Ship.life = LIFE;
  strcpy(Ship.sprite, " A / \\---");
  // strcpy(Ship.sprite, "      Ç           ÇÇÇ      ÇÇÇÇÇÇÇÇÇÇÇ ÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇÇ");

  for(i = 0; i < LIFE ; i++){
    Ship.lifes[i][LIFE] = i;
  }

  while(1) {
    pthread_mutex_lock(&lock);

    if (done != (int)*(int*)n) {

      if ((int)*(int*)n == 1) {
        pthread_cond_wait(&cond1, &lock);
      } else if ((int)*(int*)n == 2) {
        pthread_cond_wait(&cond2, &lock);
      }
      else {
        pthread_cond_wait(&cond3, &lock);
      }
    }

    if(!Ship.updated){

      if(command == 'd' && Ship.x + 3 < MAXX-1){
        strcpy(Ship.sprite, " /|/ |---");
        Ship.x++;
        Ship.updated = true;
      }

      if(command == 'a' && Ship.x  > 0){
        Ship.x--;
        Ship.updated = true;
        strcpy(Ship.sprite, "|\\ | \\---");
      }

      if(command == ' ' && !Ship.cooldown){
        pthread_create(&bul, NULL, bullet, (void *)&m1);
        Ship.cooldown = true;
      }

      if(command == 'l'){
        Ship.life--;
      }


    }

    command = 's';

    counter++;
    // mvaddstr(counter%MAXY,MAXX+1, "          ");
    mvaddstr(MAXY+1,MAXX+1, "    ");
    mvaddstr(MAXY+1,MAXX+1, "SHIP");


    if (done == 3) {
      done = 1;
      pthread_cond_signal(&cond1);
    }
    else if(done == 1) {
      done = 2;
      pthread_cond_signal(&cond2);
    } else if (done == 2) {
      done = 3;
      pthread_cond_signal(&cond3);
    }

    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

void *printInvader(void *n){
  int i, j=0, k=0;

  for(i = 0; i< ENEMIES; i++){
    Invader[i].x = k + 1;
    Invader[i].y = j;
    Invader[i].life = 2;
    Invader[i].c = i+65;
    Invader[i].dir = 0;

    k+=3;
    if((k*2%ENEMIES) == 0){
      k=0;
      j+=2;
    }
  }

  // for(i = 0; i< ENEMIES; i++){
  //   Invader[i].x = (i+rand())%MAXX-2;
  //   Invader[i].y = i;
  //   Invader[i].life = 2;
  //   Invader[i].c = i+65;
  //
  //   // Invader[i].dir = rand()%2;
  //   // Invader[i].dir = 0;
  //   Invader[i].dir = i%2;
  // }

  int dx = STEP;

  while(1) {

    pthread_mutex_lock(&lock);

    if (done != (int)*(int*)n) {

      if ((int)*(int*)n == 1) {
        pthread_cond_wait(&cond1, &lock);
      } else if ((int)*(int*)n == 2) {
        pthread_cond_wait(&cond2, &lock);
      }
      else {
        pthread_cond_wait(&cond3, &lock);
      }
    }

    // bounce();
    for(i=0;i<ENEMIES;i++){
      bounce(i);
      if(Invader[i].life){
        if(Invader[i].x + dx > MAXX-1 || Invader[i].x +dx < 1){
          Invader[i].y++;
          Invader[i].dir = !Invader[i].dir;
        }
        if(Invader[i].dir)
        Invader[i].x -= dx;
        else
        Invader[i].x += dx;
      }
    }


    // counter++;
    // mvaddstr(counter%MAXY,MAXX+1, "          ");
    mvaddstr(MAXY+2,MAXX+1, "        ");
    mvaddstr(MAXY+2,MAXX+1, "INVADER");



    usleep(DELAY);

    if (done == 3) {
      done = 1;
      pthread_cond_signal(&cond1);
    }
    else if(done == 1) {
      done = 2;
      pthread_cond_signal(&cond2);
    } else if (done == 2) {
      done = 3;
      pthread_cond_signal(&cond3);
    }

    pthread_mutex_unlock(&lock);
  }

  return NULL;
}

void bounce(int i){
  int j;
  for( j = 0 ; j < ENEMIES ; j++ ){
    if(Invader[i].y == Invader[j].y)
    // if((Invader[i].dir !=  Invader[j].dir))
    if(Invader[i].x-1 == Invader[j].x || Invader[i].x+1 == Invader[j].x){
      Invader[i].dir = !Invader[i].dir;
      Invader[j].dir = !Invader[j].dir;
    }
  }
}

void *control(void *n){

  int i;
  mvaddstr(MAXY+1, 0, "Life: ");
  clear();

  while(1) {

    pthread_mutex_lock(&lock);

    clear();
    // deleteSprite(Ship);
    // removeBullet();
    // remove_Invader();


    if (done != (int)*(int*)n) {
      if ((int)*(int*)n == 1) {
        pthread_cond_wait(&cond1, &lock);
      } else if ((int)*(int*)n == 2) {
        pthread_cond_wait(&cond2, &lock);
      }else {
        pthread_cond_wait(&cond3, &lock);
      }
    }

    // mvaddch(Ship.y, Ship.x, Ship.c);
    print_Invader();
    printSprite(Ship);
    printBullet();
    collisions();
    printLife();
    // printSpriteAccurate(Ship);
    Ship.updated = false;

    counter++;
    mvaddstr(MAXY,MAXX+1, "       ");
    mvaddstr(MAXY,MAXX+1, "CONTROL");

    // elapesed_time++;
    gameScore();
    refresh();

    usleep(DELAY);

    if (done == 3) {
      done = 1;
      pthread_cond_signal(&cond1);
    }
    else if(done == 1) {
      done = 2;
      pthread_cond_signal(&cond2);
    } else if (done == 2) {
      done = 3;
      pthread_cond_signal(&cond3);
    }
    pthread_mutex_unlock(&lock);
    pthread_mutex_unlock(&lock_Bul);

  }
  return NULL;
}


void *getCommand(void *n){
  command = -1;
  clear();
  int i = 0;
  while(1){
    if(!Ship.updated){
      command = getchar();
      i%=MAXY;
      i++;
    }
  }
}

void printSprite(Object ship){
  int i,j, index=0;
  attron(COLOR_PAIR(YELLOW));
  for (i = 0; i < SPRITE; i++){
    for (j = 0; j < SPRITE; j++){
      mvaddch(ship.y+i, ship.x+j, ship.sprite[index]);
      index++;
    }
  }
  attroff(COLOR_PAIR(YELLOW));
  strcpy(Ship.sprite, " A / \\---");
}

void printSpriteAccurate(Object ship){
  int i,j, index=0;
  for (i = 0; i < SPRITE_Y; i++){
    for (j = 0; j < SPRITE_X; j++){
      mvaddch(ship.y+i, ship.x+j, ship.sprite[index]);
      index++;
    }
  }
  strcpy(Ship.sprite, " A / \\---");
}

void deleteSprite(Object ship){
  int i,j;
  for (i = 0; i < SPRITE; i++){
    for (j = 0; j < SPRITE; j++){
      mvaddch(ship.y+i, ship.x+j, ' ');
    }
  }
}

_Bool noLife(){
  if(Ship.life == 0){
    return true;
  }else{
    return false;
  }
}

int EnemyWin(){
  int i, status=0;
  for(i=0;i<ENEMIES;i++){
    if(Invader[i].y == MAXY){
      status++;
    }
  }
  return status;
}

void printLife(){
  mvaddstr(MAXY+1,0, "              ");
  mvaddstr(MAXY+1,0, hearts[Ship.life-1]);
}

void GameOver(){
  mvaddstr(MAXY/2,MAXX/2 -4, "YOU LOST");
  mvaddstr(MAXY/2 +1,MAXX/2 -5, "GAME OVER");
}


void *bullet(void *m){

  int i = (int)*(int*)m;

  for(i=0;i<2;i++){
    Bullet[i].y = Ship.y-1;
    Bullet[i].c = '*';
    Bullet[i].life = 1;
    Bullet[i].dir = (_Bool)i;
    if (i){
      Bullet[1].x = Ship.x;
    }else{
      Bullet[0].x = Ship.x+2;
    }
  }

  while(Bullet[0].y > -1 || Bullet[1].y > -1){
    if(done==2){
      for(i=0;i<2;i++){
        // if(Bullet[i].x  != -1){

        if(Bullet[i].x > MAXX || Bullet[i].x < 0){

          Bullet[i].x = -1;
          Bullet[i].y = -1;
        }else{

          if(i){
            Bullet[i].x--;
          }
          else{
            Bullet[i].x++;
          }

          Bullet[i].y--;
        }
      }
      usleep(DELAY*2);
      // }
    }
  }
  Ship.cooldown = false;
}

void print_Invader(){
  int i;
  for (i = 0; i < ENEMIES; i++){
    if(Invader[i].life){
      attron(COLOR_PAIR(Invader[i].life));
      mvaddch(Invader[i].y, Invader[i].x, Invader[i].c);
      attroff(COLOR_PAIR(Invader[i].life));
    }
  }
}

void remove_Invader(){
  int i;
  for (i = 0; i < ENEMIES; i++){
    mvaddch(Invader[i].y, Invader[i].x, ' ');
  }
}

void printBullet(){
  int i;
  for (i = 0; i < 2; i++){
    if(Bullet[i].life)
    mvaddch(Bullet[i].y, Bullet[i].x, Bullet[i].c);
  }
}

void removeBullet(){
  int i;
  for (i = 0; i < 2; i++){
    mvaddch(Bullet[i].y, Bullet[i].x, ' ');
  }
}

void collisions(){

  int i = 2; //NUMEBER OF BULLETS;
  int j;

  for(i = 0; i < 2; i++){ //cycle for each Bullet
    for(j = ENEMIES-1; j >= 0; j--){
      if((Bullet[i].x-1 == Invader[j].x || Bullet[i].x+1 == Invader[j].x || Bullet[i].x == Invader[j].x) && (Bullet[i].y-1 == Invader[j].y || Bullet[i].y+1 == Invader[j].y || Bullet[i].y == Invader[j].y)){
        Invader[j].life--;
        if(Invader[j].life == 0){
          Invader[j].x = -1;
          Invader[j].y = -1;
          score++;
        }
        Bullet[i].y = -2;
        Bullet[i].y = -2;
        score++;
      }
    }
  }
}

void gameScore(){

  mvaddstr(0, 1, "SCORE:");
  mvprintw(0, 11, "%d", score);

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
    // mvaddstr((LINES/2) , (COLS/2) , "                                                                ");
    mvaddstr((LINES/2) , (COLS/2- strlen(intro[k])/2) , intro[k]);
    k++;
    refresh();
    usleep(1500000);
  }
  attroff(COLOR_PAIR(YELLOW));

  attron(COLOR_PAIR(GREEN));

  do{

    mvaddstr(((LINES/2) +4) , (COLS/2 - (strlen("--Press SPACE to start--")/2)) , "                              ");

    refresh();

    usleep(200000);

    mvaddstr(((LINES/2) +4) , (COLS/2 - (strlen("--Press SPACE to start--")/2)) , "--Press SPACE to start--");

    refresh();

    usleep(200000);

  }while(command != ' ');
  attroff(COLOR_PAIR(GREEN));


  command = 's'; // clear buffer, so that ship wont fire immediately

}
