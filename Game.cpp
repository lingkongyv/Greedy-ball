#include "Game.h"
#include "Graphics.h"
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <float.h>

// 全局变量定义
float playerSpeed = 10.0f;      //小球移动速度
float cameraX = 0, cameraY = 0; //当前镜头实际位置
float cameraSmoothSpeed = 0.1f; //镜头跟随平滑系数 (0~1, 越小越平滑)
int score = 0;                  //得分
bool gameRunning = true;        //游戏运行状态
bool isPaused = false;          //游戏暂停状态
int gameTime = 0;               //游戏时间（帧数）
int lastShrinkTime = 0;         //上次缩小的时间（帧数）
float initialRadius = 15.0f;    //初始半径  

// 敌人AI相关变量
float enemyBaseSpeed = 5.0f;           // 敌人基础速度
float enemyVisionRange = 300.0f;       // 敌人视野范围
float enemyDecisionInterval = 2.0f;    // 敌人决策间隔（秒）
float enemyRandomMoveInterval = 3.0f;  // 敌人随机移动间隔（秒）

// 食物再生相关变量定义
const int FD_SPAWN_INTERVAL = 300; // 每5秒生成一次（300帧）
const int FD_SPAWN_COUNT = 150;    // 每次生成150个食物
const int FD_MIN_COUNT = 800;    // 最小食物数量（低于此值时加速生成）

//窗口居中函数
void CenterWindow(HWND hWnd, int width, int height)
{
    //获取整个屏幕的宽度和高度
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    //计算窗口左上角应该处于的位置坐标
    int windowPosX = (screenWidth - width) / 2;
    int windowPosY = (screenHeight - height) / 2;

    //SWP_NOSIZE 参数表示保持窗口当前大小不变
    SetWindowPos(hWnd, NULL, windowPosX, windowPosY, 0, 0, SWP_NOSIZE);
}

//游戏绘制
void Game_Draw()
{
    M_Draw();      //清空并绘制地图背景
    FD_Draw();     //绘制食物小球（到地图上）
    PL_Draw();     //绘制玩家小球（到地图上）
    Enemy_Draw();  //绘制敌人小球（到地图上）
    W_Draw();      //将地图的相应部分显示到窗口
    DrawGameInfo(); //绘制游戏信息（在窗口上）
}

// 新增：重置单个食物
void RespawnFood(int index)
{
    // 随机生成小球的坐标、半径和颜色
    FD_Ball[index].x = rand() % (Map_W - 40) + 20;      //坐标范围[20, Map_W-20)
    FD_Ball[index].y = rand() % (Map_H - 40) + 20;      //坐标范围[20, Map_H-20)
    FD_Ball[index].r = rand() % 8 + 3;                  //半径范围[3, 10)
    FD_Ball[index].flag = true;                         //小球存活
    fd_current_count++;

    //生成更鲜艳的颜色
    int colorType = rand() % 4;
    switch (colorType)
    {
    case 0: FD_Ball[index].color = RGB(rand() % 100 + 155, 50, 50); break;    //红色系
    case 1: FD_Ball[index].color = RGB(50, rand() % 100 + 155, 50); break;    //绿色系
    case 2: FD_Ball[index].color = RGB(50, 50, rand() % 100 + 155); break;    //蓝色系
    case 3: FD_Ball[index].color = RGB(rand() % 100 + 155, rand() % 100 + 155, 50); break; //黄色系
    }
}

//食物小球初始化
void FD_Init()
{
    fd_current_count = 0;
    fd_spawn_timer = 0;

    // 先初始化所有食物为未激活状态
    for (int i = 0; i < FD_MAX_NUM; i++)
    {
        FD_Ball[i].flag = false;
    }

    // 初始生成一定数量的食物
    for (int i = 0; i < FD_MIN_COUNT; i++)
    {
        RespawnFood(i);
    }
}

