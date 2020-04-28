
#ifndef LabRun_hpp
#define LabRun_hpp

#include "OTBUtils.h"

static class LabRun{
public:
    //初始化
    static bool labIn(string src_path);
    
    //实验一：otb测试
    static bool otb_lab(string otb_path);
    
    //结束
    static bool labClose();

private:
    static string srcPath;
    
    //static ofstream opt1; //实验一写入文件
    
    //static vector<string> getFiles(string path); //获取目录下文件名
};

#endif /* LabRun_hpp */
