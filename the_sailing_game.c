/*The Sailing Game. Made by Chris Ioakimidis and Matthaios Giannopapas.*/
#include <curses.h>
#include <windows.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#define BOARD_HEIGHT 35
#define BOARD_WIDTH 167
#define PL_MAX 4

/*This basic structure is used a lot in the program, and describes a point in the terminal. 0,0 is the top left corner
and the y/x axes are increasing as down and right.*/
typedef struct point{
        int x;
        int y;
} MYPOINT;

void init_island(WINDOW *island_win,int start_y,MYPOINT *coll);
/*This function produces the main map, the island, taking as arguments the window in which the island is drown on, the starting point in the terminal
and an array of points called 'coll'. Every point in the map in which there is a rock (painter gray) is stored in this array*/
void boarder_row(WINDOW *island_win,int row,int rock,int space,int max_x,MYPOINT *coll,int *cnt);
/*The first function used to make the island. Besides the 'canvas' window, the values of 'rock' and 'space' are modified to create a row
in the map with x space (meaning sea) and y rocks. Max x serves as the boarder limit.*/
void island_row(WINDOW *island_win,int row,int forest,int rocks,int max_x,MYPOINT *coll,int *cnt);
/*Very similar to the board function, but instead of drawing on the outsides of the map, it creates rows in the middle, for the main island*/
void boat(WINDOW *boat_win,MYPOINT center_point,int dir,int color);
/*The very basic function of drawing a boat, in a square, using the corresponding color pair 'color'. The arguments 'center point' and 'dir'
determine the boats center and direction. For the whole program, direction is matched with the numbers 1-4 so that:
1=Right, 2=Up, 3=Left and 4=Down*/
void del_boat(WINDOW *boat_win,MYPOINT center_point,int dir);
/*This function erases the boat, and takes the same arguments are the 'boat' function, besides the color which is not needed*/
void move_boat(WINDOW *boat_win,MYPOINT *start_point,int *cur_dir,int dir_2,int color);
/*With a combination of the 2 above functions, this function moves a boat in a certain direction (moving the center point by 1 pixel) and
updating the new start point. Start point is the starting center of the boat, 'cur_dir' is the current direction of the boat (which may be
changed in the function)  and 'dir_2' is the direction that the boat will move to. The color is self explanatory.*/
void move_boats_to(WINDOW *game_win,int players,MYPOINT *start,MYPOINT *end,int *cur_dirs,MYPOINT *coll,int *life,int *winner);
/*The main big function of the game routine. It takes as arguments the arrays 'start','end','cur_dirs' to perform the movements using the 3
functions above, as well as the collision array that was created in the 'init_island' function. The arrays 'life' and 'winner' take values
0,1 to determine the life status of a player or if they have won the game of not.(The winner array is required to accept the possibility of a tie.)
This function, besides executing the players' desired movement, checks continuously for collisions and modifies the 'damaged' array,used by
the function below.*/
void death_anim(WINDOW *game_win,int players,MYPOINT *cur_point,int *cur_dir,int *damaged,int *life);
/*Using the 'damaged array' which is altered in the above function, the damaged players are recognized and the death/sinking animation is
shown. Then, the life status of the players is updated.*/
void paths_set(WINDOW *game_win,int players,MYPOINT *cur_points,MYPOINT *end_points,int *cur_dir,MYPOINT *coll,int *life);
/*The 2nd piece of the game routine. Using a black imaginary boat, the players choose their desired location. The imaginary boat starts from
the player's boat and moves freely. It's reset when it collides with rocks, preventing the players from suiciding.*/
void wind_print(WINDOW *wind_win,int dir);
/*The indicator of the wind direction is updated each round using this function. The wind_win is the window in the top right corner of the interface.*/
void menu(WINDOW **menu_win,char *choices[],int ch_num,int text_space,int *fin_choice);
/*This function takes as arguments an array of window pointers 'menu_win', an array of strings 'choises', the number of choices that are presented
('ch_num'), the space of each choice window ('Text space'), and a pointer to the final choice ('fin_choice'). A menu of windows is created and the
user can switch between the choices with the arrow keys, then select one using ENTER.*/
void win_blinking(WINDOW *win,bool check,int height,int width);
/*This function is used in the menu function, creating a blinking animation at the highlighted choice, If check is true. The 'height' and
'width' parameters define the windows's size.*/

