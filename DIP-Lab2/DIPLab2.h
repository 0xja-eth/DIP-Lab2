#pragma once
#pragma execution_character_set("utf-8")  

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPaintEvent>
#include <QFileDialog>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QImage>

#include "ImageProcess.h"

#include "ui_DIPLab2.h"

class DIPLab2 : public QMainWindow {
	Q_OBJECT

public:
	DIPLab2(QWidget *parent = Q_NULLPTR);

	QImage srcQImg1();
	QImage srcQImg2();

private:
	Ui::DIPLab2Class ui;

	void openFile(QLabel* tarImg, QImage* qimg);
	void loadFile(QString filename, QLabel* tarImg, QImage* qimg);

	void processStartRecting(QPoint pos); // 处理开始选择矩形
	void processRecting(QPoint pos); // 处理矩形选择
	void processEndRecting(); // 处理结束选择矩形


	QPoint transferPoint(QPoint pos, QLabel* tarImg);
	bool pointInRect(QPoint pos, QLabel* tarImg, bool global = true);

public slots:
	void openPicture1() { openFile(ui.srcImg1, &qimg1); };
	void openPicture2() { openFile(ui.srcImg2, &qimg2); }
	void setRecting() { recting = true; }
	void onRectChanged();
	void doFERNS();

protected:

	void mousePressEvent(QMouseEvent * e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent * e);

private:
	QImage qimg1, qimg2;

	bool recting = false; // 是否处于选择矩形状态下
	QPoint startPos;

//signals:
//	void sendMessageSignal(QString str);
};
