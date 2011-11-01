#ifndef __FileAllocated_h__
#define __FileAllocated_h__

#include "Exception.h"
#include "File.h"
#include <vector>
#include <string>
#include <utility> // pair

namespace io {

	/**
		<pre>
		Signature
		0x89
		version
		\r\n
		0x1A
		\n
		<allocated header size, 4 bytes, Big Endian>
		Allocated Header
			See allocated block description
		Blocks
			Flags Byte: -128
			size, 4 bytes, big endian:		number of free bytes (not including flags or size)

			- or -

			Flags byte: negative number is the size of the empty space including this byte
						(used for empty blocks from 1 to 127 bytes including flags byte)

			- or -

			Flags byte: positive even
			size, 4 bytes, big endian: number of bytes after checksum
			checksum, 1 byte: rotated xor of all bytes
			<data>
		</pre>
		@todo Add free size, block count, allocated block count, compressed block count
	*/
	class FileAllocated {
		public:
			class Allocation {
				public:
					Allocation();
					Allocation after() const;
					Allocation before() const;
					Allocation &increment();
					Allocation &decrement();
					Allocation &operator++();
					Allocation operator++(int);
					Allocation &operator--();
					Allocation operator--(int);
					Allocation(const Allocation &other);
					Allocation &operator=(const Allocation &other);
					off_t identifier() const;
					size_t size() const;
					std::string &read(std::string &buffer);
					void write(std::string &buffer);
					bool resize(size_t newSize) const;
					bool flag(uint8_t bit) const;
					bool flag(uint8_t bit, bool newValue);
				protected:
					FileAllocated	*_file;
					off_t			_offset;
					Allocation(FileAllocated *file, off_t offset);
			};
			enum Errors {
				ChecksumFailed,
			};
			typedef std::pair<Allocation, Errors>	Error;
			typedef std::vector<Error>				ErrorList;
			FileAllocated(const std::string &path, uint32_t headerSize= 0, uint32_t version= 1, const std::string &signature= "BAFF");
			~FileAllocated();
			Allocation header();
			Allocation allocate(const std::string &data, bool allowGrowth);
			Allocation allocate(size_t size, bool allowGrowth);
			Allocation find(off_t identifier);
			Allocation begin();
			Allocation end();
			/// Size of the file on disk
			off_t size();
			/// Size of the data saved in the file
			off_t data();
			bool validate(ErrorList &errors);
		private:
			File		_file;
			off_t		_dataSize;
			off_t		_header;
			off_t		_first;
			std::string	_signature;
			uint8_t		_version;
			FileAllocated(const FileAllocated&); ///< Prevent Usage
			FileAllocated &operator=(const FileAllocated&); ///< Prevent Usage
			bool _readBlockHeader(off_t offset, size_t *dataSize, size_t *compressedSize, bool *compressed, off_t *data, off_t *next, int8_t *flags, uint8_t *checksum, uint8_t *compressedChecksum);
			bool _previousHeader(off_t &offset, size_t *dataSize, size_t *compressedSize, bool *compressed, off_t *data, off_t *next, int8_t *flags, uint8_t *checksum, uint8_t *compressedChecksum);
			void _writeBlock(off_t offset, const std::string &data, int8_t flags);
			static uint8_t _checksum(const std::string &data);
	};

