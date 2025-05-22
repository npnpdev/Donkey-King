#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	600
#define BITMAP_WIDTH    16
#define BITMAP_HEIGHT   BITMAP_WIDTH					// Zachowanie proporcji
#define PLATFORM_SPACE	SCREEN_WIDTH/10					// Odstêp miêdzy platformami
#define BARREL_RESPAWN_TIME 3							// Beczka pojawia siê po tym czasie + czas trwania animacji ma³py, czas w sekundach
#define MAX_LIVES 3										// Liczba ¿yæ po odrodzeniu postaci (maksymalna liczba ¿yæ)
#define BARREL_JUMP_SCORE 300							// Liczba punktów za przeskoczenie beczki
#define TROPHY_SCORE 800								// Liczba punktów za zebranie trofeum
#define END_STAGE_SCORE 1000							// Liczba punktów za zakoñczenie etapu
#define SCORE_POPUP_TIME 1000							// Czas wyœwietlania punktów w milisekundach 
#define MAX_SCORES_PER_PAGE 10							// Maksymalna liczba wyników na jednej stronie
#define MONKEY_ANIMATION_DURATION 200					// Czas trwania animacji w milisekundach
#define PLAYER_ANIMATION_FRAMES 3						// Iloœc klatek jaka sk³ada siê na animacje gracza
#define PLAYER_ANIMATION_DURATION 100					// Czas trwania jednej klatki animacji biegania postaci
#define BARREL_ANIMATION_DURATION 150					// Czas trwania jednej klatki animacji beczki

const double PLAYER_SPEED = 300.0f;						//Prêdkoœæ chodzenia postaci (prawo-lewo)
const double CLIMBING_SPEED = 100.0f;					//Prêdkoœæ wspinania postaci (góra-dó³)

const int MAX_BARRELS = 20;								// Maksymalna liczba beczek	

const int FPS = 60000;									// Maksymalna liczba FPS
const int frameDelay = 1000 / FPS;						// Czas trwania jednej klatki

const float jumpSpeed = -245.0f;						//Prêdkoœæ pocz¹tkowa skoku
const float gravity = 1000.0f;							//Prêdkoœæ spadania

//Tekstury gry
struct GameTextures {
	SDL_Texture* scrtex;
	SDL_Texture* platformTexture;
	SDL_Texture* playerTexture;
	SDL_Texture* playerTexture0;
	SDL_Texture* playerTextureBeforeRun;
	SDL_Texture* playerTextureBeforeRunFlipped;
	SDL_Texture* playerTextureRunning;
	SDL_Texture* playerTextureRunningFlipped;
	SDL_Texture* playerTextureAfterRun;
	SDL_Texture* playerTextureAfterRunFlipped;
	SDL_Texture* playerTextureBeforeJump;
	SDL_Texture* playerTextureJumping;
	SDL_Texture* playerTextureClimbing1;
	SDL_Texture* playerTextureClimbing2;
	SDL_Texture* monkeyTexture;
	SDL_Texture* monkeyTexture0;
	SDL_Texture* monkeyTexture1;
	SDL_Texture* ledderTexture;
	SDL_Texture* barrelTexture;
	SDL_Texture* princessTexture;
	SDL_Texture* barrel_rollTexture;
	SDL_Texture* barrel_rollTexture0;
	SDL_Texture* barrel_rollTexture1;
	SDL_Texture* barrel_rollTexture2;
	SDL_Texture* heartTexture;
	SDL_Texture* trophyTexture;
};

//Kolory gry
struct GameColors {
	int czarny;
	int zielony;
	int czerwony;
	int niebieski;
	int jasnyniebieski;
};

//Hitboxy gry
struct GameRects {
	SDL_Rect* ledderRects;
	SDL_Rect* platformRects;
	SDL_Rect* staticBarrelRects;
	SDL_Rect* monkeyRects;
	SDL_Rect* princessRects;
	SDL_Rect playerRect;
	SDL_Rect* trophyRects;
};

// Zmienne wykorzystywany do przechowywania parametrów gry
struct GameState {
	int numOfPlatforms;
	int numOfLedders;
	int numOfStaticBarrels;
	int numOfMonkeys;
	int numOfPrincesses;
	int numOfTrophies;
	int quit = 0;
	int frameStart;
	int frameTime;
	int frameCount;
	int startTick;
	int lastPrintTime;
	int lastFrameTime;
	int isMoving = 0;
	int isJumping = 0;
	int isFalling = 0;
	int isClimbing = 0;
	int lives = MAX_LIVES;
	int score = 0;
	int lastScorePopupTime;
	int lastScorePoints;
	int currentStage;
	int localStage;
	int animationCurrentFrame;
	double deltaTime;
	double fps;
	double worldTime = 0.0f;
	double verticalSpeed;
	double realPosY = 0.0f;
	double realPosYladder = 0.0f;
	double realPosX = 0.0f;
	float monkeyAnimationStartTime;
	float monkeyAnimationCurrentTime;
	float playerAnimationRunningStartTime;
	float playerAnimationJumpingStartTime;
	float playerAnimationClimbingStartTime;
};

struct Barrel {
	SDL_Rect rect;				// Pozycja i rozmiar beczki
	float realPosX;				// Faktyczna pozycja x
	float realPosY;				// Faktyczna pozycja y
	int playerJumpedOver;		// Wartosc ustawiana na 1, gdy gracz juz przeskoczy³ tê beczkê.
	int targetPlatform = 5;		// Indeks docelowej platformy
	int speed = 300;			// Prêdkoœæ beczki - iloœæ pikseli jak¹ pokonuje
	int movingRight;			// Kierunek ruchu beczki, 1 oznacza ze porusza sie w prawo
	int isFalling = 0;			// Wartoœæ ustawiana na 1, gdy beczka wyjdzie poza zakres platformy
	double lastFrameChangeTime;	// Zmienna wykorzystywana przy animacji
};

Barrel barrels[MAX_BARRELS];  // Tablica struktur beczek

//Struktura wykorzystywana przy wyœwietlaniu wyniku w menu
struct Score {
	char playerName[128];
	int score;
};

