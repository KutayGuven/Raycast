#include <ncurses.h>
#include <math.h>
#include <stdlib.h>

#define SPD 0.1
#define WALL_HEIGHT 20
#define FLOOR_SHADING_RADIUS_0 24
#define FLOOR_SHADING_RADIUS_1 16

int vertLine(double x, double y1, double y2, char ch){
	for(int y=y1;y<=y2;y++)
	{
		mvaddch(y,x,ch);
	}
}

int main(int argc, char *argv[])
{
	initscr();
	keypad(stdscr,true);
	curs_set(0);
	timeout(500);
	start_color();

	init_pair(1,COLOR_WHITE,COLOR_BLACK); //default
	init_pair(2,COLOR_RED,COLOR_BLACK); //wall 1
	init_pair(3,COLOR_BLUE,COLOR_BLACK); //wall 2
	init_pair(4,COLOR_GREEN,COLOR_BLACK); //floor
	init_pair(5,COLOR_CYAN,COLOR_BLACK); //ceiling

	attron(COLOR_PAIR(1));

	int maxx = getmaxx(stdscr);
	int maxy = getmaxy(stdscr);

	int FOV = maxx;
	long unsigned int framecount = 0;

	char key = 0;
	bool exit = 0;
	
	int MAP_HEIGHT = 16;
	int MAP_WIDTH = 16;

	FILE *mapFile;

	short unsigned int METRIC_SPACE = 1;
	//write to file from 2D array
/*
	FILE *mapFile;

	mapFile = fopen("default.map","w");

	for(int i=0;i<MAP_HEIGHT;++i)
	{
		fprintf(mapFile,"%s","-");
		for(int j=0;j<MAP_WIDTH;++j)
		{
			fprintf(mapFile,"%i",map[i][j]);
		}
	}
	fprintf(mapFile,"%s","!");
	fclose(mapFile);
*/
	//read from file to 2D array
	if(argc > 1)
	{
		mapFile = fopen(argv[1],"r");
	} else {
		mapFile = fopen("map_default","r");
	}
	char mapFileStr[1000];

	fscanf(mapFile,"%s",mapFileStr);

	int i = 0;
	int ic = -1; //cursor i
	int jc = 0; //cursor j
	int n_dashes = 0;
	int n_digits = 0;
	bool fileFinished = 0;
	
	//find height and width of map
	while(fileFinished == 0)
	{
		switch(mapFileStr[i])
		{
			case '-':
				++n_dashes;
				break;
			case '!':
				fileFinished = 1;
				break;
			default:
				++n_digits;
		}
	++i;
	}
	
	//convert to 2D array form

	MAP_HEIGHT = n_dashes;
	MAP_WIDTH = n_digits/n_dashes;

	double SPHERE_RADIUS = sqrt((MAP_HEIGHT*MAP_WIDTH)/(4*M_PI));

	short unsigned int map[MAP_HEIGHT][MAP_WIDTH];
	
	i = 0;
	fileFinished = 0;
	while(fileFinished == 0)
	{
		switch(mapFileStr[i])
		{
			case '-':
				ic += 1;
				jc = 0;
				break;
			case '!':
				fileFinished = 1;
				break;
		}
		if((mapFileStr[i] >= 48)&&(mapFileStr[i] <= 57))
		{
			map[ic][jc] = mapFileStr[i] - 48;
			jc += 1;
		}
	++i;
	}
	//calculate width by number of digits divided by height



	typedef struct {
		double x,y,z;
		double rotation; //about z-axis
	}camera;

	camera player;
	player.x = 1;
	player.y = 1;
	player.z = 0;
	player.rotation = 0;

	while(!exit)
	{
		clear();
		
		mvaddch(player.y,player.x,'P');
		

		//draw floor
		attron(COLOR_PAIR(4));
		for(int i=maxy/2;i<maxy;i++)
		{
			for(int j=0;j<maxx;j++)
			{
				if((j-maxx/2)*(j-maxx/2)/16+(i-maxy)*(i-maxy)<FLOOR_SHADING_RADIUS_0*FLOOR_SHADING_RADIUS_0) //area within the semi-ellipse
				{
					mvaddch(i+2*player.z,j,'-');
				}
				else
				{
					mvaddch(i+2*player.z,j,'.');
				}

				if((j-maxx/2)*(j-maxx/2)/16+(i-maxy)*(i-maxy)<FLOOR_SHADING_RADIUS_1*FLOOR_SHADING_RADIUS_1) //area within the semi-ellipse
				{
					mvaddch(i+2*player.z,j,'+');
				}
			}
		}
		//draw ceiling
		attron(COLOR_PAIR(5));
		for(int i=0;i<maxy/2;i++)
		{
			for(int j=0;j<maxx;j++)
			{
				mvaddch(i,j,'.');
			}
		}

		//ray draw
		for(int i=0;i<=FOV;i++)
		{
			double rayAngle = player.rotation+(M_PI/180)*(i-FOV/2);
			double dx = 0;
			double dy = 0;
			double distance = 0;
			double rayx = player.x;
			double rayy = player.y;
			int rayx_approx;
			int rayy_approx;
			double ix = 0.1*cos(rayAngle);
			double iy = 0.1*sin(rayAngle);
			bool collision = 0;
			int sightRange = 300;
			char wallch = '#';
			float h_wall = 1;

			while((!collision)&&(sightRange > 0))
			{
				sightRange -= 1;
				rayx += ix;
				dx += ix;
				rayx_approx = round(rayx);
				rayy_approx = round(rayy);
				if(map[rayy_approx][rayx_approx] > 0)
				{
					//horizontal collision event
					switch(map[rayy_approx][rayx_approx]){
						case 1:
							h_wall = 1;
							break;
						case 2:
							h_wall = 2;
							break;

					}
					collision = true;
					wallch = '#';
					attron(COLOR_PAIR(2));
					
				}
				rayy -= iy;
				dy += iy;
				rayx_approx = round(rayx);
				rayy_approx = round(rayy);
				if((map[rayy_approx][rayx_approx] > 0)&&(!collision))
				{
					//vertical collision event
					switch(map[rayy_approx][rayx_approx]){
						case 1:
							h_wall = 1;
							break;
						case 2:
							h_wall = 2;
							break;

					}
					collision = true;
					wallch = '$';
					attron(COLOR_PAIR(3));
				}
				
			}
			if(!collision)
			{
			//no walls within range of sight
			}
			else
			{
				switch(METRIC_SPACE)
				{
					case 1: //euclidean
						distance = sqrt(dx*dx + dy*dy);
						break;
					case 2: //spherical
						distance = SPHERE_RADIUS*acos((player.x*(player.x + dx)+player.y*(player.y + dy))/(SPHERE_RADIUS*SPHERE_RADIUS));
						break;
					case 3: //hyperbolic
						distance = acosh(cosh(player.y)*cosh(dx)*cosh(player.y+dy) - sinh(player.y)*sinh(player.y+dy));
						break;
				}
				//distance /= cos((M_PI/180)*(i-FOV/2)); //causes fisheye lens effect, do not uncomment
				if(distance > 3)
				{
					wallch = '='; //depth shading
				}
				vertLine(maxx/2-FOV/2+i,maxy/2-(WALL_HEIGHT/2)/distance+player.z-WALL_HEIGHT*(h_wall-1),maxy/2+(WALL_HEIGHT/2)/distance+player.z,wallch);
			}

		}

		refresh();
		key = getch();
		switch(key)
		{
			case 27:
				exit = true;
				break;
			case 'w':
				player.x += SPD*cos(player.rotation);
				player.y -= SPD*sin(player.rotation);
				player.z = 1*sin(framecount*M_PI/12)+1; //walking illusion
				framecount += 1;
				break;
			case 's':
				player.x -= SPD*cos(player.rotation);
				player.y += SPD*sin(player.rotation);
				player.z = 1*sin(framecount*M_PI/12)+1; //walking illusion
				framecount += 1;
				break;
			case 'a':
				player.rotation -= M_PI/45;
				break;
			case 'd':
				player.rotation += M_PI/45;
				break;
			case '1':
				METRIC_SPACE = 1;
				break;
			case '2':
				METRIC_SPACE = 2;
				break;
			case '3':
				METRIC_SPACE = 3;
				break;
			default:
				break;
		}

	}

	endwin();
}
