/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_SADEFS_H
#define DELTAHYPER_SADEFS_H

#include "defs.h"
#include <memory>

namespace dhtoolkit
{
	class ISaProbabilityFunction;
	class ISaTempPolicyFunction;
	class ISaCallbackFunction;

	typedef std::shared_ptr<ISaProbabilityFunction>		sa_prob_func_ptr;
	typedef std::shared_ptr<ISaTempPolicyFunction>		sa_temp_func_ptr;
	typedef std::shared_ptr<ISaCallbackFunction>		sa_callback_func_ptr;

	typedef double			sa_temp_t;
	typedef double			sa_probability_t;

} // namespace dhtoolkit

#endif // DELTAHYPER_SADEFS_H