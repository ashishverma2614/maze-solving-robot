#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define PATH_STR_MAX	100

enum Direction
{
	UP	= 0,
	DOWN	= 1,
	LEFT	= 2,
	RIGHT	= 3
};

struct Optimization
{
	char long_path[4];
	char short_path;
};

const struct Optimization optimizations[] = {
	{"LDR", 'D'},
	{"LDU", 'R'},
	{"RDL", 'D'},
	{"UDL", 'R'},
	{"UDU", 'D'},
	{"LDL", 'U'}
};
const char *directions	= "UDLR";
const int opposing[]	= {DOWN, UP, RIGHT, LEFT};
const int left[]	= {LEFT, RIGHT, DOWN, UP};
const int right[]	= {RIGHT, LEFT, UP, DOWN};
const int priorities[]	= {LEFT, UP, RIGHT};

struct Point
{
	int x;
	int y;
};

struct Robot
{
	struct Point position;
	int facing;
	bool sensors[4];
	bool discovering;
	char path[PATH_STR_MAX];
	size_t path_idx;
};

/* collapse_string : enlève de la chaîne string les caractères indiqués dans le second argument
 * eg : char string[] = "1_2_3_4"; collapse_string(string, '_'); puts(string) affichera 1234
 */
void collapse_string(char *string, char delimiter)
{
	char result[strlen(string) + 1];
	int j = 0;
	for(int i = 0; string[i]; i++)
		if(string[i] != delimiter)
			result[j++] = string[i];
	result[j] = '\0';
	strncpy(string, result, PATH_STR_MAX);
}

/**
 * optimize : optimise le chemin donné en premier argument afin de remplacer les demi-tours
 * par un chemin direct.
 * Les optimizations sont stockées dans un tableau de structures constant du nom de "optimizations"
 */
void optimize(char *path)
{
	bool done;
	do
	{
		done = true;
		for(int i = 0, l = sizeof(optimizations) / sizeof(optimizations[0]); i < l; i++)
		{
			char *match = strstr(path, optimizations[i].long_path);
			if(match == NULL)
				continue;
			int idx = match - path;
			path[idx] = '_';
			path[idx + 1] = optimizations[i].short_path;
			path[idx + 2] = '_';
			collapse_string(path, '_');
			done = false;
		}

	}
	while(!done);
}

struct Maze
{
	int width;
	int height;
	size_t element_idx;
	bool *grid;
};

void init_maze(struct Maze *maze, bool *grid, int width, int height)
{
	maze->width = width;
	maze->height = height;
	maze->grid = grid;
	for(int i = 0, l = width * height; i < l; i++)
		maze->grid[i] = false;
}

bool is_path(const struct Maze* maze, const struct Point *position)
{
	return maze->grid[position->y * maze->width + position->x];
}

void set_cell(struct Maze* maze, const struct Point *position, bool value)
{
	maze->grid[position->y * maze->width + position->x] = value;
}

bool within_bounds(const struct Maze *maze, const struct Point *position)
{
	return 0 <= position->x && position->x < maze->width && 0 <= position->y && position->y < maze->height;
}

