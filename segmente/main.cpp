/**
-lbgi -lgdi32 -lcomdlg32 -luuid -loleaut32 -lole32 -lwinmm
*/

#include <iostream>
#include <graphics.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include <windows.h>
#include <mciavi.h>

#pragma comment(lib, "Winmm.lib")


#define REZOLUTION_HEIGHT 720
#define REZOLUTION_WIDTH 720
#define MAX_ERROR 0.5

using namespace std;

struct punct
{
    int x=0,y=0;
};

struct info
{
    bool sound=1;
    bool mod_de_joc=1;
    ///0-> player vs player
    ///1->player vs AI
    bool player=0;
    ///modul de joc este player vs AI determina care jucator este playerul
    int dificultate;
    ///0->easy
    ///1->medium
    ///2->hard
    punct all_points[100];
    int rand=0;
    int adiacenta[100][100];
    int nr_points=20;
    int radius = 10;
    int last_move[2]= {0};
} singleton;

const int WINDOW = initwindow(REZOLUTION_WIDTH, REZOLUTION_HEIGHT, "segmente",400,50);

void undo();
void instructiuni();
void optiuni();
void meniu();
void print_info();
void mutare_jucator();
void unire_puncte(int i1,int i2);///traseaza linie de la p1 la p2
void game_loop();
void draw_points();
void prep_game();
void generate_points();


void AI_move_easy();
void AI_move_hard();


bool onSegment(punct p, punct q, punct r);
bool doIntersect(punct p1, punct q1, punct p2, punct q2);
bool se_suprapune(int midx,int midy,int nr_cercuri);
bool puncte_egale(punct p1, punct p2);
bool game_over();
bool mutare_ok(punct p1, punct p2);


punct get_click();


int click_pe_cerc(punct aux);
int buton_apasat(punct clk,int fereastra);
int orientation(punct p, punct q, punct r);



int main()
{
    srand(time(0));
    setcurrentwindow(WINDOW);
    meniu();

    printf("a castigat jucatorul cu numarul %d",singleton.rand^1+1);

}
void game_loop()
{
    punct clk;
    draw_points();

    setcolor(BLUE);
    for(int i=1; i<=singleton.nr_points; i++)
    {
        for(int j=1; j<=singleton.nr_points; j++)
        {
            if(singleton.adiacenta[i][j]==true)
                line(singleton.all_points[i].x,singleton.all_points[i].y,singleton.all_points[j].x,singleton.all_points[j].y);
        }
    }


    while(!game_over())
    {
        if(singleton.mod_de_joc==0) ///PvP
        {
            printf("player %d\n",singleton.rand+1);
            mutare_jucator();
        }
        else                        ///PvAi
        {

            if(singleton.rand==singleton.player)///randul jucatorului
            {
                printf("player %d\n",singleton.rand+1);
                mutare_jucator();
            }
            else                                ///randul AI
            {
                printf("AI %d\n",singleton.rand+1);
                if(singleton.dificultate==0)
                    AI_move_easy();
                else if(singleton.dificultate==1)
                    AI_move_hard();

            }
        }
    }
    const int end_WINDOW = initwindow(400, 200, "castigat/pierdut",560,300);
    setcurrentwindow(end_WINDOW);
    setfillstyle(SOLID_FILL,COLOR(178, 50, 24));
    setbkcolor(COLOR(178, 50, 24));
    setcolor(BLACK);
    bar(0,0,400,300);
    settextjustify(CENTER_TEXT,CENTER_TEXT);
    settextstyle(1,0,2);
    if(singleton.rand==0)
    {
        outtextxy(200,50,"AI CASTIGAT player 2");
    }
    else
    {
        outtextxy(200,50,"AI CASTIGAT player 1");
    }
    settextjustify(RIGHT_TEXT,BOTTOM_TEXT);
    outtextxy(200,160,"inapoi  ");
    settextjustify(LEFT_TEXT,BOTTOM_TEXT);
    outtextxy(200,160,"  joc nou");

    clk=get_click();
    while(buton_apasat(clk,3)==0)
    {
        clk=get_click();
    }
    if(buton_apasat(clk,3)==1)
    {
        closegraph(end_WINDOW);
        setcurrentwindow(WINDOW);
        meniu();
    }
    else
    {
        closegraph(end_WINDOW);
        setcurrentwindow(WINDOW);
        prep_game();

        game_loop();
    }
}