// NIEU¯YWANE FUNKCJE - ZAMIAST TEGO ETAPY £ADOWANE Z PLIKU
/*
void preparePlatformsStageOne(SDL_Rect platformRects[], SDL_Renderer* renderer, SDL_Texture* platformTexture, GameState* state) {

	//Pod³oga - platforma 0
	platformRects[0].x = 0;
	platformRects[0].y = SCREEN_HEIGHT - BITMAP_HEIGHT;
	platformRects[0].w = SCREEN_WIDTH;
	platformRects[0].h = BITMAP_HEIGHT;


	int numberOfLevels = 5; // Liczba poziomów platform œrodkowych

	for (int i = 1; i <= numberOfLevels; ++i) {
		if (i % 2 == 0) {
			platformRects[i].x = BITMAP_WIDTH * 4;
			platformRects[i].y = SCREEN_HEIGHT - PLATFORM_SPACE * i;
			platformRects[i].y = platformRects[i - 1].y - PLATFORM_SPACE;
			platformRects[i].w = SCREEN_WIDTH - platformRects[i].x;

		}
		else {
			platformRects[i].x = 0;
			platformRects[i].y = platformRects[i - 1].y - PLATFORM_SPACE;
			platformRects[i].w = SCREEN_WIDTH - (BITMAP_WIDTH * 4);
		}
		platformRects[i].h = BITMAP_HEIGHT;
	}

	//Ostatnia platforma
	platformRects[6].x = (BITMAP_WIDTH * 12);
	platformRects[6].y = platformRects[5].y - PLATFORM_SPACE;
	platformRects[6].w = SCREEN_WIDTH / 2;
	platformRects[6].h = BITMAP_HEIGHT;

	for (int i = 0; i < state->numOfPlatforms; ++i) {
		DrawTiledTextureOnElement(renderer, platformTexture, platformRects[i]);
	}
}

void preparePlatformsStageTwo(SDL_Rect platformRects[], SDL_Renderer* renderer, SDL_Texture* platformTexture, GameState* state) {
	//Pod³oga - platforma 0
	platformRects[0].x = 0;
	platformRects[0].y = SCREEN_HEIGHT - BITMAP_HEIGHT;
	platformRects[0].w = SCREEN_WIDTH;

	int numberOfLevels = 7; // Liczba poziomów platform œrodkowych

	platformRects[1].x = 0;
	platformRects[1].y = platformRects[0].y - PLATFORM_SPACE;
	platformRects[1].w = SCREEN_WIDTH / 2 - BITMAP_WIDTH * 2;

	platformRects[2].x = SCREEN_WIDTH / 2 + BITMAP_WIDTH * 2;
	platformRects[2].y = platformRects[1].y;
	platformRects[2].w = SCREEN_WIDTH / 2 - BITMAP_WIDTH * 2;

	platformRects[3].x = 0;
	platformRects[3].y = platformRects[2].y - PLATFORM_SPACE;
	platformRects[3].w = SCREEN_WIDTH - (SCREEN_WIDTH / 12);

	platformRects[4].x = SCREEN_WIDTH / 12;
	platformRects[4].y = platformRects[3].y - PLATFORM_SPACE;
	platformRects[4].w = SCREEN_WIDTH;

	platformRects[5].x = 0;
	platformRects[5].y = platformRects[4].y - PLATFORM_SPACE;
	platformRects[5].w = SCREEN_WIDTH / 2 - BITMAP_WIDTH * 2;

	platformRects[6].x = SCREEN_WIDTH / 2 + BITMAP_WIDTH * 2;
	platformRects[6].y = platformRects[5].y;
	platformRects[6].w = SCREEN_WIDTH / 2 - BITMAP_WIDTH * 2;

	platformRects[7].x = 0;
	platformRects[7].y = platformRects[6].y - PLATFORM_SPACE;
	platformRects[7].w = SCREEN_WIDTH - BITMAP_WIDTH * 4;

	//Ostatnia platforma
	platformRects[8].x = SCREEN_WIDTH / 4;
	platformRects[8].y = platformRects[7].y - PLATFORM_SPACE;;
	platformRects[8].w = SCREEN_WIDTH / 2;

	for (int i = 0; i < numberOfLevels + 2; i++) {
		platformRects[i].h = BITMAP_HEIGHT;
	}

	for (int i = 0; i < state->numOfPlatforms; ++i) {
		DrawTiledTextureOnElement(renderer, platformTexture, platformRects[i]);
	}
}

void preparePlatformsStageThree(SDL_Rect platformRects[], SDL_Renderer* renderer, SDL_Texture* platformTexture) {
	//Pod³oga - platforma 0
	platformRects[0].x = 0;
	platformRects[0].y = SCREEN_HEIGHT - BITMAP_HEIGHT;
	platformRects[0].w = SCREEN_WIDTH;

	int numberOfLevels = 7; // Liczba poziomów platform œrodkowych

	//Poziom 1
	platformRects[1].x = 0;
	platformRects[1].y = platformRects[0].y - PLATFORM_SPACE;
	platformRects[1].w = SCREEN_WIDTH;


	//Poziom 2 od prawej
	platformRects[2].x = SCREEN_WIDTH / 2 + BITMAP_WIDTH * 2;
	platformRects[2].y = platformRects[1].y - PLATFORM_SPACE;
	platformRects[2].w = SCREEN_WIDTH / 2 - BITMAP_WIDTH * 2;

	platformRects[3].x = 0;
	platformRects[3].y = platformRects[1].y - PLATFORM_SPACE;
	platformRects[3].w = SCREEN_WIDTH / 2 - BITMAP_WIDTH * 2;

	//Poziom 3
	platformRects[4].x = BITMAP_WIDTH * 4;
	platformRects[4].y = platformRects[2].y - PLATFORM_SPACE;
	platformRects[4].w = SCREEN_WIDTH - BITMAP_WIDTH * 4;

	//Poziom 4 od lewej
	platformRects[5].x = 0;
	platformRects[5].y = platformRects[4].y - PLATFORM_SPACE;
	platformRects[5].w = SCREEN_WIDTH / 2 - BITMAP_WIDTH * 2;

	platformRects[6].x = SCREEN_WIDTH / 2 + BITMAP_WIDTH * 2;
	platformRects[6].y = platformRects[4].y - PLATFORM_SPACE;
	platformRects[6].w = SCREEN_WIDTH / 2 - BITMAP_WIDTH * 2;

	//Poziom 5
	platformRects[7].x = 0;
	platformRects[7].y = platformRects[6].y - PLATFORM_SPACE;
	platformRects[7].w = SCREEN_WIDTH - BITMAP_WIDTH * 4;

	//Ostatnia platforma
	platformRects[8].x = SCREEN_WIDTH / 4;
	platformRects[8].y = platformRects[7].y - PLATFORM_SPACE;;
	platformRects[8].w = SCREEN_WIDTH / 4;

	for (int i = 0; i < numberOfLevels + 2; i++) {
		platformRects[i].h = BITMAP_HEIGHT;
	}

	for (int i = 0; i < NUMBER_OF_PLATFORMS; ++i) {
		DrawTiledTextureOnElement(renderer, platformTexture, platformRects[i]);
	}
}

void prepareLeddersStageOne(SDL_Rect platformRects[], SDL_Renderer* renderer, SDL_Texture* ledderTexture, SDL_Rect ledderRects[]) {

	//Poziom 1
	ledderRects[0].x = platformRects[4].w - (BITMAP_WIDTH * 3);
	ledderRects[0].y = platformRects[0].y - PLATFORM_SPACE;

	//Poziom 2
	ledderRects[1].x = platformRects[6].x + (BITMAP_WIDTH * 6);
	ledderRects[1].y = platformRects[1].y - PLATFORM_SPACE;
	ledderRects[1].w = BITMAP_WIDTH;
	ledderRects[1].h = PLATFORM_SPACE;

	ledderRects[2].x = platformRects[5].x + (BITMAP_WIDTH * 5);
	ledderRects[2].y = platformRects[1].y - PLATFORM_SPACE;
	ledderRects[2].w = BITMAP_WIDTH;
	ledderRects[2].h = PLATFORM_SPACE;

	//Poziom 3
	ledderRects[3].x = SCREEN_WIDTH / 2;
	ledderRects[3].y = platformRects[2].y - PLATFORM_SPACE;
	ledderRects[3].w = BITMAP_WIDTH;
	ledderRects[3].h = PLATFORM_SPACE;

	ledderRects[4].x = platformRects[4].w - (BITMAP_WIDTH * 3);
	ledderRects[4].y = platformRects[2].y - PLATFORM_SPACE;
	ledderRects[4].w = BITMAP_WIDTH;
	ledderRects[4].h = PLATFORM_SPACE;

	//Poziom 4
	ledderRects[5].x = SCREEN_WIDTH / 4;
	ledderRects[5].y = platformRects[3].y - PLATFORM_SPACE;
	ledderRects[5].w = BITMAP_WIDTH;
	ledderRects[5].h = PLATFORM_SPACE;

	ledderRects[6].x = platformRects[5].x + (BITMAP_WIDTH * 5);
	ledderRects[6].y = platformRects[3].y - PLATFORM_SPACE;
	ledderRects[6].w = BITMAP_WIDTH;
	ledderRects[6].h = PLATFORM_SPACE;

	//Poziom 5
	ledderRects[7].x = platformRects[4].w - (BITMAP_WIDTH * 3);
	ledderRects[7].y = platformRects[4].y - PLATFORM_SPACE;
	ledderRects[7].w = BITMAP_WIDTH;
	ledderRects[7].h = PLATFORM_SPACE;

	//Poziom 6
	ledderRects[8].x = platformRects[5].w - (SCREEN_WIDTH / 5);
	ledderRects[8].y = platformRects[5].y - PLATFORM_SPACE;
	ledderRects[8].w = BITMAP_WIDTH;
	ledderRects[8].h = PLATFORM_SPACE;

	ledderRects[9].x = platformRects[6].x - (BITMAP_WIDTH);
	ledderRects[9].y = platformRects[5].y - PLATFORM_SPACE;
	ledderRects[9].w = BITMAP_WIDTH;
	ledderRects[9].h = PLATFORM_SPACE;

	ledderRects[10].x = platformRects[6].x - (BITMAP_WIDTH * 3);
	ledderRects[10].y = platformRects[5].y - PLATFORM_SPACE;
	ledderRects[10].w = BITMAP_WIDTH;
	ledderRects[10].h = PLATFORM_SPACE;

	for (int i = 0; i < 11; ++i) {
		DrawTiledTextureOnElement(renderer, ledderTexture, ledderRects[i]);
	}
}

void prepareLeddersStageTwo(SDL_Rect platformRects[], SDL_Renderer* renderer, SDL_Texture* ledderTexture, SDL_Rect ledderRects[]) {

	//Poziom 1
	ledderRects[0].x = platformRects[1].w - (BITMAP_WIDTH * 3);
	ledderRects[0].y = platformRects[0].y - PLATFORM_SPACE;
	ledderRects[0].w = BITMAP_WIDTH;
	ledderRects[0].h = PLATFORM_SPACE;

	ledderRects[1].x = platformRects[3].w;
	ledderRects[1].y = platformRects[0].y - PLATFORM_SPACE;
	ledderRects[1].w = BITMAP_WIDTH;
	ledderRects[1].h = PLATFORM_SPACE;

	//Poziom 2

	ledderRects[2].x = platformRects[5].x + (BITMAP_WIDTH * 5);
	ledderRects[2].y = platformRects[1].y - PLATFORM_SPACE;
	ledderRects[2].w = BITMAP_WIDTH;
	ledderRects[2].h = PLATFORM_SPACE;

	ledderRects[3].x = platformRects[3].w - PLATFORM_SPACE;
	ledderRects[3].y = platformRects[2].y - PLATFORM_SPACE;
	ledderRects[3].w = BITMAP_WIDTH;
	ledderRects[3].h = PLATFORM_SPACE;

	//Poziom 3

	ledderRects[4].x = ledderRects[0].x;
	ledderRects[4].y = platformRects[3].y - PLATFORM_SPACE;
	ledderRects[4].w = BITMAP_WIDTH;
	ledderRects[4].h = PLATFORM_SPACE;

	//Poziom 4

	ledderRects[5].x = platformRects[4].w - (BITMAP_WIDTH * 3);
	ledderRects[5].y = platformRects[4].y - PLATFORM_SPACE;
	ledderRects[5].w = BITMAP_WIDTH;
	ledderRects[5].h = PLATFORM_SPACE;

	//Poziom 5

	ledderRects[6].x = platformRects[5].w - (SCREEN_WIDTH / 5);
	ledderRects[6].y = platformRects[5].y - PLATFORM_SPACE;
	ledderRects[6].w = BITMAP_WIDTH;
	ledderRects[6].h = PLATFORM_SPACE;


	//Poziom 6
	ledderRects[7].x = platformRects[7].w - (SCREEN_WIDTH / 5);
	ledderRects[7].y = platformRects[7].y - PLATFORM_SPACE;
	ledderRects[7].w = BITMAP_WIDTH;
	ledderRects[7].h = PLATFORM_SPACE;

	for (int i = 0; i < 8; ++i) {
		DrawTiledTextureOnElement(renderer, ledderTexture, ledderRects[i]);
	}
}

void prepareLeddersStageThree(SDL_Rect platformRects[], SDL_Renderer* renderer, SDL_Texture* ledderTexture, SDL_Rect ledderRects[]) {

	//Poziom 1
	ledderRects[0].x = platformRects[4].w - BITMAP_WIDTH;
	ledderRects[0].y = platformRects[1].y;
	ledderRects[0].w = BITMAP_WIDTH;
	ledderRects[0].h = PLATFORM_SPACE;

	//Poziom 2
	ledderRects[1].x = platformRects[8].w - BITMAP_WIDTH;
	ledderRects[1].y = platformRects[1].y;
	ledderRects[1].w = BITMAP_WIDTH;
	ledderRects[1].h = PLATFORM_SPACE;

	ledderRects[2].x = SCREEN_WIDTH - SCREEN_WIDTH / 4;
	ledderRects[2].y = platformRects[3].y;
	ledderRects[2].w = BITMAP_WIDTH;
	ledderRects[2].h = PLATFORM_SPACE;

	//Poziom 3
	ledderRects[3].x = SCREEN_WIDTH / 6;
	ledderRects[3].y = platformRects[4].y;
	ledderRects[3].w = BITMAP_WIDTH;
	ledderRects[3].h = PLATFORM_SPACE;

	ledderRects[4].x = SCREEN_WIDTH - (BITMAP_WIDTH * 2);
	ledderRects[4].y = platformRects[4].y;
	ledderRects[4].w = BITMAP_WIDTH;
	ledderRects[4].h = PLATFORM_SPACE;

	//Poziom 4
	ledderRects[5].x = (SCREEN_WIDTH / 2) + SCREEN_WIDTH / 4;
	ledderRects[5].y = platformRects[5].y;
	ledderRects[5].w = BITMAP_WIDTH;
	ledderRects[5].h = PLATFORM_SPACE;

	ledderRects[6].x = SCREEN_WIDTH / 2 - SCREEN_WIDTH / 8;
	ledderRects[6].y = platformRects[6].y;
	ledderRects[6].w = BITMAP_WIDTH;
	ledderRects[6].h = PLATFORM_SPACE;

	//Poziom 5
	ledderRects[7].x = SCREEN_WIDTH / 4;
	ledderRects[7].y = platformRects[7].y;
	ledderRects[7].w = BITMAP_WIDTH;
	ledderRects[7].h = PLATFORM_SPACE;

	ledderRects[8].x = SCREEN_WIDTH / 2 + (BITMAP_WIDTH * 2);
	ledderRects[8].y = platformRects[8].y;
	ledderRects[8].w = BITMAP_WIDTH;
	ledderRects[8].h = PLATFORM_SPACE;

	ledderRects[9].x = SCREEN_WIDTH / 2;
	ledderRects[9].y = platformRects[8].y;
	ledderRects[9].w = BITMAP_WIDTH;
	ledderRects[9].h = PLATFORM_SPACE;

	//Ostatni poziom
	ledderRects[10].x = ledderRects[7].x;
	ledderRects[10].y = platformRects[8].y;
	ledderRects[10].w = BITMAP_WIDTH;
	ledderRects[10].h = BITMAP_HEIGHT;

	for (int i = 0; i < NUMBER_OF_LEDDERS; ++i) {
		DrawTiledTextureOnElement(renderer, ledderTexture, ledderRects[i]);

}

void prepareMonkeyStageOne(GameRects* rects, SDL_Renderer* renderer, SDL_Texture* monkeyTexture) {
	rects->monkeyRects->x = rects->platformRects[5].x + (BITMAP_WIDTH * 4);
	rects->monkeyRects->y = rects->platformRects[6].y + (BITMAP_HEIGHT);
	rects->monkeyRects->w = 64;
	rects->monkeyRects->h = 64;
	SDL_RenderCopy(renderer, monkeyTexture, NULL, rects->monkeyRects);
}

void prepareMonkeyStageTwo(GameRects* rects, SDL_Renderer* renderer, SDL_Texture* monkeyTexture) {
	rects->monkeyRects->x = rects->platformRects[7].x + (BITMAP_WIDTH * 4);
	rects->monkeyRects->y = rects->platformRects[8].y + (BITMAP_HEIGHT);
	rects->monkeyRects->w = 64;
	rects->monkeyRects->h = 64;
	SDL_RenderCopy(renderer, monkeyTexture, NULL, rects->monkeyRects);
}

void prepareMonkeyStageThree(GameRects* rects, SDL_Renderer* renderer, SDL_Texture* monkeyTexture) {
	rects->monkeyRects->x = rects->platformRects[5].x + (BITMAP_WIDTH * 8);
	rects->monkeyRects->y = rects->platformRects[8].y + (BITMAP_HEIGHT);
	rects->monkeyRects->w = 64;
	rects->monkeyRects->h = 64;
	SDL_RenderCopy(renderer, monkeyTexture, NULL, rects->monkeyRects);
}

void prepareBarrelsStageOne(SDL_Renderer* renderer, SDL_Rect staticBarrelRects[], SDL_Rect platformRects[], SDL_Texture* barrelTexture) {
	staticBarrelRects[0].x = platformRects[5].x;
	staticBarrelRects[0].y = platformRects[5].y - BITMAP_HEIGHT * 2;
	staticBarrelRects[0].w = BITMAP_WIDTH * 2;
	staticBarrelRects[0].h = BITMAP_HEIGHT * 2;

	staticBarrelRects[1].x = platformRects[5].x + staticBarrelRects[0].w;
	staticBarrelRects[1].y = platformRects[5].y - BITMAP_HEIGHT * 2;
	staticBarrelRects[1].w = BITMAP_WIDTH * 2;
	staticBarrelRects[1].h = BITMAP_HEIGHT * 2;

	staticBarrelRects[2].x = platformRects[5].x;
	staticBarrelRects[2].y = platformRects[5].y - (staticBarrelRects[0].h * 2);
	staticBarrelRects[2].w = BITMAP_WIDTH * 2;
	staticBarrelRects[2].h = BITMAP_HEIGHT * 2;

	staticBarrelRects[3].x = staticBarrelRects[1].x;
	staticBarrelRects[3].y = platformRects[5].y - (staticBarrelRects[1].h * 2);
	staticBarrelRects[3].w = BITMAP_WIDTH * 2;
	staticBarrelRects[3].h = BITMAP_HEIGHT * 2;

	SDL_RenderCopy(renderer, barrelTexture, NULL, &staticBarrelRects[0]);
	SDL_RenderCopy(renderer, barrelTexture, NULL, &staticBarrelRects[1]);
	SDL_RenderCopy(renderer, barrelTexture, NULL, &staticBarrelRects[2]);
	SDL_RenderCopy(renderer, barrelTexture, NULL, &staticBarrelRects[3]);
}

void prepareBarrelsStageTwo(SDL_Renderer* renderer, SDL_Rect staticBarrelRects[], SDL_Rect platformRects[], SDL_Texture* barrelTexture) {
	staticBarrelRects[0].x = platformRects[7].x;
	staticBarrelRects[0].y = platformRects[7].y - BITMAP_HEIGHT * 2;
	staticBarrelRects[0].w = BITMAP_WIDTH * 2;
	staticBarrelRects[0].h = BITMAP_HEIGHT * 2;

	staticBarrelRects[1].x = platformRects[7].x + staticBarrelRects[0].w;
	staticBarrelRects[1].y = platformRects[7].y - BITMAP_HEIGHT * 2;
	staticBarrelRects[1].w = BITMAP_WIDTH * 2;
	staticBarrelRects[1].h = BITMAP_HEIGHT * 2;

	staticBarrelRects[2].x = platformRects[7].x;
	staticBarrelRects[2].y = platformRects[7].y - (staticBarrelRects[0].h * 2);
	staticBarrelRects[2].w = BITMAP_WIDTH * 2;
	staticBarrelRects[2].h = BITMAP_HEIGHT * 2;

	staticBarrelRects[3].x = staticBarrelRects[1].x;
	staticBarrelRects[3].y = platformRects[7].y - (staticBarrelRects[1].h * 2);
	staticBarrelRects[3].w = BITMAP_WIDTH * 2;
	staticBarrelRects[3].h = BITMAP_HEIGHT * 2;

	SDL_RenderCopy(renderer, barrelTexture, NULL, &staticBarrelRects[0]);
	SDL_RenderCopy(renderer, barrelTexture, NULL, &staticBarrelRects[1]);
	SDL_RenderCopy(renderer, barrelTexture, NULL, &staticBarrelRects[2]);
	SDL_RenderCopy(renderer, barrelTexture, NULL, &staticBarrelRects[3]);
}

void prepareBarrelsStageThree(SDL_Renderer* renderer, SDL_Rect staticBarrelRects[], SDL_Rect platformRects[], SDL_Texture* barrelTexture) {
	staticBarrelRects[0].x = platformRects[7].x + BITMAP_WIDTH * 4;
	staticBarrelRects[0].y = platformRects[7].y - BITMAP_HEIGHT * 2;
	staticBarrelRects[0].w = BITMAP_WIDTH * 2;
	staticBarrelRects[0].h = BITMAP_HEIGHT * 2;

	staticBarrelRects[1].x = staticBarrelRects[0].x + staticBarrelRects[0].w;
	staticBarrelRects[1].y = platformRects[7].y - BITMAP_HEIGHT * 2;
	staticBarrelRects[1].w = BITMAP_WIDTH * 2;
	staticBarrelRects[1].h = BITMAP_HEIGHT * 2;

	staticBarrelRects[2].x = staticBarrelRects[0].x;
	staticBarrelRects[2].y = platformRects[7].y - (staticBarrelRects[0].h * 2);
	staticBarrelRects[2].w = BITMAP_WIDTH * 2;
	staticBarrelRects[2].h = BITMAP_HEIGHT * 2;

	staticBarrelRects[3].x = staticBarrelRects[1].x;
	staticBarrelRects[3].y = platformRects[7].y - (staticBarrelRects[1].h * 2);
	staticBarrelRects[3].w = BITMAP_WIDTH * 2;
	staticBarrelRects[3].h = BITMAP_HEIGHT * 2;

	SDL_RenderCopy(renderer, barrelTexture, NULL, &staticBarrelRects[0]);
	SDL_RenderCopy(renderer, barrelTexture, NULL, &staticBarrelRects[1]);
	SDL_RenderCopy(renderer, barrelTexture, NULL, &staticBarrelRects[2]);
	SDL_RenderCopy(renderer, barrelTexture, NULL, &staticBarrelRects[3]);
}

void preparePrincessStageOne(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Texture* princessTexture, SDL_Rect& princessRect, SDL_Rect platformRects[]) {
	//Ksiê¿niczka
	princessRect = { platformRects[6].x, platformRects[6].y - (BITMAP_HEIGHT * 2), 32, 32 };

	SDL_RenderCopy(renderer, princessTexture, NULL, &princessRect);
}

void preparePrincessStageTwo(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Texture* princessTexture, SDL_Rect& princessRect, SDL_Rect platformRects[]) {
	//Ksiê¿niczka
	princessRect = { platformRects[8].x, platformRects[8].y - (BITMAP_HEIGHT * 2), 32, 32 };

	SDL_RenderCopy(renderer, princessTexture, NULL, &princessRect);
}

void preparePrincessStageThree(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Texture* princessTexture, SDL_Rect& princessRect, SDL_Rect platformRects[]) {
	//Ksiê¿niczka
	princessRect = { platformRects[8].x, platformRects[8].y - (BITMAP_HEIGHT * 2), 32, 32 };

	SDL_RenderCopy(renderer, princessTexture, NULL, &princessRect);
}

void prepareTrophyStageOne(GameRects* rects) {
	if (rects->trophyRects->x != 0) {
		rects->trophyRects->x = SCREEN_WIDTH / 2;
		rects->trophyRects->y = rects->platformRects[5].y - BITMAP_WIDTH;
		rects->trophyRects->w = BITMAP_WIDTH;
		rects->trophyRects->h = BITMAP_HEIGHT;
		rects->trophyRect = { SCREEN_WIDTH / 2, rects->platformRects[5].y - BITMAP_WIDTH, BITMAP_WIDTH, BITMAP_HEIGHT };
	}
}

void prepareTrophyStageTwo(GameRects* rects) {
	if (rects->trophyRects->x != 0) {
		rects->trophyRects->x = rects->platformRects[5].x + BITMAP_WIDTH;
		rects->trophyRects->y = rects->platformRects[5].y - BITMAP_WIDTH;
		rects->trophyRects->w = BITMAP_WIDTH;
		rects->trophyRects->h = BITMAP_HEIGHT;
		rects->trophyRect = { rects->platformRects[5].x + BITMAP_WIDTH, rects->platformRects[5].y - BITMAP_WIDTH, BITMAP_WIDTH, BITMAP_HEIGHT };
	}
}

void prepareTrophyStageThree(GameRects* rects) {
	if (rects->trophyRects->x != 0) {
		rects->trophyRects->x = rects->platformRects[8].x;
		rects->trophyRects->y = rects->platformRects[7].y - BITMAP_WIDTH;
		rects->trophyRects->w = BITMAP_WIDTH;
		rects->trophyRects->h = BITMAP_HEIGHT;
		//rects->trophyRect = { rects->platformRects[8].x, rects->platformRects[7].y - BITMAP_WIDTH, BITMAP_WIDTH, BITMAP_HEIGHT };
	}
}



void prepareGameStageOne(SDL_Surface* screen, GameTextures* textures, GameRects* rects, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, GameColors* colors) {

	SDL_FillRect(screen, NULL, colors->czarny);

	////Rysowanie platform
	//preparePlatformsStageOne(rects->platformRects, renderer, textures->platformTexture);

	////Ma³pa - przeciwnik
	//prepareMonkeyStageOne(rects->platformRects, renderer, textures->monkeyTexture0);

	////Ksiê¿niczka?
	//preparePrincessStageOne(screen, renderer, textures->princessTexture, rects->princessRect, rects->platformRects);

	////Beczki
	//prepareBarrelsStageOne(renderer, rects->staticBarrelRects, rects->platformRects, textures->barrelTexture);

	////Drabinki
	//prepareLeddersStageOne(rects->platformRects, renderer, textures->ledderTexture, rects->ledderRects);

	////Trofeum
	//prepareTrophyStageOne(rects);
}

void prepareGameStageTwo(SDL_Surface* screen, GameTextures* textures, GameRects* rects, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, GameColors* colors) {

	SDL_FillRect(screen, NULL, colors->czarny);

	////Rysowanie platform
	//preparePlatformsStageTwo(rects->platformRects, renderer, textures->platformTexture);

	////Ma³pa - przeciwnik
	prepareMonkeyStageTwo(rects, renderer, textures->monkeyTexture0);

	////Ksiê¿niczka?
	//preparePrincessStageTwo(screen, renderer, textures->princessTexture, rects->princessRect, rects->platformRects);

	////Beczki
	//prepareBarrelsStageTwo(renderer, rects->staticBarrelRects, rects->platformRects, textures->barrelTexture);

	////Drabinki
	//prepareLeddersStageTwo(rects->platformRects, renderer, textures->ledderTexture, rects->ledderRects);

	////Trofeum
	//prepareTrophyStageTwo(rects);

}

void prepareGameStageThree(SDL_Surface* screen, GameTextures* textures, GameRects* rects, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer, GameColors* colors) {

	SDL_FillRect(screen, NULL, colors->czarny);

	////Rysowanie platform
	//preparePlatformsStageThree(rects->platformRects, renderer, textures->platformTexture);

	////Ma³pa - przeciwnik
	//prepareMonkeyStageThree(rects, renderer, textures->monkeyTexture0);

	////Ksiê¿niczka?
	//preparePrincessStageThree(screen, renderer, textures->princessTexture, rects->princessRect, rects->platformRects);

	////Beczki
	//prepareBarrelsStageThree(renderer, rects->staticBarrelRects, rects->platformRects, textures->barrelTexture);

	////Drabinki
	//prepareLeddersStageThree(rects->platformRects, renderer, textures->ledderTexture, rects->ledderRects);

	////Trofeum
	//prepareTrophyStageThree(rects);

}
*/

// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znakis
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};

// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};

// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};

// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};

// rysowanie prostok¹ta o d³ugoœci boków l i k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

//Skalowanie bitmapy
SDL_Surface* ScaleBitmap(const char* file, int newWidth, int newHeight) {
	SDL_Surface* originalSurface = SDL_LoadBMP(file);
	if (!originalSurface) {
		return NULL;
	}

	// Utworzenie nowej powierzchni o docelowym rozmiarze
	SDL_Surface* scaledSurface = SDL_CreateRGBSurface(0, newWidth, newHeight, originalSurface->format->BitsPerPixel,
		originalSurface->format->Rmask, originalSurface->format->Gmask,
		originalSurface->format->Bmask, originalSurface->format->Amask);
	if (!scaledSurface) {
		SDL_FreeSurface(originalSurface);
		return NULL;
	}

	// Skalowanie oryginalnej powierzchni na now¹ powierzchniê
	SDL_BlitScaled(originalSurface, NULL, scaledSurface, NULL);

	// Zwolnienie oryginalnej powierzchni
	SDL_FreeSurface(originalSurface);

	return scaledSurface;
}

int initSDL() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 0;
	}
	return 1;
}

int createWindowAndRenderer(int width, int height, int fullscreen, SDL_Window** window, SDL_Renderer** renderer) {
	Uint32 window_flags = fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
	if (SDL_CreateWindowAndRenderer(width, height, window_flags, window, renderer) != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 0;
	}
	return 1;
}

void setGraphics(SDL_Renderer* renderer, int width, int height) {
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, width, height);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

int createSurfaceAndTexture(SDL_Renderer* renderer, int width, int height, SDL_Surface** surface, SDL_Texture** texture) {
	*surface = SDL_CreateRGBSurface(0, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (!surface) return 0;

	*texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (!texture) return 0;

	return 1;
}

void timeAndFpsInfo(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Texture* scrtex, SDL_Surface* charset, int czarny, int czerwony, GameState* state) {
	int currentTick = SDL_GetTicks();
	char text[128];
	state->frameCount++;

	// Obliczenie co 0.1s
	if (currentTick - state->startTick >= 100) { // 100ms = 0.1s
		state->fps = state->frameCount * 10; // 0.1 * 10 = 1s 
		state->frameCount = 0;

		// Dok³adne obliczenie up³yniêtego czasu
		double elapsedTime = (currentTick - state->startTick) / 1000.0;
		state->worldTime += elapsedTime;
		state->startTick = currentTick;
	}

	// Rysowanie informacji o FPS i czasie
	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 20, czarny, czerwony);
	sprintf(text, "Czas trwania = %.1lf s  %.0lf klatek / s", state->worldTime, state->fps);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
}

//Powtarzanie tekstury dla obiektu wiêkszego od bitmap_height x bitmap_width (np. dla platformy)
void DrawTiledTextureOnElement(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect elementRect) {
	for (int y = elementRect.y; y < elementRect.y + elementRect.h; y += BITMAP_HEIGHT) {
		for (int x = elementRect.x; x < elementRect.x + elementRect.w; x += BITMAP_WIDTH) {
			SDL_Rect srcRect = { 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT };
			SDL_Rect destRect = { x, y, BITMAP_WIDTH, BITMAP_HEIGHT };

			// Sprawdzanie, czy tekstura nie wychodzi poza zakres
			if (x + BITMAP_WIDTH > elementRect.x + elementRect.w) {
				destRect.w = (elementRect.x + elementRect.w) - x;
				srcRect.w = destRect.w;
			}
			if (y + BITMAP_HEIGHT > elementRect.y + elementRect.h) {
				destRect.h = (elementRect.y + elementRect.h) - y;
				srcRect.h = destRect.h;
			}

			SDL_RenderCopy(renderer, texture, &srcRect, &destRect);
		}
	}
}

void drawLives(SDL_Renderer* renderer, SDL_Texture* heartTexture, int* lives) {
	SDL_Rect heartRect = { 5, BITMAP_HEIGHT * 2, BITMAP_WIDTH, BITMAP_HEIGHT };
	for (int i = 0; i < *lives; i++) {
		SDL_RenderCopy(renderer, heartTexture, NULL, &heartRect);
		heartRect.x += 20;  // Przesuniêcie kolejnego serca
	}
}

void drawScore(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Surface* charset, SDL_Renderer* renderer, int* score) {
	char text[128];
	sprintf(text, "Wynik: %d", *score);
	DrawString(screen, screen->w - strlen(text) * 9, 40, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
}

void drawInfoText(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Surface* charset, char* text, int czarny, int czerwony, int jasnyniebieski) {
	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 20, czarny, czerwony);
	sprintf(text, "Esc - wyjscie, N - nowa rozgrywka");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
}

int checkPreamble(const char* filename, GameState* state) {
	FILE* file = fopen(filename, "r");
	if (!file) {
		printf("B³¹d otwarcia pliku: %s\n", filename);
		return 1;
	}

	char buffer[256];
	int countPlatforms = 0, countLadders = 0, countStaticBarrels = 0, countMonkeys = 0, countPrincesses = 0, countTrophies = 0;

	while (fgets(buffer, sizeof(buffer), file)) {
		// Zliczanie obiektów
		if (strncmp(buffer, "Platform", 8) == 0) {
			countPlatforms++;
		}
		else if (strncmp(buffer, "Ladder", 6) == 0) {
			countLadders++;
		}
		else if (strncmp(buffer, "StaticBarrel", 12) == 0) {
			countStaticBarrels++;
		}
		else if (strncmp(buffer, "Monkey", 6) == 0) {
			countMonkeys++;
		}
		else if (strncmp(buffer, "Princess", 8) == 0) {
			countPrincesses++;
		}
		else if (strncmp(buffer, "Trophy", 6) == 0) {
			countTrophies++;
		}
	}

	// Sprawdzanie poprawnoœci
	if (countPlatforms != state->numOfPlatforms) {
		printf("B³¹d: liczba platform (%d) nie zgadza siê z deklarowan¹ w preambule (%d)\n", countPlatforms, state->numOfPlatforms);
		return 0;
	}
	if (countLadders != state->numOfLedders) {
		printf("B³¹d: liczba drabinek (%d) nie zgadza siê z deklarowan¹ w preambule (%d)\n", countLadders, state->numOfLedders);
		return 0;
	}
	if (countStaticBarrels != state->numOfStaticBarrels) {
		printf("B³¹d: liczba statycznych beczek (%d) nie zgadza siê z deklarowan¹ w preambule (%d)\n", countStaticBarrels, state->numOfStaticBarrels);
		return 0;
	}
	if (countMonkeys != state->numOfMonkeys) {
		printf("B³¹d: liczba ma³p (antagonistów) (%d) nie zgadza siê z deklarowan¹ w preambule (%d)\n", countMonkeys, state->numOfMonkeys);
		return 0;
	}
	if (countPrincesses != state->numOfPrincesses) {
		printf("B³¹d: liczba ksiê¿niczek (%d) nie zgadza siê z deklarowan¹ w preambule (%d)\n", countPrincesses, state->numOfPrincesses);
		return 0;
	}
	if (countTrophies != state->numOfTrophies) {
		printf("B³¹d: liczba trofeów (%d) nie zgadza siê z deklarowan¹ w preambule (%d)\n", countTrophies, state->numOfTrophies);
		return 0;
	}

	fclose(file);
	return 1;
}

