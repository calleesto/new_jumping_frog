#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define SCREEN_WIDTH			41
#define MAX_CARS				5
#define SCREEN_HEIGHT			3 * MAX_CARS
#define FROG_HEIGHT				1
#define FROG_WIDTH				1
#define CAR_SPEED_VAR			0.05
#define MAX_OBSTACLES			5
#define MAX_OBSTACLE_LENGHT		5
#define OBSTACLE_SYMBOL			'@'
#define SPEED_CHANGE_INTERVAL	5

#define MAP_ELEMENTS_COLOR		1
#define FINISH_LANE_COLOR		5
#define TIMER_ADDITION			0.015 // usually 0.015 DEPENDS ON CPU FREQUENCY
#define DELAY_AFTER_JUMP		0.3 // 0.3 for visible delay but still playable --- no delay seems more comfortable though



//-------------------
//------STRUCTS------
//-------------------
typedef struct GameState {
	int quit = 1;
	int frog_y = SCREEN_HEIGHT - 2;
	int frog_x = SCREEN_WIDTH / 2;
	int frog_color = 0;
	int move = 0;
	char map[SCREEN_HEIGHT][SCREEN_WIDTH] = { ' ' };
	char obstacleMap[SCREEN_HEIGHT][SCREEN_WIDTH] = { ' ' };
	float timer = 0;
	int points = 0;
	int r = CAR_SPEED_VAR + 1;
	int collisionDetected = 0;
	char frog_symbol = ' ';
	char car_symbol = ' ';
	int car_speed_variable = 0;
	int passive_car_color = 0;
	int aggressive_car_color = 0;
	int friendly_car_color = 0;
	char border_symbol = ' ';
	char lane_separator = ' ';
	int max_time = 0;
	int number_of_bounces = 0;
	int number_of_wraps = 0;
	bool moveLeft = true;
	bool moveRight = true;
	bool moveUp = true;
	bool moveDown = true;
	bool obstaclesSet = false;
	int highscore = 0;
	bool wDetected = false;
	bool aDetected = false;
	bool sDetected = false;
	bool dDetected = false;
	bool inputDetected = false;
	float save_timer;
	bool awaiting = false;
} state;

typedef struct Car {
	float car_x = 0.0;
	int car_y = 0;
	char direction = ' ';
	float speed = 0.0;
	char type = ' '; // 'w' - wrapping, 'b' - bouncing, 'd' - disappearing
	char interaction = ' '; // 'p' - passive, 'a' - aggressive
	char symbol = ' ';
	int color = 0;
	int initialized = 0;// 1 - initialized, 0 - not initialized
	int iters = 0;
	bool stopCar = false;
	bool frogRide = false;
}Car;



//-------------------
//----DECLARATIONS---
//-------------------

// Printing functions
void initColors();
void setObstacles(GameState* state);
void obstaclesToArray(GameState* state);
void setBordersAndSeparators(GameState* state);
void printMapArray(GameState* state);
void printFinishLane(GameState* state);
void printFrog(GameState* state);
int isAtWholeNumber(float timer);
void printCars(GameState* state, Car* cars);
void printGameInfo(GameState* state);
void setAndPrintVisuals(GameState* state, Car* cars);

// Frog-related functions
void checkForObstacle(GameState* state);
void setAllMovementTrue(GameState* state);
int frogRideOff(GameState* state, Car* cars);
void upCase(GameState* state);
void downCase(GameState* state);
void leftCase(GameState* state);
void rightCase(GameState* state);
void awaitingPickUp(GameState* state);
void inputDetect(GameState* state, Car* cars);
void letAnotherDetect(GameState* state);
void ifScored(GameState* state);

// Car-related functions
void typeChoiceCase(int choice, int n, Car* cars, char symbol, int i);
void interactionChoiceCase(int choice, int n, Car* cars, char symbol, int i, int color);
void carDirection(int i, Car* cars);
void initializeCars(Car* cars, GameState* state);
void changeCarAfterNumOfIters(int iterations, Car* cars, int i);
void bounceCar(Car* cars, int i, GameState* state);
void wrapCar(Car* cars, int i, GameState* state);
void disappearCar(Car* cars, int i, GameState* state);
void moveCars(Car* cars, GameState* state);
void aggressiveCase(int i, Car* cars, GameState* state);
void passiveCase(int i, Car* cars, GameState* state);
void friendlyCase(int i, Car* cars, GameState* state);
void hits(int i, Car* cars, GameState* state);
void collisionDetect(Car* cars, GameState* state);