	inline FileAllocated::Allocation::Allocation()
		:_file(NULL), _offset(static_cast<off_t>(-1)) {}
	inline FileAllocated::Allocation FileAllocated::Allocation::after() const {
		return Allocation(_file, _offset).increment();
	}
	inline FileAllocated::Allocation FileAllocated::Allocation::before() const {
		return Allocation(_file, _offset).decrement();
	}
	/**
		@todo Implement
	*/
	inline FileAllocated::Allocation &FileAllocated::Allocation::increment() {
	}
	/**
		@todo Implement
	*/
	inline FileAllocated::Allocation &FileAllocated::Allocation::decrement() {
	}
	inline FileAllocated::Allocation &FileAllocated::Allocation::operator++() {
		return increment();
	}
	inline FileAllocated::Allocation FileAllocated::Allocation::operator++(int) {
		Allocation	value(*this);
		increment();
		return value;
	}
	inline FileAllocated::Allocation &FileAllocated::Allocation::operator--() {
		return decrement();
	}
	inline FileAllocated::Allocation FileAllocated::Allocation::operator--(int) {
		Allocation	value(*this);
		decrement();
		return value;
	}
	inline FileAllocated::Allocation::Allocation(const Allocation &other)
		:_file(other._file), _offset(other._offset) {}
	inline FileAllocated::Allocation &FileAllocated::Allocation::operator=(const Allocation &other) {
		_file= other._file;
		_offset= other._offset;
	}
	inline off_t FileAllocated::Allocation::identifier() const {
		return _offset;
	}
	/**
		@todo Implement
	*/
	inline size_t FileAllocated::Allocation::size() const {
	}
	/**
		@todo Implement
	*/
	inline std::string &FileAllocated::Allocation::read(std::string &buffer, uint32_t offset, uint32_t size) {
	}
	/**
		@todo Implement
	*/
	inline void FileAllocated::Allocation::write(std::string &buffer, uint32_t offset) {
	}
	/**
		@todo Implement
	*/
	inline bool FileAllocated::Allocation::resize(size_t newSize) const {
	}
	/**
		@todo Implement
	*/
	inline bool FileAllocated::Allocation::flag(uint8_t bit) const {
	}
	/**
		@todo Implement
	*/
	inline bool FileAllocated::Allocation::flag(uint8_t bit, bool newValue) {
	}
	inline FileAllocated::Allocation::Allocation(FileAllocated *file, off_t offset)
		:_file(file), _offset(offset) {}
	/**
		@todo Implement empty file case
	*/
	inline FileAllocated::FileAllocated(const std::string &path, uint32_t headerSize, uint32_t version, const std::string &signature)
		:_file(path, File::Binary, File::WriteIfPossible), _dataSize(0), _header(0), _first(0), _signature(signature), _version(version) {
		const size_t			kMinimumAllocatedHeaderSize= sizeof(int8_t) + sizeof(uint32_t) + sizeof(uint8_t);
		std::string				header;
		std::string::size_type	endOfSignature, endOfHeader;
		const size_t			kMaxHeaderSize= 1024;
		const char				kEndOfSignatureMarker= '\0x89';
		const char				kCorruptionMarker[]= {'\0x0D','\0x0A', '\0x1A', '\0x0A', '\0'};

		if(_file.size() > 0) {
			_file.read(header, kMaxHeaderSize, 0, File::FromStart);
			endOfSignature= header.find(kEndOfSignatureMarker);
			if(std::string::npos == endOfSignature) {
				ThrowMessageException(std::string("Corrupt File: ")+path); // no end of signature marker found
			}
			_signature.assign(header, 0, endOfSignature);
			_version= _file.read<uint8_t>(File::NativeEndian, endOfSignature + 1, File::FromStart);
			endOfHeader= endOfSignature + 1;
			if(header.find(kCorruptionMarker, endOfHeader) != 0) {
				ThrowMessageException(std::string("Corrupt File: ")+path);	// End of Line conversion happened
			}
			endOfHeader+= sizeof(kCorruptionMarker) - 1; // -1 for EOS \0
			_dataSize= _file.read<uint64_t>(File::BigEndian, endOfHeader, File::FromStart);
			endOfHeader+= sizeof(uint64_t);
			endOfHeader+= sizeof(uint64_t);
			_header= endOfHeader;
			headerSize= _file.read<uint32_t>(File::BigEndian);
			_first= endOfHeader + sizeof(uint32_t) + headerSize + kMinimumAllocatedHeaderSize;
		} else {
			_file.write(_signature);
			_file.write(kEndOfSignatureMarker, File::BigEndian);
			_file.write<uint8_t>(_version, File::BigEndian);
			_file.write(kCorruptionMarker);
			_file.write<uint64_t>(0, File::BigEndian); // data size
			_header= _file.location();
			_file.write<uint32_t>(headerSize, File::BigEndian);
			_first= _header + sizeof(uint32_t) + headerSize;
		}
	}
	inline FileAllocated::~FileAllocated() {}
	/**
		@todo Implement
	*/
	inline FileAllocated::Allocation FileAllocated::header() {
		return Allocation(this, _header);
	}
	/**
		@param allowGrowth	If true, the file can grow in size
		@todo Implement
	*/
	inline FileAllocated::Allocation FileAllocated::allocate(const std::string &data, bool allowGrowth) {
	}
	/**
		@todo Implement
	*/
	inline FileAllocated::Allocation FileAllocated::allocate(size_t size, bool allowGrowth) {
	}
	inline FileAllocated::Allocation FileAllocated::find(off_t identifier) {
		return Allocation(this, identifier);
	}
	/**
		@todo Implement
	*/
	inline FileAllocated::Allocation FileAllocated::begin() {
	}
	inline FileAllocated::Allocation FileAllocated::end() {
		return Allocation(this, static_cast<off_t>(-1));
	}
	inline off_t FileAllocated::size() {
		return _file.size();
	}
	/**
		@todo Implement
	*/
	inline off_t FileAllocated::data() {
	}
	/**
	*/
	inline bool FileAllocated::validate(ErrorList &errors) {
		off_t		dataOffset, next, block= _first;
		size_t		dataSize, compressedSize;
		uint8_t		checksum, compressedChecksum;
		off_t		filesize= _file.size();
		std::string	data;
		off_t		totalDataSize= 0;

		while(_readBlockHeader(block, &dataSize, &compressedSize, NULL, NULL, &dataOffset, &next, NULL, &checksum, &compressedChecksum)
				&& (next > _first) && (next <= filesize) ) {
			_file.read(data, compressedSize, dataOffset, File::FromStart);
			if(compressedChecksum != _checksum(data)) {
				errors.push_back(Error(Allocation(this, block), RawChecksumFailed));
			}
			totalDataSize+= dataSize;
			block= next;
		}
		if(_dataSize != totalDataSize) {
			_dataSize= totalDataSize;	// correct the problem
			_file.write<uint64_t>(_dataSize, File::BigEndian, _header - sizeof(uint64_t) - sizeof(uin64_t), File::FromStart);
			errors.push_back(Error(Allocation(this, static_cast<off_t>(-1)), UncompressedSizeIncorrect));
		}
	}
	/**
		@param offset				Offset of the block header
		@param dataSize				Receives the size of the data in the block. If this is a compressed
										block, receives the uncompressed size. For a free block
										receives the size of uncompressed data that could be stored here.
		@param compressedSize		Receives the size of the data compressed. If not compressed this
										is the same as <code>dataSize</code>. For a free block
										receives the size of uncompressed data that could be stored here.
		@param compressed			Set to true if the data is compressed
		@param free					Set to true if the block is a free block
		@param data					The offset of the data in the file
		@param next					The offset in the file of the next header (or the size of the file)
		@param flags				The actual flags for the block. bit #1 and #8 are reserved and should
										not be relied on (they are compressed and free respectively).
		@param checksum				A Rotate Left, Xor of all the uncompressed bytes in the block.
		@param compressedChecksum	A Rotate Left, Xor of all the raw bytes in the block.
		@return						true if we are still within the
	*/
	inline bool FileAllocated::_readBlockHeader(off_t offset, size_t *dataSize, size_t *compressedSize, bool *compressed, bool *free, off_t *data, off_t *next, int8_t *flags, uint8_t *checksum, uint8_t *compressedChecksum) {
		const int8_t	kFreeBlockFlags= -128;
		const int8_t	kCompressedBit= 0x01;
		const size_t	kMinimumAllocatedHeaderSize= sizeof(int8_t) + sizeof(uint32_t) + sizeof(uint8_t);
		const size_t	kLargeFreeBlockHeaderSize= sizeof(int8_t) + sizeof(uint32_t);
		size_t			dsBuffer, csBuffer;
		bool			cBuffer, fBuffer;
		off_t			dBuffer, nBuffer;
		int8_t			gBuffer, sBuffer, kBuffer;

		*flags= _file.read<int8_t>(offset, File::BigEndian);
		if(NULL == dataSize) {dataSize= &dsBuffer;}
		if(NULL == compressedSize) {compressedSize= &csBuffer;}
		if(NULL == compressed) {compressed= &cBuffer;}
		if(NULL == free) {free= &cBuffer;}
		if(NULL == data) {data= &dBuffer;}
		if(NULL == next) {next= &nBuffer;}
		if(NULL == flags) {flags= &gBuffer;}
		if(NULL == checksum) {checksum= reinterpret_cast<uint8_t>(&sBuffer;}
		if(NULL == compressedChecksum) {compressedChecksum= reinterpret_cast<uint8_t>(&kBuffer;}
		*free= (kFreeBlockFlags == *flags);
		if(*free) {
			*dataSize= _file.read<uint32_t>(File::BigEndian);
			--*dataSize; // account for checksum in an allocated block
			*compressedSize= *dataSize;
			*compressed= false;
			*data= offset + kLargeFreeBlockHeaderSize;
			*next= *data + *dataSize;
			*checksum= 0;
			*compressedChecksum= 0;
		} else if(*flags < 0) {
			*free= true; // single byte free size for small items
			*dataSize= *flags * -1;
			if(*dataSize < kMinimumAllocatedHeaderSize) {
				*dataSize= 0;
			} else {
				*dataSize-= kMinimumAllocatedHeaderSize;
			}
			*compressedSize= *dataSize;
			*compressed= false;
			*data= offset + sizeof(int8_t);
			*next= offset + (*flags * -1);
			*checksum= 0;
			*compressedChecksum= 0;
		} else {
			*compressedSize= _file.read<uint32_t>(File::BigEndian);
			*compressedChecksum= _file.read<uint8_t>(File::BigEndian);
			*compressed= ( (flags & kCompressedBit) == kCompressedBit );
			*dataSize= *compressed ? _file.read<uint32_t>(File::BigEndian) : *compressedSize;
			*checksum= *compressed ? _file.read<uint8_t>(File::BigEndian) : *compressedChecksum;
		}
		return *next <= _file.size();
	}
	/**
		@param offset				in: offset of the block header, out: offset of the previous block (if found)
		@param dataSize				Receives the size of the data in the block. If this is a compressed
										block, receives the uncompressed size. For a free block
										receives the size of uncompressed data that could be stored here.
		@param compressedSize		Receives the size of the data compressed. If not compressed this
										is the same as <code>dataSize</code>. For a free block
										receives the size of uncompressed data that could be stored here.
		@param compressed			Set to true if the data is compressed
		@param free					Set to true if the block is a free block
		@param data					The offset of the data in the file
		@param next					The offset in the file of the next header (or the size of the file)
		@param flags				The actual flags for the block. bit #1 and #8 are reserved and should
										not be relied on (they are compressed and free respectively).
		@param checksum				A Rotate Left, Xor of all the uncompressed bytes in the block.
		@param compressedChecksum	A Rotate Left, Xor of all the raw bytes in the block.
		@return						true if found a previous block
	*/
	inline bool FileAllocated::_previousHeader(off_t &offset, size_t *dataSize, size_t *compressedSize, bool *compressed, off_t *data, off_t *next, int8_t *flags, uint8_t *checksum, uint8_t *compressedChecksum) {
		off_t	dBuffer, nBuffer;
		off_t	previous= offset - 1;

		if(NULL == next) {next= &nBuffer;}
		while( (previous >= _first)
				&& !__readBlockHeader(previous, dataSize, compressedSize, compressed, data, next, checksum, compressedChecksum)
				&& (*next != offset) ) {
			--previous;
		}
		if(*next != offset) {
			return false;
		}
		offset= previous;
		return true;
	}
	/**
		@todo implement compressed blocks
	*/
	inline void FileAllocated::_writeBlock(off_t offset, const std::string &data, int8_t flags) {
		const int8_t	kCompressedFlag= 0x01;
		// try compression and see if it saves more than sizeof(uint32_t) + sizeof(uint8_t)
		// if compression is better, use compressed string and add compression to flags
		// if compression is not better, make sure compressed flag is cleared
		flags&= ~kCompressedFlag;
		_file.write(flags, File::BigEndian, offset, File::FromStart);
		_file.write<uint32_t>(data.size(), File::BigEndian);
		_file.write<uint8_t>(_checksum(data), File::BigEndain);
		_file.write(data);
	}
	inline uint8_t FileAllocated::_checksum(const std::string &data) {
		uint8_t	value= 0;

		for(std::string::size_type c= 0; c < data.size(); ++c) {
			uint8_t	character= *reinterpret_cast<uint8_t*>(&data[c]);
			uint8_t	wrap= (((value << 1) >> 1) != value) ? 0x01 : 0x00;

			value= ((value << 1) | wrap) ^ character;
		}
		return value;
	}
}

#endif __FileAllocated_h__
