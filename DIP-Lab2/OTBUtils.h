#pragma once

#include <iostream>
#include <fstream>

#include "ImageProcess.h"
#include "QTCVUtils.h"
#include "FilesProcessUtils.hpp"

static class OTBUtils {
public:
	static void openDataset(string path);

	static void run(ProcessParam *param_, ofstream &opt, int frames_num=0);

private:
	typedef map<double, double> OutTable;

	static const string RectSpecFile; // ����ָ���ļ���
	static const string ImagesDir; // ͼƬ�ļ���

	static const double MaxDistanceTreshold;
	static const double MaxOSTreshold;

	static const double DeltaTreshold; // ����

	static string path;
	static Mat* frames;
	static vector<Rect> truthRects;

	static void _loadRects(string filename);
	static void _loadFrames(string path);

	static void _saveToFile(OutTable out, ofstream &opt);

	static void _runDetect(Rect2d* &rects, double* &dists, double* &oss, ObjTrackParam *param, ofstream &opt, long start_frame);
	// static void _calcEvaluation(Rect2d* rects, double* &dists, double* &oss);

	// Distance
	static double __calcDistance(Rect2d dist, Rect2d truth);

	// Overlap Space
	static double __calcOS(Rect2d dist, Rect2d truth);

	static double __calcCrossSpace(Rect2d dist, Rect2d truth);
	static double __calcMergeSpace(Rect2d dist, Rect2d truth, double cross);
};