//玩家小球初始化
void Player_Init()
{
    PL_Ball.x = Map_W / 2;    //初始位置在地图中心
    PL_Ball.y = Map_H / 2;
    PL_Ball.r = initialRadius;                    //初始半径
    PL_Ball.color = RGB(255, 100, 100);  //浅红色
    PL_Ball.flag = true;                 //玩家小球初始状态为存活
    PL_Ball.name = L"我是主角";           //玩家小球名字

    // 新增：设置无敌状态
    PL_Ball.isInvincible = true;         // 初始无敌状态
    PL_Ball.invincibleTimer = INVINCIBLE_DURATION;  // 无敌持续时间
}

//敌人小球初始化
void Enemy_Init()
{
    // 敌人名字列表
    const wchar_t* enemyNames[] = {
        L"追逐者", L"捕食者", L"潜伏者", L"猎手", L"追踪者",
        L"幽灵", L"掠食者", L"暗影", L"疾风", L"猛兽",
        L"毒刺", L"利刃", L"铁壁", L"狂战士", L"终结者"
    };

    for (int i = 0; i < ENEMY_NUM; i++)
    {
        //随机生成位置（避免离玩家太近）
        float spawnX, spawnY;
        bool validSpawn = false;

        while (!validSpawn)
        {
            spawnX = rand() % (Map_W - 100) + 50;
            spawnY = rand() % (Map_H - 100) + 50;

            // 检查是否离玩家太近（至少200像素）
            float distToPlayer = sqrt(pow(spawnX - PL_Ball.x, 2) + pow(spawnY - PL_Ball.y, 2));
            if (distToPlayer > 200.0f)
            {
                validSpawn = true;
            }
        }

        ENEMY_Ball[i].x = spawnX;
        ENEMY_Ball[i].y = spawnY;
        ENEMY_Ball[i].r = rand() % 15 + 10;  // 敌人半径范围[10, 25)
        ENEMY_Ball[i].flag = true;
        ENEMY_Ball[i].name = enemyNames[i % 15];  // 分配名字

        // 随机分配颜色（偏暗色系，看起来更具威胁性）
        int colorType = rand() % 5;
        switch (colorType)
        {
        case 0: ENEMY_Ball[i].color = RGB(150, 50, 50); break;      // 暗红
        case 1: ENEMY_Ball[i].color = RGB(50, 150, 50); break;      // 暗绿
        case 2: ENEMY_Ball[i].color = RGB(50, 50, 150); break;      // 暗蓝
        case 3: ENEMY_Ball[i].color = RGB(150, 150, 50); break;     // 暗黄
        case 4: ENEMY_Ball[i].color = RGB(100, 50, 150); break;     // 紫色
        }

        // 初始化AI状态
        ENEMY_Ball[i].currentState = AI_IDLE;
        ENEMY_Ball[i].decisionTimer = rand() % 60;  // 随机初始决策计时器
        ENEMY_Ball[i].randomMoveTimer = 0;
        ENEMY_Ball[i].idleTimer = 0;
    }
}

//游戏初始化
void Game_Init()
{
    srand((unsigned)time(NULL));    //设置随机数种子
    FD_Init();                      //初始化食物小球
    Player_Init();                  //初始化玩家小球
    Enemy_Init();                   //初始化敌人小球
    score = 0;                      //初始化分数
    gameTime = 0;                   //初始化游戏时间
    isPaused = false;               //初始非暂停状态
    lastShrinkTime = 0;             //上次缩小时间设为0
}

