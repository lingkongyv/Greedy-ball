#ifndef GAME_H
#define GAME_H
#include "Ball.h"

// 添加无敌相关常量定义
const int INVINCIBLE_DURATION = 300;  // 无敌持续时间：5秒 * 60帧/秒 = 300帧

//全局变量声明
extern float playerSpeed;
extern float cameraX, cameraY;
extern float cameraSmoothSpeed;
extern int score;
extern bool gameRunning;
extern bool isPaused;
extern int gameTime;
extern int lastShrinkTime;
extern float initialRadius;

// 敌人AI相关变量
extern float enemyBaseSpeed;  // 敌人基础速度
extern float enemyVisionRange; // 敌人视野范围

// 食物再生相关变量
extern int fd_current_count;        // 当前活跃食物数量
extern int fd_spawn_timer;          // 食物生成计时器

//函数声明
void CenterWindow(HWND hWnd, int width, int height);
void Game_Draw();
void FD_Init();
void Player_Init();
void Enemy_Init();  // 敌人初始化
void RespawnEnemy(int index);  // 重置单个敌人
void Game_Init();
void PL_Move();
void Camera_Follow();
void CheckCollisions();
void Enemy_Move();  // 敌人移动AI
void Enemy_AI_Update(Ball& enemy);  // 敌人AI逻辑更新
void UpdateEnemyCollisions();  // 敌人碰撞检测
void UpdateShrink();
void SpawnFood();  // 生成新食物
void UpdateFood(); // 更新食物系统
void RespawnFood(int index); // 重置单个食物
void UpdateInvincible();  // 更新无敌状态

#endif // GAME_H