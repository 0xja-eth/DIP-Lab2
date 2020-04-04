
#include <cmath>

#include "opencv/mycv.h"

#include "DIPLab2.h"

#include "QTCVUtils.h"

using namespace std;

DIPLab2::DIPLab2(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);

	//重置大小
	this->resize(620, 500);

	setMouseTracking(true);
	//添加关联代码，必须放在 setupUi 函数之后,信号与槽机制
	connect(ui.openBtn1, SIGNAL(clicked()), this, SLOT(openFile1()));
	connect(ui.openBtn2, SIGNAL(clicked()), this, SLOT(openFile2()));
	connect(ui.recoverBtn1, SIGNAL(clicked()), this, SLOT(recover1()));
	connect(ui.recoverBtn2, SIGNAL(clicked()), this, SLOT(recover2()));

	connect(ui.rectBtn, SIGNAL(clicked()), this, SLOT(setRecting()));

	connect(ui.xInput, SIGNAL(valueChanged(int)), this, SLOT(onRectChanged()));
	connect(ui.yInput, SIGNAL(valueChanged(int)), this, SLOT(onRectChanged()));
	connect(ui.wInput, SIGNAL(valueChanged(int)), this, SLOT(onRectChanged()));
	connect(ui.hInput, SIGNAL(valueChanged(int)), this, SLOT(onRectChanged()));

	connect(ui.doFERNS, SIGNAL(clicked()), this, SLOT(doRandFERNS()));

	connect(ui.doFeatDetect, SIGNAL(clicked()), this, SLOT(doFeatDet()));
}

QImage DIPLab2::srcQImg1() {
	return ui.srcImg1->pixmap()->toImage();
}

QImage DIPLab2::srcQImg2() {
	return ui.srcImg2->pixmap()->toImage();
}

void DIPLab2::setImg(QLabel* tarImg, QImage* qImg) {
	if (qImg == NULL)
		tarImg->clear();
	else
		tarImg->setPixmap(QPixmap::fromImage(*qImg));
}

void DIPLab2::setSrcImg1(QImage* qImg) {
	setImg(ui.srcImg1, qImg);
}

void DIPLab2::setSrcImg2(QImage* qImg) {
	setImg(ui.srcImg2, qImg);
}

void DIPLab2::setTarImg(QImage* qImg) {
	setImg(ui.tarImg, qImg);
}

void DIPLab2::openFile(QLabel* tarImg, QImage& qimg) {
	static QString title = "选择图像";
	static QString filter = "Images(*.png *.bmp *.jpg *.tif *.gif);; AllFiles(*.*)";
	QString filename = QFileDialog::getOpenFileName(this, title, "", filter);
	loadFile(filename, tarImg, qimg);
}

void DIPLab2::loadFile(QString filename, QLabel* tarImg, QImage& qimg) {
	static QString failText = "打开图像失败！";
	if (filename.isEmpty()) return;
	else {
		if (!(qimg.load(filename)))
			//加载图像
			QMessageBox::information(this, failText, failText);
		else
			setImg(tarImg, &qimg);
	}
}

#pragma region 矩形设置

void DIPLab2::processStartRecting(QPoint pos) {
	startPos = pos;
	ui.xInput->setValue(pos.x());
	ui.yInput->setValue(pos.y());
}

void DIPLab2::processRecting(QPoint pos) {
	int x1 = startPos.x();
	int y1 = startPos.y();
	int x2 = pos.x(), y2 = pos.y();
	int minx = min(x1, x2), miny = min(y1, y2);
	int maxx = max(x1, x2), maxy = max(y1, y2);
	ui.xInput->setValue(minx);
	ui.yInput->setValue(miny);
	ui.wInput->setValue(maxx - minx);
	ui.hInput->setValue(maxy - miny);
	//onRectChanged();
}

void DIPLab2::processEndRecting() {
	recting = false;
}

// 将鼠标坐标转化到Label内的坐标
QPoint DIPLab2::transferPoint(QPoint pos, QLabel* tarImg) {
	QPoint lPos = tarImg->pos();
	QPoint gMPos = tarImg->mapToGlobal(pos); // 全局鼠标坐标
	QPoint gLPos = tarImg->mapToGlobal(lPos); // 全局Label坐标

	return gMPos - gLPos;
}

bool DIPLab2::pointInRect(QPoint pos, QLabel* tarImg, bool global) {
	if (global)
		pos = transferPoint(pos, tarImg);
	auto size = tarImg->size();
	return pos.x() >= 0 && pos.y() >= 0 &&
		pos.x() < size.width() && pos.y() < size.height();
}

void DIPLab2::mouseMoveEvent(QMouseEvent *e) {
	auto pos = e->pos();

	QString posInfo = tr("位置 %1, %2").arg(pos.x()).arg(pos.y());
	ui.pointPos->setText(posInfo);

	if (recting) {
		pos = transferPoint(e->pos(), ui.srcImg1);
		if (pointInRect(pos, ui.srcImg1, true))
			processRecting(pos);
	}
}

void DIPLab2::mousePressEvent(QMouseEvent * e) {
	if (recting) {
		auto pos = transferPoint(e->pos(), ui.srcImg1);
		if (pointInRect(pos, ui.srcImg1, true))
			processStartRecting(pos);
	}
}

void DIPLab2::mouseReleaseEvent(QMouseEvent * e) {
	if (recting) processEndRecting();
}

void DIPLab2::onRectChanged() {
	int x = ui.xInput->value(), y = ui.yInput->value();
	int w = ui.wInput->value(), h = ui.hInput->value();
	auto param = RectParam(x, y, w, h);

	auto img = QTCVUtils::process(ImageProcess::drawRect, qImg1, &param);

	setSrcImg1(&img);
}

#pragma endregion

#pragma region FERN处理

void DIPLab2::doRandFERNS() {
	int x = ui.xInput->value(), y = ui.yInput->value();
	int w = ui.wInput->value(), h = ui.hInput->value();
	auto param = RectParam(x, y, w, h);

	QImage out1, out2;
	QTCVUtils::process(ImageProcess::doFERNS,
		qImg1, qImg2, out1, out2, &param);

	setSrcImg1(&out1); setSrcImg2(&out2);
}

#pragma endregion

#pragma region 特征检测处理

void DIPLab2::doFeatDet() {
	int algo = ui.fdAlgoSelect->currentIndex();
	int rType = ui.rTypeSelect->currentIndex();
	int mType = ui.mTypeSelect->currentIndex()+1;
	auto param = FeatDetParam((FeatDetParam::Algo)algo, 
		(FeatDetParam::RType)rType, (FeatDetParam::MType)mType);

	auto out = QTCVUtils::process(
		ImageProcess::doFeatDet, qImg1, qImg2, &param);

	setTarImg(&out);
}

#pragma endregion

