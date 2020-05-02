#pragma once

#include <iostream>
#include <fstream>
#include <algorithm>

#include "ImageProcess.h"
#include "QTCVUtils.h"
#include "FilesProcessUtils.hpp"

#include "lib/STRUCK/Tracker.h"
#include "lib/STRUCK/Config.h"

static class OTBUtils {
public:
	static bool showImg;

	static void openDataset(string path, string format = "%04d.jpg");

	static void run(ProcessParam *param_);
	static void run(ObjTrackParam *param, ofstream &opt, int frames_num = 0, int rect_type = 0);

private:
	typedef map<double, double> OutTable;

	static const string RectSpecFile; // ����ָ���ļ���
	static const string ImagesDir; // ͼƬ�ļ���

	static const double MaxDistanceTreshold;
	static const double MaxOSTreshold;

	static const double DeltaTreshold; // ����

	static string format;
	static string path;
	static Mat* frames;
	static vector<cv::Rect> truthRects;

	static void _loadRects(string filename);
	static void _loadFrames(string path);

	static void _saveToFile(OutTable out, ofstream &opt);

	static cv::Rect _initRect(cv::Rect orig, int rect_type); //根据类型构造初始矩阵

	static void _runDetect(Rect2d* &rects, double* &dists, double* &oss, ObjTrackParam *param, ofstream &opt, long start_frame, int rect_type);

	static void __runStdDetect(Rect2d* &rects, double* &dists, double* &oss, 
		ObjTrackParam *param, ofstream &opt, long start_frame, int rect_type);
	static void __runSTRUCKDetect(Rect2d* &rects, double* &dists, double* &oss,
		ObjTrackParam *param, ofstream &opt, long start_frame, int rect_type);
	static void __runGOTURNDetect(Rect2d* &rects, double* &dists, double* &oss,
		ObjTrackParam *param, ofstream &opt, long start_frame, int rect_type);

	static void __showProcessingImage(double dist, double os, 
		Mat &frame, Rect2d &det, ObjTrackParam * param, cv::Rect &truth);

	// static void _calcEvaluation(Rect2d* rects, double* &dists, double* &oss);

	// Distance
	static double __calcDistance(Rect2d dist, Rect2d truth);

	// Overlap Space
	static double __calcOS(Rect2d dist, Rect2d truth);

	static double __calcCrossSpace(Rect2d dist, Rect2d truth);
	static double __calcMergeSpace(Rect2d dist, Rect2d truth, double cross);
};