void unire_puncte(int i1,int i2)///traseaza linie de la p1 la p2
{

    setlinestyle(SOLID_LINE,0,1);
    if(mutare_ok(singleton.all_points[i1],singleton.all_points[i2]))
    {
        line(singleton.all_points[i1].x,singleton.all_points[i1].y,singleton.all_points[i2].x,singleton.all_points[i2].y);
        singleton.adiacenta[i1][i2]=1;
        singleton.adiacenta[i2][i1]=1;

    }
}

///trebuie verificata deoarece uneori considara mutari valide invalide
bool mutare_ok(punct p1, punct p2)///valideaza legarea punctelor p1, p2
{
    if(puncte_egale(p1,p2))
        return 0;

    for(int i=1;i<=singleton.nr_points;i++)
    {
        if(puncte_egale(p1,singleton.all_points[i]) || puncte_egale(p2,singleton.all_points[i]))
        {
            for(int j=1;j<=singleton.nr_points;j++)
            {
                if(singleton.adiacenta[i][j]==1 || singleton.adiacenta[j][i]==1)
                    return 0;
            }
        }
    }
    for (int i=1; i<=singleton.nr_points; i++)
    {
        for(int j=i+1; j<=singleton.nr_points; j++)
        {
            if(singleton.adiacenta[i][j]&&doIntersect(p1,p2,singleton.all_points[i],singleton.all_points[j]))
            {
                return 0;
            }
        }
    }
    return 1;

}

punct get_click()
{
    punct aux;
    while(1)
    {
        if(ismouseclick(WM_LBUTTONDOWN))
        {
            if(singleton.sound==1)
                PlaySound(TEXT("click5.wav"), NULL, SND_FILENAME | SND_ASYNC);

            clearmouseclick(WM_LBUTTONDOWN);
            aux.x=mousex();
            aux.y=mousey();
            return aux;
            break;
        }
    }
}

int click_pe_cerc(punct aux)
{
    for(int i=1; i<=singleton.nr_points; i++)
    {
        if(pow(singleton.all_points[i].x-aux.x,2)+pow(singleton.all_points[i].y-aux.y,2)<=pow(singleton.radius,2))
        {
            return i;
        }
    }
    return 0;
}

void undo()
{
    int i,j;
    i=singleton.last_move[0];
    j=singleton.last_move[1];
    singleton.adiacenta[i][j]=0;
    singleton.adiacenta[j][i]=0;
    singleton.rand=singleton.rand^1;
    setcolor(WHITE);
    line(singleton.all_points[i].x,singleton.all_points[i].y,singleton.all_points[j].x,singleton.all_points[j].y);
    setcolor(BLUE);
    setfillstyle(SOLID_FILL,BLUE);
    circle(singleton.all_points[i].x, singleton.all_points[i].y, singleton.radius);
    floodfill(singleton.all_points[i].x,singleton.all_points[i].y,1);
    circle(singleton.all_points[j].x, singleton.all_points[j].y, singleton.radius);
    floodfill(singleton.all_points[j].x,singleton.all_points[j].y,1);

    ///game_loop();
    printf("UNDO");
}
void mutare_jucator()
{
    int noduri_selectate[2];
    int nr=0;
    punct clk;
    while(nr<2)
    {
        punct aux=get_click();
        if(click_pe_cerc(aux))
        {
            noduri_selectate[nr]=click_pe_cerc(aux);
            nr++;
            if(nr==1)
            {
                int x,y;
                x=singleton.all_points[click_pe_cerc(aux)].x;
                y=singleton.all_points[click_pe_cerc(aux)].y,
                setcolor(YELLOW);
                setfillstyle(SOLID_FILL,YELLOW);
                fillellipse(x,y,singleton.radius,singleton.radius);
                setcolor(BLUE);
            }
        }
    }

    setfillstyle(SOLID_FILL,BLUE);
    fillellipse(singleton.all_points[noduri_selectate[0]].x,singleton.all_points[noduri_selectate[0]].y,singleton.radius,singleton.radius);

    if(mutare_ok(singleton.all_points[noduri_selectate[0]],singleton.all_points[noduri_selectate[1]]))
    {
        singleton.last_move[0]=noduri_selectate[0];
        singleton.last_move[1]=noduri_selectate[1];
        unire_puncte(noduri_selectate[0],noduri_selectate[1]);
        singleton.rand=singleton.rand^1;
        clk=get_click();


        while(buton_apasat(clk,4)==0)
        {
            clk=get_click();
        }
        if(buton_apasat(clk,4)==1)
        {
            meniu();
        }
        else if(buton_apasat(clk,4)==2)
        {
            undo();
        }
    }
    else
    {
        cout<<"MUTARE INVALIDA"<<endl;
    }
}