void refresh_sensors(const struct Maze *maze, struct Robot *robot)
{
	struct Point neighbor;
	switch(robot->facing)
	{
		case UP:
			neighbor.x = robot->position.x + 1;
			neighbor.y = robot->position.y;
			robot->sensors[RIGHT] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
			neighbor.x = robot->position.x - 1;
			neighbor.y = robot->position.y;
			robot->sensors[LEFT] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
			neighbor.x = robot->position.x;
			neighbor.y = robot->position.y - 1;
			robot->sensors[UP] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
			neighbor.x = robot->position.x;
			neighbor.y = robot->position.y + 1;
			robot->sensors[DOWN] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
		break;
		case DOWN:
			neighbor.x = robot->position.x + 1;
			neighbor.y = robot->position.y;
			robot->sensors[LEFT] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
			neighbor.x = robot->position.x - 1;
			neighbor.y = robot->position.y;
			robot->sensors[RIGHT] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
			neighbor.x = robot->position.x;
			neighbor.y = robot->position.y + 1;
			robot->sensors[UP] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
			neighbor.x = robot->position.x;
			neighbor.y = robot->position.y - 1;
			robot->sensors[DOWN] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
		break;
		case LEFT:
			neighbor.x = robot->position.x + 1;
			neighbor.y = robot->position.y;
			robot->sensors[DOWN] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
			neighbor.x = robot->position.x - 1;
			neighbor.y = robot->position.y;
			robot->sensors[UP] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
			neighbor.x = robot->position.x;
			neighbor.y = robot->position.y + 1;
			robot->sensors[LEFT] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
			neighbor.x = robot->position.x;
			neighbor.y = robot->position.y - 1;
			robot->sensors[RIGHT] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
		break;
		default:
			neighbor.x = robot->position.x + 1;
			neighbor.y = robot->position.y;
			robot->sensors[UP] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
			neighbor.x = robot->position.x - 1;
			neighbor.y = robot->position.y;
			robot->sensors[DOWN] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
			neighbor.x = robot->position.x;
			neighbor.y = robot->position.y + 1;
			robot->sensors[RIGHT] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
			neighbor.x = robot->position.x;
			neighbor.y = robot->position.y - 1;
			robot->sensors[LEFT] = within_bounds(maze, &neighbor) && is_path(maze, &neighbor);
		break;
	}
}

void init_robot(struct Robot *robot, const struct Point *start, int facing, bool discovering)
{
	robot->position.x = start->x;
	robot->position.y = start->y;
	robot->facing = facing;
	robot->path_idx = 0;
	strncpy(robot->path, "", PATH_STR_MAX);
	robot->sensors[UP]	= true;
	robot->sensors[DOWN]	= false;
	robot->sensors[LEFT]	= false;
	robot->sensors[RIGHT]	= false;
	robot->discovering = discovering;
}

void follow_path(struct Robot *robot, const struct Point *start, int facing, const char *path)
{
	init_robot(robot, start, facing, false);
	strncpy(robot->path, path, PATH_STR_MAX);
}

/**
 * at_destination : si tous les capteurs détectent le noir, c'est que le robot est arrivé à la sortie du labyrinthe
 */
bool at_destination(const struct Robot *robot)
{
	for(int i = 0; i < 4; i++)
		if(!robot->sensors[i])
			return false;
	return true;
}

/**
 * at_deadend : si tous les capteurs à l'exception de celui de derrière détectent le blanc, c'est 
 * que le robot est arrivé à la fin d'une ligne et qu'il devra faire demi-tour.
 */
bool at_deadend(const struct Robot *robot)
{
	return robot->sensors[DOWN] && !robot->sensors[LEFT] && !robot->sensors[RIGHT] && !robot->sensors[UP];
}

/**
 * at_turn : retourne true si le robot arrive à la fin d'une ligne et qu'il
 * doit tourner vers la droite ou la gauche.
 */
bool at_turn(const struct Robot *robot)
{
	return robot->sensors[DOWN] && !robot->sensors[UP] && (robot->sensors[LEFT] || robot->sensors[RIGHT]);
}

/**
 * at_intersection : retourne true si le robot arrive à une intersection d'au moins deux possibilités.
 * càd si le nombre de capteurs actifs est à au moins 3.
 */
bool at_intersection(const struct Robot *robot)
{
	int possibilities = 0;
	for(int i = 0; i < 4; i++)
		possibilities += robot->sensors[i];
	return possibilities >= 3;
}

/**
 * rotate : le robot effectue une rotation selon la valeur indiquée en second paramètre.
 * si save_path est à true, la direction sera enregistrée pour pouvoir être optimisée plus tard.
 * save_path sera true uniquement si le robot est à une intersection, sinon il ne faudra pas enregistrer
 * la nouvelle direction du robot car dans ce cas même s'il tourne, il suit toujours la même ligne.
 */
void rotate(struct Robot *robot, int direction, bool save_path)
{
	switch(direction)
	{
		case DOWN:	robot->facing = opposing[robot->facing];	break;
		case LEFT:	robot->facing = left[robot->facing];		break;
		case RIGHT:	robot->facing = right[robot->facing];		break;
		default: break;
	}
	if(robot->discovering && save_path)
	{
		robot->path[robot->path_idx++] = directions[direction];
		robot->path[robot->path_idx] = '\0';
	}
	//ajouter un bout de code pour faire tourner les moteurs
}