int loadPreamble(const char* filename, GameState* state) {
	FILE* file = fopen(filename, "r");
	if (!file) {
		printf("B³¹d otwarcia pliku: %s\n", filename);
		return 0;
	}
	fseek(file, 0, SEEK_SET);
	char buffer[256];

	while (fgets(buffer, sizeof(buffer), file)) {
		if (sscanf(buffer, "NumOfPlatforms=%d", &state->numOfPlatforms) == 1) {
			continue;
		}
		if (sscanf(buffer, "NumOfLedders=%d", &state->numOfLedders) == 1) {
			continue;
		}
		if (sscanf(buffer, "NumOfStaticBarrels=%d", &state->numOfStaticBarrels) == 1) {
			continue;
		}
		if (sscanf(buffer, "NumOfMonkeys=%d", &state->numOfMonkeys) == 1) {
			continue;
		}
		if (sscanf(buffer, "NumOfPrincesses=%d", &state->numOfPrincesses) == 1) {
			continue;
		}
		if (sscanf(buffer, "NumOfTrophies=%d", &state->numOfTrophies) == 1) {
			continue;
		}
	}

	fclose(file);

	if (checkPreamble(filename, state) == 0) {
		return 0;
	}

	return 1;
}

int AllocateMemoryForStage(GameRects* rects, GameState* state) {
	// Alokacja pamiêci dla platformRects
	rects->platformRects = (SDL_Rect*)malloc(state->numOfPlatforms * sizeof(SDL_Rect));
	if (!rects->platformRects) {
		return 0;
	}

	// Alokacja pamiêci dla ledderRects
	rects->ledderRects = (SDL_Rect*)malloc(state->numOfLedders * sizeof(SDL_Rect));
	if (!rects->ledderRects) {
		return 0;
	}

	// Alokacja pamiêci dla staticBarrelRects
	rects->staticBarrelRects = (SDL_Rect*)malloc(state->numOfStaticBarrels * sizeof(SDL_Rect));
	if (!rects->staticBarrelRects) {
		return 0;
	}

	// Alokacja pamiêci dla monkeyRects
	rects->monkeyRects = (SDL_Rect*)malloc(state->numOfMonkeys * sizeof(SDL_Rect));
	if (!rects->monkeyRects) {
		return 0;
	}

	// Alokacja pamiêci dla princessRects
	rects->princessRects = (SDL_Rect*)malloc(state->numOfPrincesses * sizeof(SDL_Rect));
	if (!rects->princessRects) {
		return 0;
	}

	// Alokacja pamiêci dla trophyRects
	rects->trophyRects = (SDL_Rect*)malloc(state->numOfTrophies * sizeof(SDL_Rect));
	if (!rects->trophyRects) {
		return 0;
	}

	return 1;
}

void loadPlatforms(const char* filename, GameRects* rects, GameState* state) {
	FILE* file = fopen(filename, "r");
	if (!file) {
		return;
	}
	//Przewijanie na pocz¹tek pliku
	fseek(file, 0, SEEK_SET);

	char buffer[256];
	for (int i = 0; i < state->numOfPlatforms; i++) {
		do {
			fgets(buffer, sizeof(buffer), file);
		} while (strncmp(buffer, "Platform", 8) != 0);

		if (sscanf(buffer, "Platform,%d,%d,%d,%d", &rects->platformRects[i].x, &rects->platformRects[i].y, &rects->platformRects[i].w, &rects->platformRects[i].h) != 4) {
			printf("B³¹d ³adowania %d platformy\n", i);
		}
	}

	fclose(file);
}

void loadLedders(const char* filename, GameRects* rects, GameState* state) {
	FILE* file = fopen(filename, "r");
	if (!file) {
		return;
	}

	fseek(file, 0, SEEK_SET);

	char buffer[256];
	for (int i = 0; i < state->numOfLedders; i++) {
		do {
			fgets(buffer, sizeof(buffer), file);
		} while (strncmp(buffer, "Ladder", 6) != 0);

		if (sscanf(buffer, "Ladder,%d,%d,%d,%d", &rects->ledderRects[i].x, &rects->ledderRects[i].y, &rects->ledderRects[i].w, &rects->ledderRects[i].h) != 4) {
			printf("B³¹d ³adowania %d drabinki\n", i);
		}
	}
	fclose(file);
}

void loadStaticBarrels(const char* filename, GameRects* rects, GameState* state) {
	FILE* file = fopen(filename, "r");
	if (!file) {
		return;
	}

	fseek(file, 0, SEEK_SET);

	char buffer[256];
	for (int i = 0; i < state->numOfStaticBarrels; i++) {
		do {
			fgets(buffer, sizeof(buffer), file);
		} while (strncmp(buffer, "StaticBarrel", 12) != 0);

		if (sscanf(buffer, "StaticBarrel,%d,%d,%d,%d", &rects->staticBarrelRects[i].x, &rects->staticBarrelRects[i].y, &rects->staticBarrelRects[i].w, &rects->staticBarrelRects[i].h) != 4) {
			printf("B³¹d ³adowania %d statycznej beczki\n", i);
		}
	}
	fclose(file);
}

void loadMonkeys(const char* filename, GameRects* rects, GameState* state) {
	FILE* file = fopen(filename, "r");
	if (!file) {
		return;
	}

	fseek(file, 0, SEEK_SET);

	char buffer[256];
	for (int i = 0; i < state->numOfMonkeys; i++) {
		do {
			fgets(buffer, sizeof(buffer), file);
		} while (strncmp(buffer, "Monkey", 6) != 0);

		if (sscanf(buffer, "Monkey,%d,%d,%d,%d", &rects->monkeyRects[i].x, &rects->monkeyRects[i].y, &rects->monkeyRects[i].w, &rects->monkeyRects[i].h) != 4) {
			printf("B³¹d ³adowania %d ma³py\n", i);
		}
	}
	fclose(file);
}

void loadPrincess(const char* filename, GameRects* rects, GameState* state) {
	FILE* file = fopen(filename, "r");
	if (!file) {
		return;
	}

	fseek(file, 0, SEEK_SET);

	char buffer[256];
	for (int i = 0; i < state->numOfPrincesses; i++) {
		do {
			fgets(buffer, sizeof(buffer), file);
		} while (strncmp(buffer, "Princess", 6) != 0);

		if (sscanf(buffer, "Princess,%d,%d,%d,%d", &rects->princessRects[i].x, &rects->princessRects[i].y, &rects->princessRects[i].w, &rects->princessRects[i].h) != 4) {
			printf("B³¹d ³adowania %d ksiê¿niczki\n", i);
		}
	}
	fclose(file);
}

void loadTrophies(const char* filename, GameRects* rects, GameState* state) {
	FILE* file = fopen(filename, "r");
	if (!file) {
		return;
	}

	fseek(file, 0, SEEK_SET);

	char buffer[256];
	for (int i = 0; i < state->numOfTrophies; i++) {
		if (rects->trophyRects[i].x == 0) {
			// Pomijanie wczytywania, jeœli trofeum zosta³o ju¿ zebrane (rect.x == 0)
			continue;
		}

		do {
			fgets(buffer, sizeof(buffer), file);
		} while (strncmp(buffer, "Trophy", 6) != 0);

		if (sscanf(buffer, "Trophy,%d,%d,%d,%d", &rects->trophyRects[i].x, &rects->trophyRects[i].y, &rects->trophyRects[i].w, &rects->trophyRects[i].h) != 4) {
			printf("B³¹d ³adowania %d trofeum\n", i);
		}
	}
	fclose(file);
}

void prepareGameStageElements(SDL_Surface* screen, GameColors* colors, GameRects* rects, GameState* state) {
	SDL_FillRect(screen, NULL, colors->czarny);

	char* file = "./Stages/stage1.donkey";

	switch (state->currentStage) {
	case 1:
		file = "./Stages/stage1.donkey";
		break;
	case 2:
		file = "./Stages/stage2.donkey";
		break;
	case 3:
		file = "./Stages/stage3.donkey";
		break;
	}

	//Wczytywanie platform
	loadPlatforms(file, rects, state);

	//Drabinki
	loadLedders(file, rects, state);

	//Ma³pa - przeciwnik
	loadMonkeys(file, rects, state);

	//Ksiê¿niczka?
	loadPrincess(file, rects, state);

	//Beczki
	loadStaticBarrels(file, rects, state);

	//Trofeum
	loadTrophies(file, rects, state);

}

int prepareGameStage(GameState* state, GameRects* rects) {
	char* file = "./Stages/stage1.donkey";

	switch (state->currentStage) {
	case 1:
		file = "./Stages/stage1.donkey";
		break;
	case 2:
		file = "./Stages/stage2.donkey";
		break;
	case 3:
		file = "./Stages/stage3.donkey";
		break;
	}

	//£adowanie preambu³y
	if (loadPreamble(file, state)) {
		//Alokowanie pamiêci
		if (!AllocateMemoryForStage(rects, state)) {
			printf("B£AD ALOKACJI PAMIÊCI!");
			return 0;
		}
	}
}

void preparePlayer(SDL_Renderer* renderer, SDL_Rect& playerRect, SDL_Texture* playerTexture, SDL_Rect platformRects[], GameState* state) {
	//Gracz
	state->isJumping = 0;
	state->isFalling = 0;
	state->isClimbing = 0;
	playerRect = { platformRects[0].x, platformRects[0].y - (BITMAP_HEIGHT * 2), 32, 32 };
	state->realPosX = platformRects[0].x;
	state->realPosY = platformRects[0].y - (BITMAP_HEIGHT * 2);
}

int isOnPlatform(SDL_Rect& playerRect, SDL_Rect platformRects[], GameState* state) {
	const int errorMargin = 2; // Margines b³êdu w pikselach

	for (int i = 0; i < state->numOfPlatforms; ++i) {
		SDL_Rect& platform = platformRects[i];

		// Sprawdzenie, czy dolna krawêdŸ gracza jest blisko górnej krawêdzi platformy + margines b³êdu
		int playerBottom = playerRect.y + playerRect.h;
		if (playerBottom >= platform.y - errorMargin && playerBottom <= platform.y + errorMargin) {
			// Sprawdzenie, czy gracz znajduje siê poziomo nad platform¹
			if (playerRect.x < platform.x + platform.w && playerRect.x + playerRect.w > platform.x) {
				return 1;
			}
		}
	}
	return 0;
}

int isBarrelLanded(SDL_Rect barrelRect, SDL_Rect platformRects[], GameState* state) {
	for (int i = state->numOfPlatforms; i >= 0; --i) {
		if (SDL_HasIntersection(&barrelRect, &platformRects[i])) {
			return 1;
		}
	}

	return 0;
}

int isOnLadder(SDL_Rect& playerRect, SDL_Rect ladderRects[], GameState* state) {
	for (int i = 0; i < state->numOfLedders; ++i) {
		SDL_Rect& ladder = ladderRects[i];
		if (SDL_HasIntersection(&playerRect, &ladder)) {
			return 1;
		}
	}
	return 0;
}

int isOnTopOfLadder(SDL_Rect& playerRect, SDL_Rect ladderRects[], GameState* state) {
	const int margin = 5; // Margines b³êdu

	for (int i = 0; i < state->numOfLedders; ++i) {
		SDL_Rect& ladder = ladderRects[i];
		// SprawdŸ, czy gracz jest w pobli¿u X drabiny i czy Y gracza jest równy lub wy¿szy ni¿ Y szczytu drabiny
		if (abs(playerRect.x - ladder.x) <= margin && playerRect.y >= ladder.w) {
			return 1;
		}
	}
	return 0;
}

int isCollisionWithPlatforms(SDL_Rect& playerRect, SDL_Rect platformRects[], GameState* state) {
	for (int i = 0; i < state->numOfPlatforms; ++i) {
		SDL_Rect& platform = platformRects[i];

		if (SDL_HasIntersection(&playerRect, &platform)) {
			return 1;
		}
	}
	return 0;
}

int isPlatformAbove(SDL_Rect& playerRect, SDL_Rect platformRects[], GameState* state) {
	int playerTop = playerRect.y;
	int playerLeft = playerRect.x;
	int playerRight = playerRect.x + playerRect.w;
	int errorMargin = 2;
	int checkHeight = playerTop - PLATFORM_SPACE - errorMargin;

	for (int i = 0; i < state->numOfPlatforms; ++i) {
		SDL_Rect& platform = platformRects[i];

		// Sprawdzenie, czy dolna krawêdŸ platformy jest w pobli¿u okreœlonej wysokoœci nad graczem
		// oraz czy gracz znajduje siê poziomo pod platform¹
		if (platform.y < playerTop && platform.y + platform.h > checkHeight &&
			playerRight > platform.x && playerLeft < platform.x + platform.w) {
			return 1;
		}
	}

	return 0; 
}

int findLastPlatformIndex(SDL_Rect platformRects[], GameState* state) {
	int lastPlatformIndex = 0;
	int highestY = platformRects[state->numOfPlatforms].y;

	for (int i = 0; i < state->numOfPlatforms; i++) {
		if (platformRects[i].y < highestY) {
			highestY = platformRects[i].y;
			lastPlatformIndex = i;
		}
	}

	return lastPlatformIndex - 1;
}

int findNearestPlatformAbove(SDL_Rect& playerRect, SDL_Rect platformRects[], GameState* state) {
	int nearestPlatformIndex = -1;
	int nearestPlatformDistanceY = SCREEN_HEIGHT; // Maksymalna odleg³oœæ w pionie
	int nearestPlatformDistanceX = SCREEN_WIDTH;  // Maksymalna odleg³oœæ w poziomie

	for (int i = 0; i < state->numOfPlatforms; i++) {
		if (platformRects[i].y + platformRects[i].h < playerRect.y) { // Platforma powy¿ej gracza
			int distanceY = playerRect.y - (platformRects[i].y + platformRects[i].h);
			int distanceX = abs((playerRect.x + playerRect.w / 2) - (platformRects[i].x + platformRects[i].w / 2));

			if (distanceY < nearestPlatformDistanceY ||
				(distanceY == nearestPlatformDistanceY && distanceX < nearestPlatformDistanceX)) {
				nearestPlatformDistanceY = distanceY;
				nearestPlatformDistanceX = distanceX;
				nearestPlatformIndex = i;

				if (distanceY > PLATFORM_SPACE / 2) {
					return -1;
				}
			}
		}
	}

	return nearestPlatformIndex;
}

int findNearestPlatform(SDL_Rect& playerRect, SDL_Rect platformRects[], GameState* state) {
	int nearestPlatformIndex = -1;
	int nearestPlatformDistance = -1;

	for (int i = 0; i < state->numOfPlatforms; i++) {
		// Tylko platformy poni¿ej gracza s¹ brane pod uwagê
		if (platformRects[i].y > playerRect.y) {
			// Sprawdzenie, czy gracz znajduje siê w zakresie poziomym platformy
			int playerCenterX = playerRect.x + playerRect.w / 2;
			if (playerCenterX >= platformRects[i].x && playerCenterX <= (platformRects[i].x + platformRects[i].w)) {
				int distance = platformRects[i].y - playerRect.y;
				if (nearestPlatformIndex == -1 || distance < nearestPlatformDistance) {
					nearestPlatformDistance = distance;
					nearestPlatformIndex = i;
				}
			}
		}
	}

	return nearestPlatformIndex;
}

