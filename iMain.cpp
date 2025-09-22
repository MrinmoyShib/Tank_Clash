#include "iGraphics.h"
#include "Constants.hpp"
#include "Structures.hpp"
#include "Globals.hpp"
#include "GameLogic.hpp"
#include "Audio.hpp"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>

// -------- Global Variable Definitions --------
int gameState = 0;
int menuImage, gamemap1Image, aboutImage, enemyImage;
int heroRotation[36];
int currentImage = -1;
int fireFront = -1, fireLeft = -1, fireRight = -1, fireBack = -1;
int enemyFireFront = -1, enemyFireLeft = -1, enemyFireRight = -1, enemyFireBack = -1;
bool hoverStart = false, hoverHighScore = false, hoverOption = false, hoverExit = false, gameAssetsLoaded = false;
bool hoverBack = false;
bool hoverBackOptions = false;
bool hoverBackHowTo = false;
bool hoverEasy = false, hoverMedium = false, hoverHard = false;
bool hoverHowToPlay = false;
bool hoverVictoryEasy = false, hoverVictoryMedium = false, hoverVictoryHard = false;
bool hoverVictoryMenu = false;
bool musicEnabled = true;
bool hoverMusicToggle = false;

int playerX = (SCREEN_width - TANK_W) / 2;
int playerY = 24;
int heroHealth = HERO_MAX_HEALTH;
int lastHeroFireTime = 0;
int gameStartTime = 0;

enum Difficulty currentDifficulty = MEDIUM;
bool projectileVisible = false;
double projectileXf = 0.0, projectileYf = 0.0;
double projVX = 0.0, projVY = 0.0;
Direction projectileDir = UP;

struct EnemyProjectile enemyProjectiles[MAX_ENEMY_PROJECTILES];
struct Box obstacles[25];
int obstacleCount = 0;
struct EnemyTank enemies[3];
int enemyCount = 3;
int gameTimer = 0;

int gMouseX = SCREEN_width / 2;
int gMouseY = SCREEN_height / 2;
double gAngleDeg = 0.0;

bool allEnemiesDestroyed = false;
bool enemyDestroyedThisFrame = false;
bool heroFiredThisFrame = false;

// -------- Helper Functions Implementation --------
int getCurrentTime() {
	return clock() * 1000 / CLOCKS_PER_SEC;
}

double calculateAngleRaw(int mx, int my, int cx, int cy){
	double dy = (double)(my - cy) * Y_AXIS_SIGN;
	double dx = (double)(mx - cx);
	double a = rad2deg(atan2(dy, dx));
	if (a < 0) a += 360.0;
	return a;
}

void updateAngleFromMouse(int mx, int my){
	gMouseX = mx; gMouseY = my;
	const int cx = playerX + (TANK_W / 2);
	const int cy = playerY + (TANK_H / 2);
	double rawDeg = calculateAngleRaw(gMouseX, gMouseY, cx, cy);
	gAngleDeg = fmod(rawDeg + ANGLE_OFFSET_DEG + 360.0, 360.0);
	int frameIndex = (int)((gAngleDeg + 5.0) / 10.0) % 36;
	currentImage = heroRotation[frameIndex];
}

int loadAngleFrame(int idx){
	char path[512];
#if defined(_MSC_VER)
	sprintf_s(path, "%s\\Angle%d.bmp", IMAGE_DIR, idx);
#else
	sprintf(path, "%s\\Angle%d.bmp", IMAGE_DIR, idx);
#endif
	int id = iLoadImage(path);
	if (id <= 0) {
#if defined(_MSC_VER)
		sprintf_s(path, "%s\\Angle%d.png", IMAGE_DIR, idx);
#else
		sprintf(path, "%s\\Angle%d.png", IMAGE_DIR, idx);
#endif
		id = iLoadImage(path);
	}
	if (id <= 0) { printf("Failed to load Angle%d\n", idx); }
	return id;
}

void forceFacingFrame(int angleIndex1Based){
	int idx = (angleIndex1Based - 1) % 36;
	if (idx < 0) idx += 36;
	currentImage = heroRotation[idx];
	gAngleDeg = idx * 10.0;
}

enum Direction dirFromVel(double vx, double vy){
	if (fabs(vy) > fabs(vx)) return (vy > 0 ? UP : DOWN);
	return (vx > 0 ? RIGHT : LEFT);
}

// -------- Obstacle Functions --------
void addObstacle(int x, int y, int w, int h){
	if (obstacleCount >= (int)(sizeof(obstacles) / sizeof(obstacles[0]))) return;
	struct Box b; b.x = x; b.y = y; b.w = w; b.h = h;
	obstacles[obstacleCount++] = b;
}

void SetObstacles(){
	obstacleCount = 0;
	addObstacle(27, 260, 88, 44);
	addObstacle(128, 260, 102, 44);
	addObstacle(371, 260, 112, 44);
	addObstacle(832, 312, 448, 62);
	addObstacle(832, 348, 192, 62);
	addObstacle(976, 330, 192, 62);
	addObstacle(211, 466, 272, 56);
	addObstacle(0, 0, 317, 158);
	addObstacle(0, 0, 541, 96);
	addObstacle(0, 552, 317, 186);
	addObstacle(0, 624, 544, 96);
	addObstacle(736, 0, 640, 102);
	addObstacle(960, 102, 480, 66);
	addObstacle(736, 630, 640, 84);
	addObstacle(960, 546, 480, 84);
}

bool isColliding(int x, int y){
	for (int i = 0; i<obstacleCount; i++){
		struct Box b = obstacles[i];
		if (x + TANK_W > b.x && x < b.x + b.w &&
			y + TANK_H > b.y && y < b.y + b.h) return true;
	}
	return false;
}

bool isCollidingWithPlayer(int enemyX, int enemyY) {
	return (enemyX + TANK_W > playerX && enemyX < playerX + TANK_W &&
		enemyY + TANK_H > playerY && enemyY < playerY + TANK_H);
}

// -------- Enemy Functions --------
bool checkAllEnemiesDestroyed() {
	for (int i = 0; i < enemyCount; i++) {
		if (enemies[i].health > 0) {
			return false;
		}
	}
	return true;
}

void initEnemyProjectiles() {
	for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
		enemyProjectiles[i].visible = false;
	}
}