//控制玩家小球移动（基于时间）
void PL_Move()
{
    //计算归一化移动向量
    float moveX = 0, moveY = 0;

    //检测按键（支持方向键和WASD）
    if ((GetAsyncKeyState(VK_UP) & 0x8000) || (GetAsyncKeyState('W') & 0x8000))
        moveY -= 1;
    if ((GetAsyncKeyState(VK_DOWN) & 0x8000) || (GetAsyncKeyState('S') & 0x8000))
        moveY += 1;
    if ((GetAsyncKeyState(VK_LEFT) & 0x8000) || (GetAsyncKeyState('A') & 0x8000))
        moveX -= 1;
    if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) || (GetAsyncKeyState('D') & 0x8000))
        moveX += 1;

    //如果没有按键，不移动
    if (moveX == 0 && moveY == 0)
        return;

    //对角线移动时保持相同速度
    if (moveX != 0 && moveY != 0)
    {
        float length = sqrt(moveX * moveX + moveY * moveY);
        moveX /= length;
        moveY /= length;
    }

    //应用移动（速度随着体积增大而减小）
    float currentSpeed = playerSpeed / sqrt(PL_Ball.r / 100.0f);
    PL_Ball.x += moveX * currentSpeed;
    PL_Ball.y += moveY * currentSpeed;

    //边界检查
    //上边界
    if (PL_Ball.y - PL_Ball.r < 0)
        PL_Ball.y = PL_Ball.r;
    //下边界
    if (PL_Ball.y + PL_Ball.r > Map_H)
        PL_Ball.y = Map_H - PL_Ball.r;
    //左边界
    if (PL_Ball.x - PL_Ball.r < 0)
        PL_Ball.x = PL_Ball.r;
    //右边界
    if (PL_Ball.x + PL_Ball.r > Map_W)
        PL_Ball.x = Map_W - PL_Ball.r;
}

// 更新无敌状态
void UpdateInvincible()
{
    if (PL_Ball.isInvincible && PL_Ball.flag)
    {
        PL_Ball.invincibleTimer--;

        // 无敌时间结束
        if (PL_Ball.invincibleTimer <= 0)
        {
            PL_Ball.isInvincible = false;
        }
    }
}