bool game_over()
{
    for (int i=1; i<=singleton.nr_points; i++)
        for(int j=1; j<=singleton.nr_points; j++)
            if (mutare_ok(singleton.all_points[i],singleton.all_points[j]))
            {
                cout<<i<<" "<<j<<endl;
                return 0;
            }
    return 1;
}

bool puncte_egale(punct p1, punct p2)
{
    if (p1.x==p2.x && p2.y==p1.y)
        return 1;
    return 0;

}

void print_info()
{
    cout<<endl<<endl<<"matricea de adiacenta"<<endl;
    for(int i=1; i<=singleton.nr_points; i++)
    {
        for(int j=1; j<=singleton.nr_points; j++)
        {
            cout<<singleton.adiacenta[i][j]<<" ";
        }
        cout<<endl;
    }
    cout<<endl;

}
void AI_move_hard()
{
    AI_move_easy();
}

void AI_move_easy()
{
    Sleep(1000);
    ///->i si j pleaca de la un punct randome
    ///->dublam vectorul de puncte pt a avea toate posibilitatile de linii
    ///     oricare ar fi valoarea random
    ///

    for(int i=rand()%singleton.nr_points; i<=2*singleton.nr_points; i++)
    {
        for(int j=rand()%singleton.nr_points; j<=2*singleton.nr_points; j++)
        {
            int ci=i%singleton.nr_points;   ///aducem cj si ci inapoi in vector
            int cj=j%singleton.nr_points;
            ci++;
            cj++;

            if(mutare_ok(singleton.all_points[ci],singleton.all_points[cj]))
            {
                unire_puncte(ci,cj);
                singleton.rand=singleton.rand^1;
                return;
            }
        }
    }
}

bool se_suprapune(int midx,int midy,int nr_cercuri)
{

    ///se verifica daca cercurile sunt out of bounds
    if(midx-singleton.radius<5)
        return true;
    if(midx+singleton.radius>REZOLUTION_WIDTH-5)
        return true;
    if(midy-singleton.radius<25)
        return true;
    if(midy+singleton.radius>REZOLUTION_HEIGHT-5)
        return true;

    ///se verifica suprapunerea cercurilor
    if(nr_cercuri==0)
        return false;
    for(int i=1; i<=nr_cercuri; i++)
    {
        int x2=singleton.all_points[i].x;
        int y2=singleton.all_points[i].y;

        if(pow(midx-x2,2)+pow(midy-y2,2)<=pow(2*singleton.radius,2))
        {
            return true;
        }
    }
}

