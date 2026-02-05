#include "Music.h"
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <tchar.h>  
//链接Windows多媒体库
#pragma comment(lib, "winmm.lib")

//背景音乐状态定义
bool musicPlaying = false;
bool musicEnabled = true; //是否启用音乐

//播放背景音乐
void PlayBackgroundMusic()
{
    if (!musicEnabled || musicPlaying) return;

    // 使用相对路径播放音乐
    // 音乐文件"Ground Stage.mp3"与可执行文件在同一目录
    MCIERROR error = mciSendString(_T("open \".\\Ground Stage.mp3\" alias bgm"), NULL, 0, NULL);
    if (error != 0)
    {
        // 如果当前目录下找不到，尝试其他相对路径
        error = mciSendString(_T("open \".\\..\\Ground Stage.mp3\" alias bgm"), NULL, 0, NULL);
        if (error != 0)
        {
            error = mciSendString(_T("open \".\\..\\..\\Ground Stage.mp3\" alias bgm"), NULL, 0, NULL);
            if (error != 0)
            {
                // 如果所有相对路径都失败，显示错误信息
                TCHAR errorMsg[256];
                mciGetErrorString(error, errorMsg, 256);
                _tprintf(_T("无法播放背景音乐: %s\n"), errorMsg);
                _tprintf(_T("请确保音乐文件'Ground Stage.mp3'存在于程序目录。\n"));
                musicEnabled = false;
                return;
            }
        }
    }

    //设置循环播放
    mciSendString(_T("play bgm repeat"), NULL, 0, NULL);
    musicPlaying = true;

    //设置音量（0-1000，500表示中等音量）
    mciSendString(_T("setaudio bgm volume to 500"), NULL, 0, NULL);
}

//停止背景音乐
void StopBackgroundMusic()
{
    if (musicPlaying)
    {
        mciSendString(_T("stop bgm"), NULL, 0, NULL);
        mciSendString(_T("close bgm"), NULL, 0, NULL);
        musicPlaying = false;
    }
}

//暂停背景音乐
void PauseBackgroundMusic()
{
    if (musicEnabled && musicPlaying)
    {
        mciSendString(_T("pause bgm"), NULL, 0, NULL);
    }
}

//继续播放背景音乐
void ResumeBackgroundMusic()
{
    if (musicEnabled && musicPlaying)
    {
        mciSendString(_T("resume bgm"), NULL, 0, NULL);
    }
}

//切换音乐开关
void ToggleMusic()
{
    musicEnabled = !musicEnabled;

    if (musicEnabled)
    {
        //如果之前音乐没在播放，现在开始播放
        if (!musicPlaying)
        {
            PlayBackgroundMusic();
        }
        else
        {
            //如果之前音乐在播放但暂停了，现在恢复
            ResumeBackgroundMusic();
        }
    }
    else
    {
        //关闭音乐
        StopBackgroundMusic();
    }
}