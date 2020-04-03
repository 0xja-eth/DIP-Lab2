
#include <cmath>

#include "opencv/mycv.h"

#include "DIPLab2.h"

#include "QTCVUtils.h"

using namespace std;

DIPLab2::DIPLab2(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);

	//���ô�С
	this->resize(620, 500);

	setMouseTracking(true);
	//��ӹ������룬������� setupUi ����֮��,�ź���ۻ���
	connect(ui.openBtn1, SIGNAL(clicked()), this, SLOT(openFile1()));
	connect(ui.openBtn2, SIGNAL(clicked()), this, SLOT(openFile2()));
	connect(ui.recoverBtn1, SIGNAL(clicked()), this, SLOT(recover1()));
	connect(ui.recoverBtn2, SIGNAL(clicked()), this, SLOT(recover2()));

	connect(ui.rectBtn, SIGNAL(clicked()), this, SLOT(setRecting()));

	connect(ui.xInput, SIGNAL(valueChanged(int)), this, SLOT(onRectChanged()));
	connect(ui.yInput, SIGNAL(valueChanged(int)), this, SLOT(onRectChanged()));
	connect(ui.wInput, SIGNAL(valueChanged(int)), this, SLOT(onRectChanged()));
	connect(ui.hInput, SIGNAL(valueChanged(int)), this, SLOT(onRectChanged()));

	connect(ui.doFERNS, SIGNAL(clicked()), this, SLOT(doFERNS()));

	connect(ui.doFeatDetect, SIGNAL(clicked()), this, SLOT(doFeatureDetect()));
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
	static QString title = "ѡ��ͼ��";
	static QString filter = "Images(*.png *.bmp *.jpg *.tif *.gif);; AllFiles(*.*)";
	QString filename = QFileDialog::getOpenFileName(this, title, "", filter);
	loadFile(filename, tarImg, qimg);
}

void DIPLab2::loadFile(QString filename, QLabel* tarImg, QImage& qimg) {
	static QString failText = "��ͼ��ʧ�ܣ�";
	if (filename.isEmpty()) return;
	else {
		if (!(qimg.load(filename)))
			//����ͼ��
			QMessageBox::information(this, failText, failText);
		else
			setImg(tarImg, &qimg);
	}
}

#pragma region ��������

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

// ���������ת����Label�ڵ�����
QPoint DIPLab2::transferPoint(QPoint pos, QLabel* tarImg) {
	QPoint lPos = tarImg->pos();
	QPoint gMPos = tarImg->mapToGlobal(pos); // ȫ���������
	QPoint gLPos = tarImg->mapToGlobal(lPos); // ȫ��Label����

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

	QString posInfo = tr("λ�� %1, %2").arg(pos.x()).arg(pos.y());
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
	auto img = QTCVUtils::drawRect(&qImg1, x, y, w, h);
	ui.srcImg1->setPixmap(QPixmap::fromImage(img)); // .scaled(tarImg->size())));
}

#pragma endregion

void DIPLab2::doFERNS() {
	int x = ui.xInput->value(), y = ui.yInput->value();
	int w = ui.wInput->value(), h = ui.hInput->value();

	QImage out1, out2;
	ImageProcess::doFERNS(qImg1, qImg2, out1, out2, x, y, w, h);

	setSrcImg1(&out1); setSrcImg2(&out2);
}

void DIPLab2::doSIFT() {
	int x = ui.xInput->value(), y = ui.yInput->value();
	int w = ui.wInput->value(), h = ui.hInput->value();

	QImage out = ImageProcess::doSIFT(qImg1, qImg2);

	setTarImg(&out);
}

void DIPLab2::doSURF() {
	int x = ui.xInput->value(), y = ui.yInput->value();
	int w = ui.wInput->value(), h = ui.hInput->value();

	QImage out = ImageProcess::doSURF(qImg1, qImg2);

	setTarImg(&out);
}

void DIPLab2::doORB() {
	int x = ui.xInput->value(), y = ui.yInput->value();
	int w = ui.wInput->value(), h = ui.hInput->value();

	QImage out = ImageProcess::doORB(qImg1, qImg2);

	setTarImg(&out);
}

void DIPLab2::doFeatureDetect() {
	int type = ui.fdAlgoSelect->currentIndex();
	switch (type) {
	case 0: doSIFT(); break;
	case 1: doSURF(); break;
	case 2: doORB(); break;
	}
}

