#pragma once

#include <QImage>

#include "QTCVUtils.h"

static class ImageProcess {
public:
	static void doFERNS(QImage img1, QImage img2, 
		QImage &out1, QImage &out2, int x, int y, int w, int h);
};

