
#include <cmath>

#include <QMetaType>

#include "DIPLab2.h"

DIPLab2::DIPLab2(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);

	DebugUtils::openConsole();

	//���ô�С
	this->resize(620, 500);

	qRegisterMetaType<MediaObject*>("MediaObject*&");

	setMouseTracking(true);
	//��ӹ������룬������� setupUi ����֮��,�ź���ۻ���
	connect(ui.openBtn1, SIGNAL(clicked()), this, SLOT(openFile1()));
	connect(ui.openBtn2, SIGNAL(clicked()), this, SLOT(openFile2()));
	connect(ui.recoverBtn1, SIGNAL(clicked()), this, SLOT(recover1()));
	connect(ui.recoverBtn2, SIGNAL(clicked()), this, SLOT(recover2()));

	connect(ui.openVideo, SIGNAL(clicked()), this, SLOT(openVideo()));

	connect(ui.saveBtn, SIGNAL(clicked()), this, SLOT(saveTarget()));

	connect(ui.manualSet, SIGNAL(clicked()), this, SLOT(setRecting()));

	connect(ui.xInput, SIGNAL(valueChanged(int)), this, SLOT(onRectChanged()));
	connect(ui.yInput, SIGNAL(valueChanged(int)), this, SLOT(onRectChanged()));
	connect(ui.wInput, SIGNAL(valueChanged(int)), this, SLOT(onRectChanged()));
	connect(ui.hInput, SIGNAL(valueChanged(int)), this, SLOT(onRectChanged()));

	connect(ui.adTypeSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(doObjDet()));

	connect(ui.doObjDet, SIGNAL(clicked()), this, SLOT(doObjDetTrack()));

	connect(ui.doFeatDet, SIGNAL(clicked()), this, SLOT(doFeatDet()));

	connect(this, SIGNAL(signalUpdate()), this, SLOT(update()));
	connect(this, SIGNAL(signalLoadCompleted(MediaObject*&)), 
		this, SLOT(onLoadCompleted(MediaObject*&)));
	connect(this, SIGNAL(signalSaveCompleted()), this, SLOT(onSaveCompleted()));

	updateThread = new std::thread(requestUpdate, this);
}

DIPLab2::~DIPLab2() {
	releaseMedia();
	releaseTargets();
	releaseParam();
	DebugUtils::closeConsole();
}

#pragma region ��������

const long DIPLab2::UpdateInterval = 10;

void DIPLab2::requestUpdate(DIPLab2* win) {
	try {
		while (true) {
			sleep();
			emit win->signalUpdate();
		}
	} catch (exception e) {
		LOG("��������" << e.what());
	}
}

void DIPLab2::sleep(long ms /*= UpdateInterval*/) {
	std::this_thread::sleep_for(
		std::chrono::milliseconds(ms));
}

void DIPLab2::update() {
	updateProgress();
	updateProcessEnable();

	// ����״̬�仯
	if (QTCVUtils::isProcessStatusChanged()) {
		updateProcessResult();
		updateRectPos();
	}

	QTCVUtils::update();
}

void DIPLab2::updateProgress() {
	double progress = 0; 
	if (isDoingMediaIO()) 
		progress = getMediaIOProgress(); 
	else if (isProcessing())
		progress = getProcessProgress();
	setProgress(progress);
}

void DIPLab2::updateProcessEnable() {
	bool enable = isProcessEnable();
	ui.processTab->setEnabled(enable);
	ui.saveBtn->setEnabled(enable);
}

void DIPLab2::updateProcessResult() {
	// ��� target1 Ϊ NULL����ʱ target2 �ض�Ϊ NULL��
	if (target1 == NULL) {
		// ����ԭͼƬ
		setSrcImg1(media1);
		setSrcImg2(media2);
		setTarImg((MediaObject*)NULL);
	} // ���ֻ�� target2 Ϊ NULL����ֻ��һ�����
	else if (target2 == NULL) {
		if (srcTarget) // ���ƾ���ʱ����Ҫ���ͼƬ��Դͼ
			setSrcImg1(target1);
		else setTarImg(target1);
	} // �������������
	else if (target2 != NULL) {
		setSrcImg1(target1);
		setSrcImg2(target2);
	}

	srcTarget = false;
}

