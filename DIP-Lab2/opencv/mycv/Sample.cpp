#pragma once
#include <vector>

namespace mycv {
	using namespace std;

	struct Sample_ {
		vector<float>	data;
		float			label;
	};

	typedef			Sample_		Sample;
	typedef			Sample_&	SampleRef;
	typedef const	Sample_&	SampleRefC;
}