int findNearestPlatformForBarrel(SDL_Rect& barrelRect, SDL_Rect platformRects[], GameState* state) {
	int nearestPlatformIndex = -1;
	int nearestPlatformDistance = -1; // Zaczynamy od wartoœci -1 jako sygna³, ¿e jeszcze nie znaleziono ¿adnej platformy

	for (int i = state->numOfPlatforms; i >= 0; i--) {
		// Tylko platformy poni¿ej gracza s¹ brane pod uwagê
		// je¿eli do warunku wstawimy (playerRect.y + playerRect.h) to nie bedzie brana pod uwagê platforma, na której stoi postaæ
		if (platformRects[i].y > barrelRect.y) {
			int distance = platformRects[i].y - (barrelRect.y);
			if (nearestPlatformIndex == -1 || distance < nearestPlatformDistance) {
				nearestPlatformDistance = distance;
				nearestPlatformIndex = i;
			}
		}
	}

	return nearestPlatformIndex;
}

int findLadderIndex(SDL_Rect& playerRect, SDL_Rect ladderRects[], GameState* state) {
	for (int i = 0; i < state->numOfLedders; ++i) {
		SDL_Rect& ladder = ladderRects[i];
		if (SDL_HasIntersection(&playerRect, &ladder)) {
			return i;
		}
	}
	return -1;
}

int isUnderLadder(SDL_Rect& playerRect, SDL_Rect ladderRects[], GameState* state) {
	const int margin = 5; // margines b³êdu

	for (int i = 0; i < state->numOfLedders; i++) {
		SDL_Rect ladderRect = ladderRects[i];

		// Sprawdzanie, czy gracz znajduje siê w poziomym zakresie drabiny
		if (playerRect.x < ladderRect.x + ladderRect.w &&
			playerRect.x + playerRect.w > ladderRect.x) {

			// Sprawdzanie, czy dolna krawêdŸ gracza jest tu¿ nad górn¹ krawêdzi¹ drabiny + margines b³êdu
			if (playerRect.y + playerRect.h > ladderRect.y - margin &&
				playerRect.y + playerRect.h <= ladderRect.y) {
				return 1; 
			}
		}
	}

	return 0; 
}

void movePlayerHorizontal(SDL_Rect& playerRect, SDL_Rect platformRects[], SDL_Rect ladderRects[], int dx, GameState* state) {
	float movement = dx * PLAYER_SPEED * state->deltaTime;
	state->realPosX += movement;

	// Ograniczenie ruchu w prawo przez rozdzielczoœæ gry
	if (state->realPosX > SCREEN_WIDTH - playerRect.w) {
		state->realPosX = SCREEN_WIDTH - playerRect.w;
	}

	// Ograniczenie ruchu w lewo przez rozdzielczoœæ gry
	if (state->realPosX < 0) {
		state->realPosX = 0;
	}

	// Ruch poziomy
	SDL_Rect potentialHorizontalMove = playerRect;
	potentialHorizontalMove.x = static_cast<int>(round(state->realPosX));
	if (isOnPlatform(potentialHorizontalMove, platformRects, state) || state->isJumping || state->isFalling) {
		playerRect.x = static_cast<int>(round(state->realPosX));
	}
	else {
		//Wychodzi poza krawêdŸ platformy
		if (!isOnLadder(playerRect, ladderRects, state) && !state->isJumping) {
			state->isFalling = 1;
			playerRect.x = static_cast<int>(round(state->realPosX)); //mo¿na +-1 dla pewnoœci, ¿e zaokr¹glenie nie zatrzyma gracza na krawêdzi platformy
		}
	}
}

void movePlayerVertical(SDL_Rect& playerRect, SDL_Rect platformRects[], SDL_Rect ladderRects[], int dy, GameState* state) {
	float movement = dy * CLIMBING_SPEED * state->deltaTime;
	int margin = 2; //Margines b³êdu
	state->realPosY += movement;

	// Ograniczenie ruchu przez rozdzielczoœæ gry
	if (state->realPosY < 0) {
		state->realPosY = 0;
	}
	else if (state->realPosY > SCREEN_HEIGHT - playerRect.h) {
		state->realPosY = SCREEN_HEIGHT - playerRect.h;
	}

	SDL_Rect potentialVerticalMove = playerRect;
	potentialVerticalMove.y = static_cast<int>(state->realPosY);

	int ladderIndex = findLadderIndex(playerRect, ladderRects, state);
	int bottomOfLadder = ladderRects[ladderIndex].y + ladderRects[ladderIndex].h;

	// Sprawdzenie, czy ruch na drabinie nie spowoduje kolizji z platform¹, warunek dla próby schodzenia poni¿ej drabiny
	if (!(dy > 0 && isCollisionWithPlatforms(potentialVerticalMove, platformRects, state)) ||
		(isUnderLadder(playerRect, ladderRects, state) == 1 && dy > 0) ||
		(isCollisionWithPlatforms(potentialVerticalMove, platformRects, state) && isOnLadder(playerRect, ladderRects, state) && dy > 0)) {
		if (isUnderLadder(playerRect, ladderRects, state)) {
			// Stawianie gracza na górnej czêœci drabiny
			playerRect.y += margin;
			state->realPosY += margin;
		}
		else {
			//Wyjœcie poza górn¹ krawêdŸ drabiny
			if (state->realPosY - BITMAP_HEIGHT - playerRect.h - margin < ladderRects[ladderIndex].y - ladderRects[ladderIndex].h) {
				// Stawianie gracza na górnej czêœci drabiny
				playerRect.y = ladderRects[ladderIndex].y - playerRect.h;
				state->realPosY = playerRect.y;
			}
			else if (state->realPosY + playerRect.h >= bottomOfLadder) {
				// Stawianie gracza na dolnej czêœci drabiny
				playerRect.y = bottomOfLadder - playerRect.h;
				state->realPosY = playerRect.y;
			}
			else {
				playerRect.y = static_cast<int>(state->realPosY);
			}
		}
	}
}

void playerMoveAnimation(GameState* state, GameTextures* textures) {
	if (state->isMoving > 0) {
		int currentTime = SDL_GetTicks();
		int elapsedTime = currentTime - state->playerAnimationRunningStartTime;

		int frameIndex = (elapsedTime / PLAYER_ANIMATION_DURATION) % 3;

		// Ustawienie odpowiedniej tekstury w zale¿noœci od kierunku ruchu
		if (state->isMoving == 2) { // Poruszanie siê w lewo
			switch (frameIndex) {
			case 0:
				textures->playerTexture = textures->playerTextureBeforeRunFlipped;
				break;
			case 1:
				textures->playerTexture = textures->playerTextureRunningFlipped;
				break;
			case 2:
				textures->playerTexture = textures->playerTextureAfterRunFlipped;
				break;
			}
		}
		else { // Poruszanie siê w prawo
			switch (frameIndex) {
			case 0:
				textures->playerTexture = textures->playerTextureBeforeRun;
				break;
			case 1:
				textures->playerTexture = textures->playerTextureRunning;
				break;
			case 2:
				textures->playerTexture = textures->playerTextureAfterRun;
				break;
			}
		}
	}
	else if (!state->isJumping) {
		// Ustaw na podstawow¹ teksturê, gdy gracz nie jest w ruchu
		textures->playerTexture = textures->playerTexture0;
		state->playerAnimationRunningStartTime = SDL_GetTicks();
	}
}

void playerJumpAnimation(GameState* state, GameTextures* textures) {
	if (state->isJumping || state->isFalling) {
		int currentTime = SDL_GetTicks();
		int elapsedTime = currentTime - state->playerAnimationJumpingStartTime;

		int frameIndex = (elapsedTime / PLAYER_ANIMATION_DURATION) % 3;

		switch (frameIndex) {
		case 0:
			textures->playerTexture = textures->playerTextureBeforeJump;
			break;
		case 1:
			textures->playerTexture = textures->playerTextureJumping;
			break;
		case 2:
			textures->playerTexture = textures->playerTextureBeforeJump;
			break;
		}
	}
	else if (state->isMoving < 1) {
		// Ustawianie na podstawow¹ teksturê, gdy gracz nie jest w ruchu
		textures->playerTexture = textures->playerTexture0;
		state->playerAnimationJumpingStartTime = SDL_GetTicks(); // Resetowanie czasu startu animacji
	}
}

void playerFallAnimation(GameState* state, GameTextures* textures) {
	if (state->isFalling) {
		textures->playerTexture = textures->playerTextureJumping;
	}
	else if (state->isMoving < 1 && !state->isJumping) {
		// Ustawianie na podstawow¹ teksturê, gdy gracz nie jest w ruchu
		textures->playerTexture = textures->playerTexture0;
		state->playerAnimationJumpingStartTime = SDL_GetTicks(); // Resetowanie czasu startu animacji
	}
}

void playerClimbingAnimation(GameState* state, GameTextures* textures, GameRects* rects) {
	//Mozna zmienic warunek na isOnLadder(rects->playerRect, rects->ledderRects), wtedy postac bedzie wykonywala animacje ci¹gle bêd¹c na drabinie
	if (state->isClimbing) {
		Uint32 currentTime = SDL_GetTicks();
		Uint32 elapsedTime = currentTime - state->playerAnimationClimbingStartTime;

		int frameIndex = (elapsedTime / (PLAYER_ANIMATION_DURATION - 1)) % 2;

		switch (frameIndex) {
		case 0:
			textures->playerTexture = textures->playerTextureClimbing1;
			break;
		case 1:
			textures->playerTexture = textures->playerTextureClimbing2;
			break;
		}
	}
	else if (state->isMoving < 1 && !state->isJumping && !state->isFalling) {
		// Ustawianie na podstawow¹ teksturê, gdy gracz nie jest w ruchu
		textures->playerTexture = textures->playerTexture0;
		state->playerAnimationClimbingStartTime = SDL_GetTicks(); // Resetowanie czasu startu animacji
	}
}

void movePlayer(SDL_Rect& playerRect, int dx, int dy, SDL_Rect platformRects[], SDL_Rect ladderRects[], GameState* state) {
	if (dy == 0) {
		movePlayerHorizontal(playerRect, platformRects, ladderRects, dx, state);
		if (!(isOnLadder(playerRect, ladderRects, state) || (isUnderLadder(playerRect, ladderRects, state)))) {

			if (dx == 1) {
				state->isMoving = 1; //Prawo
			}
			else {
				state->isMoving = 2; //Lewo
			}
		}
	}

	// Ruch pionowy dozwolony na drabinie lub, gdy gracz jest pod drabin¹
	if (isOnLadder(playerRect, ladderRects, state) || (isUnderLadder(playerRect, ladderRects, state) && dy == 1)) {
		movePlayerVertical(playerRect, platformRects, ladderRects, dy, state);
		state->isClimbing = 1;
	}
}

//Funkcja ju¿ niewykorzystywana
void jumpOnPlatform(SDL_Rect& playerRect, SDL_Rect platformRects[], int& isJumping, int nearestPlatformAboveIndex, int nearestPlatformIndex, int lastPlatformIndex) {
	int margin = SCREEN_WIDTH / BITMAP_WIDTH;
	int nearLeftEdge = abs(playerRect.x - platformRects[nearestPlatformAboveIndex].x) <= margin;
	int nearRightEdge = abs((playerRect.x + playerRect.w) - (platformRects[nearestPlatformAboveIndex].x + platformRects[nearestPlatformAboveIndex].w)) <= margin;

	if (nearLeftEdge || nearRightEdge) {
		//Wy³¹czenie wskakiwania na ostatniej platformie
		if (nearestPlatformIndex == lastPlatformIndex) {
			return;
		}

		isJumping = 0;
		playerRect.y = platformRects[nearestPlatformAboveIndex].y - playerRect.h;

		//Logika skoku na przedostatniej platformie
		if (nearestPlatformAboveIndex == lastPlatformIndex) {
			if (abs(playerRect.x - platformRects[lastPlatformIndex].x) < abs((platformRects[lastPlatformIndex].x + platformRects[lastPlatformIndex].w) - playerRect.x)) {
				// Gracz jest bli¿ej lewej krawêdzi
				playerRect.x = platformRects[nearestPlatformAboveIndex].x;
			}
			else {
				// Gracz jest bli¿ej prawej krawêdzi
				playerRect.x = platformRects[nearestPlatformAboveIndex].w + BITMAP_WIDTH * 11;
			}
			return;
		}

		if ((nearestPlatformAboveIndex) % 2 == 0) {
			playerRect.x = platformRects[nearestPlatformAboveIndex].x;
		}
		else {
			playerRect.x = platformRects[nearestPlatformAboveIndex].w - BITMAP_WIDTH;
		}
	}
}

void jumpPlayer(SDL_Rect& playerRect, SDL_Rect platformRects[], GameState* state) {
	if (state->isJumping) {
		// Aktualizacja prêdkoœci
		state->verticalSpeed += gravity * state->deltaTime;

		// Aktualizacja rzeczywistej pozycji Y
		state->realPosY += state->verticalSpeed * state->deltaTime;

		// Aktualizacja pozycji Y gracza (konwersja na ca³kowit¹)
		playerRect.y = static_cast<int>(state->realPosY);

		int nearestPlatformIndex = findNearestPlatform(playerRect, platformRects, state);

		// Sprawdzenie l¹dowania na platformie
		if (nearestPlatformIndex != -1 && isCollisionWithPlatforms(playerRect, platformRects, state) && state->verticalSpeed > 0) {
			state->isJumping = 0;
			state->verticalSpeed = 0;
			state->realPosY = platformRects[nearestPlatformIndex].y - playerRect.h;
			playerRect.y = platformRects[nearestPlatformIndex].y - playerRect.h;
		}
	}
	else if (!state->isJumping && isOnPlatform(playerRect, platformRects, state)) {
		// Inicjacja skoku
		state->isJumping = 1;
		state->verticalSpeed = (!isPlatformAbove(playerRect, platformRects, state)) ? jumpSpeed * 1.8 : jumpSpeed; //Dla *2 nastêpuj¹ kolizja z platform¹
		state->realPosY = playerRect.y;

		//Sprawdzanie czy przy aktualnej wysokosci skoku postaæ nie wyskoczy poza górn¹ krawêdŸ ekranu, wykorzystywane na ostatniej platformie
		double totalJumpHeight = playerRect.y - playerRect.h - ((jumpSpeed * jumpSpeed) / (2 * gravity));
		if (totalJumpHeight < BITMAP_HEIGHT * 2) {
			state->verticalSpeed = jumpSpeed;
		}
	}
}