void fireEnemyProjectile(int enemyIndex) {
	int enemyX = enemies[enemyIndex].x + TANK_W / 2;
	int enemyY = enemies[enemyIndex].y + TANK_H / 2;
	int heroX = playerX + TANK_W / 2;
	int heroY = playerY + TANK_H / 2;

	double dx = (double)heroX - enemyX;
	double dy = (double)heroY - enemyY;
	double len = sqrt(dx*dx + dy*dy);
	if (len < 1.0) { dx = 1.0; dy = 0.0; len = 1.0; }

	double maxDeviation = 0.0;
	double projectileSpeed = PROJECTILE_SPEED * 0.6;
	switch (currentDifficulty) {
	case EASY:
		maxDeviation = 1.5;
		projectileSpeed = PROJECTILE_SPEED * 0.4;
		break;
	case MEDIUM:
		maxDeviation = 0.8;
		projectileSpeed = PROJECTILE_SPEED * 0.5;
		break;
	case HARD:
		maxDeviation = 0.4;
		projectileSpeed = PROJECTILE_SPEED * 0.6;
		break;
	}

	if (currentDifficulty == HARD) {
		double perpX = -dy / len;
		double perpY = dx / len;
		double sideOffset = TANK_W * 0.4;

		// Fire from left side
		for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
			if (!enemyProjectiles[i].visible) {
				double leftX = enemyX + perpX * sideOffset;
				double leftY = enemyY + perpY * sideOffset;

				double deviationX = ((double)rand() / RAND_MAX - 0.5) * 2.0 * maxDeviation;
				double deviationY = ((double)rand() / RAND_MAX - 0.5) * 2.0 * maxDeviation;
				double adjDx = dx + deviationX * len * 0.015;
				double adjDy = dy + deviationY * len * 0.015;
				double adjLen = sqrt(adjDx*adjDx + adjDy*adjDy);
				if (adjLen < 1.0) adjLen = 1.0;
				double ux = adjDx / adjLen;
				double uy = adjDy / adjLen;

				enemyProjectiles[i].xf = leftX + ux * PROJECTILE_SPAWN_OFFSET - PROJECTILE_W * 0.5;
				enemyProjectiles[i].yf = leftY + uy * PROJECTILE_SPAWN_OFFSET - PROJECTILE_H * 0.5;
				enemyProjectiles[i].vx = ux * projectileSpeed;
				enemyProjectiles[i].vy = uy * projectileSpeed;
				enemyProjectiles[i].dir = dirFromVel(enemyProjectiles[i].vx, enemyProjectiles[i].vy);
				enemyProjectiles[i].visible = true;
				break;
			}
		}

		// Fire from right side
		for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
			if (!enemyProjectiles[i].visible) {
				double rightX = enemyX - perpX * sideOffset;
				double rightY = enemyY - perpY * sideOffset;

				double deviationX = ((double)rand() / RAND_MAX - 0.5) * 2.0 * maxDeviation;
				double deviationY = ((double)rand() / RAND_MAX - 0.5) * 2.0 * maxDeviation;
				double adjDx = dx + deviationX * len * 0.015;
				double adjDy = dy + deviationY * len * 0.015;
				double adjLen = sqrt(adjDx*adjDx + adjDy*adjDy);
				if (adjLen < 1.0) adjLen = 1.0;
				double ux = adjDx / adjLen;
				double uy = adjDy / adjLen;

				enemyProjectiles[i].xf = rightX + ux * PROJECTILE_SPAWN_OFFSET - PROJECTILE_W * 0.5;
				enemyProjectiles[i].yf = rightY + uy * PROJECTILE_SPAWN_OFFSET - PROJECTILE_H * 0.5;
				enemyProjectiles[i].vx = ux * projectileSpeed;
				enemyProjectiles[i].vy = uy * projectileSpeed;
				enemyProjectiles[i].dir = dirFromVel(enemyProjectiles[i].vx, enemyProjectiles[i].vy);
				enemyProjectiles[i].visible = true;
				break;
			}
		}
	}
	else {
		for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
			if (!enemyProjectiles[i].visible) {
				double deviationX = ((double)rand() / RAND_MAX - 0.5) * 2.0 * maxDeviation;
				double deviationY = ((double)rand() / RAND_MAX - 0.5) * 2.0 * maxDeviation;
				dx += deviationX * len * 0.015;
				dy += deviationY * len * 0.015;

				len = sqrt(dx*dx + dy*dy);
				if (len < 1.0) len = 1.0;
				double ux = dx / len;
				double uy = dy / len;

				enemyProjectiles[i].xf = enemyX + ux * PROJECTILE_SPAWN_OFFSET - PROJECTILE_W * 0.5;
				enemyProjectiles[i].yf = enemyY + uy * PROJECTILE_SPAWN_OFFSET - PROJECTILE_H * 0.5;
				enemyProjectiles[i].vx = ux * projectileSpeed;
				enemyProjectiles[i].vy = uy * projectileSpeed;
				enemyProjectiles[i].dir = dirFromVel(enemyProjectiles[i].vx, enemyProjectiles[i].vy);
				enemyProjectiles[i].visible = true;
				break;
			}
		}
	}
}

void updateEnemyProjectiles() {
	for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
		if (enemyProjectiles[i].visible) {
			enemyProjectiles[i].xf += enemyProjectiles[i].vx;
			enemyProjectiles[i].yf += enemyProjectiles[i].vy;

			if (enemyProjectiles[i].xf < -PROJECTILE_W ||
				enemyProjectiles[i].xf > SCREEN_width + PROJECTILE_W ||
				enemyProjectiles[i].yf < -PROJECTILE_H ||
				enemyProjectiles[i].yf > SCREEN_height + PROJECTILE_H) {
				enemyProjectiles[i].visible = false;
			}

			for (int j = 0; j < obstacleCount; j++) {
				struct Box b = obstacles[j];
				if (enemyProjectiles[i].xf + PROJECTILE_W > b.x &&
					enemyProjectiles[i].xf < b.x + b.w &&
					enemyProjectiles[i].yf + PROJECTILE_H > b.y &&
					enemyProjectiles[i].yf < b.y + b.h) {
					enemyProjectiles[i].visible = false;
					break;
				}
			}
		}
	}
}

void drawEnemyProjectiles() {
	for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
		if (enemyProjectiles[i].visible) {
			int img = enemyFireFront;
			switch (enemyProjectiles[i].dir) {
			case UP:    img = enemyFireFront; break;
			case DOWN:  img = enemyFireBack;  break;
			case RIGHT: img = enemyFireRight; break;
			case LEFT:  img = enemyFireLeft;  break;
			}
			iShowImage((int)enemyProjectiles[i].xf, (int)enemyProjectiles[i].yf,
				PROJECTILE_W, PROJECTILE_H, img);
		}
	}
}

