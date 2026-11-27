#include "config.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "databuffer.h"
#include "patch.h"
#include "patchfallback.h"

unsigned int num_threads = 1;

namespace
{
bool all_finite(const DataBuffer<float> &data)
{
	return std::all_of(data.buffer.begin(), data.buffer.end(),
		[](float value) { return std::isfinite(value); });
}
}

int main()
{
	DataBuffer<float> bad_data(4, 2);
	bad_data.counter = bad_data.nsamples;
	bad_data.buffer = {1.F, 1.F, 2.F, 2.F, 3.F, 3.F, 4.F, 4.F};
	const auto original_bad_buffer = bad_data.buffer;

	Patch bad_patch;
	bad_patch.filltype = "mean";
	bad_patch.prepare(bad_data);
	DataBuffer<float> *bad_result =
		PulsarX::filter_patch_with_fallback(bad_patch, bad_data);
	if (bad_result != bad_data.get() ||
		bad_data.buffer != original_bad_buffer || !all_finite(bad_data))
	{
		std::cerr << "all-masked patch block was not restored" << std::endl;
		return 1;
	}

	DataBuffer<float> good_data(4, 2);
	good_data.counter = good_data.nsamples;
	good_data.buffer = {1.F, 2.F, 2.F, 4.F, 3.F, 6.F, 4.F, 8.F};
	const auto original_good_buffer = good_data.buffer;

	Patch good_patch;
	good_patch.filltype = "mean";
	good_patch.prepare(good_data);
	DataBuffer<float> *good_result =
		PulsarX::filter_patch_with_fallback(good_patch, good_data);
	if (good_result != good_data.get() ||
		good_data.buffer != original_good_buffer || !all_finite(good_data))
	{
		std::cerr << "valid patch block changed unexpectedly" << std::endl;
		return 1;
	}

	return 0;
}
