#include "Game.h"
#include "Graphics.h"
#include "Music.h"
#include <windows.h>
#include<iostream>
using namespace std;

int main()
{
    //初始化图形窗口
    initgraph(Win_W, Win_H);
    CenterWindow(GetHWnd(), Win_W, Win_H);

    //开启双缓冲
    BeginBatchDraw();

    Game_Init();                           //游戏初始化

    //开始播放背景音乐
    if (musicEnabled)
    {
        PlayBackgroundMusic();
    }

    //游戏主循环
    while (gameRunning)
    {
        //处理按键输入
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            gameRunning = false; //按ESC退出游戏
        }

        if (GetAsyncKeyState('P') & 0x8000) //按P键暂停
        {
            isPaused = !isPaused;
            Sleep(200); //防抖

            //根据暂停状态控制音乐
            if (isPaused)
            {
                PauseBackgroundMusic();
            }
            else
            {
                ResumeBackgroundMusic();
            }
        }

        //按M键切换音乐开关
        if (GetAsyncKeyState('M') & 0x8000)
        {
            ToggleMusic();
            Sleep(200); //防抖
        }

        if (!isPaused)
        {
            //游戏逻辑更新
            PL_Move();                         //控制玩家移动
            UpdateInvincible();                // 新增：更新无敌状态
            Enemy_Move();                      //控制敌人AI移动
            CheckCollisions();                 //玩家与食物碰撞检测
            UpdateEnemyCollisions();           //敌人碰撞检测
            UpdateShrink();                    //更新缩小逻辑
            UpdateFood();                      // 新增：更新食物系统（生成新食物）
            gameTime++;
        }

        //游戏绘制
        Game_Draw();

        //绘制暂停菜单（如果需要）
        if (isPaused)
        {
            DrawPauseMenu();
        }

        //双缓冲需要刷新
        FlushBatchDraw();

        //控制帧率 (大约60帧每秒)
        Sleep(16);
    }

    //停止背景音乐
    StopBackgroundMusic();

    EndBatchDraw(); //结束双缓冲
    closegraph(); //关闭图形窗口

    if (!PL_Ball.flag)
    {
        cout << "游戏结束！你被敌人吃掉了！最终得分：" << score << endl;
    }
    else
    {
        cout << "游戏结束！最终得分：" << score << endl;
    }

    system("pause");
    return 0;
}