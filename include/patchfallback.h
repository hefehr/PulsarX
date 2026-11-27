#ifndef PATCHFALLBACK_H
#define PATCHFALLBACK_H

#include <cmath>
#include <cstddef>
#include <utility>

#include "databuffer.h"
#include "logging.h"
#include "patch.h"

namespace PulsarX
{
inline DataBuffer<float> *filter_patch_with_fallback(Patch &patch,
	DataBuffer<float> &databuffer)
{
	if (patch.filltype != "mean" && patch.filltype != "rand")
		return patch.filter(databuffer);

	auto original_buffer = databuffer.buffer;
	const bool original_equalized = databuffer.equalized;
	const bool original_mean_var_ready = databuffer.mean_var_ready;
	const bool original_isbusy = databuffer.isbusy;
	const bool original_closable = databuffer.closable;

	DataBuffer<float> *result = patch.filter(databuffer);
	std::size_t nan_count = 0;
	std::size_t infinity_count = 0;
	for (float value : result->buffer)
	{
		if (std::isnan(value))
			nan_count++;
		else if (std::isinf(value))
			infinity_count++;
	}

	if (nan_count == 0 && infinity_count == 0)
		return result;

	databuffer.buffer = std::move(original_buffer);
	databuffer.equalized = original_equalized;
	databuffer.mean_var_ready = original_mean_var_ready;
	databuffer.isbusy = original_isbusy;
	databuffer.closable = original_closable;

	const long int sample_end = databuffer.counter;
	const long int sample_begin = sample_end - databuffer.nsamples;
	BOOST_LOG_TRIVIAL(warning)
		<< "Patch filltype='" << patch.filltype << "' produced "
		<< nan_count << " NaN and " << infinity_count
		<< " infinite samples for input samples [" << sample_begin
		<< ", " << sample_end << "). Falling back to fillPatch=none "
		<< "for this block; original samples were restored.";

	return databuffer.get();
}
}

#endif
