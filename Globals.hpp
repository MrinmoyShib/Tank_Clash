#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "Structures.hpp"
#include <ctime>
#include <cstdlib>

// -------- Global Variables --------
extern int gameState; // 0=menu, 1=game, 2=about, 3=options, 4=howtoplay, 5=victory

// -------- Image handles --------
extern int menuImage, gamemap1Image, aboutImage, enemyImage;
extern int heroRotation[36];
extern int currentImage;
extern int fireFront, fireLeft, fireRight, fireBack;
extern int enemyFireFront, enemyFireLeft, enemyFireRight, enemyFireBack;

// -------- UI State Variables --------
extern bool hoverStart, hoverHighScore, hoverOption, hoverExit, gameAssetsLoaded;
extern bool hoverBack;
extern bool hoverBackOptions;
extern bool hoverBackHowTo;
extern bool hoverEasy, hoverMedium, hoverHard;
extern bool hoverHowToPlay;
extern bool hoverVictoryEasy, hoverVictoryMedium, hoverVictoryHard;
extern bool hoverVictoryMenu;
extern bool musicEnabled;
extern bool hoverMusicToggle;

// -------- Player Variables --------
extern int playerX, playerY;
extern int heroHealth;
extern int lastHeroFireTime;
extern int gameStartTime;

// -------- Game Variables --------
extern enum Difficulty currentDifficulty;
extern bool projectileVisible;
extern double projectileXf, projectileYf;
extern double projVX, projVY;
extern Direction projectileDir;

// -------- Enemy Projectiles --------
extern struct EnemyProjectile enemyProjectiles[MAX_ENEMY_PROJECTILES];

// -------- Obstacles --------
extern struct Box obstacles[25];
extern int obstacleCount;

// -------- Enemies --------
extern struct EnemyTank enemies[3];
extern int enemyCount;
extern int gameTimer;

// -------- Mouse/Angle --------
extern int gMouseX, gMouseY;
extern double gAngleDeg;

// -------- Victory and Audio Tracking --------
extern bool allEnemiesDestroyed;
extern bool enemyDestroyedThisFrame;
extern bool heroFiredThisFrame;

// -------- Helper Functions --------
int getCurrentTime();

#endif // GLOBALS_HPP