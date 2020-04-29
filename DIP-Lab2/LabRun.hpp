
#ifndef LabRun_hpp
#define LabRun_hpp

#include "FilesProcessUtils.hpp"

#include "OTBUtils.h"

static class LabRun{
public:
    //初始化
    static bool labIn(string src_path);
    
    //实验一：otb测试
    static bool otb_lab(string otb_path);
    
    //实验二：otb_TRE 时间鲁棒性
    static bool otb_lab_tre(string otb_path);
    static bool otb_after_tre(string inpath, string outpath);//后处理
    
    //结束
    static bool labClose();

private:
    static string srcPath;
    
    //static ofstream opt1; //实验一写入文件
    
    //static vector<string> getFiles(string path); //获取目录下文件名
};

#endif /* LabRun_hpp */