//敌人AI逻辑更新
void Enemy_AI_Update(Ball& enemy)
{
    // 更新决策计时器
    enemy.decisionTimer -= 1.0f / 60.0f;

    // 如果决策时间到了，重新做决策
    if (enemy.decisionTimer <= 0)
    {
        enemy.decisionTimer = enemyDecisionInterval + (rand() % 100) / 100.0f;

        // 1. 检查视野范围内的物体
        vector<pair<float, int>> visibleFoods;  // 距离, 食物索引
        vector<pair<float, int>> visibleEnemies; // 距离, 敌人索引
        bool playerVisible = false;
        float distToPlayer = 0;

        // 检查食物
        for (int i = 0; i < FD_MAX_NUM; i++)
        {
            if (!FD_Ball[i].flag) continue;

            float dx = FD_Ball[i].x - enemy.x;
            float dy = FD_Ball[i].y - enemy.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < enemyVisionRange && enemy.r > FD_Ball[i].r * 1.2f)
            {
                visibleFoods.push_back({ distance, i });
            }
        }

        // 检查玩家
        float dx = PL_Ball.x - enemy.x;
        float dy = PL_Ball.y - enemy.y;
        distToPlayer = sqrt(dx * dx + dy * dy);

        if (distToPlayer < enemyVisionRange * 1.5f)
        {
            playerVisible = true;
        }

        // 检查其他敌人（避免碰撞）
        for (int i = 0; i < ENEMY_NUM; i++)
        {
            if (&ENEMY_Ball[i] == &enemy || !ENEMY_Ball[i].flag) continue;

            float dx2 = ENEMY_Ball[i].x - enemy.x;
            float dy2 = ENEMY_Ball[i].y - enemy.y;
            float distance = sqrt(dx2 * dx2 + dy2 * dy2);

            if (distance < enemyVisionRange * 0.7f)
            {
                visibleEnemies.push_back({ distance, i });
            }
        }

        // 2. 决策逻辑（优先级：逃跑 > 追逐玩家 > 追逐食物 > 随机移动）

        // 检查是否有需要逃跑的对象（比自己大的敌人或玩家）
        bool needToRun = false;
        float closestThreatDist = FLT_MAX;
        int closestThreatIndex = -1;
        bool threatIsPlayer = false;

        // 检查玩家是否是威胁
        if (playerVisible && PL_Ball.r > enemy.r * 1.2f && distToPlayer < enemyVisionRange)
        {
            needToRun = true;
            closestThreatDist = distToPlayer;
            threatIsPlayer = true;
        }

        // 检查是否有比自己大的敌人
        for (auto& e : visibleEnemies)
        {
            if (ENEMY_Ball[e.second].r > enemy.r * 1.2f)
            {
                if (e.first < closestThreatDist)
                {
                    needToRun = true;
                    closestThreatDist = e.first;
                    closestThreatIndex = e.second;
                    threatIsPlayer = false;
                }
            }
        }

        if (needToRun)
        {
            enemy.currentState = AI_RUN_AWAY;
            // 设置逃跑目标（远离威胁的方向）
            float runDistance = 250.0f;

            if (threatIsPlayer)
            {
                float angle = atan2(enemy.y - PL_Ball.y, enemy.x - PL_Ball.x);
                enemy.targetX = enemy.x + cos(angle) * runDistance;
                enemy.targetY = enemy.y + sin(angle) * runDistance;
            }
            else
            {
                float angle = atan2(enemy.y - ENEMY_Ball[closestThreatIndex].y,
                    enemy.x - ENEMY_Ball[closestThreatIndex].x);
                enemy.targetX = enemy.x + cos(angle) * runDistance;
                enemy.targetY = enemy.y + sin(angle) * runDistance;
            }

            // 确保目标在地图范围内
            enemy.targetX = max(enemy.r + 20.0f, min((float)Map_W - enemy.r - 20.0f, enemy.targetX));
            enemy.targetY = max(enemy.r + 20.0f, min((float)Map_H - enemy.r - 20.0f, enemy.targetY));
        }
        // 追逐玩家（如果玩家比自己小）
        else if (playerVisible && PL_Ball.r < enemy.r * 0.9f)
        {
            enemy.currentState = AI_CHASE_PLAYER;
            enemy.targetX = PL_Ball.x;
            enemy.targetY = PL_Ball.y;
        }
        // 追逐食物
        else if (!visibleFoods.empty())
        {
            enemy.currentState = AI_CHASE_FOOD;
            // 选择最近的食物
            sort(visibleFoods.begin(), visibleFoods.end());
            int closestFoodIdx = visibleFoods[0].second;
            enemy.targetX = FD_Ball[closestFoodIdx].x;
            enemy.targetY = FD_Ball[closestFoodIdx].y;
        }
        // 随机移动（这是敌人大部分时间的状态）
        else
        {
            enemy.currentState = AI_RANDOM_MOVE;
            // 随机目标位置（当前位置周围一定范围内）
            float angle = (rand() % 360) * 3.14159f / 180.0f;
            float distance = 150.0f + rand() % 300;  // 增加距离范围
            enemy.targetX = enemy.x + cos(angle) * distance;
            enemy.targetY = enemy.y + sin(angle) * distance;

            // 确保目标在地图范围内
            enemy.targetX = max(enemy.r + 20.0f, min((float)Map_W - enemy.r - 20.0f, enemy.targetX));
            enemy.targetY = max(enemy.r + 20.0f, min((float)Map_H - enemy.r - 20.0f, enemy.targetY));
        }
    }

    // 3. 持续状态检查（让敌人更智能地切换状态，而不是等待决策计时器）

    // 如果正在追逐玩家，持续更新目标位置
    if (enemy.currentState == AI_CHASE_PLAYER)
    {
        // 检查玩家是否还在视野范围内且比敌人小
        float dx = PL_Ball.x - enemy.x;
        float dy = PL_Ball.y - enemy.y;
        float distToPlayer = sqrt(dx * dx + dy * dy);

        // 如果玩家不在视野范围内或者玩家变得比敌人大，切换到随机移动
        if (distToPlayer > enemyVisionRange * 1.5f || PL_Ball.r > enemy.r * 0.95f)
        {
            enemy.currentState = AI_RANDOM_MOVE;
            enemy.decisionTimer = 0;  // 立即重新决策
        }
        else
        {
            // 更新目标为玩家当前位置
            enemy.targetX = PL_Ball.x;
            enemy.targetY = PL_Ball.y;
        }
    }
    // 如果正在追逐食物，检查是否已经吃到或食物消失
    else if (enemy.currentState == AI_CHASE_FOOD)
    {
        // 计算到目标的距离
        float dx = enemy.targetX - enemy.x;
        float dy = enemy.targetY - enemy.y;
        float distance = sqrt(dx * dx + dy * dy);

        // 如果非常接近目标（可能已经吃到），切换到随机移动
        if (distance < 10.0f)
        {
            enemy.currentState = AI_RANDOM_MOVE;
            enemy.decisionTimer = 0;  // 立即重新决策
        }
    }
}