int main()
{
    /*Game variables*/
    MYPOINT boats[PL_MAX],end_points[PL_MAX];
    WINDOW *game_win,*wind_win,*menu_win[PL_MAX];
    MYPOINT collision[1220];
    int i,j,wind,difficulty,players;
    int cur_dirs[PL_MAX]={1,1,1,1};
    /*Status / Winner checking variables*/
    int lives[PL_MAX]={1,1,1,1};
    int winner[PL_MAX]={0,0,0,0};
    int winner_cnt,winner_num[PL_MAX],death_cnt,round;
    /*Menus Variables*/
    int max_y,max_x,choice;
    char *choices1[2]={"PLAY","RULES"};
    char *choices2[4]={"1 PLAYER","2 PLAYERS","3 PLAYERS","4 PLAYERS"};
    char *choices3[3]={"EASY","MEDIUM","HARD"};
    /*The terminal is maximized*/
    ShowWindow(GetConsoleWindow(),SW_MAXIMIZE);
    getmaxyx(stdscr,max_y,max_x);
    /*With the following settings, no user input is shown on the terminal,
    the cursor is hidden and colors can be used.*/
    initscr();
    raw();
    noecho();
    start_color();
    curs_set(0);
    srand(time(0));
    /*Definition of each pair*/
    init_pair(1,COLOR_RED,COLOR_CYAN);/*Player 1*/
    init_pair(2,COLOR_YELLOW,COLOR_CYAN);/*Player 2*/
    init_pair(3,COLOR_MAGENTA,COLOR_CYAN);/*Player 3*/
    init_pair(4,COLOR_WHITE,COLOR_CYAN);/*Player 4*/
    init_pair(5,COLOR_BLACK,COLOR_CYAN);/*Test boat / Crashed boat*/
    init_pair(6,COLOR_BLUE,COLOR_CYAN); /*Deep Sea*/
    init_pair(7,COLOR_CYAN,COLOR_CYAN); /*Sea*/
    /*These color pairs are used to create the map*/
    init_pair(11,COLOR_CYAN,COLOR_CYAN);/*Sea (island part)*/
    init_pair(12,8,COLOR_BLUE); /*Grey and Yellow*/
    init_pair(13,8,COLOR_GREEN);/*Grey and Green*/
    keypad(stdscr,TRUE); /*Arrow keys can be read*/
    /*Menu and player/difficulty choices WIP*/
    refresh();
    mvprintw(10,65,"WELCOME TO THE SAILING GAME!");
    /*Main menu*/
    while(1)
    {
        refresh();
        menu(menu_win,choices1,2,5,&choice);
        erase();
        if(choice==1)
        {
            /*Players menu*/
            menu(menu_win,choices2,4,9,&choice);
            players=choice;
            erase();
            /*Difficulty menu*/
            menu(menu_win,choices3,3,6,&choice);
            difficulty=choice;
            erase();
            break;
        }
        else
        {
            /*Rules / How to play.*/
            mvprintw(18,30,"The goal of each player is to make a full lap of the island.");
            mvprintw(19,30,"Each player can choose which way to loop the island, just be careful of crashing with your opponents!");
            mvprintw(21,30,"CONTROLS:");
            mvprintw(22,30,"Each round, move your test boat with the arrow keys, then press SPACE to lock your choice.");
            mvprintw(23,30,"After all players have made their choices, the boats will move in a straight line to their desired locations");
            mvprintw(25,30,"BE CAREFUL OF THE WIND!!! It is changing every round and will affect your end points! Don't get too greedy!");
            mvprintw(27,30,"Press any key to return to the main menu.");
            getch();
            erase();
            fflush(stdin);
        }
    }
    /*Since 'players' and 'difficulty' are chosen, the game begins.*/
    mvprintw(22,65,"Press any key to begin");
    refresh();
    getch();
    /*Initialization of the island and game interface*/
    game_win=newwin(BOARD_HEIGHT,BOARD_WIDTH,9,0);
    wind_win=newwin(5,7,2,BOARD_WIDTH-13);
    keypad(game_win,TRUE);
    keypad(wind_win,TRUE);
    init_island(game_win,9,collision);
    box(wind_win,0,0);
    wrefresh(wind_win);
    mvprintw(7,BOARD_WIDTH-17,"Wind Direction");
    refresh();
    /*Set the Boat starting points.*/
    if(players==1)
    {
        boats[0].x=end_points[0].x=BOARD_WIDTH/2;
        boats[0].y=end_points[0].y=BOARD_HEIGHT-7;
        boat(game_win,boats[0],cur_dirs[0],1);
    }
    else
        for(i=0;i<players;i++)
        {
            boats[i].x=end_points[i].x=BOARD_WIDTH/2;
            boats[i].y=end_points[i].y=BOARD_HEIGHT-2-(11-2*players)*i;
            boat(game_win,boats[i],cur_dirs[i],i+1);
        }
    wrefresh(game_win);
    wind=1+rand()%4; /*Random initialization of the wind direction*/
    winner_cnt=0;
    round=1;
    /*Main game routine.*/
    while(1)
    {
        mvprintw(0,BOARD_WIDTH/2-2,"ROUND %d",round);
        refresh();
        death_cnt=0;
        /*Players choose their path*/
        paths_set(game_win,players,boats,end_points,cur_dirs,collision,lives);
        /*Wind affects the end points*/
        wind=(7+wind+(int)pow(-1,1+rand()%2)*((1+rand()%difficulty)))%4+1;
        switch(wind)
        {
        case 1:
            for(i=0;i<players;i++)
                end_points[i].x+=difficulty*4;
            break;
        case 2:
            for(i=0;i<players;i++)
                end_points[i].y-=difficulty*2;
            break;
        case 3:
            for(i=0;i<players;i++)
                end_points[i].x-=difficulty*4;
            break;
        case 4:
            for(i=0;i<players;i++)
                end_points[i].y+=difficulty*2;
            if(end_points[i].y>=BOARD_HEIGHT-1)
                end_points[i].y=BOARD_HEIGHT-2;
            break;
        }
        /*The wind indicator is updated*/
        wind_print(wind_win,wind);
        /*Movement Happens*/
        move_boats_to(game_win,players,boats,end_points,cur_dirs,collision,lives,winner);
        /*Winner/Death Check*/
        for(i=0;i<players;i++)
        {
            if(winner[i]==1)
                winner_cnt++;
            if(lives[i]==0)
                death_cnt++;
        }
        /*The loop/round ends if one player wins or all die*/
        if(winner_cnt>0||death_cnt==players)
            break;
    }
    /*Clears the interface above the island to display the ending messages.*/
    for(i=0;i<8;i++)
        for(j=0;j<BOARD_WIDTH-20;j++)
            mvprintw(i,j," ");
    /*Message if all die.*/
    if(winner_cnt==0)
        mvprintw(3,55,"All of you have lost, shame on you...");
    /*Winner Messages*/
    else if(winner_cnt==1)
    {
        for(i=0;i<players;i++)
            if(winner[i]==1)
                winner_num[0]=i+1;
        mvprintw(3,55,"PLAYER %d IS THE WINNER!!!",winner_num[0]);
    }
    /*Tie messages*/
    else
    {
        int j=0;
        for(i=0;i<players;i++)
        {
            if(winner[i]==1)
            {
                winner_num[j]=i+1;
                j++;
            }
        }
        mvprintw(3,55,"Wow, there was a die between %d players!",winner_cnt);
        mvprintw(4,55,"Congratulations to players");
        for(i=0;i<winner_cnt;i++)
                printw(" %d,",i+1);
        mvprintw(5,55,"You are not losers, lul.");
    }
    refresh();
    getch();
    erase();
    /*Ending messages*/
    mvprintw(BOARD_HEIGHT/2,BOARD_WIDTH/2-7,"THANKS FOR PLAYING THE SAILING GAME!");
    mvprintw(0,0,"Made by Chris Ioakimidis and Matthaios Giannopappas");
    mvprintw(BOARD_HEIGHT/2+10,0,"Press any key to exit.");
    refresh();
    getch();
    endwin();
    return 0;
}

