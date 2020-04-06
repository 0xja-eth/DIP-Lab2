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
#include <QThread>

#include "ImageProcess.h"
#include "QTCVUtils.h"

#include "ui_DIPLab2.h"

using namespace std;

class DIPLab2 : public QMainWindow {
	Q_OBJECT

public:
	DIPLab2(QWidget *parent = Q_NULLPTR);
	~DIPLab2();

	QImage srcQImg1();
	QImage srcQImg2();

	void setImg(QLabel* tarImg, QImage* qImg);
	void setSrcImg1(QImage* qImg);
	void setSrcImg2(QImage* qImg);
	void setTarImg(QImage* qImg);

	void setImg(QLabel* tarImg, MediaObject* qImg);
	void setSrcImg1(MediaObject* qImg);
	void setSrcImg2(MediaObject* qImg);
	void setTarImg(MediaObject* qImg);

	void setProgress(double progress);

	bool isProcessEnable();
	bool isProcessing();
	bool isDoingMediaIO();

	double getProcessProgress();
	double getMediaIOProgress();

private:
	Ui::DIPLab2Class ui;

	static const QString PictureTitle;
	static const QString VideoTitle;

	static const QString SaveTitle;

	static const QString PictureFilter;
	static const QString VideoFilter;

	static const QString OpenFailText;
	static const QString SaveSuccText;

	static const long UpdateInterval;

	void openFile(QLabel* tarImg, 
		MediaObject*& media, bool isVideo = false);
	void loadFile(QString filename, QLabel* tarImg, 
		MediaObject*& media, bool isVideo = false);

	void saveFile(MediaObject* media);

	void releaseMedia();
	void releaseTargets();
	void releaseParam();

	void processStartRecting(QPoint pos); // 处理开始选择矩形
	void processRecting(QPoint pos); // 处理矩形选择
	void processEndRecting(); // 处理结束选择矩形

	QPoint transferPoint(QPoint pos, QLabel* tarImg);
	bool pointInRect(QPoint pos, QLabel* tarImg, bool global = true);

	void doImageObjDetTrack(ProcessParam* param);
	void doVideoObjDetTrack(ProcessParam* param);

	static void requestUpdate(DIPLab2* win);
	static void sleep(long ms = UpdateInterval);

	void updateProgress();
	void updateProcessEnable();
	void updateProcessResult();
	void updateRectPos();

private slots:
	void update();
	void onLoadCompleted(MediaObject*& media);
	void onSaveCompleted();

signals:
	void signalUpdate();
	void signalLoadCompleted(MediaObject*& media);
	void signalSaveCompleted();

public slots:
	void openFile1() { openFile(ui.srcImg1, media1); }
	void openFile2() { openFile(ui.srcImg2, media2); }
	void recover1() { setSrcImg1(media1); };
	void recover2() { setSrcImg2(media2); }

	void openVideo() { openFile(ui.srcImg1, media1, true); }

	void saveTarget() { saveFile(target1); }

	void setRecting() { recting = true; }
	
	void onRectChanged();

	void doObjDet();
	void doObjDetTrack();
	void doFeatDet();

protected:
	void mousePressEvent(QMouseEvent * e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent * e);

private:
	MediaObject *media1 = NULL, *media2 = NULL;
	MediaObject *target1 = NULL, *target2 = NULL;

	/* 矩形选择相关属性 */
	bool recting = false; // 是否处于选择矩形状态下
	QPoint startPos;

	bool srcTarget = false;
	bool needUpdateRect = false;

	std::thread *updateThread;

	ProcessParam* param;
};
