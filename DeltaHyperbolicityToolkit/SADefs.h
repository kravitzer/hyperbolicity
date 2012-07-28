/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_SADEFS_H
#define DELTAHYPER_SADEFS_H

#include "defs.h"
#include <memory>

namespace graphs
{
	class ISaProbabilityFunction;
	class ISaTempPolicyFunction;

	typedef std::shared_ptr<ISaProbabilityFunction>		sa_prob_func_ptr;
	typedef std::shared_ptr<ISaTempPolicyFunction>		sa_temp_func_ptr;

	typedef double			sa_temp_t;
	typedef double			sa_probability_t;

} // namespace graphs

#endif // DELTAHYPER_SADEFS_H