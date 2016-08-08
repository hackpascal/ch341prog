// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#include "targetver.h"
#include <tchar.h>
#endif

#include <stdio.h>
#include <stdbool.h>



// TODO:  在此处引用程序需要的其他头文件


extern const unsigned char BitSwapTable[256];

void ProgressInit(void);
void ProgressShow(int percentage);
void ProgressDone(void);