void DIPLab2::updateRectPos() {
	if (!needUpdateRect || QTCVUtils::isProcessing()) return;
	RectParam* rectParam = (RectParam*)param;

	ui.xInput->setValue(rectParam->x);
	ui.yInput->setValue(rectParam->y);
	ui.wInput->setValue(rectParam->w);
	ui.hInput->setValue(rectParam->h);

	needUpdateRect = false;
}

#pragma endregion

#pragma region �����ͷ�

void DIPLab2::releaseMedia() {
	delete media1, media2;
	media1 = media2 = NULL;
}

void DIPLab2::releaseTargets() {
	delete target1, target2;
	target1 = target2 = NULL;
}

void DIPLab2::releaseParam() {
	delete param; param = NULL;
}

#pragma endregion

#pragma region ״̬�ж�

void DIPLab2::setProgress(double progress) {
	ui.progressBar->setValue(progress * 100);
}

bool DIPLab2::isProcessing() {
	return QTCVUtils::isProcessing();
}

bool DIPLab2::isDoingMediaIO() {
	return (media1 != NULL && media1->isLoading()) ||
		(media2 != NULL && media2->isLoading()) ||
		(target1 != NULL && target1->isLoading());
}

bool DIPLab2::isProcessEnable() {
	// �Ǵ������ҵ�һ��ý�����ǿ�
	return !isProcessing() && !isDoingMediaIO() 
		&& media1 != NULL && !media1->isEmpty();
}

double DIPLab2::getProcessProgress() {
	return QTCVUtils::progress();
}

double DIPLab2::getMediaIOProgress() {
	if (media1 != NULL && media1->isLoading())
		return media1->getProgress();
	if (media2 != NULL && media2->isLoading())
		return media2->getProgress();
	if (target1 != NULL && target1->isLoading())
		return target1->getProgress();
	return 0;
}

#pragma endregion

#pragma region �ļ�����

const QString DIPLab2::PictureTitle = "ѡ��ͼƬ";
const QString DIPLab2::VideoTitle = "ѡ����Ƶ";

const QString DIPLab2::SaveTitle = "ѡ�񱣴�λ��";

const QString DIPLab2::PictureFilter = "Images(*.png *.bmp *.jpg *.tif *.gif);; AllFiles(*.*)";
const QString DIPLab2::VideoFilter = "Videos(*.mp4 *.mov *.avi);; AllFiles(*.*)";

const QString DIPLab2::OpenFailText = "�ļ���ʧ�ܣ�";

const QString DIPLab2::SaveSuccText = "�ļ�����ɹ���";

void DIPLab2::openFile(QLabel* tarImg, MediaObject*& media, bool isVideo) {
	QString title = isVideo ? VideoTitle : PictureTitle;
	QString filter = isVideo ? VideoFilter : PictureFilter;
	QString filename = QFileDialog::getOpenFileName(this, title, "", filter);
	new std::thread([this, &filename, &tarImg, &media, &isVideo]() {
		this->loadFile(filename, tarImg, media, isVideo); 
	});
}

void DIPLab2::loadFile(QString filename, QLabel* tarImg,
	MediaObject*& media, bool isVideo) {
	if (!filename.isEmpty()) {
		delete media;
		media = new MediaObject();
		media->load(filename, isVideo);
	}
	emit signalLoadCompleted(media);
}

void DIPLab2::saveFile(MediaObject* media) {
	if (media == NULL) return;
	QString filter = media->isVideo() ? VideoFilter : PictureFilter;
	QString filename = QFileDialog::getSaveFileName(this, SaveTitle, "", filter);
	new std::thread([this, &media, &filename]() { 
		media->save(filename);
		emit this->signalSaveCompleted();
	});
}

