#pragma once

#include <QImage>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/xfeatures2d.hpp>

#include "QTCVUtils.h"

static class ImageProcess {
public:
	typedef vector<KeyPoint> KeyPoints;

	static void doFERNS(QImage img1, QImage img2, 
		QImage &out1, QImage &out2, int x, int y, int w, int h);
	static QImage doSURF(QImage img1, QImage img2);
	static QImage doSIFT(QImage img1, QImage img2);
	static QImage doORB(QImage img1, QImage img2);

private:
	static Mat _featureDectect(Feature2D* algo, 
		Mat &data1, Mat &data2);
};