// Configuration functions
void extractValue(char key[128], char value[128], const char* var_name, int* var_value_int, char* var_value_char);
void extractValues(char key[128], char value[128], GameState* state);
void readConfigFile(const char* filename, GameState* state);

// Game-related functions
void changeOfSpeed(GameState* state, Car* cars);
void setNewHighscore(GameState* state);
void resetVariables(GameState* state, Car* cars);
void resetGame(GameState* state, Car* cars);
void noNegativeScore(GameState* state);



//-------------------
//-----PRINTING------
//-------------------

void initColors() {
	if (has_colors()) {
		start_color();
		init_pair(1, COLOR_YELLOW, COLOR_BLACK);
		init_pair(2, COLOR_BLUE, COLOR_BLACK);
		init_pair(3, COLOR_RED, COLOR_BLACK);
		init_pair(4, COLOR_GREEN, COLOR_BLACK);
		init_pair(5, COLOR_RED, COLOR_RED);
		init_pair(6, COLOR_WHITE, COLOR_BLACK);
	}
}

void setObstacles(GameState* state) {
	int rand_x;
	int rand_y;
	int n;
	if (state->obstaclesSet == false) {
		for (int i = 0; i < MAX_OBSTACLES; i++) {
		Repeat:
			rand_x = (rand() % SCREEN_WIDTH) + 1;
			rand_y = (rand() % (SCREEN_HEIGHT));

			while ((rand_y % 2 != 0) || (rand_y == 0) || (rand_y == SCREEN_HEIGHT - 1)) {
				rand_y = (rand() % (SCREEN_HEIGHT));
			}

			n = (rand() % 10) + 1;
			if (n < 3) {
				n = 3;
			}
			for (int i = 0; i < n; i++) {
				if (state->obstacleMap[rand_y][rand_x + i] == OBSTACLE_SYMBOL) {
					goto Repeat;
				}
				state->obstacleMap[rand_y][rand_x + i] = OBSTACLE_SYMBOL;
			}
		}
		for (int i = 0; i < SCREEN_HEIGHT; i++) {
			state->obstacleMap[i][0] = ' ';
			state->obstacleMap[i][SCREEN_WIDTH - 1] = ' ';
		}
		for (int j = 1; j < SCREEN_HEIGHT; j += 2) {
			for (int i = 0; i < SCREEN_WIDTH; i++) {
				state->obstacleMap[j][i] = ' ';

			}
		}
		state->obstaclesSet = true;
	}
}

void obstaclesToArray(GameState* state) {
	//set obstacles and put them on the map array
	setObstacles(state);
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		for (int j = 0; j < SCREEN_WIDTH; j++) {
			if (state->obstacleMap[i][j] == OBSTACLE_SYMBOL) {
				state->map[i][j] = OBSTACLE_SYMBOL;
			}
		}
	}
}

void setBordersAndSeparators(GameState* state) {
	//put these objects on the map array:
	//-top border
	//-bottom border
	//-lane separators
	for (int i = 0; i < SCREEN_WIDTH; i++) {
		for (int j = 2; j < SCREEN_HEIGHT; j++) {
			if (j % 2 == 0) {
				state->map[j][i] = state->lane_separator;
			}
		}
		state->map[0][i] = state->border_symbol;
		state->map[SCREEN_HEIGHT - 1][i] = state->border_symbol;
	}
	//put side borders on the map array
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		state->map[i][0] = state->border_symbol;
		state->map[i][SCREEN_WIDTH - 1] = state->border_symbol;
	}
}

void printMapArray(GameState* state) {
	//chooses color nr 1 which is yellow
	attron(COLOR_PAIR(MAP_ELEMENTS_COLOR));
	//print map array
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		for (int j = 0; j < SCREEN_WIDTH; j++) {
			mvprintw(i, j, "%c", state->map[i][j]);
		}
	}
	//color off
	attroff(COLOR_PAIR(MAP_ELEMENTS_COLOR));
}

