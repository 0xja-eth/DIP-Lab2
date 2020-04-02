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

#include <opencv2\highgui\highgui.hpp>
#include <opencv2/opencv.hpp>

#include "ui_DIPLab2.h"

class DIPLab2 : public QMainWindow {
	Q_OBJECT

public:
	DIPLab2(QWidget *parent = Q_NULLPTR);

	QImage srcQImg1();
	QImage srcQImg2();

private:
	Ui::DIPLab2Class ui;

	void openFile(QLabel* tarImg);
	void loadFile(QString filename, QLabel* tarImg);

	void processStartRecting(QPoint pos); // ����ʼѡ�����
	void processRecting(QPoint pos); // �������ѡ��
	void processEndRecting(); // �������ѡ�����


	QPoint transferPoint(QPoint pos, QLabel* tarImg);
	bool pointInRect(QPoint pos, QLabel* tarImg, bool global = true);

public slots:
	void openPicture1() { openFile(ui.srcImg1); };
	void openPicture2() { openFile(ui.srcImg2); }
	void setRecting() { recting = true; }
	void onRectChanged();

protected:

	void mousePressEvent(QMouseEvent * e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent * e);

private:
	bool recting = false; // �Ƿ���ѡ�����״̬��
	QPoint startPos;

//signals:
//	void sendMessageSignal(QString str);
};