void fallPlayer(SDL_Rect& playerRect, SDL_Rect platformRects[], GameState* state) {
	if (state->isFalling) {
		//Ustawianie prêdkoœci
		state->verticalSpeed = (gravity / 4);

		// Aktualizacja prêdkoœci
		state->verticalSpeed += (gravity / 4) * state->deltaTime; // /2, zeby postaæ za szybko nie spada³a

		// Aktualizacja rzeczywistej pozycji Y
		state->realPosY += state->verticalSpeed * state->deltaTime;

		// Aktualizacja pozycji Y gracza (konwersja na ca³kowit¹)
		playerRect.y = static_cast<int>(state->realPosY);


		// Sprawdzenie czy gracz osi¹gn¹³ platformê poni¿ej / czy skoñczy³ spadaæ
		if (isOnPlatform(playerRect, platformRects, state) || isCollisionWithPlatforms(playerRect, platformRects, state)) {
			int nearestPlatformIndex = findNearestPlatform(playerRect, platformRects, state);

			state->isFalling = 0;
			playerRect.y = platformRects[nearestPlatformIndex].y - playerRect.h;
			state->realPosY = playerRect.y;
			state->verticalSpeed = 0;
		}
	}
}

void createBarrel(SDL_Renderer* renderer, GameRects* rects) {
	for (int i = 0; i < MAX_BARRELS; ++i) {
		if (barrels[i].rect.w == 0) {  // Zak³adaj¹c, ¿e beczka o szerokosci 0 jest "nieaktywna"
			barrels[i].movingRight = 1;
			barrels[i].rect.w = 20;
			barrels[i].rect.h = 20;
			barrels[i].rect.x = rects->staticBarrelRects[1].x;
			barrels[i].realPosX = barrels[i].rect.x;
			barrels[i].rect.y = rects->staticBarrelRects[1].y + (rects->staticBarrelRects[1].h - barrels[i].rect.h);
			barrels[i].realPosY = barrels[i].rect.y;
			break;  // Przerywa pêtlê po dodaniu beczki
		}
	}
}

void barrelMove(Barrel& barrel, GameRects* rects, GameState* state) {
	// Obliczanie przesuniêcia beczki
	double moveDistance = barrel.speed * state->deltaTime;

	if (barrel.isFalling) {
		// Spadanie beczki
		barrel.realPosY += (gravity / 3) * state->deltaTime;

		// Aktualizacja pozycji beczki
		barrel.rect.y = static_cast<int>(barrel.realPosY);

		// Sprawdzenie, czy beczka wyl¹dowa³a na platformie
		if (isOnPlatform(barrel.rect, rects->platformRects, state) || isBarrelLanded(barrel.rect, rects->platformRects, state)) {
			barrel.isFalling = 0;
			barrel.targetPlatform = findNearestPlatformForBarrel(barrel.rect, rects->platformRects, state);

			//Problem tunelowania, poprawiamy pozycjê beczki
			if (isBarrelLanded(barrel.rect, rects->platformRects, state)) {
				barrel.rect.y = rects->platformRects[barrel.targetPlatform].y - barrel.rect.h;
				barrel.realPosY = barrel.rect.y;
			}

			SDL_Rect currentPlatform = rects->platformRects[barrel.targetPlatform];

			//Kierowanie beczki do nastêpnej "luki" (przez któr¹ mo¿e spaœæ)
			if (currentPlatform.x + currentPlatform.w / 2 < SCREEN_WIDTH / 2) {
				barrel.movingRight = 1;
			}
			else {
				// Jeœli œrodek platformy jest bli¿ej prawej strony ekranu
				barrel.movingRight = 0;
			}
		}
	}
	else {
		int isPlatformUnderBarrel = isOnPlatform(barrel.rect, rects->platformRects, state);

		//Pod beczk¹ nie ma platformy
		if (!isPlatformUnderBarrel || barrel.rect.x > SCREEN_WIDTH) {
			barrel.isFalling = 1;
			//Problem z tunelowaniem
			if (barrel.rect.x > SCREEN_WIDTH) {
				if (barrel.rect.x > SCREEN_WIDTH) {
					barrel.rect.x = SCREEN_WIDTH - barrel.rect.w;
				}
				//Dla beczki.x < 0 staje siê ona po prostu nieaktywna
			}
		}

		barrel.realPosX += barrel.movingRight ? moveDistance : -moveDistance;
	}
	//Aktualizacja pozycji becczki
	barrel.rect.x = static_cast<int>(barrel.realPosX);
}

void barrelAnimation(GameTextures* textures, Barrel* barrel) {
	int currentTime = SDL_GetTicks();
	if (currentTime - barrel->lastFrameChangeTime > BARREL_ANIMATION_DURATION / 3) {
		int frameIndex = (currentTime / BARREL_ANIMATION_DURATION) % 3;

		switch (frameIndex) {
		case 0:
			textures->barrel_rollTexture = textures->barrel_rollTexture1;
			break;
		case 1:
			textures->barrel_rollTexture = textures->barrel_rollTexture0;
			break;
		case 2:
			textures->barrel_rollTexture = textures->barrel_rollTexture2;
			break;
		}
		barrel->lastFrameChangeTime = currentTime;
	}
}

void updateBarrels(GameRects* rects, GameState* state, GameTextures* textures) {
	for (int i = 0; i < MAX_BARRELS; ++i) {
		if (barrels[i].rect.w != 0) {  // Zak³adaj¹c, ¿e beczka o szerokoœci 0 jest nieaktywna
			// Aktualizuj pozycjê beczki
			barrelMove(barrels[i], rects, state);
			barrelAnimation(textures, &barrels[i]);

			//Deaktywacja beczki, gdy dotrze do konca
			if (barrels[i].rect.x < 0) {
				barrels[i].rect = { 0,0,0,0 };
				barrels[i].playerJumpedOver = 0;
			}
		}
	}
}

int isBarrelUnderPlayer(SDL_Rect& playerRect, SDL_Rect platformRects[], GameState* state) {
	int nearestPlatformToPlayer = findNearestPlatform(playerRect, platformRects, state);

	for (int i = 0; i < MAX_BARRELS; ++i) {
		if (barrels[i].rect.w != 0 && !barrels[i].playerJumpedOver) {
			int playerX = playerRect.x;
			int playerWidth = playerRect.w;
			int playerY = playerRect.y;
			int barrelX = barrels[i].rect.x;
			int barrelWidth = barrels[i].rect.w;
			int barrelTopY = barrels[i].rect.y - barrels[i].rect.h;
			int nearestPlatformToBarrel = findNearestPlatform(barrels[i].rect, platformRects, state);

			if (playerX + playerWidth > barrelX && playerX < barrelX + barrelWidth) {
				if (playerY < barrelTopY &&
					(nearestPlatformToPlayer == nearestPlatformToBarrel) &&
					state->isJumping) {
					barrels[i].playerJumpedOver = 1; // Ustawienie flagi, ¿e gracz przeskoczy³ beczkê
					return 1;
				}
			}
		}
	}
	return 0;
}

void destroyElements(GameRects* rects, GameState* state) {
	SDL_Rect defaultRect = { 0, 0, 0, 0 };

	//Resetowanie platform
	for (int i = 0; i < state->numOfPlatforms; i++) {
		rects->platformRects[i] = defaultRect;
	}

	//Resetowanie drabinek
	for (int i = 0; i < state->numOfLedders; i++) {
		rects->ledderRects[i] = defaultRect;
	}

	//Resetowanie beczek
	for (int i = 0; i < MAX_BARRELS; i++) {
		barrels[i].rect = defaultRect;
		barrels[i].playerJumpedOver = 0;
	}

	//Resetowanie czasu pojawiania siê beczek
	state->lastPrintTime = SDL_GetTicks();

	//Restowanie czasu etapu
	state->worldTime = 0.0f;

	//Resetowanie trofeum
	rects->trophyRects->x = 1;
	rects->trophyRects->y = 0;
	rects->trophyRects->w = 0;
	rects->trophyRects->h = 0;
}

int isCollisionWithBarrel(SDL_Rect& playerRect) {
	for (int i = 0; i < MAX_BARRELS; i++) {
		if (SDL_HasIntersection(&playerRect, &barrels[i].rect)) {
			return 1;
		}
	}
	return 0;
}

void handleLifeLoss(SDL_Surface* screen, SDL_Surface* charset, SDL_Renderer* renderer, SDL_Texture* scrtex, int czarny, int czerwony, GameState* state) {
	SDL_Event event;
	char text[128];
	int lineSpacing = 20; // Odstêp miêdzy liniami

	while (!state->quit) {
		sprintf(text, "Graj dalej - ENTER");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

		sprintf(text, "Wynik: %d", state->score);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10 + lineSpacing * 2, text, charset);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					state->quit = 1;
				}
				else if (event.key.keysym.sym == SDLK_RETURN) {
					return;
				}
				break;
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
				state->quit = 1;
				break;
			};
		};

		// Aktualizacja tekstury i renderera
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
	}
}

void saveScore(const char* playerName, int* score) {
	FILE* file = fopen("scores.txt", "a"); // Otwarcie pliku w trybie dopisywania
	if (file != NULL) {
		fprintf(file, "%s %d\n", playerName, *score); // Zapisanie pseudonimu i wyniku
		fclose(file);
	}
	else {
		printf("Nie mo¿na otworzyæ pliku do zapisu wyników.\n");
	}
}

void gameOver(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Surface* charset, SDL_Texture* scrtex, GameState* state, int czarny) {
	char text[128];
	char playerName[128] = "";
	int playerNameLength = 0;
	SDL_Event e;

	while (!state->quit) {
		SDL_RenderClear(renderer);
		SDL_FillRect(screen, NULL, czarny);

		sprintf(text, "GAME OVER!");
		DrawString(screen, SCREEN_WIDTH / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT / 2 - 16, text, charset);

		sprintf(text, "UZYSKANY WYNIK: %d", state->score);
		DrawString(screen, SCREEN_WIDTH / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT / 2, text, charset);

		sprintf(text, "Podaj swoj pseudonim: %s", playerName);
		DrawString(screen, SCREEN_WIDTH / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT / 2 + 16, text, charset);

		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				state->quit = 1;
			}
			else if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_ESCAPE) {
					state->quit = 1;
				}
				else if (e.key.keysym.sym == SDLK_BACKSPACE && playerNameLength > 0) {
					// Usuwanie ostatniego znaku
					playerName[--playerNameLength] = '\0';
				}
				else if ((e.key.keysym.sym >= SDLK_a && e.key.keysym.sym <= SDLK_z) ||
					(e.key.keysym.sym >= SDLK_0 && e.key.keysym.sym <= SDLK_9) &&
					playerNameLength < 127) {
					// Dodawanie nowego znaku (uwzglêdniaj¹c limit d³ugoœci + pseudonim moze zawierac tylko litery i cyfry)
					playerName[playerNameLength++] = (char)e.key.keysym.sym;
					playerName[playerNameLength] = '\0';
				}
				else if (e.key.keysym.sym == SDLK_RETURN) {
					//Zapisanie do pliku je¿eli gracz wpisa³ pseudonim
					if (playerNameLength > 0) {
						saveScore(playerName, &state->score);
					}
					state->currentStage = 1;
					return;
				}
			}
		}

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
	}
}

void setupGameEnviroment(GameState* state, GameRects* rects) {
	state->startTick = SDL_GetTicks();
	state->frameCount = 0;
	state->lastPrintTime = SDL_GetTicks();
	state->fps = 0;
	state->worldTime = 0.0f;
	state->localStage = state->currentStage;
	state->lastFrameTime = SDL_GetTicks();

	//Wykorzystywane przy skoku
	state->verticalSpeed = 0.0f;

	//Przygotowanie beczek
	for (int i = 0; i < MAX_BARRELS; i++) {
		barrels[i].rect = { 0, 0, 0, 0 };
	}

	state->lives = MAX_LIVES;
	state->score = 0;
	state->isJumping = 0;
	state->isFalling = 0;

	//Liczenie punktów
	state->lastScorePopupTime = -SCORE_POPUP_TIME;
	state->lastScorePoints = 0;


}

void handleInGameKey(SDL_Renderer* renderer, SDL_Texture* playerTexture, GameRects* rects, GameState* state) {
	SDL_Event event;
	SDL_PollEvent(&event);

	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	if (event.key.keysym.sym == SDLK_ESCAPE) {
		state->quit = 1;
		return;
	}
	else if (keystate[SDL_SCANCODE_RIGHT]) {
		movePlayer(rects->playerRect, 1, 0, rects->platformRects, rects->ledderRects, state);
	}
	else if (keystate[SDL_SCANCODE_LEFT]) {
		movePlayer(rects->playerRect, -1, 0, rects->platformRects, rects->ledderRects, state);
	}
	else if (keystate[SDL_SCANCODE_UP] && !state->isJumping) {
		movePlayer(rects->playerRect, 0, -1, rects->platformRects, rects->ledderRects, state);
	}
	else if (keystate[SDL_SCANCODE_DOWN] && !state->isJumping) {
		movePlayer(rects->playerRect, 0, 1, rects->platformRects, rects->ledderRects, state);
	}
	else if (keystate[SDL_SCANCODE_SPACE] && !state->isFalling && !state->isJumping) {
		jumpPlayer(rects->playerRect, rects->platformRects, state);
	}
	else if (event.key.keysym.sym == SDLK_1) {
		preparePlayer(renderer, rects->playerRect, playerTexture, rects->platformRects, state);
		destroyElements(rects, state);
		state->currentStage = 1;
	}
	else if (event.key.keysym.sym == SDLK_2) {
		preparePlayer(renderer, rects->playerRect, playerTexture, rects->platformRects, state);
		destroyElements(rects, state);
		state->currentStage = 2;
	}
	else if (event.key.keysym.sym == SDLK_3) {
		preparePlayer(renderer, rects->playerRect, playerTexture, rects->platformRects, state);
		destroyElements(rects, state);
		state->currentStage = 3;
	}

	//Reset potrzebny przy animacjach
	if (!keystate[SDL_SCANCODE_RIGHT] && !keystate[SDL_SCANCODE_LEFT]) {
		state->isMoving = 0;
	}

	if (!keystate[SDL_SCANCODE_UP] && !keystate[SDL_SCANCODE_DOWN]) {
		state->isClimbing = 0;
	}

}