void printFinishLane(GameState* state) {
	//finish lane background color = red
	attron(COLOR_PAIR(FINISH_LANE_COLOR));
	for (int i = 1; i < SCREEN_WIDTH - 1; i++) {
		state->map[1][i] = '_';
		mvprintw(1, i, "%c", state->map[1][i]);
	}
	attroff(COLOR_PAIR(FINISH_LANE_COLOR));
}

void printFrog(GameState* state) {
	//choose color for the frog
	attron(COLOR_PAIR(state->frog_color));
	//print frog
	for (int j = 0; j < FROG_HEIGHT; j++) {
		for (int i = 0; i < FROG_WIDTH; i++) {
			mvaddch(state->frog_y + j, state->frog_x + i, state->frog_symbol);
		}
	}
	attroff(COLOR_PAIR(state->frog_color));
}

int isAtWholeNumber(float timer) {
	if (timer - floor(timer) >= 0.0 && timer - floor(timer) <= TIMER_ADDITION) {
		return true;
	}
	return false;
}

void printCars(GameState* state, Car* cars) {
	//print cars
	for (int i = 0; i < MAX_CARS; i++) {
		//choose color for cars
		attron(COLOR_PAIR(cars[i].color));
		if (isAtWholeNumber(state->timer)) {
			mvaddch(cars[i].car_y, (int)cars[i].car_x, cars[i].symbol);
		}
		else if (!isAtWholeNumber(state->timer)) {
			mvaddch(cars[i].car_y, floor(cars[i].car_x), cars[i].symbol);
		}
		attroff(COLOR_PAIR(cars[i].color));
	}
}

void printGameInfo(GameState* state) {
	mvprintw(0, 1, "Time: [%.2f]s", state->timer);
	mvprintw(0, SCREEN_WIDTH - 20, "Points: [%d]", state->points);
	mvprintw(0, SCREEN_WIDTH - 7, "HS: [%d]", state->highscore);
	mvprintw(SCREEN_HEIGHT + 1, 1, "Stanislaw Swirydczuk 197896");
}

void setAndPrintVisuals(GameState* state, Car* cars) {
	setBordersAndSeparators(state);
	obstaclesToArray(state);
	printMapArray(state);
	printFinishLane(state);
	printCars(state, cars);
	printFrog(state);
	printGameInfo(state);
}



//-------------------
//-------FROG--------
//-------------------

void checkForObstacle(GameState* state) {
	if (state->map[state->frog_y - 1][state->frog_x] == OBSTACLE_SYMBOL) {
		state->moveUp = false;
	}
	if (state->map[state->frog_y + 1][state->frog_x] == OBSTACLE_SYMBOL) {
		state->moveDown = false;
	}
	if (state->map[state->frog_y][state->frog_x - 1] == OBSTACLE_SYMBOL) {
		state->moveLeft = false;
	}
	if (state->map[state->frog_y][state->frog_x + 1] == OBSTACLE_SYMBOL) {
		state->moveRight = false;
	}
}

void setAllMovementTrue(GameState* state) {
	state->moveLeft = true;
	state->moveRight = true;
	state->moveUp = true;
	state->moveDown = true;
}

int frogRideOff(GameState* state, Car* cars) {
	for (int i = 0; i < MAX_CARS; i++) {
		if (state->map[state->frog_y][state->frog_x] != OBSTACLE_SYMBOL && cars[i].frogRide == true) {
			cars[i].frogRide = false;
			return true;
		}
	}
	return false;
}

void upCase(GameState* state) {
	if (state->frog_y > 1 && state->moveUp) {
		state->frog_y -= 1;
		state->inputDetected = true;
		state->save_timer = state->timer;
	}
}

void downCase(GameState* state) {
	if (state->frog_y < SCREEN_HEIGHT - FROG_HEIGHT - 1 && state->moveDown) {
		state->frog_y += 1;
		state->inputDetected = true;
		state->save_timer = state->timer;
	}
}

void leftCase(GameState* state) {
	if (state->frog_x > 2 && state->moveLeft) {
		state->frog_x -= 1;
		state->inputDetected = true;
		state->save_timer = state->timer;
	}
}

void rightCase(GameState* state) {
	if (state->frog_x < SCREEN_WIDTH - FROG_WIDTH - 1 && state->moveRight) {
		state->frog_x += 1;
		state->inputDetected = true;
		state->save_timer = state->timer;
	}
}