void meniu()
{
    punct clk;
    setfillstyle(SOLID_FILL,COLOR(178, 50, 24));
    setcolor(COLOR(178, 50, 24));
    bar(0,0,REZOLUTION_WIDTH,REZOLUTION_HEIGHT);

    settextjustify(CENTER_TEXT,CENTER_TEXT);
    settextstyle(1,0,4);
    setcolor(WHITE);
    setbkcolor(COLOR(178, 50, 24));
    outtextxy(REZOLUTION_WIDTH/2,REZOLUTION_HEIGHT/2+100,"instructiuni");
    outtextxy(REZOLUTION_WIDTH/2,REZOLUTION_HEIGHT/2,"optiuni");
    outtextxy(REZOLUTION_WIDTH/2,REZOLUTION_HEIGHT/2-100,"joc nou");
    settextstyle(4,0,8);
    outtextxy(REZOLUTION_WIDTH/2,REZOLUTION_HEIGHT/2-200,"SEGMENTE");

    clk=get_click();
    while(buton_apasat(clk,0)==0)
    {
        clk=get_click();
    }

    if(buton_apasat(clk,0)==1)///joc nou
    {
        prep_game();

        game_loop();
    }
    else if(buton_apasat(clk,0)==2)///optiuni
    {
        optiuni();
    }
    else if(buton_apasat(clk,0)==3)///instructiuni
    {
        instructiuni();
    }
}

