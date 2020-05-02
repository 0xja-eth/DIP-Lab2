#pragma once

#include <iostream>
#include <fstream>
#include <algorithm>

#include "ImageProcess.h"
#include "QTCVUtils.h"
#include "FilesProcessUtils.hpp"

#include "lib/STRUCK/Tracker.h"
#include "lib/STRUCK/Config.h"

struct VOTBB {
	cv::Point a, b, c, d;

	cv::Rect toRect() {
		cv::Point minP = a, maxP = a;

		minP.x = min(minP.x, b.x); minP.y = min(minP.y, b.y);
		minP.x = min(minP.x, c.x); minP.y = min(minP.y, c.y);
		minP.x = min(minP.x, d.x); minP.y = min(minP.y, d.y);

		maxP.x = max(maxP.x, b.x); maxP.y = max(maxP.y, b.y);
		maxP.x = max(maxP.x, c.x); maxP.y = max(maxP.y, c.y);
		maxP.x = max(maxP.x, d.x); maxP.y = max(maxP.y, d.y);

		return cv::Rect(minP, maxP);
	}
};

static class TestUtils {
public:
	static bool showImg;

	static void openOtbDataset(string path, string format = "%04d.jpg");
	static void openVotDataset(string path, string format = "%08d.jpg");

	static void runOtb(ProcessParam *param_);
	static void runOtb(ObjTrackParam *param, ofstream &opt, int frames_num = 0, int rect_type = 0, bool head = false);

	static void runVot(ProcessParam *param_);
	static void runVot(ObjTrackParam *param, ofstream &opt, bool head = false);

private:
	typedef map<double, double> OutTable;

	static const string OtbRectSpecFile; // ����ָ���ļ���
	static const string VotRectSpecFile; // ����ָ���ļ���

	static const string ImagesDir; // ͼƬ�ļ���

	static const double MaxDistanceTreshold;
	static const double MaxOSTreshold;

	static const double DeltaTreshold; // ����

	static string mode; // OTB / VOT

	static string format;
	static string path;
	static Mat* frames;
	static vector<cv::Rect> truthRects;

	static void _loadOtbRects(string filename);
	static void _loadVotRects(string filename);
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

	// 计算失败率
	static double __calcFailRate(long len, vector<long> &failPos);
	static double __deltaFail(long len, vector<long> &failPos, int pos);
};