void checkEnemyProjectileHeroCollision() {
	int hitCount = 0;
	for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++) {
		if (enemyProjectiles[i].visible) {
			if (enemyProjectiles[i].xf + PROJECTILE_W > playerX &&
				enemyProjectiles[i].xf < playerX + TANK_W &&
				enemyProjectiles[i].yf + PROJECTILE_H > playerY &&
				enemyProjectiles[i].yf < playerY + TANK_H) {
				enemyProjectiles[i].visible = false;
				hitCount++;
			}
		}
	}

	if (hitCount > 0) {
		heroHealth -= hitCount;
		mciSendString("play tankhit from 0", NULL, 0, NULL);
		if (heroHealth <= 0) {
			printf("Game Over! Hero destroyed!\n");
			gameState = 0;
		}
	}
}

void initEnemies() {
	for (int i = 0; i < enemyCount; i++) {
		enemies[i].active = false;
		switch (currentDifficulty) {
		case EASY:
			enemies[i].maxHealth = ENEMY_MAX_HEALTH_EASY;
			break;
		case MEDIUM:
			enemies[i].maxHealth = ENEMY_MAX_HEALTH_MEDIUM;
			break;
		case HARD:
			enemies[i].maxHealth = ENEMY_MAX_HEALTH_HARD;
			break;
		}
		enemies[i].health = enemies[i].maxHealth;
		enemies[i].moveTimer = 0;
		enemies[i].directionX = 0;
		enemies[i].directionY = -1;
		enemies[i].lastFireTime = 0;
	}
	gameTimer = 0;
	heroHealth = HERO_MAX_HEALTH;
	gameStartTime = getCurrentTime();
	initEnemyProjectiles();
	allEnemiesDestroyed = false;
	enemyDestroyedThisFrame = false;
}

void updateEnemies() {
	int currentTime = getCurrentTime();

	int spawnTime1 = 10000, spawnTime2 = 25000, spawnTime3 = 45000;
	switch (currentDifficulty) {
	case EASY:
		spawnTime1 = 15000; spawnTime2 = 30000; spawnTime3 = 50000;
		break;
	case MEDIUM:
		spawnTime1 = 10000; spawnTime2 = 25000; spawnTime3 = 45000;
		break;
	case HARD:
		spawnTime1 = 5000; spawnTime2 = 15000; spawnTime3 = 30000;
		break;
	}

	if (gameTimer >= spawnTime1 && gameTimer < spawnTime1 + 100 && !enemies[0].active && enemies[0].health > 0) {
		enemies[0].x = SCREEN_width / 2 - TANK_W / 2;
		enemies[0].y = SCREEN_height - TANK_H - 10;
		enemies[0].active = true;
		enemies[0].directionX = 0;
		enemies[0].directionY = -1;
		enemies[0].health = enemies[0].maxHealth;
		enemies[0].moveTimer = 0;
		enemies[0].lastFireTime = currentTime;
	}
	else if (gameTimer >= spawnTime2 && gameTimer < spawnTime2 + 100 && !enemies[1].active && enemies[1].health > 0) {
		enemies[1].x = 10;
		enemies[1].y = SCREEN_height / 2;
		enemies[1].active = true;
		enemies[1].directionX = 1;
		enemies[1].directionY = 0;
		enemies[1].health = enemies[1].maxHealth;
		enemies[1].moveTimer = 0;
		enemies[1].lastFireTime = currentTime;
	}
	else if (gameTimer >= spawnTime3 && gameTimer < spawnTime3 + 100 && !enemies[2].active && enemies[2].health > 0) {
		enemies[2].x = SCREEN_width - TANK_W - 10;
		enemies[2].y = SCREEN_height / 2 + 100;
		enemies[2].active = true;
		enemies[2].directionX = -1;
		enemies[2].directionY = 0;
		enemies[2].health = enemies[2].maxHealth;
		enemies[2].moveTimer = 0;
		enemies[2].lastFireTime = currentTime;
	}

	int enemySpeed = SPEED;
	int enemyFireCooldown = ENEMY_FIRE_COOLDOWN_MEDIUM;
	switch (currentDifficulty) {
	case EASY:
		enemySpeed = 2;
		enemyFireCooldown = ENEMY_FIRE_COOLDOWN_HARD;
		break;
	}

	for (int i = 0; i < enemyCount; i++) {
		if (enemies[i].active) {
			enemies[i].moveTimer++;

			if (currentTime - enemies[i].lastFireTime >= enemyFireCooldown) {
				fireEnemyProjectile(i);
				enemies[i].lastFireTime = currentTime;
			}

			if (enemies[i].moveTimer % (40 - (currentDifficulty * 5)) == 0) {
				if (enemies[i].moveTimer % (1200 - (currentDifficulty * 200)) == 0) {
					enemies[i].directionX = (rand() % 3) - 1;
					enemies[i].directionY = (rand() % 3) - 1;
					if (enemies[i].directionX == 0 && enemies[i].directionY == 0) {
						enemies[i].directionY = -1;
					}
				}

				int newX = enemies[i].x + enemies[i].directionX * enemySpeed;
				int newY = enemies[i].y + enemies[i].directionY * enemySpeed;

				if (newX >= 0 && newX <= SCREEN_width - TANK_W &&
					newY >= 0 && newY <= SCREEN_height - TANK_H &&
					!isColliding(newX, newY) && !isCollidingWithPlayer(newX, newY)) {
					enemies[i].x = newX;
					enemies[i].y = newY;
				}
				else {
					enemies[i].directionX = -enemies[i].directionX;
					enemies[i].directionY = -enemies[i].directionY;
				}
			}
		}
	}

	if (!allEnemiesDestroyed && checkAllEnemiesDestroyed() && gameTimer > spawnTime3 + 1000) {
		allEnemiesDestroyed = true;
		gameState = 5;
	}
}

void drawEnemies() {
	for (int i = 0; i < enemyCount; i++) {
		if (enemies[i].active) {
			iShowImage(enemies[i].x, enemies[i].y, TANK_W, TANK_H, enemyImage);
			if (enemies[i].maxHealth > 1) {
				iSetColor(50, 50, 50);
				iFilledRectangle(enemies[i].x, enemies[i].y + TANK_H + 5, TANK_W, 8);
				iSetColor(255, 0, 0);
				int healthBarWidth = (TANK_W * enemies[i].health) / enemies[i].maxHealth;
				iFilledRectangle(enemies[i].x, enemies[i].y + TANK_H + 5, healthBarWidth, 8);
			}
		}
	}
}

