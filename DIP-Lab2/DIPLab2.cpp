
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
	connect(ui.openBtn1, SIGNAL(clicked()), this, SLOT(openPicture1()));
	connect(ui.openBtn2, SIGNAL(clicked()), this, SLOT(openPicture2()));
	connect(ui.rectBtn, SIGNAL(clicked()), this, SLOT(setRecting()));

	connect(ui.xInput, SIGNAL(valueChanged()), this, SLOT(onRectChanged()));
	connect(ui.yInput, SIGNAL(valueChanged()), this, SLOT(onRectChanged()));
	connect(ui.wInput, SIGNAL(valueChanged()), this, SLOT(onRectChanged()));
	connect(ui.hInput, SIGNAL(valueChanged()), this, SLOT(onRectChanged()));

}

QImage DIPLab2::srcQImg1() {
	return ui.srcImg1->pixmap()->toImage();
}

QImage DIPLab2::srcQImg2() {
	return ui.srcImg2->pixmap()->toImage();
}

void DIPLab2::openFile(QLabel* tarImg) {
	static QString title = "选择图像";
	static QString filter = "Images(*.png *.bmp *.jpg *.tif *.gif);; AllFiles(*.*)";
	QString filename = QFileDialog::getOpenFileName(this, title, "", filter);
	loadFile(filename, tarImg);
}

void DIPLab2::loadFile(QString filename, QLabel* tarImg) {
	static QString failText = "打开图像失败！";
	if (filename.isEmpty()) return;
	else {
		QImage img;
		if (!(img.load(filename))) 
			//加载图像
			QMessageBox::information(this, failText, failText);
		else
			tarImg->setPixmap(QPixmap::fromImage(img));// .scaled(tarImg->size())));
	}
}

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
	ui.wInput->setValue(maxx-minx);
	ui.hInput->setValue(maxy-miny);
}

void DIPLab2::processEndRecting() {
	recting = false;
	onRectChanged();
}

QPoint DIPLab2::transferPoint(QPoint pos, QLabel* tarImg) {
	QPoint gPos = tarImg->mapToParent(tarImg->pos());
	return pos - gPos;
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

void DIPLab2::onRectChanged() {
	int x = ui.xInput->value(), y = ui.yInput->value();
	int w = ui.wInput->value(), h = ui.hInput->value();
	auto img = QTCVUtils::drawRect(&srcQImg1(), x, y, w, h);
	QLabel* tarImg = ui.srcImg1;
	tarImg->setPixmap(QPixmap::fromImage(img)); // .scaled(tarImg->size())));
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