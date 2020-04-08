//
//  LabRun.cpp
//  CV-Lab2-Lab
//
//  Created by 陈以勒 on 2020/4/8.
//  Copyright © 2020 FoBird. All rights reserved.
//

#include "LabRun.hpp"

const string LabRun::srcString = "./src/";
const string LabRun::resString = "./res/";

ofstream LabRun::opt1;
ofstream LabRun::opt2;
ofstream LabRun::opt3;

bool LabRun::labIn(){
    opt1.open(resString+"lab1.csv");
    opt1<<"图像"<<","<<"方法"<<","<<"特征点数"<<","<<"用时(s)"<<"ANMS去除野点后"<<"用时(s)"<<endl;
    
    opt2.open(resString+"lab2/lab2.csv");
    opt2<<"图像"<<","<<"方法"<<","<<"匹配数"<<","<<"用时(s)"<<endl;
    
    opt3.open(resString+"lab3/lab3.csv");
    opt3<<"图像"<<","<<"方法"<<","<<"匹配数"<<","<<"用时(s)"<<endl;
    
    return true;
}

bool LabRun::labrun1(const int num){
    double start, end, run_time; //记录用时
    int keynum; //特征点数
    
    Mat src1 = imread(srcString+to_string(num)+"-1.jpg");
    Mat src2 = imread(srcString+to_string(num)+"-2.jpg");
    
    keynum = 0;
    FeatDetParam featderparamSIFT(FeatDetParam::SIFT, FeatDetParam::NoneR, FeatDetParam::NoneM, false);
    start = static_cast<double>(getTickCount());
    Mat resSIFT = ImageProcess::doFeatDet(src1, src2, keynum, &featderparamSIFT);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt1<<num<<","<<"SIFT"<<","<<keynum<<","<<run_time;//写入用时
    imwrite(resString+"lab1/lab1-"+to_string(num)+"-SIFT.jpg", resSIFT);//保存图片
    //imshow("lab1-SIFT", resSIFT);
    
    keynum = 0;
    FeatDetParam featderparamSIFT_ANMS(FeatDetParam::SIFT, FeatDetParam::NoneR, FeatDetParam::NoneM, true);
    start = static_cast<double>(getTickCount());
    Mat resSIFT_ANMS = ImageProcess::doFeatDet(src1, src2, keynum, &featderparamSIFT_ANMS);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt1<<","<<keynum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab1/lab1-"+to_string(num)+"-SIFT-ANMS.jpg", resSIFT_ANMS);//保存图片
    
    keynum = 0;
    FeatDetParam featderparamSURF(FeatDetParam::SURF, FeatDetParam::NoneR, FeatDetParam::NoneM, false);
    start = static_cast<double>(getTickCount());
    Mat resSURF = ImageProcess::doFeatDet(src1, src2, keynum, &featderparamSURF);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt1<<num<<","<<"SURF"<<","<<keynum<<","<<run_time;
    imwrite(resString+"lab1/lab1-"+to_string(num)+"-SURF.jpg", resSURF);
    //imshow("lab1-SURF", resSURF);
    
    keynum = 0;
    FeatDetParam featderparamSURF_ANMS(FeatDetParam::SIFT, FeatDetParam::NoneR, FeatDetParam::NoneM, true);
    start = static_cast<double>(getTickCount());
    Mat resSURF_ANMS = ImageProcess::doFeatDet(src1, src2, keynum, &featderparamSURF_ANMS);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt1<<","<<keynum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab1/lab1-"+to_string(num)+"-SURF-ANMS.jpg", resSURF_ANMS);//保存图片
    
    keynum = 0;
    FeatDetParam featderparamORB(FeatDetParam::ORB, FeatDetParam::NoneR, FeatDetParam::NoneM, false);
    start = static_cast<double>(getTickCount());
    Mat resORB = ImageProcess::doFeatDet(src1, src2, keynum, &featderparamORB);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt1<<num<<","<<"ORB"<<","<<keynum<<","<<run_time;
    imwrite(resString+"lab1/lab1-"+to_string(num)+"-ORB.jpg", resORB);
    //imshow("lab1-ORB", resORB);
    
    keynum = 0;
    FeatDetParam featderparamORB_ANMS(FeatDetParam::SIFT, FeatDetParam::NoneR, FeatDetParam::NoneM, true);
    start = static_cast<double>(getTickCount());
    Mat resORB_ANMS = ImageProcess::doFeatDet(src1, src2, keynum, &featderparamORB_ANMS);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt1<<","<<keynum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab1/lab1-"+to_string(num)+"-ORB-ANMS.jpg", resORB_ANMS);//保存图片
    
    return true;
}

