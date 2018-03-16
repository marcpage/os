#ifndef __OutputStreamMirror_h__
#define __OutputStreamMirror_h__

#include "OutputStream.h"
#include "Mutex.h"
#include <vector>

namespace io {

	class OutputStreamMirror : public OutputStream {
	public:
		enum Dispose {
			DisposeWhenDone,
			DoNotDisposeWhenDone
		};
		OutputStreamMirror();
		virtual ~OutputStreamMirror();
		OutputStreamMirror &add(OutputStream *stream, Dispose dispose);
	protected:
		virtual void writeBytes(const void *buffer, int count);
		virtual void flushBytes();
		virtual void closeStream();
	private:
		struct Stream {
			OutputStream	*stream;
			bool			dispose;
		};
		typedef std::vector<Stream>	List;
		List		_streams;
		exec::Mutex	_mutex;
	};

	inline OutputStreamMirror::OutputStreamMirror(FILE *file)
			:_streams(),_mutex() {}
	inline OutputStreamMirror::~OutputStreamMirror() {
		exec::Mutex::Locker	lock(_mutex);

		try	{
			for(List::iterator stream= _streams.begin(); stream != _streams.end(); ++stream) {
				try	{
					stream->stream->close();
				} catch(const std::exception &exception) {}
				if(stream->dispose) {
					try	{
						stream->stream->join();
						delete stream;
					} catch(const std::exception &exception) {}
				}
			}
		} catch(const std::exception &exception) {}
	}
	inline OutputStreamMirror &OutputStreamMirror::add(OutputStream *stream, Dispose dispose) {
		Stream				next= {stream, dispose == DisposeWhenDone};
		exec::Mutex::Locker	lock(_mutex);

		_streams.push_back(next);
	}
	inline void OutputStreamMirror::writeBytes(const void *buffer, int count) {
		exec::Mutex::Locker	lock(_mutex);

		for(List::iterator stream= _streams.begin(); stream != _streams.end(); ++stream) {
			try	{
				stream->stream->write(buffer, count);
			} catch(const std::exception &exception) {}
		}
	}
	inline void OutputStreamMirror::flushBytes() {
		exec::Mutex::Locker	lock(_mutex);

		for(List::iterator stream= _streams.begin(); stream != _streams.end(); ++stream) {
			try	{
				stream->stream->flush();
			} catch(const std::exception &exception) {}
		}
	}
	inline void OutputStreamMirror::closeStream() {
		exec::Mutex::Locker	lock(_mutex);

		for(List::iterator stream= _streams.begin(); stream != _streams.end(); ++stream) {
			try	{
				stream->stream->close();
			} catch(const std::exception &exception) {}
		}
	}
}

#endif // __OutputStreamMirror_h__
