#include "Graphics.h"
#include "Game.h"
#include <string>
#include <sstream>
using namespace std;

// 地图图像定义
IMAGE MAP(Map_W, Map_H);    //创建地图

//游戏窗口绘制
void W_Draw()
{
    //恢复当前绘图目标为窗口
    SetWorkingImage();
    //将地图图像的指定区域绘制到窗口上，实现视角跟随玩家小球
    Camera_Follow();
}

//地图窗口绘制
void M_Draw()
{
    SetWorkingImage(&MAP);      //设置当前绘图目标为MAP图像
    setbkcolor(WHITE);          //设置当前图片背景为白色
    cleardevice();              //清空当前绘图目标并填充背景色

    //绘制网格背景
    setlinecolor(RGB(240, 240, 240));
    for (int x = 0; x < Map_W; x += 50)
    {
        line(x, 0, x, Map_H);
    }
    for (int y = 0; y < Map_H; y += 50)
    {
        line(0, y, Map_W, y);
    }
}

//食物小球绘制
void FD_Draw()
{
    //设置绘图目标为地图
    SetWorkingImage(&MAP);

    //计算当前摄像机可见区域
    int left = (int)cameraX;
    int top = (int)cameraY;
    int right = left + Win_W;
    int bottom = top + Win_H;

    for (int i = 0; i < FD_MAX_NUM; i++)
    {
        if (FD_Ball[i].flag) //如果小球存活则绘制
        {
            //只绘制在可见区域内的食物（加上一些边界余量）
            if (FD_Ball[i].x >= left - FD_Ball[i].r * 2 &&
                FD_Ball[i].x <= right + FD_Ball[i].r * 2 &&
                FD_Ball[i].y >= top - FD_Ball[i].r * 2 &&
                FD_Ball[i].y <= bottom + FD_Ball[i].r * 2)
            {
                setfillcolor(FD_Ball[i].color); //设置填充颜色
                solidcircle((int)FD_Ball[i].x, (int)FD_Ball[i].y, (int)FD_Ball[i].r); //绘制小球
            }
        }
    }
}

//玩家小球绘制
void PL_Draw()
{
    //设置绘图目标为地图
    SetWorkingImage(&MAP);

    if (PL_Ball.flag) //如果玩家小球存活则绘制
    {
        // 如果无敌状态，绘制闪烁效果
        if (PL_Ball.isInvincible)
        {
            // 闪烁效果：每10帧切换一次颜色
            if ((gameTime / 10) % 2 == 0)
            {
                // 绘制无敌光晕效果
                setfillcolor(RGB(255, 255, 100)); // 金黄色光晕
                setlinecolor(RGB(255, 255, 0));
                setlinestyle(PS_SOLID, 3);
                circle((int)PL_Ball.x, (int)PL_Ball.y, (int)PL_Ball.r + 5);
                setlinestyle(PS_SOLID, 1);

                // 绘制玩家小球主体（无敌时颜色变化）
                setfillcolor(RGB(255, 200, 50)); // 更亮的颜色
                solidcircle((int)PL_Ball.x, (int)PL_Ball.y, (int)PL_Ball.r);
            }
            else
            {
                // 正常绘制玩家小球阴影
                setfillcolor(RGB(150, 150, 150));
                solidcircle((int)PL_Ball.x + 3, (int)PL_Ball.y + 3, (int)PL_Ball.r);

                // 绘制玩家小球主体
                setfillcolor(PL_Ball.color);
                solidcircle((int)PL_Ball.x, (int)PL_Ball.y, (int)PL_Ball.r);
            }
        }
        else
        {
            // 正常绘制（非无敌状态）
            //绘制玩家小球阴影
            setfillcolor(RGB(150, 150, 150));
            solidcircle((int)PL_Ball.x + 3, (int)PL_Ball.y + 3, (int)PL_Ball.r);

            //绘制玩家小球主体
            setfillcolor(PL_Ball.color);
            solidcircle((int)PL_Ball.x, (int)PL_Ball.y, (int)PL_Ball.r);
        }

        //绘制名称，居中显示在小球正上方
        setbkmode(TRANSPARENT);                         //设置文字背景为透明
        settextstyle(15, 0, _T("黑体"));                //设置文字样式

        // 根据无敌状态设置文字颜色
        if (PL_Ball.isInvincible)
        {
            settextcolor(RGB(255, 255, 0));  // 无敌时显示黄色文字
        }
        else
        {
            settextcolor(BLACK);              // 正常时显示黑色文字
        }

        //直接使用宽字符串，不需要转换
        const wchar_t* name = PL_Ball.name.c_str();

        //计算文字宽度
        int textWidth = textwidth(name);

        //计算文字起始位置：x坐标 = 小球x坐标 - 文字宽度/2，y坐标 = 小球y坐标 - 小球半径
        int textX = (int)PL_Ball.x - textWidth / 2;
        int textY = (int)PL_Ball.y - (int)PL_Ball.r - 20;  //减去20给文字留出空间

        outtextxy(textX, textY, name);  //居中显示文字
    }
}