//敌人移动
void Enemy_Move()
{
    for (int i = 0; i < ENEMY_NUM; i++)
    {
        if (!ENEMY_Ball[i].flag) continue;

        // 更新AI状态
        Enemy_AI_Update(ENEMY_Ball[i]);

        float moveX = 0, moveY = 0;
        float currentSpeed = enemyBaseSpeed / sqrt(ENEMY_Ball[i].r / 100.0f);
        float length = 0;

        // 根据状态计算移动方向
        switch (ENEMY_Ball[i].currentState)
        {
        case AI_CHASE_FOOD:
        case AI_CHASE_PLAYER:
        case AI_RUN_AWAY:
        case AI_RANDOM_MOVE:
            // 计算朝向目标的向量
            moveX = ENEMY_Ball[i].targetX - ENEMY_Ball[i].x;
            moveY = ENEMY_Ball[i].targetY - ENEMY_Ball[i].y;

            // 归一化
            length = sqrt(moveX * moveX + moveY * moveY);
            if (length > 0.1f)  // 降低最小移动阈值
            {
                moveX /= length;
                moveY /= length;

                // 如果是逃跑状态，方向相反
                if (ENEMY_Ball[i].currentState == AI_RUN_AWAY)
                {
                    moveX = -moveX;
                    moveY = -moveY;
                }
            }
            else
            {
                // 接近目标时，如果目标很近，立即重新决策
                if (length < 10.0f)
                {
                    ENEMY_Ball[i].decisionTimer = 0;  // 立即重新决策
                }
                // 但仍然轻微移动，避免完全停止
                moveX = (rand() % 100 - 50) / 100.0f;
                moveY = (rand() % 100 - 50) / 100.0f;

                // 归一化微小的随机移动
                float randomLength = sqrt(moveX * moveX + moveY * moveY);
                if (randomLength > 0.1f)
                {
                    moveX /= randomLength * 2.0f;  // 减半速度
                    moveY /= randomLength * 2.0f;
                }
            }
            break;

        case AI_IDLE:
            // 空闲状态很少出现，但如果出现，也给予轻微移动
            moveX = (rand() % 100 - 50) / 200.0f;  // 更慢的速度
            moveY = (rand() % 100 - 50) / 200.0f;

            // 减少空闲时间
            ENEMY_Ball[i].idleTimer -= 1.0f / 60.0f;
            if (ENEMY_Ball[i].idleTimer <= 0)
            {
                ENEMY_Ball[i].decisionTimer = 0;  // 立即重新决策
            }
            break;
        }

        // 应用移动（敌人应该始终有移动，除非被暂停）
        ENEMY_Ball[i].x += moveX * currentSpeed;
        ENEMY_Ball[i].y += moveY * currentSpeed;

        // 边界检查
        if (ENEMY_Ball[i].y - ENEMY_Ball[i].r < 10)
            ENEMY_Ball[i].y = ENEMY_Ball[i].r + 10;
        if (ENEMY_Ball[i].y + ENEMY_Ball[i].r > Map_H - 10)
            ENEMY_Ball[i].y = Map_H - ENEMY_Ball[i].r - 10;
        if (ENEMY_Ball[i].x - ENEMY_Ball[i].r < 10)
            ENEMY_Ball[i].x = ENEMY_Ball[i].r + 10;
        if (ENEMY_Ball[i].x + ENEMY_Ball[i].r > Map_W - 10)
            ENEMY_Ball[i].x = Map_W - ENEMY_Ball[i].r - 10;
    }
}

