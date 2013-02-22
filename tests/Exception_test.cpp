#define USE_DEPRECATED_ERRNO_EXCEPTIONS
#include "os/Exception.h"

#define fail fprintf(stderr, "FAIL: %s:%d\n", __FILE__, __LINE__);

int main(const int /*argc*/, const char * const /*argv*/[]) {
	//compileTimeAssert(sizeof(int) == sizeof(char));
	compileTimeAssert(sizeof(int) == sizeof(unsigned int));
	void		*null= NULL;
	void		*notNull= &null;
	const int	noerror= 0;
	const int	error= ERANGE;

	try	{
		ThrowMessageException("Testing");
		fail
	} catch(const std::exception &exception) {
		printf("exception='%s'\n",exception.what());
	}
	try	{
		ThrowMessageExceptionIfNULL(null);
		fail
	} catch(const std::exception &exception) {
		printf("exception='%s'\n",exception.what());
	}
	try	{
		ThrowMessageExceptionIfNULL(notNull);
	} catch(const std::exception &exception) {
		printf("exception='%s'\n",exception.what());
		fail
	}
	try	{
		AssertMessageException(null == notNull);
		fail
	} catch(const std::exception &exception) {
		printf("exception='%s'\n",exception.what());
	}
	try	{
		AssertMessageException(null != notNull);
	} catch(const std::exception &exception) {
		printf("exception='%s'\n",exception.what());
		fail
	}
	try	{
		AssertCodeMessageException(error);
		fail
	} catch(const std::exception &exception) {
		printf("exception='%s'\n",exception.what());
	}
	try	{
		AssertCodeMessageException(noerror);
	} catch(const std::exception &exception) {
		printf("exception='%s'\n",exception.what());
		fail
	}
	try	{
		errnoThrowMessageException(error, "error");
		fail
	} catch(const std::exception &exception) {
		printf("exception='%s'\n",exception.what());
	}
	try	{
		errnoThrowMessageException(noerror, "no error");
		fail
	} catch(const std::exception &exception) {
		printf("exception='%s'\n",exception.what());
	}
	try	{
		errnoCodeThrowMessageException(error, "error");
		fail
	} catch(const std::exception &exception) {
		printf("exception='%s'\n",exception.what());
	}
	try	{
		errnoCodeThrowMessageException(noerror, "no error");
	} catch(const std::exception &exception) {
		printf("exception='%s'\n",exception.what());
		fail
	}
	return 0;
}