void awaitingPickUp(GameState* state) {
	if (state->move == 32 && state->awaiting == false) {
		state->frog_symbol = tolower(state->frog_symbol);
		state->awaiting = true;
	}
	else if (state->move == 32 && state->awaiting == true) {
		state->awaiting = false;
		state->frog_symbol = toupper(state->frog_symbol);
	}
}

void inputDetect(GameState* state, Car* cars) {
	state->move = getch();
	awaitingPickUp(state);
	if (state->inputDetected == false && state->awaiting == false) {
		switch (state->move) {
		case 'w':
		case 'W':
		case KEY_UP:
			if (frogRideOff(state, cars)) {
				break;
			}
			upCase(state);
			break;
		case 's':
		case 'S':
		case KEY_DOWN:
			if (frogRideOff(state, cars)) {
				break;
			}
			downCase(state);
			break;
		case 'a':
		case 'A':
		case KEY_LEFT:
			if (frogRideOff(state, cars)) {
				break;
			}
			leftCase(state);
			break;
		case 'd':
		case 'D':
		case KEY_RIGHT:
			if (frogRideOff(state, cars)) {
				break;
			}
			rightCase(state);
			break;
		case 'q':
		case 'Q':
		case 27:
			state->quit = 0;
			break;

		case 'n':
		case 'N':
			state->timer = state->max_time + 1;
			break;
		}
	}
}

void letAnotherDetect(GameState* state) {
	if (state->timer - state->save_timer > DELAY_AFTER_JUMP) {
		state->inputDetected = false;
	}
}

void ifScored(GameState* state) {
	if (state->frog_y == 1) {
		state->points += 10;
		state->frog_y = SCREEN_HEIGHT - 2;
		state->frog_x = SCREEN_WIDTH / 2;
	}
}



//-------------------
//-------CARS--------
//-------------------

void typeChoiceCase(int choice, int n, Car* cars, char symbol, int i) {
	if (choice == n) {
		cars[i].type = symbol;
	}
}

void interactionChoiceCase(int choice, int n, Car* cars, char symbol, int i, int color) {
	if (choice == n) {
		cars[i].interaction = symbol;
		cars[i].color = color;
	}
}

void carDirection(int i, Car* cars) {
	if (i % 2) { // nieparzyste go right
		cars[i].car_x = 2;
		cars[i].direction = 'r';
	}
	else if (!(i % 2)) { //parzyste go left
		cars[i].car_x = SCREEN_WIDTH - 2;
		cars[i].direction = 'l';
	}
}

void initializeCars(Car* cars, GameState* state) {
	for (int i = 0; i < MAX_CARS; i++) {
		if (cars[i].initialized == 0) {

			int speed_choice = (rand() % 3) + 1;
			int type_choice = (rand() % 3) + 1;
			int interaction_choice = (rand() % 3) + 1;

			cars[i].car_y = (i * 2) + 3;

			carDirection(i, cars);

			typeChoiceCase(type_choice, 1, cars, 'w', i);
			typeChoiceCase(type_choice, 2, cars, 'b', i);
			typeChoiceCase(type_choice, 3, cars, 'd', i);

			interactionChoiceCase(interaction_choice, 1, cars, 'p', i, state->passive_car_color);
			interactionChoiceCase(interaction_choice, 2, cars, 'a', i, state->aggressive_car_color);
			interactionChoiceCase(interaction_choice, 3, cars, 'f', i, state->friendly_car_color);

			cars[i].speed = speed_choice;
			cars[i].symbol = state->car_symbol;
			cars[i].iters = 0;
			cars[i].initialized = 1;
		}
	}
}

void changeCarAfterNumOfIters(int iterations, Car* cars, int i) {
	if (cars[i].iters > iterations) {
		cars[i].iters = 0;
		cars[i].initialized = 0;
	}
}

void bounceCar(Car* cars, int i, GameState* state) {
	if (cars[i].type == 'b') {
		if (cars[i].car_x - cars[i].speed * CAR_SPEED_VAR <= 0) {
			cars[i].iters++;
			changeCarAfterNumOfIters(state->number_of_bounces, cars, i);
			cars[i].direction = 'r';
		}
		else if (cars[i].car_x + cars[i].speed * CAR_SPEED_VAR >= SCREEN_WIDTH) {
			cars[i].iters++;
			changeCarAfterNumOfIters(state->number_of_bounces, cars, i);
			cars[i].direction = 'l';
		}
	}
}