void checkProjectileEnemyCollision() {
	if (!projectileVisible) return;
	for (int i = 0; i < enemyCount; i++) {
		if (enemies[i].active) {
			if (projectileXf + PROJECTILE_W > enemies[i].x && projectileXf < enemies[i].x + TANK_W &&
				projectileYf + PROJECTILE_H > enemies[i].y && projectileYf < enemies[i].y + TANK_H) {
				enemies[i].health--;
				projectileVisible = false;
				if (enemies[i].health <= 0) {
					enemies[i].active = false;
					enemyDestroyedThisFrame = true;
				}
				break;
			}
		}
	}

	for (int i = 0; i < obstacleCount; i++) {
		struct Box b = obstacles[i];
		if (projectileXf + PROJECTILE_W > b.x && projectileXf < b.x + b.w &&
			projectileYf + PROJECTILE_H > b.y && projectileYf < b.y + b.h) {
			projectileVisible = false;
			break;
		}
	}
}

void drawHUD() {
	iSetColor(255, 0, 0);
	char healthText[50];
#if defined(_MSC_VER)
	sprintf_s(healthText, "Health: %d/%d", heroHealth, HERO_MAX_HEALTH);
#else
	sprintf(healthText, "Health: %d/%d", heroHealth, HERO_MAX_HEALTH);
#endif
	iText(20, SCREEN_height - 30, healthText, GLUT_BITMAP_HELVETICA_18);

	iSetColor(255, 255, 0);
	char diffText[30];
	switch (currentDifficulty) {
#if defined(_MSC_VER)
	case EASY: sprintf_s(diffText, "EASY"); break;
	case MEDIUM: sprintf_s(diffText, "MEDIUM"); break;
	case HARD: sprintf_s(diffText, "HARD"); break;
#else
	case EASY: sprintf(diffText, "EASY"); break;
	case MEDIUM: sprintf(diffText, "MEDIUM"); break;
	case HARD: sprintf(diffText, "HARD"); break;
#endif
	}
	iText(20, SCREEN_height - 60, diffText, GLUT_BITMAP_HELVETICA_12);

	int activeEnemies = 0;
	for (int i = 0; i < enemyCount; i++) {
		if (enemies[i].active) activeEnemies++;
	}
	iSetColor(255, 255, 255);
	char enemyText[30];
#if defined(_MSC_VER)
	sprintf_s(enemyText, "Enemies: %d", activeEnemies);
#else
	sprintf(enemyText, "Enemies: %d", activeEnemies);
#endif
	iText(20, SCREEN_height - 90, enemyText, GLUT_BITMAP_HELVETICA_12);
}

// -------- Audio Functions --------
void initializeAudio() {
	char openCommandBG[512];
	char openCommandDestroy[512];
	char openCommandFire[512];
	char openCommandHit[512];

#if defined(_MSC_VER)
	sprintf_s(openCommandBG, sizeof(openCommandBG), "open \"%sBG.mp3\" alias bgmusic wait", AUDIO_DIR);
	sprintf_s(openCommandDestroy, sizeof(openCommandDestroy), "open \"%sDestroy.mp3\" alias destroy wait", AUDIO_DIR);
	sprintf_s(openCommandFire, sizeof(openCommandFire), "open \"%sFiring.mp3\" alias herofire wait", AUDIO_DIR);
	sprintf_s(openCommandHit, sizeof(openCommandHit), "open \"%sHit.mp3\" alias tankhit wait", AUDIO_DIR);
#else
	sprintf(openCommandBG, "open \"%sBG.mp3\" alias bgmusic wait", AUDIO_DIR);
	sprintf(openCommandDestroy, "open \"%sDestroy.mp3\" alias destroy wait", AUDIO_DIR);
	sprintf(openCommandFire, "open \"%sFiring.mp3\" alias herofire wait", AUDIO_DIR);
	sprintf(openCommandHit, "open \"%sHit.mp3\" alias tankhit wait", AUDIO_DIR);
#endif

	mciSendString(openCommandBG, NULL, 0, NULL);
	mciSendString(openCommandDestroy, NULL, 0, NULL);
	mciSendString(openCommandFire, NULL, 0, NULL);
	mciSendString(openCommandHit, NULL, 0, NULL);
}

void playBackgroundMusic() {
	if (musicEnabled) {
		mciSendString("play bgmusic repeat", NULL, 0, NULL);
	}
}

void pauseBackgroundMusic() {
	mciSendString("pause bgmusic", NULL, 0, NULL);
}

void playDestroySound() {
	mciSendString("play destroy from 0", NULL, 0, NULL);
}

void playFireSound() {
	mciSendString("play herofire from 0", NULL, 0, NULL);
}

void playHitSound() {
	mciSendString("play tankhit from 0", NULL, 0, NULL);
}

void stopAllAudio() {
	mciSendString("stop bgmusic", NULL, 0, NULL);
	mciSendString("close bgmusic", NULL, 0, NULL);
	mciSendString("close destroy", NULL, 0, NULL);
	mciSendString("close herofire", NULL, 0, NULL);
	mciSendString("close tankhit", NULL, 0, NULL);
}