bool LabRun::labrun2(const int num){
    double start, end, run_time; //记录用时
    int matchnum; //匹配数

    Mat src1 = imread(srcString+to_string(num)+"-1.jpg");
    Mat src2 = imread(srcString+to_string(num)+"-2.jpg");

    matchnum = 0;
    FeatDetParam featderparamBF(FeatDetParam::SIFT, FeatDetParam::NoneR, FeatDetParam::BruteForce, false);
    start = static_cast<double>(getTickCount());
    Mat resBF = ImageProcess::doFeatDet(src1, src2, matchnum, &featderparamBF);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt2<<num<<","<<"BF"<<","<<matchnum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab2/lab2-"+to_string(num)+"-BF.jpg", resBF);//保存图片

    /*matchnum = 0;
    FeatDetParam featderparamBF_ANMS(FeatDetParam::SIFT, FeatDetParam::NoneR, FeatDetParam::BruteForce, true);
    start = static_cast<double>(getTickCount());
    Mat resBF_ANMS = ImageProcess::doFeatDet(src1, src2, matchnum, &featderparamBF_ANMS);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt2<<","<<matchnum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab2/lab2-"+to_string(num)+"-BF-ANMS.jpg", resBF_ANMS);//保存图片*/
    
    matchnum = 0;
    FeatDetParam featderparamBFL1(FeatDetParam::SIFT, FeatDetParam::NoneR, FeatDetParam::BFL1, false);
    start = static_cast<double>(getTickCount());
    Mat resBFL1 = ImageProcess::doFeatDet(src1, src2, matchnum, &featderparamBFL1);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt2<<num<<","<<"BFL1"<<","<<matchnum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab2/lab2-"+to_string(num)+"-BFL1.jpg", resBFL1);//保存图片

    /*matchnum = 0;
    FeatDetParam featderparamBFL1_ANMS(FeatDetParam::SIFT, FeatDetParam::NoneR, FeatDetParam::BFL1, true);
    start = static_cast<double>(getTickCount());
    Mat resBFL1_ANMS = ImageProcess::doFeatDet(src1, src2, matchnum, &featderparamBFL1_ANMS);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt2<<","<<matchnum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab2/lab2-"+to_string(num)+"-BFL1-ANMS.jpg", resBFL1_ANMS);//保存图片*/
    
    matchnum = 0;
    FeatDetParam featderparamFLANN(FeatDetParam::SIFT, FeatDetParam::NoneR, FeatDetParam::FLANN, false);
    start = static_cast<double>(getTickCount());
    Mat resFLANN = ImageProcess::doFeatDet(src1, src2, matchnum, &featderparamFLANN);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt2<<num<<","<<"FLANN"<<","<<matchnum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab2/lab2-"+to_string(num)+"-FLANN.jpg", resBFL1);//保存图片

    /*matchnum = 0;
    FeatDetParam featderparamFLANN_ANMS(FeatDetParam::SIFT, FeatDetParam::NoneR, FeatDetParam::FLANN, true);
    start = static_cast<double>(getTickCount());
    Mat resFLANN_ANMS = ImageProcess::doFeatDet(src1, src2, matchnum, &featderparamFLANN_ANMS);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt2<<","<<matchnum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab2/lab2-"+to_string(num)+"-FLANN-ANMS.jpg", resFLANN_ANMS);//保存图片*/
    
    matchnum = 0;
    FeatDetParam featderparamRANSAC(FeatDetParam::SIFT, FeatDetParam::NoneR, FeatDetParam::RANSAC, false);
    start = static_cast<double>(getTickCount());
    Mat resRANSAC = ImageProcess::doFeatDet(src1, src2, matchnum, &featderparamRANSAC);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt2<<num<<","<<"RANSAC"<<","<<matchnum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab2/lab2-"+to_string(num)+"-RANSAC.jpg", resRANSAC);//保存图片

    /*matchnum = 0;
    FeatDetParam featderparamRANSAC_ANMS(FeatDetParam::SIFT, FeatDetParam::NoneR, FeatDetParam::RANSAC, true);
    start = static_cast<double>(getTickCount());
    Mat resRANSAC_ANMS = ImageProcess::doFeatDet(src1, src2, matchnum, &featderparamRANSAC_ANMS);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt2<<","<<matchnum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab2/lab2-"+to_string(num)+"-RANSAC-ANMS.jpg", resRANSAC_ANMS);//保存图片*/
    
    return true;
}

bool LabRun::labrun3(const int num){
    double start, end, run_time; //记录用时
    int matchnum; //匹配数量

    Mat src1 = imread(srcString+to_string(num)+"-1.jpg");
    Mat src2 = imread(srcString+to_string(num)+"-2.jpg");

    matchnum = 0;
    FeatDetParam featderparamNN(FeatDetParam::SIFT, FeatDetParam::NN, FeatDetParam::BruteForce, false);
    start = static_cast<double>(getTickCount());
    Mat resNN = ImageProcess::doFeatDet(src1, src2, matchnum, &featderparamNN);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt3<<num<<","<<"NN"<<","<<matchnum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab3/lab3-"+to_string(num)+"-NN.jpg", resNN);//保存图片

    matchnum = 0;
    FeatDetParam featderparamTwoNN(FeatDetParam::SIFT, FeatDetParam::TwoNN, FeatDetParam::BruteForce, false);
    start = static_cast<double>(getTickCount());
    Mat resTwoNN = ImageProcess::doFeatDet(src1, src2, matchnum, &featderparamTwoNN);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt3<<num<<","<<"NNDR"<<","<<matchnum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab3/lab3-"+to_string(num)+"-NNDR.jpg", resTwoNN);//保存图片
    
    matchnum = 0;
    FeatDetParam featderparamHomography(FeatDetParam::SIFT, FeatDetParam::Homography, FeatDetParam::BruteForce, false);
    start = static_cast<double>(getTickCount());
    Mat resHomography = ImageProcess::doFeatDet(src1, src2, matchnum, &featderparamHomography);
    end = static_cast<double>(getTickCount());
    run_time = (end - start) / getTickFrequency();
    opt3<<num<<","<<"Homography"<<","<<matchnum<<","<<run_time<<endl;//写入用时
    imwrite(resString+"lab3/lab3-"+to_string(num)+"-Homography.jpg", resHomography);//保存图片
    
    return true;
}

bool LabRun::labClose(){
    opt1.close();
    opt2.close();
    opt3.close();
    
    return true;
}
