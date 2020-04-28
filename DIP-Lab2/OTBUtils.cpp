#include "OTBUtils.h"

const std::string OTBUtils::RectSpecFile = "groundtruth_rect.txt";

const std::string OTBUtils::ImagesDir = "img/";

const double OTBUtils::MaxDistanceTreshold = 50;

const double OTBUtils::MaxOSTreshold = 1;

const double OTBUtils::DeltaTreshold = 0.01;

std::string OTBUtils::path = "";

Mat* OTBUtils::frames = NULL;

vector<Rect> OTBUtils::truthRects;

void OTBUtils::openDataset(string path) {
	OTBUtils::path = path;
	auto rectFile = path + "/" + RectSpecFile;
	auto imgDir = path + "/" + ImagesDir;

	_loadRects(rectFile); _loadFrames(imgDir);
}

void OTBUtils::run(ProcessParam *param_) {
	auto param = (ObjTrackParam*)param_;

	// 检测，获取每帧检测矩形
	Rect2d *detRects; double *dists, *oss;
	_runDetect(detRects, dists, oss, param);

	OutTable distRates, osRates;
	long len = truthRects.size();

	for (double t = 0; t < 1; t += DeltaTreshold) {

		ImageProcess::progress = 0.9 + t / 1 * 0.1;

		auto distT = t * MaxDistanceTreshold;
		auto osT = t * MaxOSTreshold;

		double distRate, osRate;
		long distCnt = 0, osCnt = 0;
		for (int i = 1; i < len; ++i) {
			double dist = dists[i], os = oss[i];

			if (dist <= distT) distCnt++;
			if (os >= osT) osCnt++;
		}

		if (len > 1) {
			distRate = distCnt * 1.0 / (len - 1);
			osRate = osCnt * 1.0 / (len - 1);
		} else distRate = osRate = 0;

		distRates[distT] = distRate;
		osRates[osT] = osRate;
	}

	_saveToFile(distRates, to_string(param->algo)+"DistResult.r");
	_saveToFile(osRates, to_string(param->algo)+"OSResult.r");
}

void OTBUtils::_saveToFile(OutTable out, string title) {
	string filename = path + "/" + title;

	ofstream file(filename, ios::out);
	if (!file) { //打开失败
		LOG("Error opening save file."); return;
	}

	OutTable::iterator oit = out.begin();
	for (; oit != out.end(); ++oit) {
		auto pair = *oit;
		file << pair.first << " " << pair.second << endl;
	}
}

void OTBUtils::_runDetect(Rect2d* &rects, double* &dists, double* &oss, ObjTrackParam *param) {
	bool newDet = true;
	Ptr<Tracker> tracker = NULL;

	long len = truthRects.size();
	rects = new Rect2d[len];
	dists = new double[len];
	oss = new double[len];

	for (int i = 0; i < len; ++i) {
		ImageProcess::progress = i * 1.0 / len * 0.9;

		auto truth = truthRects[i];
		auto frame = frames[i];

		if (i == 0) {
			param->setRect(rects[i] = truth);
			ImageProcess::doObjTrack(frame, tracker, newDet, param);
			continue;
		}

		auto det = ImageProcess::doObjTrack(frame, tracker, newDet, param);

		// 匹配不到
		if (newDet) det = Rect2d();
		rects[i] = det;

		// 展示
		//Mat drawFrame = frame.clone();

		double dist = dists[i] = __calcDistance(det, truth);
		double os = oss[i] = __calcOS(det, truth);

		/*char showText[256];

		sprintf(showText, "Dist: %.4f, OS: %.4f", dist, os);

		putText(drawFrame, showText, Point(20, 20), 
			FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 255, 255), 2);

		rectangle(drawFrame, det, param->color, 2);
		rectangle(drawFrame, truth, Scalar(255, 0, 0), 2);

		imshow("OTBTest", drawFrame);

		waitKey(1);*/
	}
}

//void OTBUtils::_calcEvaluation(Rect2d* rects, double* &dists, double* &oss) {
//
//	long len = truthRects.size();
//	dists = new double[len];
//	oss = new double[len];
//	
//	for (int i = 0; i < len; ++i) {
//		auto det = rects[i];
//		auto truth = truthRects[i];
//
//		dists[i] = __calcDistance(det, truth);
//		oss[i] = __calcOS(det, truth);
//
//		LOG(i << ". det:" << det.x << "," << det.y << ","
//			<< det.width << "," << det.height
//			<< " truth:" << truth.x << "," << truth.y << ","
//			<< truth.width << "," << truth.height <<
//			" Dist: " << dists[i] << " OS: " << oss[i]);
//	}
//}


void OTBUtils::_loadRects(string filename) {
	truthRects.clear();

	ifstream file(filename, ios::in); // 以文本模式打开in.txt备读
	if (!file) { //打开失败
		LOG("Error opening source file."); return;
	}

	int x, y, w, h; char ch;
	while (file >> x) {
		file >> ch >> y; file >> ch >> w; file >> ch >> h;
		truthRects.push_back(Rect(x, y, w, h));
	}
}

void OTBUtils::_loadFrames(string path) {
	long len = truthRects.size();

	if (frames != NULL) {
		delete[] frames; frames = NULL;
	}

	frames = new Mat[len];
	for (int i = 0; i < len; ++i) {
		ImageProcess::progress = i * 1.0 / len;

		char name_[12];
		sprintf(name_, "%04d.jpg", i+1);
		string filename(name_);
		filename = path + filename;

		frames[i] = imread(filename);
	}
}

double OTBUtils::__calcDistance(Rect2d dist, Rect2d truth) {
	double dx = dist.x + (dist.width / 2);
	double dy = dist.y + (dist.height / 2);
	double tx = truth.x + (truth.width / 2);
	double ty = truth.y + (truth.height / 2);
	return sqrt((tx - dx)*(tx - dx) + (ty - dy)*(ty - dy));
}

double OTBUtils::__calcOS(Rect2d dist, Rect2d truth) {
	auto cross = __calcCrossSpace(dist, truth);
	return __calcCrossSpace(dist, truth) / __calcMergeSpace(dist, truth, cross);
}

double OTBUtils::__calcCrossSpace(Rect2d dist, Rect2d truth) {
	double dx = dist.x, dy = dist.y,
		dw = dist.width, dh = dist.height;
	double tx = truth.x, ty = truth.y,
		tw = truth.width, th = truth.height;

	double minX = max(dx, tx), maxX = min(dx + dw, tx + tw);
	double minY = max(dy, ty), maxY = min(dy + dh, ty + th);

	double width = maxX - minX, height = maxY - minY;
	if (width <= 0 || height <= 0) return 0;
	return width * height;
}

double OTBUtils::__calcMergeSpace(Rect2d dist, Rect2d truth, double cross) {
	return dist.area() + truth.area() - cross;
}