/**
 * turn : arrivé à une intersection, cette fonction fera faire au robot une rotation
 * selon l'ordre de priorité défini dans la constante priorities.
 */
void turn(struct Robot *robot)
{
	for(int i = 0, l = sizeof(priorities) / sizeof(priorities[0]); i < l; i++)
	{
		if(robot->sensors[priorities[i]])
		{
			rotate(robot, priorities[i], true);
			return;
		}
	}
	puts("Invalid state : all sensors are off.");
}

/**
 * move : le robot bouge selon l'état des capteurs et le mode de fonctionnement du robot (découverte ou autre)
 */
void move(struct Robot *robot)
{
	if(at_intersection(robot))
	{
		if(robot->discovering)
			turn(robot);
		else
			rotate(robot, strchr(directions, robot->path[robot->path_idx++]) - directions, false);
	}
	else if(at_turn(robot))
		rotate(robot, robot->sensors[LEFT] ? LEFT : RIGHT, false);
	else if(at_deadend(robot))
		rotate(robot, DOWN, true);

	switch(robot->facing)
	{
		case UP:	robot->position.y--; break;
		case DOWN:	robot->position.y++; break;
		case LEFT:	robot->position.x--; break;
		case RIGHT:	robot->position.x++; break;
	}
}

void draw_maze(const struct Maze *maze, const struct Robot *robot)
{
	for(int row = 0; row < maze->height; row++)
	{
		for(int col = 0; col < maze->width; col++)
		{
			struct Point position = {.x = col, .y = row};
			if(robot->position.x == col && robot->position.y == row)
				printf("R");
			else
				printf("%c", is_path(maze, &position) ? '.' : ' ');
		}
		printf("\n");
	}
}

/**
 * print_robot : affiche des informations sur l'état du robot dont la position, l'état des capteurs ou le chemin.
 */
void print_robot(const struct Robot *robot)
{
	printf("Mode : %s\n", robot->discovering ? "discovery" : "intelligent");
	printf("[%d, %d]\n", robot->position.x, robot->position.y);
	printf("Facing : %c\n", directions[robot->facing]);
	for(int i = 0; i < 4; i++)
		printf("\t%c : %d\n", directions[i], robot->sensors[i]);
	printf("Path so far : %s\n", robot->path);
}

int main(void)
{
	int width, height;
	FILE *fh = fopen("input", "r");
	fscanf(fh, "%d\n", &width);
	fscanf(fh, "%d\n", &height);
	struct Robot robot;
	struct Point start = {.x = 0, .y = 0};
	init_robot(&robot, &start, RIGHT, true);
	puts("Robot is ready.");

	struct Maze maze;
	bool grid[width * height];
	init_maze(&maze, grid, width, height);
	char line[100];
	struct Point current = {.x = 0, .y = 0};
	struct Point end = {.x = 4, .y = 7};
	while(fgets(line, 100, fh))
	{
		printf("%s", line);
		for(current.x = 0; current.x < width; current.x++)
			set_cell(&maze, &current, line[current.x] == '.');
		current.y++;
	}
	puts("Maze is loaded.");

	while(true)
	{
		refresh_sensors(&maze, &robot);
		draw_maze(&maze, &robot);
		print_robot(&robot);
		printf("\n");
		move(&robot);
		if(robot.position.x == end.x && robot.position.y == end.y)
			break;
		system("pause");
		system("cls");
	}
	printf("%s\n", robot.path);

	char confirm;
	do
	{
		printf("Optimize path ? Y/n ");
		scanf("%c", &confirm);
	}
	while(confirm != 'n' && confirm != 'Y');
	if(confirm == 'n')
		return 0;

	char short_path[PATH_STR_MAX];
	strncpy(short_path, robot.path, PATH_STR_MAX);
	optimize(short_path);
	follow_path(&robot, &start, RIGHT, short_path);

	printf("New path : %s\n", robot.path);
	while(true)
	{
		refresh_sensors(&maze, &robot);
		draw_maze(&maze, &robot);
		print_robot(&robot);
		printf("\n");
		move(&robot);
		if(robot.position.x == end.x && robot.position.y == end.y)
			break;
		system("pause");
		system("cls");
	}
	return 0;
}
