#include "Ball.h"

// 定义全局小球数组
Ball FD_Ball[FD_MAX_NUM];   // 改为FD_MAX_NUM
Ball PL_Ball;               // 创建玩家小球
Ball ENEMY_Ball[ENEMY_NUM]; // 创建敌人小球数组

// 定义食物相关变量
int fd_current_count = 0;        // 当前活跃食物数量
int fd_spawn_timer = 0;          // 食物生成计时器