//敌人小球绘制
void Enemy_Draw()
{
    //设置绘图目标为地图
    SetWorkingImage(&MAP);

    //计算当前摄像机可见区域
    int left = (int)cameraX;
    int top = (int)cameraY;
    int right = left + Win_W;
    int bottom = top + Win_H;

    for (int i = 0; i < ENEMY_NUM; i++)
    {
        if (ENEMY_Ball[i].flag) //如果敌人存活则绘制
        {
            //只绘制在可见区域内的敌人
            if (ENEMY_Ball[i].x >= left - ENEMY_Ball[i].r * 2 &&
                ENEMY_Ball[i].x <= right + ENEMY_Ball[i].r * 2 &&
                ENEMY_Ball[i].y >= top - ENEMY_Ball[i].r * 2 &&
                ENEMY_Ball[i].y <= bottom + ENEMY_Ball[i].r * 2)
            {
                //绘制敌人小球阴影
                setfillcolor(RGB(100, 100, 100));
                solidcircle((int)ENEMY_Ball[i].x + 2, (int)ENEMY_Ball[i].y + 2, (int)ENEMY_Ball[i].r);

                //绘制敌人小球主体
                setfillcolor(ENEMY_Ball[i].color);
                solidcircle((int)ENEMY_Ball[i].x, (int)ENEMY_Ball[i].y, (int)ENEMY_Ball[i].r);

                //绘制敌人名称
                setbkmode(TRANSPARENT);
                settextstyle(14, 0, _T("黑体"));

                //根据敌人状态设置文字颜色
                switch (ENEMY_Ball[i].currentState)
                {
                case AI_CHASE_PLAYER:
                    settextcolor(RED);  // 追逐玩家时显示红色
                    break;
                case AI_RUN_AWAY:
                    settextcolor(BLUE); // 逃跑时显示蓝色
                    break;
                case AI_CHASE_FOOD:
                    settextcolor(GREEN); // 追逐食物时显示绿色
                    break;
                default:
                    settextcolor(BLACK);
                    break;
                }

                const wchar_t* name = ENEMY_Ball[i].name.c_str();
                int textWidth = textwidth(name);
                int textX = (int)ENEMY_Ball[i].x - textWidth / 2;
                int textY = (int)ENEMY_Ball[i].y - (int)ENEMY_Ball[i].r - 20;

                outtextxy(textX, textY, name);

                // 绘制AI状态指示器（小型状态图标）
                int indicatorSize = 6;
                int indicatorX = (int)ENEMY_Ball[i].x;
                int indicatorY = (int)ENEMY_Ball[i].y - (int)ENEMY_Ball[i].r - 35;

                switch (ENEMY_Ball[i].currentState)
                {
                case AI_CHASE_PLAYER:
                    setfillcolor(RED);
                    solidcircle(indicatorX, indicatorY, indicatorSize);
                    break;
                case AI_CHASE_FOOD:
                    setfillcolor(GREEN);
                    solidcircle(indicatorX, indicatorY, indicatorSize);
                    break;
                case AI_RUN_AWAY:
                    setfillcolor(BLUE);
                    solidcircle(indicatorX, indicatorY, indicatorSize);
                    break;
                case AI_RANDOM_MOVE:
                    setfillcolor(YELLOW);
                    solidcircle(indicatorX, indicatorY, indicatorSize);
                    break;
                case AI_IDLE:
                    setfillcolor(DARKGRAY);
                    solidcircle(indicatorX, indicatorY, indicatorSize);
                    break;
                }
            }
        }
    }
}

