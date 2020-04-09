//
//  LabRun.hpp
//  CV-Lab2-Lab
//
//  Created by 陈以勒 on 2020/4/8.
//  Copyright © 2020 FoBird. All rights reserved.
//

#ifndef LabRun_hpp
#define LabRun_hpp

#include "ImageProcess.hpp"

//以下是文件读入输出需要的头文件
#include<fstream>
#include<cstdlib>
#include<streambuf>
#include <string>


static class LabRun{
public:
    // 初始化
    static bool labIn();
    
    // 实验一：比较SIFT、SURF和ORB特征点检测
    static bool labrun1(const int num);
    
    // 实验二：特征匹配算法比较(SIFT方法)
    static bool labrun2(const int num);
    
    // 实验三：错误匹配去除算法比较(SIFT方法、BF匹配)
    static bool labrun3(const int num);
    
    // 结束
    static bool labClose();
    
private:
    static const string srcString;
    static const string resString;
    static ofstream opt1; //实验一写入文件
    static ofstream opt2; //实验二写入文件
    static ofstream opt3; //实验三写入文件
    
};
#endif /* LabRun_hpp */
