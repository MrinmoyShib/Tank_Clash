#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include "Constants.hpp"

// -------- Structure Definitions --------

struct Box {
	int x, y, w, h;
};

// -------- Enemy Structure --------
struct EnemyTank {
	int x, y;
	bool active;
	int health;
	int maxHealth;
	int moveTimer;
	int directionX, directionY;
	int lastFireTime;
};

// -------- Enemy Projectile Structure --------
struct EnemyProjectile {
	bool visible;
	double xf, yf;
	double vx, vy;
	Direction dir;
};

#endif // STRUCTURES_HPP