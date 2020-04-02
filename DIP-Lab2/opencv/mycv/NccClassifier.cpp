#pragma once
#include <opencv2/opencv.hpp>
//#include <opencv2/legacy/legacy.hpp>
#include <vector>	
#include <list>
#include "Img.cpp"
#include "BoundingBox.cpp"
#include <omp.h>

#define DEBUG_NCC_CLASSIFIER 1

namespace mycv
{
	using namespace cv;
	using namespace std;

	struct NccSample_
	{
		Mat				data;
		float			label;
		NccSample_&		setLabel(float lbl){label=lbl;return (*this);}
	};

	typedef NccSample_				NccSample;
	typedef NccSample&				NccSampleRef;
	typedef const NccSample&		NccSampleRefC;

	class NccExtractor
	{
		struct {
			vector<Mat> resized;
			vector<Mat> converted;		
			vector<Mat> normed;
		} _cache;
	public:
		typedef NccSample sample_type;
					NccExtractor();
		sample_type calc(ImgTRefC img, BBRefC bb);
	};

	inline NccExtractor::NccExtractor() 
	{
		_cache.resized.resize(omp_get_max_threads());
		_cache.converted.resize(omp_get_max_threads());
		_cache.normed.resize(omp_get_max_threads());
	}

	inline NccExtractor::sample_type NccExtractor::calc(ImgTRefC img, BBRefC bb)
	{
		static const auto sampleSize =  Size(15,15);
		int threadId = omp_get_thread_num();
		Mat& resized	= _cache.resized[	threadId ];
		Mat& converted = _cache.converted[ threadId ];
		Mat& normed	= _cache.normed[	threadId ];
		sample_type sample;
		cv::resize( img.img(bb), resized, sampleSize);
		resized.convertTo(converted, CV_32F);
	
		//Scalar mean, stddev; meanStdDev(converted, mean, stddev);		
		//normed			= (converted - mean[0])/stddev[0];
	
		Scalar avg		= cv::mean(converted);		
		normed			= converted - avg[0];
		
		sample.data		= normed.reshape(1, 1).clone();
		return sample;
	}



	class NccClassifier
	{
		list<Mat>	_positiveData;
		list<Mat>	_negativeData;

		//'thr_nn',0.65,'thr_nn_valid',0.7
		float _treshNN;
		float _treshNNValid;
		float _treshNNTheSame;

		#if DEBUG_NCC_CLASSIFIER
		struct {
			Mat posImg;
			Mat negImg;
		} dbg;
		#endif

	public:
	
				NccClassifier();
		void	train(NccSampleRefC smpl);
		float	predict(NccSampleRefC smpl, float& cConf, float& aConf, int& in, int& idx);
		void	show();

	};//class NccClassifier

	inline NccClassifier::NccClassifier()
		: _treshNN(0.65f), _treshNNValid(0.70f), _treshNNTheSame(0.90f)		
	{ 
	}
	

	inline void NccClassifier::train(NccSampleRefC smpl)
	{
		/* measure sample agains model */
		float conf2(0.0f), conf3(0.0f); int in(-1), idx(-1);
		float conf1 = this->predict(smpl, conf2, conf3, in, idx);

		if(smpl.label == 1.0f && conf1 <= _treshNN){
			if(idx <0) { _positiveData.clear(); }			
			_positiveData.push_back(smpl.data);
		} 

		if (smpl.label == 0.0f && conf1 > 0.5f){
			_negativeData.push_back(smpl.data);
		}
	}

	inline float NccClassifier::predict(NccSampleRefC smpl, float& cConf, float& aConf, int& in, int& idx)
	{
		float rConf(0.0f); in=idx=(-1);
		if(_positiveData.empty()){
			aConf = rConf = cConf = 0.0f;
			return rConf;
		}

		if(_negativeData.empty()){
			aConf = rConf = cConf = 1.0f;
			return rConf;
		}
		

		int n_pos = _positiveData.size();
		int n_neg = _negativeData.size();

		Mat_<float> r;
		vector<float> pr;
		for_each(_positiveData.begin(), _positiveData.end(), [&pr, &smpl, &r](const Mat& psmpl){
			matchTemplate(psmpl, smpl.data, r, TM_CCORR_NORMED);
			pr.push_back( *r.ptr<float>(0) );
		});
		vector<float> nr;
		for_each(_negativeData.begin(), _negativeData.end(), [&nr, &smpl, &r](const Mat& nsmpl){
			matchTemplate(nsmpl, smpl.data, r, TM_CCORR_NORMED);
			nr.push_back( *r.ptr<float>(0) );
		});


		auto Pi = max_element(begin(pr),end(pr));
		auto Ni = max_element(begin(nr),end(nr));

		float P = 1.0f - *Pi;
		float N = 1.0f - *Ni;

		/* relative conf */
		rConf  = N / (N+P);

		if(P > _treshNNTheSame )
			in = 1;

		if(N > _treshNNTheSame )
			in = 0;

		idx = Pi - begin(pr);

		auto Pi2 = max_element(begin(pr),begin(pr)+pr.size()/2);
		float P2 = 1.0f - *Pi2;

		/* conservative conf */
		cConf = N / (N+P2);
		
		return rConf;
	}

	inline void NccClassifier::show()
	{
		#if DEBUG_NCC_CLASSIFIER
		{
			//#pragma omp parallel
			{
				{
					int N_POS = (int)_positiveData.size();;
					int ROWS = 10;
					int COLS = N_POS/10 + (N_POS % 10 > 0);
					dbg.posImg.create( 15 * ROWS, 15 * COLS, CV_8U);
					dbg.posImg.setTo(0);
					
					int i=0;
					for(auto it = _positiveData.begin(); it != _positiveData.end(); it++, i++) {		
						int col		= i / ROWS; int row = i % ROWS;
						Mat slot	= dbg.posImg(Rect(col*15, row*15,15,15));
						Mat d		= (*it).reshape(1,15);
						normalize(d, slot, 0.0, 255.0, NORM_MINMAX, CV_8U);					
					}					
				}
				{
					int N_NEG = (int)_negativeData.size();
					int ROWS = 10;
					int COLS = N_NEG/10 + (N_NEG % 10 > 0);
					COLS = max(COLS, 1);
					dbg.negImg.create( 15 * ROWS, 15 * COLS, CV_8U);
					dbg.negImg.setTo(0);
					int i=0;
					for(auto it = _negativeData.begin(); it != _negativeData.end(); it++, i++) {
						int col		= i / ROWS; int row = i % ROWS;
						Mat slot	= dbg.negImg(Rect(col*15, row*15,15,15));
						Mat d		= (*it).reshape(1,15);
						normalize(d, slot, 0.0, 255.0, NORM_MINMAX, CV_8U);						
					}
				}
			}
			imshow("POSITIVE PATCHES", dbg.posImg);
			imshow("NEGATIVE PATCHES", dbg.negImg);
			waitKey(5);
		}
		#endif
	}

} //namespace mycv

