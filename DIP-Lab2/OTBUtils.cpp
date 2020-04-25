#include "OTBUtils.h"

const std::string OTBUtils::RectSpecFile = "groundtruth_rect.txt";

const std::string OTBUtils::ImagesDir = "img/";

const double OTBUtils::MaxDistanceTreshold = 50;

const double OTBUtils::MaxOSTreshold = 1;

const double OTBUtils::DeltaTreshold = 0.1;

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

	OutTable distRates, osRates;
	double distRate, osRate;
	for (double t = 0.1; t < 1; t += DeltaTreshold) {
		auto distT = t * MaxDistanceTreshold;
		auto osT = t * MaxOSTreshold;
		_runRound(distT, osT, distRate, osRate, param);
		distRates[distT] = distRate;
		osRates[osT] = osRate;
	}

	_saveToFile(distRates, "DistResult.r");
	_saveToFile(osRates, "OSResult.r");
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

void OTBUtils::_loadRects(string filename) {
	truthRects.clear();

	ifstream file(filename, ios::in); // 以文本模式打开in.txt备读
	if (!file) { //打开失败
		LOG("Error opening source file."); return;
	}

	int x, y, w, h;
	while (file >> x) {
		file >> y; file >> w; file >> h;
		truthRects.push_back(Rect(x, y, w, h));
	}
}

void OTBUtils::_loadFrames(string path) {
	long len = truthRects.size();

	if (frames != NULL) {
		delete frames; frames = NULL;
	}

	frames = new Mat[len];
	for (int i = 0; i < len; ++i) {
		ImageProcess::progress = i * 1.0 / len;

		char name_[12];
		sprintf(name_, "%04d.jpg", i+1);
		string filename(name_);
		filename = path + filename;

		frames[i] = imread(filename);
		//imshow("OriImage", frames[i]);
	}
}

void OTBUtils::_runRound(double distThreshold, double osThreshold, 
	double &distRate, double &osRate, ObjTrackParam *param) {

	bool newDet = true;
	Ptr<Tracker> tracker = NULL;

	long len = truthRects.size();
	long distCnt = 0, osCnt = 0;
	for (int i = 0; i < len; ++i) {
		ImageProcess::progress = i * 1.0 / len;

		auto truth = truthRects[i];
		auto frame = frames[i];

		if (i == 0) {
			param->setRect(truth); 
			ImageProcess::doObjTrack(frame, tracker, newDet, param);
			continue;
		}

		auto dist = ImageProcess::doObjTrack(frame, tracker, newDet, param);

		if (newDet) continue; // 匹配不到

		auto distRes = __testDistance(dist, truth, distThreshold);
		auto osRes = __testOS(dist, truth, osThreshold);
		if (distRes) distCnt++;
		if (osRes) osCnt++;

		//Mat drawFrame = frame.clone();

		//rectangle(drawFrame, dist, param->color, 2);
		//rectangle(drawFrame, truth, Scalar(255, 0, 0), 2);

		//imshow("OTBTest", drawFrame);
	}

	if (len > 0) {
		distRate = distCnt * 1.0 / (len-1);
		osRate = osCnt * 1.0 / (len-1);
	} else distRate = osRate = 0;
}

bool OTBUtils::__testDistance(Rect2d dist, Rect2d truth, double threshold) {
	if (threshold <= 0) return false;
	return __calcDistance(dist, truth) <= threshold;
}

double OTBUtils::__calcDistance(Rect2d dist, Rect2d truth) {
	double dx = dist.x + (dist.width / 2);
	double dy = dist.y + (dist.height / 2);
	double tx = truth.x + (truth.width / 2);
	double ty = truth.y + (truth.height / 2);
	return sqrt((tx - dx)*(tx - dx) + (ty - dy)*(ty - dy));
}

bool OTBUtils::__testOS(Rect2d dist, Rect2d truth, double threshold) {
	if (threshold <= 0) return false;
	return __calcOS(dist, truth) >= threshold;
}

double OTBUtils::__calcOS(Rect2d dist, Rect2d truth) {
	return __calcCrossSpace(dist, truth) / __calcMergeSpace(dist, truth);
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

double OTBUtils::__calcMergeSpace(Rect2d dist, Rect2d truth) {
	return dist.area() + truth.area() - __calcCrossSpace(dist, truth);
}