void wrapCar(Car* cars, int i, GameState* state) {
	if (cars[i].type == 'w') {
		if (cars[i].car_x - cars[i].speed * CAR_SPEED_VAR <= 0 && cars[i].direction == 'l') {
			cars[i].iters++;
			changeCarAfterNumOfIters(state->number_of_wraps, cars, i);
			cars[i].car_x = SCREEN_WIDTH;
		}
		else if (cars[i].car_x + cars[i].speed * CAR_SPEED_VAR >= SCREEN_WIDTH && cars[i].direction == 'r') {
			cars[i].iters++;
			changeCarAfterNumOfIters(state->number_of_wraps, cars, i);
			cars[i].car_x = 0;
		}
	}
}

void disappearCar(Car* cars, int i, GameState* state) {
	if (cars[i].type == 'd') {
		if (cars[i].car_x - cars[i].speed * CAR_SPEED_VAR <= 0 && cars[i].direction == 'l' || cars[i].car_x + cars[i].speed * CAR_SPEED_VAR >= SCREEN_WIDTH && cars[i].direction == 'r') {
			cars[i].frogRide = false;
			if (cars[i].interaction == 'f' && cars[i].frogRide == true) {
				if (cars[i].direction == 'r') {
					state->frog_x -= 5;
				}
				else if (cars[i].direction == 'l') {
					state->frog_x += 5;
				}
			}
			cars[i].initialized = 0;
		}
	}
}

void moveCars(Car* cars, GameState* state) {
	for (int i = 0; i < MAX_CARS; i++) {
		if (cars[i].stopCar == false) {
			bounceCar(cars, i, state);
			wrapCar(cars, i, state);
			disappearCar(cars, i, state);
			if (cars[i].direction == 'r') {
				cars[i].car_x += cars[i].speed * CAR_SPEED_VAR;
			}
			else if (cars[i].direction == 'l') {
				cars[i].car_x -= cars[i].speed * CAR_SPEED_VAR;
			}
		}
	}
}

void aggressiveCase(int i, Car* cars, GameState* state) {
	if (cars[i].interaction == 'a') {
		state->collisionDetected++;
		state->points -= 5;
		state->frog_y = SCREEN_HEIGHT - 2;
		state->frog_x = SCREEN_WIDTH / 2;
	}
}

void passiveCase(int i, Car* cars, GameState* state) {
	if (cars[i].interaction == 'p') {
		cars[i].stopCar = true;
	}
}

void friendlyCase(int i, Car* cars, GameState* state) {
	if (cars[i].interaction == 'f' && !state->awaiting) {
		cars[i].stopCar = true;
	}
	else if (cars[i].interaction == 'f' && state->awaiting) {
		cars[i].stopCar = false;
		cars[i].frogRide = true;
		state->frog_x = cars[i].car_x;
		state->frog_y = cars[i].car_y;
	}
}

void hits(int i, Car* cars, GameState* state) {
	if ((state->frog_x <= cars[i].car_x + state->r) &&
		(state->frog_x >= cars[i].car_x - state->r) &&
		(state->frog_y == cars[i].car_y)) {
		aggressiveCase(i, cars, state);
		passiveCase(i, cars, state);
		friendlyCase(i, cars, state);
	}
	else if ((((cars[i].car_x <= state->frog_x + 4) &&
		(cars[i].car_x >= state->frog_x) &&
		(cars[i].car_y == state->frog_y)) &&
		(cars[i].direction == 'l')) ||
		(((cars[i].car_x >= state->frog_x - 4) &&
			(cars[i].car_x <= state->frog_x) &&
			(cars[i].car_y == state->frog_y)) &&
			(cars[i].direction == 'r'))) {
		passiveCase(i, cars, state);
		friendlyCase(i, cars, state);
	}
	else if ((((state->frog_x <= cars[i].car_x + state->r) &&
		(state->frog_x >= cars[i].car_x - state->r) &&
		(state->frog_y == cars[i].car_y)) &&
		(state->awaiting == true)) ||
		(cars[i].frogRide == true)) {
		friendlyCase(i, cars, state);
	}
	else {
		cars[i].stopCar = false;
	}
}

void collisionDetect(Car* cars, GameState* state) {
	for (int i = 0; i < MAX_CARS; i++) {
		hits(i, cars, state);
	}
}



