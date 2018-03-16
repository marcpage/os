#ifndef __OutputStream_h__
#define __OutputStream_h__

#include "Thread.h"
#include "Queue.h"
#include <string>
#include <stdarg.h>

namespace io {

	class OutputStream : public exec::Thread {
	public:
		OutputStream();
		virtual ~OutputStream();
		void write(const void *buffer, int count);
		void write(const std::string &buffer);
		void write(const char *string);
		void printf(size_t max, const char *format, ...);
		void vprintf(size_t max, const char *format, va_list ap);
		void flush();
		void close();
	protected:
		virtual void *run();
		virtual void writeBytes(const void *buffer, int count)= 0;
		virtual void flushBytes()= 0;
		virtual void closeStream()= 0;
	private:
		exec::Queue<std::string>	_data;
		bool						_running;
	};

	inline OutputStream::OutputStream()
			:_data(),_running(true) {
		start();
	}
	inline OutputStream::~OutputStream() {
		try	{
			flush();
		} catch(const std::exception &exception) {}
		try	{
			close();
		} catch(const std::exception &exception) {}
	}
	inline void OutputStream::write(const void *buffer, int count) {
		write(std::string(reinterpret_cast<const char*>(buffer), count);
	}
	inline void OutputStream::write(const std::string &buffer) {
		_data.enqueue(buffer);
	}
	inline void OutputStream::write(const char *string) {
		write(string, strlen(string));
	}
	inline void OutputStream::printf(size_t max, const char *format, ...) {
		va_list	args;

		va_start(args, format);
		vprintf(max, format, args);
		va_end(args);
	}
	inline void OutputStream::vprintf(size_t max, const char *format, va_list ap) {
		std::string	buffer(max, '\0');
		int			count;

		count= vsnprintf(const_cast<char*>(buffer.data()), max, format, ap);
		buffer.erase(count);
		write(buffer);
	}
	inline void OutputStream::flush() {
		while(!_data.empty()) {
			exec::Thread::sleep(1, exec::Thread::Milliseconds);
		}
		write("");
	}
	inline void OutputStream::close() {
		flush();
		_running= false;
		_data.close();
	}
	inline void *run() {
		try	{
			while(_running) {
				std::string	value;

				value= data.dequeue();
				if(data.empty() || (0 == value.size()) ) {
					flushBytes();
				}
				writeBytes(value.data(), value.size());
			}
		} catch(const std::exception &exception) {}
		try	{
			flushBytes();
		} catch(const std::exception &exception) {}
		try	{
			closeStream();
		} catch(const std::exception &exception) {}
	}
}

#endif // __OutputStream_h__