// 新增：生成新食物
void SpawnFood()
{
    if (fd_current_count >= FD_MAX_NUM) return; // 已达到最大数量

    // 计算需要生成的数量
    int needSpawn = FD_SPAWN_COUNT;
    if (fd_current_count < FD_MIN_COUNT)
    {
        // 如果食物太少，加速生成
        needSpawn = min(FD_SPAWN_COUNT * 2, FD_MAX_NUM - fd_current_count);
    }
    else
    {
        needSpawn = min(FD_SPAWN_COUNT, FD_MAX_NUM - fd_current_count);
    }

    // 寻找空闲位置生成食物
    int spawned = 0;
    for (int i = 0; i < FD_MAX_NUM && spawned < needSpawn; i++)
    {
        if (!FD_Ball[i].flag)
        {
            RespawnFood(i);
            spawned++;
        }
    }
}

// 新增：更新食物系统
void UpdateFood()
{
    // 更新生成计时器
    fd_spawn_timer++;

    // 检查是否需要生成新食物
    if (fd_spawn_timer >= FD_SPAWN_INTERVAL)
    {
        SpawnFood();
        fd_spawn_timer = 0;
    }

    // 如果食物数量太少，立即生成一些（紧急情况）
    if (fd_current_count < FD_MIN_COUNT / 2)
    {
        SpawnFood();
    }
}

//更新敌人碰撞检测
void UpdateEnemyCollisions()
{
    // 1. 敌人与食物的碰撞
    for (int i = 0; i < ENEMY_NUM; i++)
    {
        if (!ENEMY_Ball[i].flag) continue;

        for (int j = 0; j < FD_MAX_NUM; j++)
        {
            if (!FD_Ball[j].flag) continue;

            double dx = ENEMY_Ball[i].x - FD_Ball[j].x;
            double dy = ENEMY_Ball[i].y - FD_Ball[j].y;
            double distance = sqrt(dx * dx + dy * dy);

            // 敌人可以吃比自己小的食物
            if (distance < ENEMY_Ball[i].r && ENEMY_Ball[i].r > FD_Ball[j].r)
            {
                FD_Ball[j].flag = false; // 食物消失
                fd_current_count--; // 更新食物数量

                // 敌人体积增加
                double enemyArea = ENEMY_Ball[i].r * ENEMY_Ball[i].r * 3.14159;
                double foodArea = FD_Ball[j].r * FD_Ball[j].r * 3.14159;
                double newArea = enemyArea + foodArea;
                ENEMY_Ball[i].r = sqrt(newArea / 3.14159);
            }
        }
    }

    // 2. 敌人与玩家的碰撞
    if (PL_Ball.flag)
    {
        for (int i = 0; i < ENEMY_NUM; i++)
        {
            if (!ENEMY_Ball[i].flag) continue;

            double dx = PL_Ball.x - ENEMY_Ball[i].x;
            double dy = PL_Ball.y - ENEMY_Ball[i].y;
            double distance = sqrt(dx * dx + dy * dy);

            // 碰撞检测
            if (distance < PL_Ball.r + ENEMY_Ball[i].r)
            {
                // 如果玩家比敌人大，玩家吃掉敌人
                if (PL_Ball.r > ENEMY_Ball[i].r * 1.1f)
                {
                    ENEMY_Ball[i].flag = false; // 敌人被消灭
                    score += (int)ENEMY_Ball[i].r * 5; // 得分更多

                    // 玩家体积增加
                    double playerArea = PL_Ball.r * PL_Ball.r * 3.14159;
                    double enemyArea = ENEMY_Ball[i].r * ENEMY_Ball[i].r * 3.14159;
                    double newArea = playerArea + enemyArea;
                    PL_Ball.r = sqrt(newArea / 3.14159);

                    // 敌人重生 - 修复：只重置这个敌人
                    RespawnEnemy(i);
                }
                // 如果敌人比玩家大，敌人吃掉玩家（检查无敌状态）
                else if (ENEMY_Ball[i].r > PL_Ball.r * 1.1f)
                {
                    // 检查玩家是否处于无敌状态
                    if (!PL_Ball.isInvincible)
                    {
                        PL_Ball.flag = false; // 玩家死亡
                        gameRunning = false;   // 游戏结束
                    }
                    // 如果无敌，玩家不会被吃掉，但可以给一个提示效果
                }
            }
        }
    }

    // 3. 敌人之间的碰撞
    for (int i = 0; i < ENEMY_NUM; i++)
    {
        if (!ENEMY_Ball[i].flag) continue;

        for (int j = i + 1; j < ENEMY_NUM; j++)
        {
            if (!ENEMY_Ball[j].flag) continue;

            double dx = ENEMY_Ball[i].x - ENEMY_Ball[j].x;
            double dy = ENEMY_Ball[i].y - ENEMY_Ball[j].y;
            double distance = sqrt(dx * dx + dy * dy);

            // 碰撞检测
            if (distance < ENEMY_Ball[i].r + ENEMY_Ball[j].r)
            {
                // 大的吃掉小的
                if (ENEMY_Ball[i].r > ENEMY_Ball[j].r * 1.2f)
                {
                    ENEMY_Ball[j].flag = false; // 小的被吃掉

                    // 大的体积增加
                    double area1 = ENEMY_Ball[i].r * ENEMY_Ball[i].r * 3.14159;
                    double area2 = ENEMY_Ball[j].r * ENEMY_Ball[j].r * 3.14159;
                    double newArea = area1 + area2;
                    ENEMY_Ball[i].r = sqrt(newArea / 3.14159);

                    // 被吃掉的敌人重生 - 修复：只重置这个敌人
                    RespawnEnemy(j);
                }
                else if (ENEMY_Ball[j].r > ENEMY_Ball[i].r * 1.2f)
                {
                    ENEMY_Ball[i].flag = false; // 小的被吃掉

                    // 大的体积增加
                    double area1 = ENEMY_Ball[j].r * ENEMY_Ball[j].r * 3.14159;
                    double area2 = ENEMY_Ball[i].r * ENEMY_Ball[i].r * 3.14159;
                    double newArea = area1 + area2;
                    ENEMY_Ball[j].r = sqrt(newArea / 3.14159);

                    // 被吃掉的敌人重生 - 修复：只重置这个敌人
                    RespawnEnemy(i);
                }
            }
        }
    }
}

