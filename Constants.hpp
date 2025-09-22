#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <math.h>

// -------- Paths --------
// IMPORTANT: Make sure these paths are correct for your system
#define IMAGE_DIR "I:\\SD\\Tank Clash\\Tank Clash\\Tank Clash\\Tank Clash\\image\\"
#define AUDIO_DIR "I:\\SD\\Tank Clash\\Tank Clash\\Tank Clash\\Tank Clash\\Audios\\Audios\\"

// -------- Rotation tuning --------
#define ANGLE_OFFSET_DEG 90.0
#define Y_AXIS_SIGN -1

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// -------- Constants --------
#define SCREEN_width  1280
#define SCREEN_height 720
#define TANK_W 85
#define TANK_H 85
#define SPEED  1.8
#define PROJECTILE_W 20
#define PROJECTILE_H 20
#define PROJECTILE_SPEED 0.5
#define PROJECTILE_SPAWN_OFFSET  (TANK_W * 0.55)

// -------- Firing intervals (in milliseconds) --------
#define HERO_FIRE_COOLDOWN_EASY 2000
#define HERO_FIRE_COOLDOWN_MEDIUM 1800
#define HERO_FIRE_COOLDOWN_HARD 1500
#define ENEMY_FIRE_COOLDOWN_EASY 4000
#define ENEMY_FIRE_COOLDOWN_MEDIUM 3000
#define ENEMY_FIRE_COOLDOWN_HARD 2500

// -------- Health constants --------
#define HERO_MAX_HEALTH 3
#define ENEMY_MAX_HEALTH_EASY 1
#define ENEMY_MAX_HEALTH_MEDIUM 1
#define ENEMY_MAX_HEALTH_HARD 2

// -------- Max projectiles --------
#define MAX_ENEMY_PROJECTILES 20

// -------- Types/State --------
enum Direction { UP, RIGHT, DOWN, LEFT };
enum Difficulty { EASY, MEDIUM, HARD };

// -------- Helper Functions --------
inline double rad2deg(double r) { return r * (180.0 / M_PI); }
inline double deg2rad(double d) { return d * (M_PI / 180.0); }

#endif // CONSTANTS_HPP