void monkeyAnimation(GameTextures* textures, GameState* state) {
	state->monkeyAnimationCurrentTime = SDL_GetTicks();
	int elapsedTime = state->monkeyAnimationCurrentTime - state->monkeyAnimationStartTime;

	if (elapsedTime < MONKEY_ANIMATION_DURATION) {
		if (elapsedTime < MONKEY_ANIMATION_DURATION / 2) {
			textures->monkeyTexture = textures->monkeyTexture1;
		}
		else {
			textures->monkeyTexture = textures->monkeyTexture0;
		}
	}
}

//Wyœwietlanie punktów nad postaci¹
void pointsAnimation(SDL_Surface* screen, SDL_Surface* charset, GameTextures* textures, GameRects* rects, GameState* state) {
	if (state->frameStart - state->lastScorePopupTime < SCORE_POPUP_TIME) {
		char scoreText[16];
		sprintf(scoreText, "+%d", state->lastScorePoints);
		DrawString(screen, rects->playerRect.x, rects->playerRect.y - 20, scoreText, charset);
	}
}

int findTrophyIndex(SDL_Rect& playerRect, SDL_Rect trophyRects[], GameState* state) {
	for (int i = 0; i < state->numOfTrophies; ++i) {
		SDL_Rect& trophy = trophyRects[i];
		if (SDL_HasIntersection(&playerRect, &trophy)) {
			return i;
		}
	}
	return -1;
}

void updatePlayerVerticalStatus(GameRects* rects, GameState* state) {
	if (state->isJumping) {
		jumpPlayer(rects->playerRect, rects->platformRects, state);
	}

	if (state->isFalling) {
		fallPlayer(rects->playerRect, rects->platformRects, state);
	}
}

void freeRects(GameRects* rects) {
	if (rects->ledderRects != NULL) {
		delete[] rects->ledderRects;
		rects->ledderRects = NULL;
	}

	if (rects->platformRects != NULL) {
		delete[] rects->platformRects;
		rects->platformRects = NULL;
	}

	if (rects->staticBarrelRects != NULL) {
		delete[] rects->staticBarrelRects;
		rects->staticBarrelRects = NULL;
	}

	if (rects->monkeyRects != NULL) {
		delete rects->monkeyRects;
		rects->monkeyRects = NULL;
	}

	if (rects->princessRects != NULL) {
		delete rects->princessRects;
		rects->princessRects = NULL;
	}

	if (rects->trophyRects != NULL) {
		delete rects->trophyRects;
		rects->trophyRects = NULL;
	}
}

void drawGameElements(SDL_Surface* screen, SDL_Surface* charset, SDL_Renderer* renderer, SDL_Texture* scrtex, GameRects* rects, GameTextures* textures, GameState* state, GameColors *colors) {
	//Teksty - wymagane wywo³anie przed innymi obietkami
	
	// Wynik
	drawScore(screen, scrtex, charset, renderer, &state->score);

	// Tekst informacyjny
	timeAndFpsInfo(screen, renderer, scrtex, charset, colors->czarny, colors->czerwony, state);
	
	// Rysowanie platform
	for (int i = 0; i < state->numOfPlatforms; i++) {
		DrawTiledTextureOnElement(renderer, textures->platformTexture, rects->platformRects[i]);
	}

	// Rysowanie drabinek
	for (int i = 0; i < state->numOfLedders; i++) {
		DrawTiledTextureOnElement(renderer, textures->ledderTexture, rects->ledderRects[i]);
	}

	// Rysowanie statycznych beczek
	for (int i = 0; i < state->numOfStaticBarrels; i++) {
		SDL_RenderCopy(renderer, textures->barrelTexture, NULL, &rects->staticBarrelRects[i]);
	}

	//Renderowanie ruchomych beczek
	for (int i = 0; i < MAX_BARRELS; i++) {
		SDL_RenderCopy(renderer, textures->barrel_rollTexture, NULL, &barrels[i].rect);
	}

	// Rysowanie ma³p
	for (int i = 0; i < state->numOfMonkeys; i++) {
		SDL_RenderCopy(renderer, textures->monkeyTexture, NULL, &rects->monkeyRects[i]);
	}

	// Rysowanie ksiê¿niczek
	for (int i = 0; i < state->numOfPrincesses; i++) {
		SDL_RenderCopy(renderer, textures->princessTexture, NULL, &rects->princessRects[i]);
	}

	// Rysowanie trofeów
	for (int i = 0; i < state->numOfTrophies; i++) {
		SDL_RenderCopy(renderer, textures->trophyTexture, NULL, &rects->trophyRects[i]);
	}

	//Rysowanie serc
	drawLives(renderer, textures->heartTexture, &state->lives);
}

void updateAnimations(SDL_Surface* screen, SDL_Surface* charset, GameTextures *textures, GameState *state, GameRects *rects) {
	monkeyAnimation(textures, state);
	playerMoveAnimation(state, textures);
	playerJumpAnimation(state, textures);
	playerFallAnimation(state, textures);
	playerClimbingAnimation(state, textures, rects);
	pointsAnimation(screen, charset, textures, rects, state);
}

void checkBarrelSpawnCondition(SDL_Renderer* renderer, GameRects *rects, GameState *state) {
	if (state->frameStart - state->lastPrintTime >= BARREL_RESPAWN_TIME * 1000) { //*1000 bo czas w milisekundach
		state->monkeyAnimationStartTime = SDL_GetTicks();
		if (state->frameStart - state->lastPrintTime >= BARREL_RESPAWN_TIME * 1000 + MONKEY_ANIMATION_DURATION) {
			state->lastPrintTime = state->frameStart;
			createBarrel(renderer, rects);
		}
	}
}

void jumpBarrel(GameRects *rects, GameState *state) {
	if (isBarrelUnderPlayer(rects->playerRect, rects->platformRects, state) && !state->isClimbing) {
		state->lastScorePopupTime = SDL_GetTicks();
		state->lastScorePoints = BARREL_JUMP_SCORE;
		state->score += BARREL_JUMP_SCORE;
	}
}

void collectTrophy(GameState *state, GameRects *rects) {
	for (int i = 0; i < state->numOfTrophies; i++) {
		if (SDL_HasIntersection(&rects->playerRect, &rects->trophyRects[i])) {
			state->score += TROPHY_SCORE;
			state->lastScorePopupTime = SDL_GetTicks();
			state->lastScorePoints = TROPHY_SCORE;
			rects->trophyRects[i] = { 0, 0, 0, 0 }; // Ukrywamy trofeum
			break; // Przerywamy pêtlê po pierwszym znalezionym trofeum, zak³adamy, ¿e dwa trofea nie znajduj¹ siê w tym samym miejscu
		}
	}
}

int inGameEvents(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Surface* charset, SDL_Texture* scrtex, GameRects *rects, GameState *state, GameTextures *textures, GameColors *colors) {
	//Sprawdzanie, czy gracz przeskoczy³ beczkê
	jumpBarrel(rects, state);

	//Kolizja z beczk¹
	if (isCollisionWithBarrel(rects->playerRect)) {
		state->lives--;
		if (state->lives <= 0) {
			gameOver(screen, renderer, charset, scrtex, state, colors->czarny);
			return 0;
		}
		handleLifeLoss(screen, charset, renderer, scrtex, colors->czarny, colors->czerwony, state);
		preparePlayer(renderer, rects->playerRect, textures->playerTexture, rects->platformRects, state);
		destroyElements(rects, state);
		state->score = 0;
		return 1;
	}

	//Tworzenie beczek
	checkBarrelSpawnCondition(renderer, rects, state);

	//Aktualizacja animacji
	updateAnimations(screen, charset, textures, state, rects);

	//Aktualizacja ruchu beczek
	updateBarrels(rects, state, textures);

	//Aktualizacja pozycji - skakanie, spadanie
	updatePlayerVerticalStatus(rects, state);

	return 1;
}

int endStage(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Surface* charset, SDL_Texture* scrtex, GameState *state, GameTextures *textures, GameRects *rects, int czarny) {
	for (int i = 0; i < state->numOfPrincesses; i++) {
		if (SDL_HasIntersection(&rects->playerRect, &rects->princessRects[i])) {
			preparePlayer(renderer, rects->playerRect, textures->playerTexture, rects->platformRects, state);
			destroyElements(rects, state);
			state->score += END_STAGE_SCORE;
			state->currentStage = state->currentStage + 1;

			if (state->currentStage > 3) {
				gameOver(screen, renderer, charset, scrtex, state, czarny);
				return 0;
			}
		}
	}
	return 1;
}

void changeStage(SDL_Surface* screen, GameState *state, GameRects *rects, GameColors *colors) {
	if (state->localStage != state->currentStage) {
		if (prepareGameStage(state, rects) == 0) {
			return;
		}
		prepareGameStageElements(screen, colors, rects, state);
	}
}

void fpsLimiter(GameState *state) {
	state->frameTime = SDL_GetTicks() - state->frameStart;
	if (frameDelay > state->frameTime) {
		SDL_Delay(frameDelay - state->frameTime);
	}
}

void prepareBeforeGame(SDL_Surface* screen, SDL_Surface* charset, SDL_Renderer* renderer, SDL_Texture* scrtex, GameState *state, GameTextures *textures, GameRects *rects, GameColors *colors) {
	setupGameEnviroment(state, rects);
	if (prepareGameStage(state, rects) == 0) {
		return;
	}
	prepareGameStageElements(screen, colors, rects, state);
	drawGameElements(screen, charset, renderer, scrtex, rects, textures, state, colors);
	preparePlayer(renderer, rects->playerRect, textures->playerTexture, rects->platformRects, state);
}

void play(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Surface* charset, SDL_Window* window, SDL_Texture* scrtex, GameTextures* textures, GameColors* colors, GameState* state) {
	GameRects rects;

	//Przygotowanie gry
	prepareBeforeGame(screen, charset, renderer, scrtex, state, textures, &rects, colors);

	//G³ówna pêtla gry
	while (!state->quit) {
		state->frameStart = SDL_GetTicks();
		state->deltaTime = (state->frameStart - state->lastFrameTime) / 1000.0f;

		SDL_RenderClear(renderer);

		//Dotarcie do punktu koñcowego, nowy etap
		if (endStage(screen, renderer, charset, scrtex, state, textures, &rects, colors->czarny) == 0) {
			break;
		}

		// obs³uga klawiszy
		handleInGameKey(renderer, textures->playerTexture, &rects, state);

		//Zmiana etapu - je¿eli potrzebna
		changeStage(screen, state, &rects, colors);

		//Gracz zebra³ trofeum
		collectTrophy(state, &rects);

		//Rysowanie elementów gry
		drawGameElements(screen, charset, renderer, scrtex, &rects, textures, state, colors);

		//Przygotowanie etapu
		prepareGameStageElements(screen, colors, &rects, state);

		//Wydarzenia w grze + animacje
		if (inGameEvents(screen, renderer, charset, scrtex, &rects, state, textures, colors) == 0) {
			break;
		}
		
		//Ogranicznik fps - je¿eli potrzebny
		fpsLimiter(state);

		state->localStage = state->currentStage;

		SDL_RenderCopy(renderer, textures->playerTexture, NULL, &rects.playerRect);
		SDL_RenderPresent(renderer);

		state->lastFrameTime = state->frameStart;
	};

	freeRects(&rects);
	return;
}

void drawMainMenu(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Texture* scrtex, SDL_Surface* charset) {
	char text[128];

	const char* menuOptions[] = {
		"N - nowa rozgrywka",
		"E - etapy",
		"W - wyniki",
		"Esc - wyjscie"
	};

	const int menuOptionsCount = sizeof(menuOptions) / sizeof(menuOptions[0]);
	int y = SCREEN_HEIGHT / menuOptionsCount;

	SDL_RenderClear(renderer);

	// Rysowanie opcji menu
	for (int i = 0; i < menuOptionsCount; i++) {
		sprintf(text, "%s", menuOptions[i]);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, y, text, charset);
		y += PLATFORM_SPACE; // Zwiêkszenie Y o PLATFORM_SPACE dla kolejnego tekstu
	}

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}