int buton_apasat(punct clk,int fereastra)///fereastra =0-> meniu principal
{
    ///fereastra =0-> main menu
    ///fereastra =1-> optiuni
    ///fereastra =2-> instructiuni
    ///fereastra =3->ai castigat/pierdut
    ///fereastra =4->in joc
    /*cout<<clk.x-REZOLUTION_WIDTH/2<<" "<<clk.x+REZOLUTION_WIDTH/2<<endl;
    cout<<textwidth("player vs player")<<endl;
    */
    ///cout<<(clk.x < REZOLUTION_WIDTH/2-10)<<" "<< (clk.x >= REZOLUTION_WIDTH/2 - 10 - textwidth("+"))<<endl;

    if(fereastra==0)
    {
        if( (clk.x<=439&&clk.x>=277)&&(clk.y>=235 && clk.y<=268))
        {
            cout<<"JOC NOU\n";
            return 1;///joc nou
        }
        else if((clk.x<=433&&clk.x>=268)&&(clk.y>=335 && clk.y<=366))
        {
            cout<<"OPTIUNI\n";
            return 2;///optiuni
        }
        else if((clk.x<=482&&clk.x>=239)&&(clk.y>=436 && clk.y<=466))
        {
            cout<<"INSTRUCTIUNI\n";
            return 3;///instructiuni
        }
        else
        {
            return 0;///pe langa
        }
    }
    if (fereastra==1)
    {
        /**
        outtextxy(REZOLUTION_WIDTH/2,250,"player versur player");
        outtextxy(REZOLUTION_WIDTH/2,300,"player versur AI");
        outtextxy(REZOLUTION_WIDTH/2,450," easy ");
        outtextxy(REZOLUTION_WIDTH/2,450," hard ");
        outtextxy(REZOLUTION_WIDTH/2,600,"")
        */

        if(clk.x<150&&clk.y<35)
        {
            return 1;
        }
        if((abs (clk.x-REZOLUTION_WIDTH/2) < textwidth("player versur player")/2 )&& (abs(clk.y-250)<textheight("player versur player")/2))///p vs p
        {
            cout<<"PLAYER VS PLAYER\n";
            return 2;
        }
        if((abs (clk.x-REZOLUTION_WIDTH/2) <= textwidth("player versur AI")/2 )&& (abs(clk.y-300)<textheight("player versur AI")/2))/// p vs ai
        {
            cout<<"PLAYER VS AI\n";
            return 3;
        }
        else if((clk.x<=REZOLUTION_WIDTH/2) && (clk.x>=REZOLUTION_WIDTH/2-textwidth(" easy ") ) && ( abs(clk.y-450)<textheight("easy")/2))/// easy
        {
            cout<<"EASY DIFICULTI SET\n";
            return 4;
        }
        else if((clk.x>=REZOLUTION_WIDTH/2) && (clk.x<=REZOLUTION_WIDTH/2+textwidth(" hard ") ) && ( abs(clk.y-450)<textheight(" hard ")/2))/// hard
        {
            cout<<"HARD DIFICULTI SET\n";
            return 5;
        }
        else if((abs(clk.x-REZOLUTION_WIDTH/2)<=textwidth("sunet"))&&(abs(clk.y-600)<textheight("sunet")))///sunet
        {
            cout<<"SUNET RESETAT\n";
            return 6;
        }
        else if((clk.x<REZOLUTION_WIDTH/2-10)&&(clk.x>=REZOLUTION_WIDTH/2-10-textwidth("+"))&&(abs(clk.y-550)<textheight("-")))

        {
            /**settextjustify(RIGHT_TEXT,CENTER_TEXT);
            outtextxy(REZOLUTION_WIDTH/2-10,550,"-");
            */
            cout<<"---\n";
            return 7;
        }
        else if((clk.x>REZOLUTION_WIDTH/2+10)&&(clk.x<=REZOLUTION_WIDTH/2+10+textwidth("+"))&&(abs(clk.y-550)<textheight("+")))

        {
            /**
            settextjustify(LEFT_TEXT,CENTER_TEXT);
            outtextxy(REZOLUTION_WIDTH/2+10,550,"+");
            */
            cout<<"+++\n";

            return 8;
        }
        else
        {

            return 0;
        }
    }
    if(fereastra==2)///instructiuni
    {
        settextstyle(1,0,4);
        if(clk.x<150&&clk.y<35)///back
        {
            return 1;
        }

    }
    if(fereastra==3)///ai castigat / pierdut
    {

        /**
            settextjustify(RIGHT_TEXT,BOTTOM_TEXT);
            outtextxy(200,160,"inapoi  ");
            settextjustify(LEFT_TEXT,BOTTOM_TEXT);
            outtextxy(200,160,"  joc nou");
        */

        if(200-textwidth("inapoi  ")<clk.x&&clk.x<200 && 160-textheight("inapoi  ")<clk.y&&clk.y<<160)///click main menu
        {
            cout<<"main menu";
            return 1;
        }
        if(200+textwidth("  joc nou")>clk.x && clk.x>200 && 160-textheight("  joc nou")< clk.y && clk.y<160)///click joc nou
        {
            cout<<"joc nou";
            return 2;
        }
        else///click invalid
        {
            return 0;
        }
    }
    else if(fereastra==4)
    {
        if(clk.y<textheight("BACK"))
        {
            if(clk.x<textwidth("BACK"))
            {
                return 1;
            }
            else if(abs(clk.x-REZOLUTION_WIDTH/2)<=textwidth("UNDO"))
            {
                return 2;
            }
            else if(clk.x>=REZOLUTION_WIDTH-textwidth("Confirma mutare"))
            {
                return 3;
            }
        }
        return 0;

    }
}
void optiuni()
{
    punct clk;
    setfillstyle(SOLID_FILL,COLOR(178, 50, 24));
    setcolor(BLUE);
    bar(0,0,REZOLUTION_WIDTH,REZOLUTION_HEIGHT);
    setbkcolor(COLOR(178, 50, 24));
    bar(0, 0,150,35);
    setcolor(BLACK);

    settextjustify(LEFT_TEXT,TOP_TEXT);
    settextstyle(1,0,4);
    outtextxy(0,0,"BACK");
    settextjustify(CENTER_TEXT,CENTER_TEXT);
    settextstyle(1,0,4);
    setcolor(WHITE);
    setbkcolor(COLOR(178, 50, 24));
    outtextxy(REZOLUTION_WIDTH/2,100,"OPTIUNI");
    outtextxy(REZOLUTION_WIDTH/2,200,"mod de joc");
    outtextxy(REZOLUTION_WIDTH/2,400,"dificultate");
    if(singleton.sound==1)
        setbkcolor(YELLOW);
    outtextxy(REZOLUTION_WIDTH/2,600,"sunet");
    setbkcolor(COLOR(178, 50, 24));

    if(singleton.mod_de_joc==0)
        setbkcolor(YELLOW);
    outtextxy(REZOLUTION_WIDTH/2,250,"player versur player");
    setbkcolor(COLOR(178, 50, 24));
    if(singleton.mod_de_joc==1)
        setbkcolor(YELLOW);
    outtextxy(REZOLUTION_WIDTH/2,300,"player versur AI");
    setbkcolor(COLOR(178, 50, 24));

    settextjustify(RIGHT_TEXT,CENTER_TEXT);

    if(singleton.dificultate==0)
        setbkcolor(YELLOW);
    outtextxy(REZOLUTION_WIDTH/2,450," easy ");
    setbkcolor(COLOR(178, 50, 24));

    settextjustify(LEFT_TEXT,CENTER_TEXT);
    if(singleton.dificultate==1)
        setbkcolor(YELLOW);
    outtextxy(REZOLUTION_WIDTH/2,450," hard ");
    setbkcolor(COLOR(178, 50, 24));

    char text[100]="nr cercuri: ";
    sprintf(text, "%s%d",text,singleton.nr_points);

    settextjustify(CENTER_TEXT,CENTER_TEXT);
    outtextxy(REZOLUTION_WIDTH/2,500,text);


    settextjustify(RIGHT_TEXT,CENTER_TEXT);

    outtextxy(REZOLUTION_WIDTH/2-10,550,"-");

    settextjustify(LEFT_TEXT,CENTER_TEXT);

    outtextxy(REZOLUTION_WIDTH/2+10,550,"+");

    clk=get_click();
    while(buton_apasat(clk,1)==0)
    {
        clk=get_click();
    }

    if(buton_apasat(clk,1)==1)///back
    {
        meniu();
    }
    else if(buton_apasat(clk,1)==2)///p vs p
    {
        singleton.mod_de_joc=0;
        optiuni();
    }
    else if(buton_apasat(clk,1)==3)///p vs AI
    {
        singleton.mod_de_joc=1;
        optiuni();
    }
    else if(buton_apasat(clk,1)==4)///easy
    {
        singleton.dificultate=0;
        optiuni();
    }
    else if(buton_apasat(clk,1)==5) ///hard
    {
        singleton.dificultate=1;
        optiuni();
    }
    else if(buton_apasat(clk,1)==6)///sunet
    {
        singleton.sound=singleton.sound^1;
        if(singleton.sound==1)
        {
            PlaySound(TEXT("background.wav"), NULL, SND_FILENAME | SND_ASYNC);
        }
        else
        {
            PlaySound(NULL,NULL,NULL);
        }
        optiuni();
    }
    else if(buton_apasat(clk,1)==7)///- puncte
    {
        if(singleton.nr_points>=3)
        {
            singleton.nr_points--;
        }
        else
        {
            PlaySound("ERROR.wav",NULL,NULL);
        }
        optiuni();
    }
    else if(buton_apasat(clk,1)==8)///+puncte
    {
        if(singleton.nr_points<=30)
        {
            singleton.nr_points++;
        }
        else
        {
            PlaySound("ERROR.wav",NULL,NULL);
        }
        optiuni();
    }


}
void instructiuni()
{
    punct clk;
    setfillstyle(SOLID_FILL,COLOR(178, 50, 24));
    setcolor(BLUE);
    bar(0,0,REZOLUTION_WIDTH,REZOLUTION_HEIGHT);
    setbkcolor(COLOR(178, 50, 24));
    bar(0, 0,150,35);
    setcolor(BLACK);

    /*  line (0,15,30,30);
        line (0,15,30,0);
        line(30,30,30,0);
    */

    settextjustify(LEFT_TEXT,TOP_TEXT);
    settextstyle(1,0,4);
    outtextxy(0,0,"BACK");


    settextjustify(CENTER_TEXT,CENTER_TEXT);
    settextstyle(1,0,4);
    setcolor(WHITE);
    setbkcolor(COLOR(178, 50, 24));
    outtextxy(REZOLUTION_WIDTH/2,100,"REGULI");
    settextstyle(1,0,2);
    settextjustify(LEFT_TEXT,CENTER_TEXT);
    outtextxy(10,200,"Fiecare jucator uneste puncte si castiga cel care a");
    outtextxy(10,220,"unit ultimele 2 puncte disponibile. Daca s-au");
    outtextxy(10,240,"intersectat segmente la unirea punctelor jucatorul");
    outtextxy(10,260,"respectiv pierde jocul.");
    clk=get_click();

    while(buton_apasat(clk,2)==0)
    {
        clk=get_click();
    }
    if(buton_apasat(clk,2)==1)
    {
        meniu();
    }
}
void generate_points()
{
    int midx,midy;

    for(int i=1; i<=singleton.nr_points; i++)
    {
        /// Sleep(1000);
        midx = rand()%REZOLUTION_WIDTH;
        midy = rand()%REZOLUTION_HEIGHT;

        if(se_suprapune(midx,midy,i)==false)
        {
            singleton.all_points[i].x=midx;
            singleton.all_points[i].y=midy;
        }
        else
        {
            i--;
        }
    }
}