// -------- Main Render Function --------
void iDraw(){
	iClear();

	if (enemyDestroyedThisFrame) {
		playDestroySound();
		enemyDestroyedThisFrame = false;
	}

	if (heroFiredThisFrame) {
		playFireSound();
		heroFiredThisFrame = false;
	}

	if (gameState == 0){
		// Main Menu
		iShowImage(0, 0, SCREEN_width, SCREEN_height, menuImage);
		iSetColor(hoverStart ? 255 : 255, hoverStart ? 0 : 255, hoverStart ? 0 : 255);
		iText(976, 384, (char*)"START", GLUT_BITMAP_HELVETICA_18);
		iSetColor(hoverHighScore ? 255 : 255, hoverHighScore ? 0 : 255, hoverHighScore ? 0 : 255);
		iText(957.8, 336, (char*)"ABOUT US", GLUT_BITMAP_HELVETICA_18);
		iSetColor(hoverOption ? 255 : 255, hoverOption ? 0 : 255, hoverOption ? 0 : 255);
		iText(968, 288, (char*)"OPTION", GLUT_BITMAP_HELVETICA_18);
		iSetColor(hoverExit ? 255 : 255, hoverExit ? 0 : 255, hoverExit ? 0 : 255);
		iText(982, 240, (char*)"EXIT", GLUT_BITMAP_HELVETICA_18);
		return;
	}

	if (gameState == 2){
		// About Us Page
		iSetColor(20, 30, 50);
		iFilledRectangle(0, 0, SCREEN_width, SCREEN_height);
		iSetColor(255, 255, 255);
		iText(SCREEN_width / 2 - 100, 600, (char*)"ABOUT US", GLUT_BITMAP_TIMES_ROMAN_24);
		iSetColor(200, 200, 200);
		iText(SCREEN_width / 2 - 200, 550, (char*)"Tank Clash - A 2D Tank Combat Game", GLUT_BITMAP_HELVETICA_18);
		iSetColor(255, 255, 255);
		iText(SCREEN_width / 2 - 150, 480, (char*)"DEVELOPMENT TEAM", GLUT_BITMAP_HELVETICA_18);
		iSetColor(100, 255, 100);
		iText(SCREEN_width / 2 - 200, 420, (char*)"Shirsha Chowdhury", GLUT_BITMAP_HELVETICA_18);
		iSetColor(200, 200, 200);
		iText(SCREEN_width / 2 - 200, 395, (char*)"Developer", GLUT_BITMAP_HELVETICA_12);
		iSetColor(100, 255, 100);
		iText(SCREEN_width / 2 - 200, 350, (char*)"Mrinmoy Shib", GLUT_BITMAP_HELVETICA_18);
		iSetColor(200, 200, 200);
		iText(SCREEN_width / 2 - 200, 325, (char*)"Developer", GLUT_BITMAP_HELVETICA_12);
		iSetColor(100, 255, 100);
		iText(SCREEN_width / 2 - 200, 280, (char*)"Naimul Islam Jehan", GLUT_BITMAP_HELVETICA_18);
		iSetColor(200, 200, 200);
		iText(SCREEN_width / 2 - 200, 255, (char*)"Developer", GLUT_BITMAP_HELVETICA_12);
		iSetColor(255, 255, 255);
		iText(SCREEN_width / 2 - 250, 190, (char*)"This game features 360-degree tank rotation,", GLUT_BITMAP_HELVETICA_12);
		iText(SCREEN_width / 2 - 250, 170, (char*)"realistic projectile physics, and collision detection.", GLUT_BITMAP_HELVETICA_12);
		iText(SCREEN_width / 2 - 250, 150, (char*)"Built using iGraphics library in C++.", GLUT_BITMAP_HELVETICA_12);
		iSetColor(hoverBack ? 255 : 200, hoverBack ? 0 : 200, hoverBack ? 0 : 200);
		iText(100, 100, (char*)"BACK TO MENU", GLUT_BITMAP_HELVETICA_18);
		return;
	}

	if (gameState == 3){
		// Options Page
		iSetColor(30, 40, 60);
		iFilledRectangle(0, 0, SCREEN_width, SCREEN_height);
		iSetColor(255, 255, 255);
		iText(SCREEN_width / 2 - 100, 600, (char*)"OPTIONS", GLUT_BITMAP_TIMES_ROMAN_24);
		iSetColor(255, 255, 255);
		iText(SCREEN_width / 2 - 150, 500, (char*)"DIFFICULTY LEVEL:", GLUT_BITMAP_HELVETICA_18);

		iSetColor(hoverEasy ? 255 : 200, hoverEasy ? 255 : 200, hoverEasy ? 0 : 200);
		if (currentDifficulty == EASY) {
			iSetColor(255, 255, 0);
		}
		iText(SCREEN_width / 2 - 100, 450, (char*)"EASY", GLUT_BITMAP_HELVETICA_18);

		iSetColor(hoverMedium ? 255 : 200, hoverMedium ? 255 : 200, hoverMedium ? 0 : 200);
		if (currentDifficulty == MEDIUM) {
			iSetColor(255, 255, 0);
		}
		iText(SCREEN_width / 2 - 100, 400, (char*)"MEDIUM", GLUT_BITMAP_HELVETICA_18);

		iSetColor(hoverHard ? 255 : 200, hoverHard ? 255 : 200, hoverHard ? 0 : 200);
		if (currentDifficulty == HARD) {
			iSetColor(255, 255, 0);
		}
		iText(SCREEN_width / 2 - 100, 350, (char*)"HARD", GLUT_BITMAP_HELVETICA_18);

		iSetColor(255, 255, 255);
		iText(SCREEN_width / 2 - 150, 300, (char*)"MUSIC:", GLUT_BITMAP_HELVETICA_18);
		iSetColor(100, 100, 100);
		iRectangle(SCREEN_width / 2 - 100, 270, 50, 25);

		if (musicEnabled) {
			iSetColor(0, 255, 0);
		}
		else {
			iSetColor(255, 0, 0);
		}
		iFilledRectangle(SCREEN_width / 2 - 100 + 2, 270 + 2, 46, 21);

		iSetColor(hoverMusicToggle ? 255 : 200, hoverMusicToggle ? 255 : 200, hoverMusicToggle ? 0 : 200);
		iText(SCREEN_width / 2 - 30, 275, musicEnabled ? (char*)"ON" : (char*)"OFF", GLUT_BITMAP_HELVETICA_18);

		iSetColor(hoverHowToPlay ? 255 : 200, hoverHowToPlay ? 255 : 200, hoverHowToPlay ? 0 : 200);
		iText(SCREEN_width / 2 - 120, 225, (char*)"HOW TO PLAY", GLUT_BITMAP_HELVETICA_18);
		iSetColor(hoverBackOptions ? 255 : 200, hoverBackOptions ? 0 : 200, hoverBackOptions ? 0 : 200);
		iText(100, 100, (char*)"BACK TO MENU", GLUT_BITMAP_HELVETICA_18);
		return;
	}

	if (gameState == 4){
		// How to Play Page
		iSetColor(25, 35, 55);
		iFilledRectangle(0, 0, SCREEN_width, SCREEN_height);
		iSetColor(255, 255, 255);
		iText(SCREEN_width / 2 - 120, 650, (char*)"HOW TO PLAY", GLUT_BITMAP_TIMES_ROMAN_24);

		iSetColor(100, 255, 100);
		iText(100, 580, (char*)"MOVEMENT CONTROLS:", GLUT_BITMAP_HELVETICA_18);
		iSetColor(255, 255, 255);
		iText(120, 550, (char*)"W - Move tank forward (upward)", GLUT_BITMAP_HELVETICA_12);
		iText(120, 530, (char*)"S - Move tank backward (downward)", GLUT_BITMAP_HELVETICA_12);
		iText(120, 510, (char*)"A - Move tank left", GLUT_BITMAP_HELVETICA_12);
		iText(120, 490, (char*)"D - Move tank right", GLUT_BITMAP_HELVETICA_12);

		iSetColor(100, 255, 100);
		iText(100, 450, (char*)"AIMING & SHOOTING:", GLUT_BITMAP_HELVETICA_18);
		iSetColor(255, 255, 255);
		iText(120, 420, (char*)"Mouse Movement - Rotate tank to aim at cursor", GLUT_BITMAP_HELVETICA_12);
		iText(120, 400, (char*)"Left Mouse Button - Fire projectile toward cursor", GLUT_BITMAP_HELVETICA_12);
		iText(120, 380, (char*)"Note: There's a cooldown between shots based on difficulty", GLUT_BITMAP_HELVETICA_12);

		iSetColor(100, 255, 100);
		iText(100, 340, (char*)"GAME MECHANICS:", GLUT_BITMAP_HELVETICA_18);
		iSetColor(255, 255, 255);
		iText(120, 310, (char*)"Health: You have 3 health points", GLUT_BITMAP_HELVETICA_12);
		iText(120, 290, (char*)"Enemies: Appear over time and shoot at you", GLUT_BITMAP_HELVETICA_12);
		iText(120, 270, (char*)"Obstacles: Block movement and projectiles", GLUT_BITMAP_HELVETICA_12);
		iText(120, 250, (char*)"Objective: Survive and destroy all enemy tanks!", GLUT_BITMAP_HELVETICA_12);

		iSetColor(100, 255, 100);
		iText(650, 580, (char*)"DIFFICULTY LEVELS:", GLUT_BITMAP_HELVETICA_18);
		iSetColor(255, 255, 255);
		iText(670, 550, (char*)"EASY: Slower enemies, poor accuracy, 1 HP", GLUT_BITMAP_HELVETICA_12);
		iText(670, 530, (char*)"MEDIUM: Balanced gameplay, 1 HP", GLUT_BITMAP_HELVETICA_12);
		iText(670, 510, (char*)"HARD: Fast enemies, dual projectiles, 2 HP", GLUT_BITMAP_HELVETICA_12);
		iText(670, 490, (char*)"HARD MODE: Enemies fire TWO projectiles!", GLUT_BITMAP_HELVETICA_12);

		iSetColor(100, 255, 100);
		iText(650, 430, (char*)"OTHER CONTROLS:", GLUT_BITMAP_HELVETICA_18);
		iSetColor(255, 255, 255);
		iText(670, 400, (char*)"ESC - Return to main menu from any screen", GLUT_BITMAP_HELVETICA_12);
		iText(670, 380, (char*)"Click buttons to navigate menus", GLUT_BITMAP_HELVETICA_12);

		iSetColor(255, 255, 100);
		iText(SCREEN_width / 2 - 80, 180, (char*)"SURVIVAL TIPS:", GLUT_BITMAP_HELVETICA_18);
		iSetColor(200, 200, 255);
		iText(SCREEN_width / 2 - 200, 150, (char*)"- Keep moving to avoid enemy fire", GLUT_BITMAP_HELVETICA_12);
		iText(SCREEN_width / 2 - 200, 130, (char*)"- Use obstacles as cover", GLUT_BITMAP_HELVETICA_12);
		iText(SCREEN_width / 2 - 200, 110, (char*)"- Each enemy projectile deals 1 damage", GLUT_BITMAP_HELVETICA_12);

		iSetColor(hoverBackHowTo ? 255 : 200, hoverBackHowTo ? 0 : 200, hoverBackHowTo ? 0 : 200);
		iText(100, 50, (char*)"BACK TO OPTIONS", GLUT_BITMAP_HELVETICA_18);
		return;
	}

	if (gameState == 5){
		// Victory Screen
		iSetColor(10, 50, 10);
		iFilledRectangle(0, 0, SCREEN_width, SCREEN_height);
		iSetColor(255, 215, 0);
		iFilledRectangle(SCREEN_width / 2 - 400, SCREEN_height / 2 + 200, 800, 4);
		iFilledRectangle(SCREEN_width / 2 - 400, SCREEN_height / 2 - 280, 800, 4);

		iSetColor(255, 255, 0);
		iText(SCREEN_width / 2 - 180, SCREEN_height / 2 + 150, (char*)"VICTORY!", GLUT_BITMAP_TIMES_ROMAN_24);
		iSetColor(100, 255, 100);
		iText(SCREEN_width / 2 - 300, SCREEN_height / 2 + 100, (char*)"You have destroyed all enemies, Congrats!", GLUT_BITMAP_HELVETICA_18);
		iSetColor(255, 255, 255);
		iText(SCREEN_width / 2 - 280, SCREEN_height / 2 + 50, (char*)"Would you like to play in any other difficulty?", GLUT_BITMAP_HELVETICA_18);
		iText(SCREEN_width / 2 - 200, SCREEN_height / 2, (char*)"SELECT NEW DIFFICULTY:", GLUT_BITMAP_HELVETICA_18);

		iSetColor(hoverVictoryEasy ? 255 : 200, hoverVictoryEasy ? 255 : 200, hoverVictoryEasy ? 0 : 200);
		if (currentDifficulty == EASY) {
			iSetColor(100, 100, 100);
		}
		iText(SCREEN_width / 2 - 150, SCREEN_height / 2 - 50, (char*)"EASY MODE", GLUT_BITMAP_HELVETICA_18);

		iSetColor(hoverVictoryMedium ? 255 : 200, hoverVictoryMedium ? 255 : 200, hoverVictoryMedium ? 0 : 200);
		if (currentDifficulty == MEDIUM) {
			iSetColor(100, 100, 100);
		}
		iText(SCREEN_width / 2 - 150, SCREEN_height / 2 - 90, (char*)"MEDIUM MODE", GLUT_BITMAP_HELVETICA_18);

		iSetColor(hoverVictoryHard ? 255 : 200, hoverVictoryHard ? 255 : 200, hoverVictoryHard ? 0 : 200);
		if (currentDifficulty == HARD) {
			iSetColor(100, 100, 100);
		}
		iText(SCREEN_width / 2 - 150, SCREEN_height / 2 - 130, (char*)"HARD MODE", GLUT_BITMAP_HELVETICA_18);

		iSetColor(hoverVictoryMenu ? 255 : 200, hoverVictoryMenu ? 0 : 200, hoverVictoryMenu ? 0 : 200);
		iText(SCREEN_width / 2 - 150, SCREEN_height / 2 - 200, (char*)"RETURN TO MAIN MENU", GLUT_BITMAP_HELVETICA_18);

		iSetColor(200, 200, 200);
		char diffText[50];
#if defined(_MSC_VER)
		sprintf_s(diffText, "Completed on: %s",
			currentDifficulty == EASY ? "EASY" :
			currentDifficulty == MEDIUM ? "MEDIUM" : "HARD");
#else
		sprintf(diffText, "Completed on: %s",
			currentDifficulty == EASY ? "EASY" :
			currentDifficulty == MEDIUM ? "MEDIUM" : "HARD");
#endif
		iText(SCREEN_width / 2 - 120, SCREEN_height / 2 - 250, diffText, GLUT_BITMAP_HELVETICA_12);
		return;
	}

	// Game State (gameState == 1)
	iShowImage(0, 0, SCREEN_width, SCREEN_height, gamemap1Image);

	gameTimer += 16;
	updateEnemies();
	updateEnemyProjectiles();
	checkProjectileEnemyCollision();
	checkEnemyProjectileHeroCollision();

	if (currentImage < 0){
		updateAngleFromMouse(gMouseX, gMouseY);
	}

	iShowImage(playerX, playerY, TANK_W, TANK_H, currentImage);
	drawEnemies();

	if (projectileVisible){
		projectileDir = dirFromVel(projVX, projVY);
		int img = fireFront;
		switch (projectileDir){
		case UP:    img = fireFront; break;
		case DOWN:  img = fireBack;  break;
		case RIGHT: img = fireRight; break;
		case LEFT:  img = fireLeft;  break;
		}
		iShowImage((int)projectileXf, (int)projectileYf, PROJECTILE_W, PROJECTILE_H, img);
		projectileXf += projVX;
		projectileYf += projVY;
		if (projectileXf < -PROJECTILE_W || projectileXf > SCREEN_width + PROJECTILE_W ||
			projectileYf < -PROJECTILE_H || projectileYf > SCREEN_height + PROJECTILE_H){
			projectileVisible = false;
		}
	}

	drawEnemyProjectiles();
	drawHUD();
}

