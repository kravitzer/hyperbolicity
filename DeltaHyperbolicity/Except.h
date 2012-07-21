#ifndef DELTAHYPER_EXCEPT_H
#define DELTAHYPER_EXCEPT_H

#include <exception>

namespace graphs
{

	class DeltaHyperbolicityException : public std::exception
	{
	public:
		DeltaHyperbolicityException() : std::exception() {}
		DeltaHyperbolicityException(const char* what) : std::exception(what) {}
	};

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