//绘制游戏信息（分数、提示等）
void DrawGameInfo()
{
    //恢复绘图目标为窗口
    SetWorkingImage();

    setbkmode(TRANSPARENT);                 //设置文字背景为透明

    //绘制分数
    settextstyle(20, 0, _T("宋体"));
    settextcolor(BLUE);
    TCHAR scoreText[50];
    _stprintf_s(scoreText, _T("得分: %d"), score);
    outtextxy(10, 10, scoreText);

    //绘制玩家大小
    settextcolor(RED);
    TCHAR sizeText[50];
    _stprintf_s(sizeText, _T("大小: %.1f"), PL_Ball.r);
    outtextxy(10, 40, sizeText);

    // 新增：绘制无敌状态
    if (PL_Ball.isInvincible)
    {
        settextcolor(RGB(255, 255, 0)); // 黄色
        TCHAR invincibleText[50];
        int invincibleSeconds = PL_Ball.invincibleTimer / 60 + 1; // 剩余秒数（向上取整）
        _stprintf_s(invincibleText, _T("无敌: %d秒"), invincibleSeconds);
        outtextxy(10, 70, invincibleText);
    }

    //绘制当前速度
    settextcolor(GREEN);
    TCHAR speedText[50];
    float currentSpeed = playerSpeed / sqrt(PL_Ball.r / 100.0f);
    _stprintf_s(speedText, _T("速度: %.1f"), currentSpeed);
    outtextxy(10, PL_Ball.isInvincible ? 100 : 70, speedText); // 根据无敌状态调整位置

    //绘制游戏时间
    settextcolor(DARKGRAY);
    TCHAR timeText[50];
    _stprintf_s(timeText, _T("时间: %d秒"), gameTime / 60);
    outtextxy(10, PL_Ball.isInvincible ? 130 : 100, timeText); // 根据无敌状态调整位置

    // 调整后续所有项目的Y坐标
    int yOffset = PL_Ball.isInvincible ? 30 : 0; // 如果有无敌状态显示，所有项目向下偏移30像素

    //绘制下一个缩小时间
    settextcolor(MAGENTA);
    TCHAR shrinkText[50];
    int nextShrink = 5 - (gameTime - lastShrinkTime) / 60; //距离下次缩小的秒数
    if (nextShrink < 0) nextShrink = 0;
    _stprintf_s(shrinkText, _T("缩小倒计时: %d秒"), nextShrink);
    outtextxy(10, 130 + yOffset, shrinkText);

    //绘制食物数量
    settextcolor(BROWN);
    TCHAR foodText[50];
    _stprintf_s(foodText, _T("食物: %d/%d"), fd_current_count, FD_MAX_NUM);
    outtextxy(10, 160 + yOffset, foodText);

    //绘制音乐状态
    extern bool musicEnabled; // 声明来自Music.cpp的变量
    settextcolor(BLUE);
    TCHAR musicText[50];
    _stprintf_s(musicText, _T("音乐: %s"), musicEnabled ? _T("开") : _T("关"));
    outtextxy(10, 190 + yOffset, musicText);

    //绘制操作提示
    settextcolor(DARKGRAY);
    settextstyle(15, 0, _T("宋体"));
    outtextxy(10, Win_H - 100, _T("方向键/WASD: 移动"));
    outtextxy(10, Win_H - 80, _T("P: 暂停/继续  M: 音乐开关"));
    outtextxy(10, Win_H - 60, _T("ESC: 退出"));
    outtextxy(10, Win_H - 40, _T("每5秒自动缩小5%，保持进食！"));
    outtextxy(10, Win_H - 20, _T("食物会持续再生，吃不完！"));

    // 新增无敌状态提示
    if (PL_Ball.isInvincible)
    {
        settextcolor(RGB(255, 200, 0));
        outtextxy(Win_W - 200, 10, _T("★ 无敌状态 ★"));
    }
}

//绘制暂停菜单
void DrawPauseMenu()
{
    SetWorkingImage();

    //绘制半透明背景
    setfillcolor(RGB(0, 0, 0, 128));
    solidrectangle(0, 0, Win_W, Win_H);

    //绘制暂停文本
    setbkmode(TRANSPARENT);
    settextstyle(50, 0, _T("黑体"));
    settextcolor(YELLOW);

    int textWidth = textwidth(_T("游戏暂停"));
    outtextxy(Win_W / 2 - textWidth / 2, Win_H / 2 - 50, _T("游戏暂停"));

    settextstyle(25, 0, _T("宋体"));
    settextcolor(WHITE);

    textWidth = textwidth(_T("按 P 键继续游戏"));
    outtextxy(Win_W / 2 - textWidth / 2, Win_H / 2 + 20, _T("按 P 键继续游戏"));

    textWidth = textwidth(_T("按 ESC 键退出游戏"));
    outtextxy(Win_W / 2 - textWidth / 2, Win_H / 2 + 60, _T("按 ESC 键退出游戏"));
}