#ifndef BALL_H
#define BALL_H

#include <string>
#include <graphics.h>
#include <vector>
using namespace std;

//定义窗口的宽度和高度
const int Win_W = 1152;
const int Win_H = 648;
//定义地图的宽度和高度
const int Map_W = Win_W * 4;
const int Map_H = Win_H * 4;
//定义食物小球最大数量
const int FD_MAX_NUM = 800;
//定义敌人小球数量
const int ENEMY_NUM = 15;

// AI行为状态枚举
enum AIState {
    AI_IDLE,        // 空闲状态
    AI_CHASE_FOOD,  // 追逐食物
    AI_CHASE_PLAYER,// 追逐玩家
    AI_RUN_AWAY,    // 逃跑
    AI_RANDOM_MOVE  // 随机移动
};

//小球类
class Ball
{
public:
    double x, y; //小球的坐标，决定小球在地图上的位置
    double r;    //小球的半径
    bool flag;   //决定小球是否存活
    COLORREF color; //小球的颜色
    wstring name;    //小球的名字

    // 以下为AI敌人特有属性
    AIState currentState;  // 当前AI状态
    float decisionTimer;   // 决策计时器
    float targetX, targetY; // 目标位置
    float randomMoveTimer; // 随机移动计时器
    float idleTimer;       // 空闲计时器

    // 以下为无敌状态相关属性
    bool isInvincible;     // 是否无敌状态
    int invincibleTimer;   // 无敌计时器（帧数）

    // 构造函数
    Ball() : x(0), y(0), r(10), flag(true), color(RGB(255, 100, 100)),
        currentState(AI_IDLE), decisionTimer(0), targetX(0), targetY(0),
        randomMoveTimer(0), idleTimer(0),
        isInvincible(false), invincibleTimer(0) {
    }
};

// 声明全局小球数组
extern Ball FD_Ball[FD_MAX_NUM];  // 改名为FD_MAX_NUM
extern Ball PL_Ball;
extern Ball ENEMY_Ball[ENEMY_NUM];  // 敌人数组

// 添加食物相关变量声明
extern int fd_current_count;        // 当前活跃食物数量
extern int fd_spawn_timer;          // 食物生成计时器
extern const int FD_SPAWN_INTERVAL; // 食物生成间隔（帧数）
extern const int FD_SPAWN_COUNT;    // 每次生成的食物数量
extern const int FD_MIN_COUNT;      // 最小食物数量

#endif // BALL_H