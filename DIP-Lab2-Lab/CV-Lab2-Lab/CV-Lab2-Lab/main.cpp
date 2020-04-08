//
//  main.cpp
//  CV-Lab2-Lab
//
//  Created by 陈以勒 on 2020/4/8.
//  Copyright © 2020 FoBird. All rights reserved.
//

#include <iostream>
#include "LabRun.hpp"

int main(int argc, const char * argv[]) {
    // 实验开始
    std::cout << "实验准备\n";
    LabRun::labIn();
    
    // 实验一
    std::cout << "Lab1: 特征提取算法(SIFT、SURF、ORB)比较&ANMS去除野点效果\n";
    
    int num = 5;
    for(int i=0; i<num; i++){
        LabRun::labrun1(i);
    }
    
    // 实验二
    std::cout << "Lab2: 特征匹配算法(BruteForce、BFL1、FLANN、RANSAC)比较(SIFT方法)\n";
    int num = 5;
    for(int i=0; i<num; i++){
        LabRun::labrun2(i);
    }
    
    // 实验三
    std::cout << "Lab3: 错误匹配去除算法(NN、NNDR、Homography)比较(SIFT方法、BF匹配)\n";
    int num = 5;
    for(int i=0; i<num; i++){
        LabRun::labrun3(i);
    }
    
    // 实验结束
    LabRun::labClose();
    std::cout << "实验结束\n";
    
    return 0;
}