void DIPLab2::onLoadCompleted(MediaObject*& media) {
	if (media->isEmpty()) QMessageBox::information(this, OpenFailText, OpenFailText);
	else updateProcessResult();
}

void DIPLab2::onSaveCompleted() {
	QMessageBox::information(this, SaveSuccText, SaveSuccText);
}

#pragma endregion

#pragma region ͼ������

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

void DIPLab2::setImg(QLabel* tarImg, MediaObject* media) {
	if (media == NULL) setImg(tarImg, (QImage*)NULL);
	else {
		auto image = media->getQImage();
		setImg(tarImg, &image);
	}
}

void DIPLab2::setSrcImg1(QImage* qImg) {
	setImg(ui.srcImg1, qImg);
}

void DIPLab2::setSrcImg1(MediaObject* media) {
	setImg(ui.srcImg1, media);
}

void DIPLab2::setSrcImg2(QImage* qImg) {
	setImg(ui.srcImg2, qImg);
}

void DIPLab2::setSrcImg2(MediaObject* media) {
	setImg(ui.srcImg2, media);
}

void DIPLab2::setTarImg(QImage* qImg) {
	setImg(ui.tarImg, qImg);
}

void DIPLab2::setTarImg(MediaObject* media) {
	setImg(ui.tarImg, media);
}

#pragma endregion

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
	releaseTargets(); releaseParam();

	int x = ui.xInput->value(), y = ui.yInput->value();
	int w = ui.wInput->value(), h = ui.hInput->value();
	param = new RectParam(x, y, w, h);

	srcTarget = true;
	QTCVUtils::processSync(ImageProcess::drawRect, media1, target1, param);

	updateProcessResult();
}

#pragma endregion

#pragma region Ŀ������ٴ���

void DIPLab2::doObjDet() {
	releaseTargets(); releaseParam();

	int x = ui.xInput->value(), y = ui.yInput->value();
	int w = ui.wInput->value(), h = ui.hInput->value();
	int algo = ui.odtAlgoSelect->currentIndex();
	int adType = ui.adTypeSelect->currentIndex();

	param = new ObjDetTrackParam(x, y, w, h,
		(ObjDetTrackParam::Algo)algo, 
		(ObjDetTrackParam::ADType)adType);

	srcTarget = needUpdateRect = true;
	QTCVUtils::process(ImageProcess::doObjDet, media1, target1, param);
}

void DIPLab2::doObjDetTrack() {
	releaseTargets(); releaseParam();

	int x = ui.xInput->value(), y = ui.yInput->value();
	int w = ui.wInput->value(), h = ui.hInput->value();
	int algo = ui.odtAlgoSelect->currentIndex();
	int adType = ui.adTypeSelect->currentIndex();
	
	param = new ObjDetTrackParam(x, y, w, h,
		(ObjDetTrackParam::Algo)algo,
		(ObjDetTrackParam::ADType)adType);

	if (media1->isVideo()) doImageObjDetTrack(param);
	else doVideoObjDetTrack(param);
}

void DIPLab2::doImageObjDetTrack(ProcessParam* param) {
	QTCVUtils::process(ImageProcess::doObjDetTrack, 
		media1, target1, param);
}

void DIPLab2::doVideoObjDetTrack(ProcessParam* param) {
	QTCVUtils::process(ImageProcess::doObjTrack,
		media1, media2, target1, target2, param);
}

#pragma endregion

#pragma region ���ƥ�䴦��

void DIPLab2::doFeatDet() {
	releaseTargets(); releaseParam();

	int algo = ui.fdAlgoSelect->currentIndex();
	int rType = ui.rTypeSelect->currentIndex();
	int mType = ui.mTypeSelect->currentIndex()+1;
	
	param = new FeatDetParam((FeatDetParam::Algo)algo, 
		(FeatDetParam::RType)rType, (FeatDetParam::MType)mType);

	QTCVUtils::process(ImageProcess::doFeatDet, 
		media1, media2, target1, param);
}

#pragma endregion