// -------- Mouse Functions --------
void iMouseMove(int mx, int my){
	if (gameState == 1){
		updateAngleFromMouse(mx, my);
	}
}

void iPassiveMouseMove(int mx, int my){
	if (gameState == 0){
		hoverStart = (mx >= 792 && mx <= 1260 && my >= 372 && my <= 408);
		hoverHighScore = (mx >= 792 && mx <= 1260 && my >= 324 && my <= 360);
		hoverOption = (mx >= 792 && mx <= 1260 && my >= 276 && my <= 312);
		hoverExit = (mx >= 792 && mx <= 1260 && my >= 228 && my <= 264);
	}
	else if (gameState == 2){
		hoverBack = (mx >= 50 && mx <= 250 && my >= 90 && my <= 120);
	}
	else if (gameState == 3){
		hoverBackOptions = (mx >= 50 && mx <= 250 && my >= 90 && my <= 120);
		hoverEasy = (mx >= SCREEN_width / 2 - 120 && mx <= SCREEN_width / 2 + 120 && my >= 440 && my <= 470);
		hoverMedium = (mx >= SCREEN_width / 2 - 120 && mx <= SCREEN_width / 2 + 120 && my >= 390 && my <= 420);
		hoverHard = (mx >= SCREEN_width / 2 - 120 && mx <= SCREEN_width / 2 + 120 && my >= 340 && my <= 370);
		hoverMusicToggle = (mx >= SCREEN_width / 2 - 100 && mx <= SCREEN_width / 2 - 50 && my >= 270 && my <= 295);
		hoverHowToPlay = (mx >= SCREEN_width / 2 - 140 && mx <= SCREEN_width / 2 + 140 && my >= 210 && my <= 240);
	}
	else if (gameState == 4){
		hoverBackHowTo = (mx >= 50 && mx <= 280 && my >= 40 && my <= 70);
	}
	else if (gameState == 5){
		hoverVictoryEasy = (mx >= SCREEN_width / 2 - 170 && mx <= SCREEN_width / 2 + 170 && my >= SCREEN_height / 2 - 60 && my <= SCREEN_height / 2 - 40);
		hoverVictoryMedium = (mx >= SCREEN_width / 2 - 170 && mx <= SCREEN_width / 2 + 170 && my >= SCREEN_height / 2 - 100 && my <= SCREEN_height / 2 - 80);
		hoverVictoryHard = (mx >= SCREEN_width / 2 - 170 && mx <= SCREEN_width / 2 + 170 && my >= SCREEN_height / 2 - 140 && my <= SCREEN_height / 2 - 120);
		hoverVictoryMenu = (mx >= SCREEN_width / 2 - 170 && mx <= SCREEN_width / 2 + 170 && my >= SCREEN_height / 2 - 210 && my <= SCREEN_height / 2 - 190);
	}
	else {
		updateAngleFromMouse(mx, my);
	}
}