void init_island(WINDOW *island_win,int start_y,MYPOINT *coll)
{
    int i,j,cur_row;
    int max_x=BOARD_WIDTH;
    int max_y=BOARD_HEIGHT;
    int cnt=0;
    refresh();
    /*Initialization of the sea layer*/
    wattron(island_win,COLOR_PAIR(11));
    for(i=0;i<BOARD_HEIGHT;i++)
    {
        for(j=0;j<max_x;j++)
            wprintw(island_win," ");
    }
    wattroff(island_win,COLOR_PAIR(11));
    wrefresh(island_win);
    /*Initialization of the boarder rocks*/
    wattron(island_win,COLOR_PAIR(12)); /*Just sand*/
    for(i=0;i<2;++i)
        boarder_row(island_win,i,0,0,max_x,coll,&cnt);
    cur_row=i;
    /*The first curve on the top of the boarder*/
    for(i=0;i<8;i++)
    {
        if(i==0)
            boarder_row(island_win,cur_row+i,15+5*(int)sqrt(10*(i+1)),0,max_x,coll,&cnt);
        boarder_row(island_win,cur_row+i,15+5*(int)sqrt(10*(i+1)),10+10*(int)sqrt(10*i),max_x,coll,&cnt);
    }
    cur_row+=i;
    /*The side boarders*/
    for(i=0;i<18;i++)
        boarder_row(island_win,cur_row+i-1,1+i%4,max_x/2-i%4,max_x,coll,&cnt);
    cur_row+=i;
    /*Down Boarder*/
    for(i=0;i<max_y-cur_row+1;i++)
        boarder_row(island_win,cur_row+i-1,6+3*i*i,max_x/2-4-i*i,max_x,coll,&cnt);
    wattroff(island_win,COLOR_PAIR(12));
    wattron(island_win,COLOR_PAIR(13));
    /*Main Island*/
        island_row(island_win,10,0,20,max_x,coll,&cnt);
        for(i=0;i<3;i++)
            island_row(island_win,11+i,10+4*(int)sqrt(10*i),15-(int)sqrt(i),max_x,coll,&cnt);
        cur_row=11+i;

        for(j=0;j<2;j++)
            island_row(island_win,cur_row+j,10+4*(int)sqrt(10*i),15-(int)sqrt(i),max_x,coll,&cnt);
        cur_row+=j;
        for(j=0;j<7;j++)
            island_row(island_win,cur_row+j,10+4*(int)sqrt(10*i)-j*j,15-(int)sqrt(i)+j,max_x,coll,&cnt);
        cur_row+=j;

    wattroff(island_win,COLOR_PAIR(13));
    wrefresh(island_win);
    refresh();
}

