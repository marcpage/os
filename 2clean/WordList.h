#ifndef __WordList_h__
#define __WordList_h__

#include <string>
#include "File.h"
#include "Exception.h"

class WordList {
	public:
		typedef off_t	ID;
		WordList(const char * const path);
		~WordList();
		ID lookup(const void * const word, size_t bytes);
		ID lookup(const std::string &word);
		size_t lookup(ID wordID, void *buffer, size_t bufferSize);
		std::string &lookup(ID wordID, std::string &word);
	private:
		io::File	_file;
};

inline WordList::WordList(const char * const path)
	:_file(path, io::File::Binary, io::File::WriteIfPossible) {
	const std::string	header("WordList\n");
	std::string	buffer;

	if(_file.size() == 0) {
		_file.write(header);
	} else {
		AssertMessageException(_file.read(buffer, header.size()) == header)
	}
}
inline WordList::~WordList() {
}
inline WordList::ID WordList::lookup(const void * const word, size_t bytes) {
}
inline WordList::ID WordList::lookup(const std::string &word) {
}
inline size_t WordList::lookup(WordList::ID wordID, void *buffer, size_t bufferSize) {
}
inline std::string &WordList::lookup(WordList::ID wordID, std::string &word) {
}

#endif // __WordList_h__
