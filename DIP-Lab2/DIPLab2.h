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

	void processStartRecting(QPoint pos); // ����ʼѡ�����
	void processRecting(QPoint pos); // �������ѡ��
	void processEndRecting(); // �������ѡ�����


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

	bool recting = false; // �Ƿ���ѡ�����״̬��
	QPoint startPos;

//signals:
//	void sendMessageSignal(QString str);
};