void iMouse(int button, int state, int mx, int my){
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		if (gameState == 0){
			if (hoverStart){
				gameState = 1;
				if (!gameAssetsLoaded){
					gamemap1Image = iLoadImage(IMAGE_DIR "\\2.bmp");
					enemyImage = iLoadImage(IMAGE_DIR "\\Enemy.bmp");
					fireFront = iLoadImage(IMAGE_DIR "\\fireFront.bmp");
					fireLeft = iLoadImage(IMAGE_DIR "\\fireLeft.bmp");
					fireRight = iLoadImage(IMAGE_DIR "\\fireRight.bmp");
					fireBack = iLoadImage(IMAGE_DIR "\\fireBack.bmp");

					enemyFireFront = fireFront;
					enemyFireLeft = fireLeft;
					enemyFireRight = fireRight;
					enemyFireBack = fireBack;

					for (int i = 0; i < 36; ++i) heroRotation[i] = loadAngleFrame(i + 1);
					currentImage = heroRotation[0];
					SetObstacles();
					initEnemies();
					gameAssetsLoaded = true;
				}

				playerX = (SCREEN_width - TANK_W) / 2;
				playerY = 24;
				heroHealth = HERO_MAX_HEALTH;
				projectileVisible = false;
				initEnemies();
				updateAngleFromMouse(mx, my);
			}
			else if (hoverHighScore){
				gameState = 2;
			}
			else if (hoverOption){
				gameState = 3;
			}
			else if (hoverExit){
				exit(0);
			}
		}
		else if (gameState == 2){
			if (hoverBack){
				gameState = 0;
			}
		}
		else if (gameState == 3){
			if (hoverBackOptions){
				gameState = 0;
			}
			else if (hoverEasy){
				currentDifficulty = EASY;
			}
			else if (hoverMedium){
				currentDifficulty = MEDIUM;
			}
			else if (hoverHard){
				currentDifficulty = HARD;
			}
			else if (hoverMusicToggle) {
				musicEnabled = !musicEnabled;
				if (musicEnabled) {
					mciSendString("play bgmusic repeat", NULL, 0, NULL);
				}
				else {
					mciSendString("pause bgmusic", NULL, 0, NULL);
				}
			}
			else if (hoverHowToPlay){
				gameState = 4;
			}
		}
		else if (gameState == 4){
			if (hoverBackHowTo){
				gameState = 3;
			}
		}
		else if (gameState == 5){
			if (hoverVictoryEasy && currentDifficulty != EASY){
				currentDifficulty = EASY;
				gameState = 1;
				playerX = (SCREEN_width - TANK_W) / 2;
				playerY = 24;
				heroHealth = HERO_MAX_HEALTH;
				projectileVisible = false;
				initEnemies();
				updateAngleFromMouse(mx, my);
			}
			else if (hoverVictoryMedium && currentDifficulty != MEDIUM){
				currentDifficulty = MEDIUM;
				gameState = 1;
				playerX = (SCREEN_width - TANK_W) / 2;
				playerY = 24;
				heroHealth = HERO_MAX_HEALTH;
				projectileVisible = false;
				initEnemies();
				updateAngleFromMouse(mx, my);
			}
			else if (hoverVictoryHard && currentDifficulty != HARD){
				currentDifficulty = HARD;
				gameState = 1;
				playerX = (SCREEN_width - TANK_W) / 2;
				playerY = 24;
				heroHealth = HERO_MAX_HEALTH;
				projectileVisible = false;
				initEnemies();
				updateAngleFromMouse(mx, my);
			}
			else if (hoverVictoryMenu){
				gameState = 0;
			}
		}
		else if (gameState == 1){
			int currentTime = getCurrentTime();
			int heroCooldown = HERO_FIRE_COOLDOWN_MEDIUM;
			switch (currentDifficulty) {
			case EASY: heroCooldown = HERO_FIRE_COOLDOWN_EASY; break;
			case MEDIUM: heroCooldown = HERO_FIRE_COOLDOWN_MEDIUM; break;
			case HARD: heroCooldown = HERO_FIRE_COOLDOWN_HARD; break;
			}

			if (!projectileVisible && (currentTime - lastHeroFireTime >= heroCooldown)){
				const int cx = playerX + (TANK_W / 2);
				const int cy = playerY + (TANK_H / 2);
				double dx = (double)gMouseX - cx;
				double dy = (double)gMouseY - cy;
				double len = sqrt(dx*dx + dy*dy);
				if (len < 1.0) { dx = 1.0; dy = 0.0; len = 1.0; }
				double ux = dx / len;
				double uy = dy / len;

				projectileXf = cx + ux * PROJECTILE_SPAWN_OFFSET - PROJECTILE_W * 0.5;
				projectileYf = cy + uy * PROJECTILE_SPAWN_OFFSET - PROJECTILE_H * 0.5;
				projVX = ux * PROJECTILE_SPEED;
				projVY = uy * PROJECTILE_SPEED;
				projectileDir = dirFromVel(projVX, projVY);
				projectileVisible = true;
				lastHeroFireTime = currentTime;
				heroFiredThisFrame = true;
			}
		}
	}
}

