#pragma once

#include <QImage>

#include "opencv/mycv.h"

using namespace cv;
using namespace mycv;

static class QTCVUtils {
public:
	static Mat qImage2Mat(const QImage& image);
	static QImage mat2QImage(const Mat& mat);

	static QImage drawRect(QImage* image,
		int x, int y, int w, int h);
};