void draw_points()
{
    setbkcolor(WHITE);

    cleardevice();
    int midx,midy;
    setcolor(BLUE);
    setfillstyle(SOLID_FILL,BLUE);

    for(int i=1; i<=singleton.nr_points; i++)
    {
        cout<<singleton.all_points[i].x<<" "<<singleton.all_points[i].y<<" "<<singleton.radius<<endl;

        circle(singleton.all_points[i].x, singleton.all_points[i].y, singleton.radius);
        floodfill(singleton.all_points[i].x,singleton.all_points[i].y,1);
        printf("circle %d drawn\n",i);
    }

    setbkcolor(RED);
    setcolor(WHITE);
    setfillstyle(SOLID_FILL,RED);
    bar(0,0,REZOLUTION_WIDTH,20);
    settextjustify(LEFT_TEXT,TOP_TEXT);
    settextstyle(1,0,2);
    outtextxy(0,0,"BACK");
    settextjustify(CENTER_TEXT,TOP_TEXT);
    outtextxy(REZOLUTION_WIDTH/2,0,"UNDO");
    settextjustify(RIGHT_TEXT,TOP_TEXT);
    outtextxy(REZOLUTION_WIDTH,0,"Confirma mutare");
    cout<<"DRAW POINTS 1\n";

    cout<<"PUNCTE AFISATE";
}