// -------- Continuous Input Handling --------
void fixedUpdate()
{
	if (isKeyPressed(27)) { // ESC key
		if (gameState == 2) gameState = 0;
		else if (gameState == 3) gameState = 0;
		else if (gameState == 4) gameState = 3;
		else if (gameState == 5) gameState = 0;
		else if (gameState == 1) gameState = 0;
	}

	if (gameState != 1) {
		return;
	}

	int newX = playerX;
	int newY = playerY;

	if (isKeyPressed('w') || isKeyPressed('W')) {
		newY += SPEED;
		if (newY <= SCREEN_height - TANK_H && !isColliding(playerX, newY)) {
			playerY = newY;
			forceFacingFrame(1);
		}
	}
	else if (isKeyPressed('s') || isKeyPressed('S')) {
		newY -= SPEED;
		if (newY >= 0 && !isColliding(playerX, newY)) {
			playerY = newY;
			forceFacingFrame(19);
		}
	}
	else if (isKeyPressed('a') || isKeyPressed('A')) {
		newX -= SPEED;
		if (newX >= 0 && !isColliding(newX, playerY)) {
			playerX = newX;
			forceFacingFrame(28);
		}
	}
	else if (isKeyPressed('d') || isKeyPressed('D')) {
		newX += SPEED;
		if (newX <= SCREEN_width - TANK_W && !isColliding(newX, playerY)) {
			playerX = newX;
			forceFacingFrame(10);
		}
	}

	if (heroFiredThisFrame) {
		playFireSound();
		heroFiredThisFrame = false;
	}
}

// -------- Main Entry Point --------
int main(){
	srand(time(NULL)); // Initialize random seed

	// Initialize audio system
	initializeAudio();

	// Start background music if enabled
	if (musicEnabled) {
		playBackgroundMusic();
	}

	// Initialize iGraphics with window dimensions and title
	iInitialize(SCREEN_width, SCREEN_height, "Tank Clash - Enhanced Combat System");

	// Load the main menu background image
	menuImage = iLoadImage(IMAGE_DIR "\\1.bmp");

	// Start the main game loop
	iStart();

	// Optional cleanup when program exits
	// Note: This may not be reached depending on how iStart() is implemented
	stopAllAudio();

	return 0;
}