#ifndef GAMELOGIC_HPP
#define GAMELOGIC_HPP

#include "Globals.hpp"

// -------- Angle and Movement Functions --------
double calculateAngleRaw(int mx, int my, int cx, int cy);
void updateAngleFromMouse(int mx, int my);
int loadAngleFrame(int idx);
void forceFacingFrame(int angleIndex1Based);
enum Direction dirFromVel(double vx, double vy);

// -------- Obstacle Functions --------
void addObstacle(int x, int y, int w, int h);
void SetObstacles();
bool isColliding(int x, int y);
bool isCollidingWithPlayer(int enemyX, int enemyY);

// -------- Enemy Functions --------
void initEnemies();
void updateEnemies();
void drawEnemies();
bool checkAllEnemiesDestroyed();

// -------- Projectile Functions --------
void fireEnemyProjectile(int enemyIndex);
void initEnemyProjectiles();
void updateEnemyProjectiles();
void drawEnemyProjectiles();
void checkProjectileEnemyCollision();
void checkEnemyProjectileHeroCollision();

// -------- Game State Functions --------
void drawHUD();
void fixedUpdate();

#endif // GAMELOGIC_HPP