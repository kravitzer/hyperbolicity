/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#pragma once

#include "Graph\defs.h"
#include <memory>

namespace hyperbolicity
{
	class ISaProbabilityFunction;
	class ISaTempPolicyFunction;
	class ISaCallbackFunction;

	typedef std::shared_ptr<ISaProbabilityFunction>		sa_prob_func_ptr;
	typedef std::shared_ptr<ISaTempPolicyFunction>		sa_temp_func_ptr;
	typedef std::shared_ptr<ISaCallbackFunction>		sa_callback_func_ptr;

	typedef double			sa_temp_t;
	typedef double			sa_probability_t;

} // namespace hyperbolicity