void boarder_row(WINDOW *island_win,int row,int rock, int space,int max_x,MYPOINT *coll,int *cnt)
{
    int f_cnt,y,x;
    for(int i=0;i<max_x;++i)
    {
        if(i<=max_x/2-space-rock||i>=max_x/2+space+rock)
            mvwprintw(island_win,row,i," ");
        else if(i>max_x/2-space&&i<max_x/2+space);
        else
        {
            mvwprintw(island_win,row,i,"%c",219);
            f_cnt=*cnt;
            getyx(island_win,y,x);
            coll[f_cnt].y=y;
            coll[f_cnt].x=x;
            (*cnt)++;
        }
    }
}

void island_row(WINDOW *island_win,int row,int forest,int rocks,int max_x,MYPOINT *coll,int *cnt)
{
    int start_col=max_x/2-forest-rocks;
    int f_cnt,y,x;
    for(int i=0;i<forest*2+2*rocks;i++)
    {
        if(i<=rocks||i>=forest*2+rocks)
        {
            mvwprintw(island_win,row,start_col+i,"%c",219);
            f_cnt=*cnt;
            getyx(island_win,y,x);
            coll[f_cnt].y=y;
            coll[f_cnt].x=x;
            (*cnt)++;
        }
        else
            mvwprintw(island_win,row,start_col+i," ");
    }
}