int prepareTextures(SDL_Renderer* renderer, GameTextures* textures) {
	SDL_Surface* platform = ScaleBitmap("./Bitmapy/platform.bmp", BITMAP_WIDTH, BITMAP_HEIGHT);
	SDL_Surface* heart = ScaleBitmap("./Bitmapy/heart.bmp", BITMAP_WIDTH, BITMAP_HEIGHT);
	SDL_Surface* trophy = ScaleBitmap("./Bitmapy/trophy.bmp", BITMAP_WIDTH, BITMAP_HEIGHT);

	SDL_Surface* ledder = ScaleBitmap("./Bitmapy/ledder2.bmp", BITMAP_WIDTH, BITMAP_HEIGHT);
	SDL_Surface* barrel = ScaleBitmap("./Bitmapy/barrel.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* player0 = ScaleBitmap("./Bitmapy/player0.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* playerBeforeRun = ScaleBitmap("./Bitmapy/playerBeforeRun.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* playerBeforeRunFlipped = ScaleBitmap("./Bitmapy/playerBeforeRunFlipped.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* playerRunning = ScaleBitmap("./Bitmapy/playerRunning.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* playerRunningFlipped = ScaleBitmap("./Bitmapy/playerRunningFlipped.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* playerAfterRun = ScaleBitmap("./Bitmapy/playerAfterRun.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* playerAfterRunFlipped = ScaleBitmap("./Bitmapy/playerAfterRunFlipped.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* playerBeforeJump = ScaleBitmap("./Bitmapy/playerBeforeJump.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* playerJumping = ScaleBitmap("./Bitmapy/playerJumping.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* playerClimbing1 = ScaleBitmap("./Bitmapy/playerClimbing1.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* playerClimbing2 = ScaleBitmap("./Bitmapy/playerClimbing2.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* princess = ScaleBitmap("./Bitmapy/princess.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* barrel_roll0 = ScaleBitmap("./Bitmapy/barrel_roll0.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* barrel_roll1 = ScaleBitmap("./Bitmapy/barrel_roll1.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);
	SDL_Surface* barrel_roll2 = ScaleBitmap("./Bitmapy/barrel_roll2.bmp", BITMAP_WIDTH * 2, BITMAP_HEIGHT * 2);

	SDL_Surface* monkey0 = ScaleBitmap("./Bitmapy/monkey0.bmp", BITMAP_WIDTH * 4, BITMAP_HEIGHT * 4);
	SDL_Surface* monkey1 = ScaleBitmap("./Bitmapy/monkey1.bmp", BITMAP_WIDTH * 4, BITMAP_HEIGHT * 4);

	textures->platformTexture = SDL_CreateTextureFromSurface(renderer, platform);
	textures->playerTexture0 = SDL_CreateTextureFromSurface(renderer, player0);
	textures->playerTextureBeforeRun = SDL_CreateTextureFromSurface(renderer, playerBeforeRun);
	textures->playerTextureBeforeRunFlipped = SDL_CreateTextureFromSurface(renderer, playerBeforeRunFlipped);
	textures->playerTextureRunning = SDL_CreateTextureFromSurface(renderer, playerRunning);
	textures->playerTextureRunningFlipped = SDL_CreateTextureFromSurface(renderer, playerRunningFlipped);
	textures->playerTextureAfterRun = SDL_CreateTextureFromSurface(renderer, playerAfterRun);
	textures->playerTextureAfterRunFlipped = SDL_CreateTextureFromSurface(renderer, playerAfterRunFlipped);
	textures->playerTextureBeforeJump = SDL_CreateTextureFromSurface(renderer, playerBeforeJump);
	textures->playerTextureJumping = SDL_CreateTextureFromSurface(renderer, playerJumping);
	textures->playerTextureClimbing1 = SDL_CreateTextureFromSurface(renderer, playerClimbing1);
	textures->playerTextureClimbing2 = SDL_CreateTextureFromSurface(renderer, playerClimbing2);
	textures->monkeyTexture0 = SDL_CreateTextureFromSurface(renderer, monkey0);
	textures->monkeyTexture1 = SDL_CreateTextureFromSurface(renderer, monkey1);
	textures->ledderTexture = SDL_CreateTextureFromSurface(renderer, ledder);
	textures->barrelTexture = SDL_CreateTextureFromSurface(renderer, barrel);
	textures->princessTexture = SDL_CreateTextureFromSurface(renderer, princess);
	textures->barrel_rollTexture0 = SDL_CreateTextureFromSurface(renderer, barrel_roll0);
	textures->barrel_rollTexture1 = SDL_CreateTextureFromSurface(renderer, barrel_roll1);
	textures->barrel_rollTexture2 = SDL_CreateTextureFromSurface(renderer, barrel_roll2);
	textures->heartTexture = SDL_CreateTextureFromSurface(renderer, heart);
	textures->trophyTexture = SDL_CreateTextureFromSurface(renderer, trophy);

	//Przypisywanie podstawowych tekstur - wykorzystywane przy animacjach
	textures->monkeyTexture = textures->monkeyTexture0;
	textures->playerTexture = textures->playerTexture0;
	textures->barrel_rollTexture = textures->barrel_rollTexture0;

	SDL_Surface* surfacesToFree[] = {
		platform, barrel, ledder, player0, playerBeforeRun,
		playerBeforeRunFlipped, playerRunning, playerRunningFlipped,
		playerAfterRun, playerAfterRunFlipped, playerBeforeJump,
		playerJumping, playerClimbing1, playerClimbing2, monkey0,
		monkey1, princess, barrel_roll0, barrel_roll1, barrel_roll2,
		heart, trophy
	};

	SDL_FreeSurface(platform);
	SDL_FreeSurface(barrel);
	SDL_FreeSurface(ledder);
	SDL_FreeSurface(player0);
	SDL_FreeSurface(playerBeforeRun);
	SDL_FreeSurface(playerBeforeRunFlipped);
	SDL_FreeSurface(playerRunning);
	SDL_FreeSurface(playerRunningFlipped);
	SDL_FreeSurface(playerAfterRun);
	SDL_FreeSurface(playerAfterRunFlipped);
	SDL_FreeSurface(playerBeforeJump);
	SDL_FreeSurface(playerJumping);
	SDL_FreeSurface(playerClimbing1);
	SDL_FreeSurface(playerClimbing2);
	SDL_FreeSurface(monkey0);
	SDL_FreeSurface(monkey1);
	SDL_FreeSurface(princess);
	SDL_FreeSurface(barrel_roll0);
	SDL_FreeSurface(barrel_roll1);
	SDL_FreeSurface(barrel_roll2);
	SDL_FreeSurface(heart);
	SDL_FreeSurface(trophy);

	return 0;
}

void freeTextures(GameTextures* textures) {
	SDL_Texture** texturesArray[] = {
		&textures->platformTexture, &textures->playerTexture, &textures->playerTexture0,
		&textures->playerTextureBeforeRun, &textures->playerTextureBeforeRunFlipped,
		&textures->playerTextureRunning, &textures->playerTextureRunningFlipped,
		&textures->playerTextureAfterRun, &textures->playerTextureAfterRunFlipped,
		&textures->playerTextureBeforeJump, &textures->playerTextureJumping,
		&textures->playerTextureClimbing1, &textures->playerTextureClimbing2,
		&textures->monkeyTexture, &textures->monkeyTexture0, &textures->monkeyTexture1,
		&textures->ledderTexture, &textures->barrelTexture, &textures->princessTexture,
		&textures->barrel_rollTexture, &textures->barrel_rollTexture0,
		&textures->barrel_rollTexture1, &textures->barrel_rollTexture2,
		&textures->heartTexture, &textures->trophyTexture
	};

	const int numTextures = sizeof(texturesArray) / sizeof(texturesArray[0]);

	for (int i = 0; i < numTextures; ++i) {
		if (*texturesArray[i]) {
			SDL_DestroyTexture(*texturesArray[i]);
			*texturesArray[i] = NULL;
		}
	}
}

void prepareColors(SDL_Surface* screen, GameColors* colors) {
	colors->czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	colors->zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	colors->czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	colors->niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	colors->jasnyniebieski = SDL_MapRGB(screen->format, 0, 229, 229);
}

int setupEnvironment(SDL_Window** window, SDL_Renderer** renderer, SDL_Surface** screen, SDL_Texture** scrtex) {
	// Tworzenie okna i renderera
	if (!createWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, false, window, renderer)) {
		return 1; // B³¹d przy tworzeniu okna/renderera
	}

	// Ustawienie grafiki
	setGraphics(*renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Tworzenie powierzchni i tekstury
	if (!createSurfaceAndTexture(*renderer, SCREEN_WIDTH, SCREEN_HEIGHT, screen, scrtex)) {
		SDL_DestroyRenderer(*renderer);
		SDL_DestroyWindow(*window);
		SDL_Quit();
		return 1; // B³¹d przy tworzeniu powierzchni/tekstury
	}

	SDL_SetWindowTitle(*window, "Projekt King Donkey s194103");
	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	return 0;
}

void drawStagesInMenu(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Texture* scrtex, SDL_Surface* charset) {
	char text[128];

	const char* menuOptions[] = {
		"ETAP I",
		"ETAP II",
		"ETAP III"
	};

	const int menuOptionsCount = sizeof(menuOptions) / sizeof(menuOptions[0]);
	int y = SCREEN_HEIGHT / menuOptionsCount;

	SDL_RenderClear(renderer);

	// Rysowanie opcji menu
	for (int i = 0; i < menuOptionsCount; i++) {
		sprintf(text, "%s", menuOptions[i]);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, y, text, charset);
		y += PLATFORM_SPACE; // Zwiêkszenie Y o PLATFORM_SPACE dla kolejnego tekstu
	}

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void selectStageInMenu(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Texture* scrtex, SDL_Surface* charset, GameState* state, GameColors* colors) {
	SDL_Event event;

	while (!state->quit) {
		SDL_RenderClear(renderer);
		SDL_FillRect(screen, NULL, colors->czarny);

		drawStagesInMenu(screen, renderer, scrtex, charset);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					state->quit = 1;
				}
				else if (event.key.keysym.sym == SDLK_1) {
					//etap 1
					state->currentStage = 1;
				}
				else if (event.key.keysym.sym == SDLK_2) {
					//etap 2
					state->currentStage = 2;
				}
				else if (event.key.keysym.sym == SDLK_3) {
					//etap 3
					state->currentStage = 3;
				}
				else if (event.key.keysym.sym == SDLK_BACKSPACE) {
					return;
				}
				break;
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
				state->quit = 1;
				break;
			};
		};
	}
}

//Sortowanie b¹belkowe
void sortScores(Score* scores, int count) {
	for (int i = 0; i < count - 1; i++) {
		for (int j = 0; j < count - i - 1; j++) {
			if (scores[j].score < scores[j + 1].score) {
				Score temp = scores[j];
				scores[j] = scores[j + 1];
				scores[j + 1] = temp;
			}
		}
	}
}

Score* loadScores(const char* filename, int* count) {
	FILE* file = fopen(filename, "r");
	if (!file) {
		*count = 0;
		return NULL;
	}

	int capacity = 0;
	*count = 0;
	Score* scores = NULL;
	Score* tempScores = NULL;
	Score temp;

	while (fscanf(file, "%127s %d", temp.playerName, &temp.score) == 2) {
		if (*count >= capacity) {
			if (capacity == 0) {
				capacity = 1;
			}
			else {
				capacity = capacity * 2;
			}
			tempScores = new Score[capacity];

			// Kopiowanie istniej¹cych danych, jeœli istniej¹
			for (int i = 0; i < *count; ++i) {
				tempScores[i] = scores[i];
			}

			// Usuwanie starej tablicy i przypisanie nowej
			delete[] scores;
			scores = tempScores;
		}
		scores[*count] = temp;
		(*count)++;
	}

	fclose(file);
	return scores;
}

void displayScores(SDL_Surface* screen, SDL_Surface* charset, Score* scores, int startEntry, int endEntry) {
	int lineHeight = (SCREEN_HEIGHT / MAX_SCORES_PER_PAGE) / 2; // Wysokoœæ linii tekstu, odstêpy miêdzy wynikami
	int totalLines = endEntry - startEntry; // Ca³kowita liczba linii do wyœwietlenia
	int textHeight = totalLines * lineHeight; // Ca³kowita wysokoœæ tekstu
	int startY = (SCREEN_HEIGHT - textHeight) / 2; // Pocz¹tkowa pozycja y, aby wyœrodkowaæ w pionie

	for (int i = startEntry; i < endEntry; i++) {
		char line[256];
		sprintf(line, "%s: %d", scores[i].playerName, scores[i].score);
		int textWidth = strlen(line) * 8; // Zak³adamy, ¿e szerokoœæ znaku to 8 pikseli
		int startX = (SCREEN_WIDTH - textWidth) / 2; // Pocz¹tkowa pozycja x, aby wyœrodkowaæ w poziomie
		DrawString(screen, startX, startY + (i - startEntry) * lineHeight, line, charset);
	}
}

void showScores(SDL_Surface* screen, SDL_Surface* charset, SDL_Renderer* renderer, SDL_Texture* scrtex, GameColors* colors, GameState* state) {
	int totalEntries;
	Score* allScores = loadScores("scores.txt", &totalEntries); // Wczytanie wszystkich wyników
	if (!allScores) {
		return; // Jeœli nie uda³o siê wczytaæ wyników
	}

	sortScores(allScores, totalEntries); // Sortowanie wyników

	int totalPages = (totalEntries + MAX_SCORES_PER_PAGE - 1) / MAX_SCORES_PER_PAGE;
	int currentPage = 0;
	SDL_Event event;

	while (!state->quit) {
		SDL_RenderClear(renderer);
		SDL_FillRect(screen, NULL, colors->czarny);

		int start = currentPage * MAX_SCORES_PER_PAGE;
		int end = (start + MAX_SCORES_PER_PAGE < totalEntries) ? start + MAX_SCORES_PER_PAGE : totalEntries;

		// Wywo³anie funkcji drawScores do wyœwietlenia wyników
		displayScores(screen, charset, allScores, start, end);

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				state->quit = 1;
			}
			else if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					state->quit = 1;
					break;
				case SDLK_RIGHT:
					if (currentPage < totalPages - 1) currentPage++;
					break;
				case SDLK_LEFT:
					if (currentPage > 0) currentPage--;
					break;
				case SDLK_BACKSPACE:
					delete[] allScores; // Zwolnienie zaalokowanej pamiêci
					return; // Powrót do menu
				}
			}
		}
	}

	delete[] allScores;
}

void handleMenuOptions(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Surface* charset, SDL_Window* window, SDL_Texture* scrtex, GameTextures* textures, GameColors* colors, GameState* state) {
	SDL_Event event;
	SDL_PollEvent(&event);

	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	if (event.key.keysym.sym == SDLK_ESCAPE) {
		state->quit = 1;
		return;
	}
	else if (keystate[SDL_SCANCODE_N]) {
		play(screen, renderer, charset, window, scrtex, textures, colors, state);
	}
	else if (keystate[SDL_SCANCODE_E]) {
		selectStageInMenu(screen, renderer, scrtex, charset, state, colors);
	}
	else if (keystate[SDL_SCANCODE_W]) {
		showScores(screen, charset, renderer, scrtex, colors, state);
	}
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	SDL_Surface* screen, * charset;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;

	GameTextures textures;
	GameColors colors;
	GameState state;

	if (!initSDL()) {
		return 1; // B³¹d inicjalizacji
	}

	//Przygotowanie œrodowiska
	setupEnvironment(&window, &renderer, &screen, &scrtex);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./Bitmapy/cs8x8.bmp");
	SDL_SetColorKey(charset, true, 0x000000);

	//Przygotowanie kolorów
	prepareColors(screen, &colors);

	//Przygotowanie tekstur
	prepareTextures(renderer, &textures);

	state.quit = 0;
	state.currentStage = 1;

	while (!state.quit) {
		SDL_RenderClear(renderer);
		SDL_FillRect(screen, NULL, colors.czarny);

		drawMainMenu(screen, renderer, scrtex, charset);

		// obs³uga zdarzeñ (o ile jakieœ zasz³y)
		handleMenuOptions(screen, renderer, charset, window, scrtex, &textures, &colors, &state);
	};

	// zwolnienie zasobów
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	freeTextures(&textures);

	SDL_Quit();
	return 0;
};