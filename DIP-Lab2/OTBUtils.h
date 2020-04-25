#pragma once

#include <iostream>
#include <fstream>

#include "ImageProcess.h"
#include "QTCVUtils.h"

static class OTBUtils {
public:
	static void openDataset(string path);

	static void run(ProcessParam *param_);

private:
	typedef map<double, double> OutTable;

	static const string RectSpecFile; // 矩形指定文件名
	static const string ImagesDir; // 图片文件夹

	static const double MaxDistanceTreshold;
	static const double MaxOSTreshold;

	static const double DeltaTreshold; // 增量

	static string path;
	static Mat* frames;
	static vector<Rect> truthRects;

	static void _loadRects(string filename);
	static void _loadFrames(string path);

	static void _saveToFile(OutTable out, string title);

	static void _runRound(double distThreshold, double osThreshold, 
		double &distRate, double &osRate, ObjTrackParam *param);

	// Distance
	static bool __testDistance(Rect2d dist, Rect2d truth, double threshold);
	static double __calcDistance(Rect2d dist, Rect2d truth);

	// Overlap Space
	static bool __testOS(Rect2d dist, Rect2d truth, double threshold);
	static double __calcOS(Rect2d dist, Rect2d truth);

	static double __calcCrossSpace(Rect2d dist, Rect2d truth);
	static double __calcMergeSpace(Rect2d dist, Rect2d truth);
};