void boat(WINDOW *boat_win,MYPOINT center_point,int dir,int color)
{
    wattron(boat_win,COLOR_PAIR(color));
    switch(dir)
    {
        int i,j;
    case 1:
        mvwprintw(boat_win,center_point.y,center_point.x-2,"%c",223);
        for(i=0;i<4;i++)
            wprintw(boat_win,"%c",219);
        wprintw(boat_win,"%c",220);
        mvwprintw(boat_win,center_point.y+1,center_point.x-2,"%c",220);
        for(i=0;i<4;i++)
            wprintw(boat_win,"%c",219);
        wprintw(boat_win,"%c",223);
        break;
    case 2:
        mvwprintw(boat_win,center_point.y-1,center_point.x-1,"%c",220);
        for(i=0;i<2;i++)
            wprintw(boat_win,"%c",219);
        wprintw(boat_win,"%c",220);
        for(i=0;i<4;i++)
            mvwprintw(boat_win,center_point.y,center_point.x-1+i,"%c",219);
        mvwprintw(boat_win,center_point.y+1,center_point.x-1,"%c",219);
        for(i=0;i<2;i++)
            wprintw(boat_win,"%c",223);
        wprintw(boat_win,"%c",219);
        break;
    case 3:
        mvwprintw(boat_win,center_point.y,center_point.x-2,"%c",220);
        for(i=0;i<4;i++)
            wprintw(boat_win,"%c",219);
        wprintw(boat_win,"%c",223);
        mvwprintw(boat_win,center_point.y+1,center_point.x-2,"%c",223);
        for(i=0;i<4;i++)
            wprintw(boat_win,"%c",219);
        wprintw(boat_win,"%c",220);
        break;
    case 4:
        for(i=0;i<2;i++)
            mvwprintw(boat_win,center_point.y-1,center_point.x-1+3*i,"%c",219);
        for(i=0;i<2;i++)
            mvwprintw(boat_win,center_point.y-1,center_point.x+i,"%c",220);
        for(i=0;i<4;i++)
            mvwprintw(boat_win,center_point.y,center_point.x-1+i,"%c",219);
        for(i=0;i<2;i++)
            mvwprintw(boat_win,center_point.y+1,center_point.x-1+3*i,"%c",223);
        mvwprintw(boat_win,center_point.y+1,center_point.x,"%c%c",219,219);
    }
    wattroff(boat_win,COLOR_PAIR(color));
}

void del_boat(WINDOW *boat_win,MYPOINT center_point,int dir)
{
    wattron(boat_win,COLOR_PAIR(1));
    int i,j;
    switch(dir)
    {
    case 1:
    case 3:
        for(i=0;i<2;i++)
            for(j=0;j<6;j++)
                mvwprintw(boat_win,center_point.y+i,center_point.x-2+j," ");
        break;
    case 2:
    case 4:
        for(i=0;i<3;i++)
            for(j=0;j<4;j++)
                mvwprintw(boat_win,center_point.y-1+i,center_point.x-1+j," ");
        break;
    }
    wattroff(boat_win,COLOR_PAIR(1));
}

void move_boat(WINDOW *boat_win,MYPOINT *start_point,int *cur_dir,int dir_end,int color)
{
    del_boat(boat_win,*start_point,*cur_dir);
    switch(dir_end)
    {
    case 1:
        start_point->x++;
        break;
    case 2:
        start_point->y--;
        break;
    case 3:
        start_point->x--;
        break;
    case 4:
        start_point->y++;
        break;
    }
    *cur_dir=dir_end;
    boat(boat_win,*start_point,*cur_dir,color);
    wrefresh(boat_win);
}