// 重置单个敌人（用于敌人被吃掉后的重生）
void RespawnEnemy(int index)
{
    // 敌人名字列表
    const wchar_t* enemyNames[] = {
        L"追逐者", L"捕食者", L"潜伏者", L"猎手", L"追踪者",
        L"幽灵", L"掠食者", L"暗影", L"疾风", L"猛兽",
        L"毒刺", L"利刃", L"铁壁", L"狂战士", L"终结者"
    };

    //随机生成位置（避免离玩家太近）
    float spawnX, spawnY;
    bool validSpawn = false;
    int attempts = 0;

    while (!validSpawn && attempts < 100) // 最多尝试100次
    {
        spawnX = rand() % (Map_W - 100) + 50;
        spawnY = rand() % (Map_H - 100) + 50;

        // 检查是否离玩家太近（至少200像素）
        float distToPlayer = sqrt(pow(spawnX - PL_Ball.x, 2) + pow(spawnY - PL_Ball.y, 2));
        if (distToPlayer > 200.0f)
        {
            validSpawn = true;
        }
        attempts++;
    }

    // 如果找不到合适位置，使用默认位置
    if (!validSpawn)
    {
        spawnX = rand() % (Map_W - 100) + 50;
        spawnY = rand() % (Map_H - 100) + 50;
    }

    ENEMY_Ball[index].x = spawnX;
    ENEMY_Ball[index].y = spawnY;
    ENEMY_Ball[index].r = rand() % 15 + 10;  // 敌人半径范围[10, 25)
    ENEMY_Ball[index].flag = true;
    ENEMY_Ball[index].name = enemyNames[index % 15];  // 分配名字

    // 随机分配颜色（偏暗色系，看起来更具威胁性）
    int colorType = rand() % 5;
    switch (colorType)
    {
    case 0: ENEMY_Ball[index].color = RGB(150, 50, 50); break;      // 暗红
    case 1: ENEMY_Ball[index].color = RGB(50, 150, 50); break;      // 暗绿
    case 2: ENEMY_Ball[index].color = RGB(50, 50, 150); break;      // 暗蓝
    case 3: ENEMY_Ball[index].color = RGB(150, 150, 50); break;     // 暗黄
    case 4: ENEMY_Ball[index].color = RGB(100, 50, 150); break;     // 紫色
    }

    // 初始化AI状态
    ENEMY_Ball[index].currentState = AI_IDLE;
    ENEMY_Ball[index].decisionTimer = rand() % 60;  // 随机初始决策计时器
    ENEMY_Ball[index].randomMoveTimer = 0;
    ENEMY_Ball[index].idleTimer = 0;
}

