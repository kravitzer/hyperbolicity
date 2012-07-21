/*
 * Written by Eran Kravitz
 * Email: kravitzer@gmail.com
 */

#ifndef DELTAHYPER_EXCEPT_H
#define DELTAHYPER_EXCEPT_H

#include <exception>

namespace graphs
{
	/* 
	 * @brief	A general delta hyperbolicity exception.
	 */
	class DeltaHyperbolicityException : public std::exception
	{
	public:
		DeltaHyperbolicityException() : std::exception() {}
		DeltaHyperbolicityException(const char* what) : std::exception(what) {}
	};

	/* the rest of these exceptions are pretty self explanatory... */

	class OutOfBoundsException : public DeltaHyperbolicityException
	{
	public:
		OutOfBoundsException() : DeltaHyperbolicityException() {}
		OutOfBoundsException(const char* what) : DeltaHyperbolicityException(what) {}
	};

	class InvalidParamException : public DeltaHyperbolicityException
	{
	public:
		InvalidParamException() : DeltaHyperbolicityException() {}
		InvalidParamException(const char* what) : DeltaHyperbolicityException(what) {}
	};

	class InvalidFormatException : public DeltaHyperbolicityException
	{
	public:
		InvalidFormatException() : DeltaHyperbolicityException() {}
		InvalidFormatException(const char* what) : DeltaHyperbolicityException(what) {}
	};

} // namespace graphs

#endif // DELTAHYPER_EXCEPT_H