void move_boats_to(WINDOW *game_win,int players,MYPOINT *start,MYPOINT *end,int *cur_dirs,MYPOINT *coll,int *life,int *winner)
{
    /*Counter variables*/
    int i,j,k;
    /*Movement/Collision Check variables*/
    int step=0,full_check=0;
    int check[PL_MAX],damaged[PL_MAX];
    int tot_pace[PL_MAX]={0,0,0,0,};
    MYPOINT dist[PL_MAX],dir[PL_MAX],pace[PL_MAX],coll_check;
    /*Round/Winner Check variables*/
    static int rounds=1;
    static int winner_counter=0;
    static int circle_dir[PL_MAX]={2,2,2,2};
    static int path_found[PL_MAX]={0,0,0,0};
    /*Show each player's boat status (alive or dead)*/
    for(i=0;i<players;i++)
    {
        if(life[i]==1)
            mvprintw(i,0,"Player %d: Alive",i+1);
        else
            mvprintw(i,0,"Player %d: Dead",i+1);
    }
    /*Neutralization of the collision check vars*/
    for(j=0;j<players;j++)
        damaged[j]=0;
    for(i=0;i<players;i++)
        check[i]=0;
    /*Figuring out each general path*/
    for(i=0;i<players;i++)
    {
        dist[i].x=end[i].x-(start[i].x);
        dist[i].y=end[i].y-(start[i].y);
        if(dist[i].x>=0)
            dir[i].x=1;
        else
            dir[i].x=3;
        if(dist[i].y>=0)
            dir[i].y=4;
        else
            dir[i].y=2;
    }
    /*Final determination of the path*/
    for(i=0;i<players;i++)
    {
        if(dist[i].x==0)
        {
            pace[i].x=0;
            pace[i].y=1;
            tot_pace[i]=1;
        }
        else if(dist[i].y==0)
        {
            pace[i].x=1;
            pace[i].y=0;
            tot_pace[i]=1;
        }
        else if(abs(dist[i].x/(2*dist[i].y))<1)
        {
            pace[i].y=abs((dist[i].y*2)/dist[i].x);
            pace[i].x=1;
            tot_pace[i]=pace[i].x+pace[i].y;
        }
        else
        {
            pace[i].x=2*abs(dist[i].x/(2*dist[i].y));
            pace[i].y=1;
            tot_pace[i]=pace[i].x+pace[i].y;
        }
    }
    /*Movement and continuous collision check*/
    while(1)
    {
        full_check=0;
        for(i=0;i<players;i++)
        {
            /*Actual Movement*/
            if(life[i]==0)
                check[i]=1;
            else if((end[i].x==start[i].x)&&(end[i].y==start[i].y))
                check[i]=1;
            else if((start[i].x==end[i].x))
                move_boat(game_win,&start[i],&cur_dirs[i],dir[i].y,i+1);
            else if((start[i].y==end[i].y))
                move_boat(game_win,&start[i],&cur_dirs[i],dir[i].x,i+1);
            else if((step%tot_pace[i])<pace[i].x)
                move_boat(game_win,&start[i],&cur_dirs[i],dir[i].x,i+1);
            else
                move_boat(game_win,&start[i],&cur_dirs[i],dir[i].y,i+1);
            /*Collision check, between boats and rocks*/
            if(life[i]==0);
            else for(j=0;j<1220;j++)
            {
                coll_check.x=start[i].x-coll[j].x;
                coll_check.y=start[i].y-coll[j].y;
                if(cur_dirs[i]%2!=0&&coll_check.x>=-4&&coll_check.x<=2&&coll_check.y>=-1&&coll_check.y<=0)
                    {
                        check[i]=1;
                        damaged[i]=1;
                        coll[j].x=coll[j].y=0;
                        break;
                    }
                if(cur_dirs[i]%2==0&&coll_check.x>=-2&&coll_check.x<=1&&abs(coll_check.y)<=1)
                    {
                        check[i]=1;
                        damaged[i]=1;
                        coll[j].x=coll[j].y=0;
                        break;
                    }
            }
            /*Collision check, between boats*/
            if(life[i]==0);
            else for(j=0;j<players;j++)
            {
                coll_check.x=abs(start[i].x-start[j].x);
                coll_check.y=abs(start[i].y-start[j].y);

                if(j==i || life[j]==0);
                /*If the 2 objects are both horizontal/vertical */
                else if(life[j]!=0&&(cur_dirs[i]+cur_dirs[j])%2==0)
                {
                    if(cur_dirs[i]%2==0&&(coll_check.x<=3&&coll_check.y<=2))
                    {
                        check[i]=check[j]=1;
                        damaged[i]=damaged[j]=1;
                    }
                    if(life[j]!=0&&cur_dirs[j]%2!=0&&(coll_check.x<=6&&coll_check.y<=2))
                    {
                        check[i]=check[j]=1;
                        damaged[i]=damaged[j]=1;
                    }
                }
                /*If they are in different general directions*/
                else
                    if(life[j]!=0&&coll_check.x<=4&&coll_check.y<=2)
                    {
                        check[i]=check[j]=1;
                        life[i]=life[j]=0;
                        damaged[i]=damaged[j]=1;
                    }
            }
            /*Check for win condition*/
            if((circle_dir[i]==1)&&(path_found[i]==1)&&(start[i].x>BOARD_WIDTH/2)&&(start[i].y>BOARD_HEIGHT/2+3))
            {
                winner[i]=1;
                winner_counter++;
            }
            if((circle_dir[i]==0)&&(path_found[i]==1)&&(start[i].x<BOARD_WIDTH/2)&&(start[i].y>BOARD_HEIGHT/2+3))
            {
                winner[i]=1;
                winner_counter++;
            }
            /*Figuring out if the player chooses to loop the island clockwise=0 or counterclockwise=1*/
            for(j=0;j<players;j++)
            {
                if(path_found[j]==1);
                else if((start[j].x<BOARD_WIDTH/2)&&(start[j].y<(BOARD_HEIGHT/2-3)))
                {
                    circle_dir[j]=0;
                    path_found[j]=1;
                }
                else if((start[j].x>BOARD_WIDTH/2)&&(start[j].y<(BOARD_HEIGHT/2-3)))
                {
                    circle_dir[j]=1;
                    path_found[j]=1;
                }
            }
        }
        death_anim(game_win,players,start,cur_dirs,damaged,life);
        step++;
        Sleep(100);
        wrefresh(game_win);
        /*Check for every boat status*/
        for(i=0;i<players;i++)
            damaged[i]=0;
        for(j=0;j<players;j++)
            if(check[j]==1)
                full_check++;
        refresh();
        if(full_check==players||winner_counter>0)
            break;
        refresh();
    }
    rounds++;
}