bool doIntersect(punct p1, punct q1, punct p2, punct q2)
{
    // Find the four orientations needed for general and
    // special cases
    if(puncte_egale(p1,p2)&&(!puncte_egale(q1,q2)))
        return false;
    else if(puncte_egale(q1,q2)&&(!puncte_egale(p1,p2)))
        return false;
    else if(puncte_egale(p1,q2)&&(!puncte_egale(p2,q1)))
        return false;
    else if(puncte_egale(p2,q1)&&(!puncte_egale(p1,q2)))
        return false;
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    // Special Cases
    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1))
        return true;

    // p1, q1 and q2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1))
        return true;

    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2))
        return true;

    // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2))
        return true;

    return false; // Doesn't fall in any of the above cases
}

int orientation(punct p, punct q, punct r)
{
    int val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);

    if (val == 0)
        return 0;

    if(val>0)
    {
        return 1;
    }
    else
    {
        return 2;
    }
}

bool onSegment(punct p, punct q, punct r)
{
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
            q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
        return true;

    return false;
}

void prep_game()
{
    singleton.rand=0;
    generate_points();
    cout<<"puncte generate\n";
    for(int i=1; i<=singleton.nr_points; i++)
    {
        for(int j=1; j<=singleton.nr_points; j++)
        {
            singleton.adiacenta[i][j]=0;
        }
    }
    cout<<"adiacenta curatata\n";
}