//-------------------
//------CONFIG-------
//-------------------

void extractValue(char key[128], char value[128], const char* var_name, int* var_value_int, char* var_value_char) {
	if (strcmp(key, var_name) == 0) {
		if (var_value_int != NULL) {
			*var_value_int = atoi(value);
		}
		else if (var_value_char != NULL) {
			*var_value_char = value[0];
		}
	}
}

void extractValues(char key[128], char value[128], GameState* state) {
	extractValue(key, value, "frog_color", &state->frog_color, NULL);
	extractValue(key, value, "passive_car_color", &state->passive_car_color, NULL);
	extractValue(key, value, "aggressive_car_color", &state->aggressive_car_color, NULL);
	extractValue(key, value, "friendly_car_color", &state->friendly_car_color, NULL);
	extractValue(key, value, "number_of_bounces", &state->number_of_bounces, NULL);
	extractValue(key, value, "number_of_wraps", &state->number_of_wraps, NULL);
	extractValue(key, value, "frog_symbol", NULL, &state->frog_symbol);
	extractValue(key, value, "car_symbol", NULL, &state->car_symbol);
	extractValue(key, value, "border_symbol", NULL, &state->border_symbol);
	extractValue(key, value, "lane_separator", NULL, &state->lane_separator);
	extractValue(key, value, "max_time", &state->max_time, NULL);
}

void readConfigFile(const char* filename, GameState* state) {
	FILE* file = fopen(filename, "r");

	char line[256];
	while (fgets(line, sizeof(line), file)) {
		char key[128], value[128];
		if (sscanf(line, "%[^=]=%s", key, value) == 2) {
			extractValues(key, value, state);
		}
	}
	fclose(file);
}



//-------------------
//-------GAME--------
//-------------------

void changeOfSpeed(GameState* state, Car* cars) {
	if (((int)state->timer % SPEED_CHANGE_INTERVAL) == 0 && state->timer != 0) {
		for (int i = 0; i < MAX_CARS; i++) {
			int speed_choice = (rand() % 3) + 1;
			cars[i].speed = speed_choice;
		}
	}
}

void setNewHighscore(GameState* state) {
	if (state->highscore < state->points) {
		state->highscore = state->points;
	}
}

void resetVariables(GameState* state, Car* cars) {
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		for (int j = 0; j < SCREEN_WIDTH; j++) {
			state->map[i][j] = ' ';
			state->obstacleMap[i][j] = ' ';
		}
	}
	state->obstaclesSet = false;
	state->frog_y = SCREEN_HEIGHT - 2;
	state->frog_x = SCREEN_WIDTH / 2;
	state->timer = 0;
	state->points = 0;
	state->r = CAR_SPEED_VAR + 1;
	state->collisionDetected = 0;
	state->moveLeft = true;
	state->moveRight = true;
	state->moveUp = true;
	state->moveDown = true;
	for (int i = 0; i < MAX_CARS; i++) {
		cars[i].frogRide = false;
		cars[i].initialized = 0;
	}
}

void resetGame(GameState* state, Car* cars) {
	if (state->timer > state->max_time) {
		setNewHighscore(state);
		resetVariables(state, cars);
	}
}

void noNegativeScore(GameState* state) {
	if (state->points < 0) {
		state->points = 0;
	}
}



//-------------------
//-------MAIN--------
//-------------------

int main() {
	GameState state;
	Car cars[MAX_CARS];
	srand(time(NULL));
	state.quit = 1;
	initscr(); cbreak(); noecho(); curs_set(0); keypad(stdscr, TRUE); nodelay(stdscr, TRUE);
	initColors();
	readConfigFile("config.txt", &state);
	while (state.quit) {
		clear();
		setAndPrintVisuals(&state, cars);
		refresh();
		napms(1);
		state.timer += TIMER_ADDITION; //pretty accurate portrayal of real time
		setAllMovementTrue(&state);
		checkForObstacle(&state);
		inputDetect(&state, cars);
		letAnotherDetect(&state);
		ifScored(&state);
		initializeCars(cars, &state);
		changeOfSpeed(&state, cars);
		moveCars(cars, &state);
		collisionDetect(cars, &state);
		noNegativeScore(&state);
		resetGame(&state, cars);
	}
	endwin();
	return 0;
}