void death_anim(WINDOW *game_win,int players,MYPOINT *cur_point,int *cur_dir,int *damaged,int *life)
{
    int i,j;
    int pre_check=0;
    for(j=0;j<players;j++)
            if(damaged[j]==0)
                pre_check++;
    if(pre_check==players)
            return;
    mvprintw(7,BOARD_WIDTH/2-24,"Oh no! Someone was not careful... F in the chat");
    for(i=0;i<10;i++) /*Blinking with black color boats*/
    {
        for(j=0;j<PL_MAX;j++)
            if(life[j]==0)
                continue;
        for(j=0;j<PL_MAX;j++)
            if(damaged[j]==1)
                boat(game_win,cur_point[j],cur_dir[j],5);
        wrefresh(game_win);
        Sleep(75);
        for(j=0;j<PL_MAX;j++)
            if(damaged[j]==1)
                del_boat(game_win,cur_point[j],cur_dir[j]);
        wrefresh(game_win);
        Sleep(50);
    }
    for(i=0;i<PL_MAX;i++) /*Deep blue (sinking)*/
            if(damaged[i]==1)
                boat(game_win,cur_point[i],cur_dir[i],6);
    wrefresh(game_win);
    Sleep(500);
    for(i=0;i<PL_MAX;i++) /*End of animation, boat deletion*/
            if(damaged[i]==1)
                del_boat(game_win,cur_point[i],cur_dir[i]);
    for(i=0;i<PL_MAX;i++) /*Life status update.*/
    {
        if(damaged[i]==1)
            life[i]=0;
        damaged[j]=0;
    }
    getch();
    for(i=0;i<strlen("Oh no! Someone was not careful... F in the chat");i++)
        mvprintw(7,BOARD_WIDTH/2-24+i," ");
}

void paths_set(WINDOW *game_win,int players,MYPOINT *cur_points,MYPOINT *end_points,int *cur_dir,MYPOINT *coll,int *life)
{
    int i,j,choice,buffer_dir,check;
    MYPOINT buffer_point,coll_check;
    for(i=0;i<players;i++)
    {
        check=0;
        buffer_point.x=cur_points[i].x;
        buffer_point.y=cur_points[i].y;
        buffer_dir=cur_dir[i];
        mvprintw(3,BOARD_WIDTH/2-15,"Player %d is choosing their path...",i+1);
        refresh();
        while(1)
        {
            if(life[i]==0)
                break;
            del_boat(game_win,buffer_point,buffer_dir);
            /*User Controlling the test boat (Black)*/
            choice=getch();
            switch(choice)
            {
            case KEY_UP:
                if(buffer_point.y<=2);
                else
                {
                    buffer_point.y--;
                    buffer_dir=2;
                }
                break;
            case KEY_RIGHT:
                if(buffer_point.x>=BOARD_WIDTH-3);
                else
                {
                    buffer_point.x+=2;
                    buffer_dir=1;
                }
                break;
            case KEY_LEFT:
                if(buffer_point.x<=3);
                else
                {
                    buffer_point.x-=2;
                    buffer_dir=3;
                }
                break;
            case KEY_DOWN:
                if(buffer_point.y>=BOARD_HEIGHT-2);
                else
                {
                    buffer_point.y++;
                    buffer_dir=4;
                }
                break;
            case 32:
                end_points[i].x=buffer_point.x;
                end_points[i].y=buffer_point.y;
                check=1;
                break;
            default:
                mvprintw(6,0,"That's not an arrow or space");
            }
            /*The black boat cannot enter the rocks,else it resets in the starting position*/
            for(j=0;j<1220;j++)
            {
                coll_check.x=buffer_point.x-coll[j].x;
                coll_check.y=buffer_point.y-coll[j].y;
                if(buffer_dir%2!=0&&coll_check.x>=-4&&coll_check.x<=2&&coll_check.y>=-1&&coll_check.y<=0)
                    {
                        buffer_point.x=cur_points[i].x;
                        buffer_point.y=cur_points[i].y;
                    }
                if(buffer_dir%2==0&&coll_check.x>=-2&&coll_check.x<=1&&abs(coll_check.y)<=1)
                    {
                        buffer_point.x=cur_points[i].x;
                        buffer_point.y=cur_points[i].y;
                    }
            }
            boat(game_win,buffer_point,buffer_dir,5); /*Prints the black boat*/
            for(j=0;j<players;j++)
                if(life[j]==1)
                    boat(game_win,cur_points[j],cur_dir[j],j+1);
            wrefresh(game_win);
            /*While loops ends when SPACE is entered and the chosen path is locked*/
            if(check==1)
            {
                del_boat(game_win,buffer_point,buffer_dir);
                for(j=0;j<players;j++)
                    if(life[j]==1)
                        boat(game_win,cur_points[j],cur_dir[j],j+1);
                wrefresh(game_win);
                break;
            }
            choice=0;
            fflush(stdin);
        }
    }
}

