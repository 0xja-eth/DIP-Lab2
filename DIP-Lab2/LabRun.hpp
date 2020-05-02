
#ifndef LabRun_hpp
#define LabRun_hpp

#include "FilesProcessUtils.hpp"

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <algorithm>

#include "OTBUtils.h"

static class LabRun{
public:
	static int maxNum;

	//初始化
	static bool labIn(string outPath);
	static void setOtbPath(string otbPath);

	//实验一：otb测试
	static void otbLab();
	static bool otbLab(string otbPath);
    
	//实验二：otb_TRE 时间鲁棒性
	static void otbLabTRE();
	static bool otbLabTRE(string otbPath);
    static bool otbAfterTRE(string inPath, string outPath);//后处理
    
	//实验三：otb_SRE 空间鲁棒性
	static void otbLabSRE();
	static bool otbLabSRE(string otbPath);
    static bool otbAfterSRE(string inPath, string outPath);//后处理

private:
    static string outPath, otbPath;
    
    //static ofstream opt1; //实验一写入文件
    
    //static vector<string> getFiles(string path); //获取目录下文件名
};

#endif /* LabRun_hpp */