//碰撞检测：检查玩家小球是否吃到食物小球
void CheckCollisions()
{
    for (int i = 0; i < FD_MAX_NUM; i++)
    {
        if (!FD_Ball[i].flag) continue; //跳过已消失的食物

        //计算两球心距离
        double dx = PL_Ball.x - FD_Ball[i].x;
        double dy = PL_Ball.y - FD_Ball[i].y;
        double distance = sqrt(dx * dx + dy * dy);

        //碰撞条件：距离小于玩家半径，并且玩家半径大于食物半径（只能吃比自己小的）
        if (distance < PL_Ball.r && PL_Ball.r > FD_Ball[i].r)
        {
            //玩家"吃掉"食物
            FD_Ball[i].flag = false; //食物消失
            fd_current_count--; // 更新食物数量
            score += (int)FD_Ball[i].r; //得分根据食物大小增加

            //玩家体积增加（基于面积累加）
            double playerArea = PL_Ball.r * PL_Ball.r * 3.14159;
            double foodArea = FD_Ball[i].r * FD_Ball[i].r * 3.14159;
            double newArea = playerArea + foodArea;
            PL_Ball.r = sqrt(newArea / 3.14159);
        }
    }
}

//更新缩小逻辑
void UpdateShrink()
{
    //每5秒（300帧）缩小一次
    const int shrinkInterval = 300; //5秒 * 60帧/秒 = 300帧

    if (gameTime - lastShrinkTime >= shrinkInterval)
    {
        //缩小5%（按面积缩小）
        double currentArea = PL_Ball.r * PL_Ball.r * 3.14159;
        double shrinkFactor = 0.95; //缩小到95%（相当于缩小5%）
        double newArea = currentArea * shrinkFactor;
        PL_Ball.r = sqrt(newArea / 3.14159);

        //确保不会小于最小半径（初始半径的一半）
        float minRadius = initialRadius * 0.5f;
        if (PL_Ball.r < minRadius)
        {
            PL_Ball.r = minRadius;
        }

        //更新上次缩小时间
        lastShrinkTime = gameTime;
    }
}

//将视角定位到以玩家小球为中心的相对坐标
void Camera_Follow()
{
    //计算理论目标位置（让玩家始终在窗口中心）
    int targetCamX = (int)PL_Ball.x - Win_W / 2;
    int targetCamY = (int)PL_Ball.y - Win_H / 2;

    //边界修正
    if (targetCamX < 0) targetCamX = 0;
    if (targetCamY < 0) targetCamY = 0;
    if (targetCamX > Map_W - Win_W) targetCamX = Map_W - Win_W;
    if (targetCamY > Map_H - Win_H) targetCamY = Map_H - Win_H;

    //平滑移动（线性插值）
    cameraX += (targetCamX - cameraX) * cameraSmoothSpeed;
    cameraY += (targetCamY - cameraY) * cameraSmoothSpeed;

    //更新显示
    putimage(0, 0, Win_W, Win_H, &MAP, (int)cameraX, (int)cameraY);
}