void wind_print(WINDOW *wind_win,int dir)
{
    int i,j;
    for(i=0;i<3;i++)
        for(j=0;j<5;j++)
            mvwprintw(wind_win,1+i,1+j," ");
    switch(dir)
    {
    case 1:
        for(i=0;i<5;i++)
            mvwprintw(wind_win,2,1+i,"-");
        mvwprintw(wind_win,1,5,"\\");
        mvwprintw(wind_win,3,5,"/");
        break;
    case 2:
        for(i=0;i<3;i++)
            mvwprintw(wind_win,1+i,3,"|");
        for(i=0;i<2;i++)
        {
            mvwprintw(wind_win,2-i,1+i,"/");
            mvwprintw(wind_win,2-i,5-i,"\\");
        }
        break;
    case 3:
        for(i=0;i<5;i++)
            mvwprintw(wind_win,2,1+i,"-");
        mvwprintw(wind_win,1,2,"/");
        mvwprintw(wind_win,3,2,"\\");
        break;
    case 4:
        for(i=0;i<3;i++)
            mvwprintw(wind_win,1+i,3,"|");
        for(i=0;i<2;i++)
        {
            mvwprintw(wind_win,2+i,1+i,"\\");
            mvwprintw(wind_win,2+i,5-i,"/");
        }
        break;
    }
    wrefresh(wind_win);
}

void menu(WINDOW **menu_win,char *choices[],int ch_num,int text_space,int *fin_choice)
{
    int i,max_x,max_y,space,highlight=0,cur_choice;
    getmaxyx(stdscr,max_y,max_x);
    space=(max_x-text_space*ch_num)/(ch_num+1);
    for(i=0;i<ch_num;i++)
    {
        menu_win[i]=newwin(5,text_space+2,(max_y-5)/2,space+(text_space+space)*i);
        box(menu_win[i],0,0);
        refresh();
        mvwprintw(menu_win[i],2,1,"%s",choices[i]);
        keypad(menu_win[i],TRUE);
        wrefresh(menu_win[i]);
    }
    /*Highlighting current choice and blinking effect*/
    do
    {
        win_blinking(menu_win[highlight],TRUE,5,text_space+2);
        /*Takes input immediately,and changes the highlight*/
        cur_choice=getch();
        switch(cur_choice)
        {
        case KEY_RIGHT:
            refresh();
            if(highlight==ch_num-1)
                highlight=0;
            else
                highlight++;
            break;
        case KEY_LEFT:
            refresh();
            if(highlight==0)
                highlight=ch_num-1;
            else
                highlight--;
            break;
        case 13:
            *fin_choice=highlight+1;
            break;
        default:
            mvprintw(0,0,"Move the Left/Right arrow keys to move and Enter to select an option.");
            break;
        }
    }while(cur_choice!=13);
    for(i=0;i<ch_num;i++)
        delwin(menu_win[i]);
}

void win_blinking(WINDOW *win,bool check,int height,int width)
{
    do
    {
        if(check==FALSE)
        wattroff(win,A_REVERSE);
        if(check==TRUE)
        {
            for(int i=1;i<height-1;i++)
                mvwchgat(win,i,1,width-2,A_REVERSE,0,NULL);
            wrefresh(win);
            Sleep(200);
            for(int i=1;i<height-1;i++)
                mvwchgat(win,i,1,width-2,A_NORMAL,0,NULL);
            wrefresh(win);
            Sleep(200);
        }
    } while(!kbhit());
}
