#ifndef __FileBlocks_h__
#define __FileBlocks_h__

#include <vector>

namespace io {

	class BlockedFile : public File {
		public:
			BlockedFile(const char *path, Protection protection);
			BlockedFile(const std::string &path, Protection protection);
			~BlockedFile();
			off_t allocate(const void *buffer, size_t bytes, int8_t flags= 0);
			off_t allocate(const std::string &buffer, int8_t flags= 0);
			void free(off_t block);
			size_t sizeOf(off_t block);
			int8_t flagsOf(off_t block);
			BlockedFile &setFlags(off_t block, int8_t flags);
		private:
			struct _Block {
				off_t	address;
				bool	free;
				size_t	size;
				_Block(off_t a, bool f, size_t s):address(a),free(f),size(s) {}
			};
			typedef std::vector<_Block>	_BlockList;
			_BlockList	_blocks;
			void _fetchBlocks();
			off_t _findFreeBlock(size_t bytes);
			void _markFree(off_t block, size_t bytes);
			void _splitFreeBlock(off_t block, size_t dataSize);
	};

	inline BlockedFile::BlockedFile(const char *path, Protection protection)
		:File(path, Binary, protection), _blocks() {
		_fetchBlocks();
	}
	inline BlockedFile::BlockedFile(const std::string &path, Protection protection)
		:File(path, Binary, protection), _blocks() {
		_fetchBlocks();
	}
	inline BlockedFile::~BlockedFile() {
	}
	inline off_t BlockedFile::allocate(const void *buffer, size_t bytes, int8_t flags) {
		const off_t		start= location();
		const uint8_t	kMaxSingleByteFreeBlock= 0x7F;
		const uint8_t	kFreeBlockBit= 0x80;

		for(_BlockList::iterator block= _blocks.begin(); block != _blocks.end(); ++block) {
			if(block->free && block->size >= bytes) {
				block->free= false;
				moveto(block->address);
				write<uint8_t>(flags & ~kFreeBlockBit, BigEndian);
				write<uint32_t>(bytes, BigEndian);
				write(buffer, bytes);
				if(bytes < block->size) {
					const size_t	oldSize= block->size;

					if(bytes - block->size < kMaxSingleByteFreeBlock) {
						const uint8_t	flags= static_cast<uint8_t>(kFreeBlockBit | bytes - block->size);

						write<uint8_t>(flags);
					} else if() {
					}

				}
			}
		}
		moveto(start);
	}
	inline off_t BlockedFile::allocate(const std::string &buffer, int8_t flags) {
		return allocate(buffer.data(), buffer.size(), flags);
	}
	inline void BlockedFile::free(off_t block) {
		const uint8_t	kFreeBlockFullSizeFlags= 0xFF;

		write<uint8_t>(kFreeBlockFlagsBit, BigEndian, Block, FromStart);
		for(_BlockList::iterator block= _blocks.begin(); block != _blocks.end(); ++block) {
			if(block->address == block) {
				block->free= true;
				return;
			}
		}
		// We failed to find the block in the list, reload the list
		_fetchBlocks();
	}
	inline size_t BlockedFile::sizeOf(off_t block) {
		uint8_t	flags= read<uint8_t>(BigEndian, block, FromStart);

		if(flags < 0) {
			return 0; // free block
		}
		return static_cast<size_t>(read<uint32_t>(BigEndian, block+1, FromStart));
	}
	inline int8_t BlockedFile::flagsOf(off_t block) {
		return read<int8_t>(BigEndian, block, FromStart);
	}
	inline BlockedFile &BlockedFile::setFlags(off_t block, int8_t flags) {
		const uint8_t	kValidUserFlagsMask= 0x7F;

		write<uint8_t>(flags & kValidUserFlagsMask, BigEndian, block, FromStart);
		return *this;
	}
	void BlockedFile::_fetchBlocks() {
		const off_t		end= size();
		const uint8_t	kFreeBlockFlagsBit= 0x80;
		const uint8_t	kFreeBlockFullSizeFlags= 0xFF;

		_blocks.clear();
		moveto(0);
		while(true) {
			const off_t		block= location();
			const uint8_t	flags= read<uint8_t>(BigEndian);
			const bool		free= (flags & kFreeBlockFlagsBit) != 0;
			const bool		shortSize= free && (flags != kFreeBlockFullSizeFlags);
			const uint32_t	bytes= shortSize ? (flags & ~kFreeBlockFlagsBit) : read<uint32_t>(BigEndian);
			const size_t	entireBlockSize= sizeof(uint8_t) + (shortSize ? 0 : sizeof(uint32_t)) + bytes;
			const size_t	allocatedBlockHeaderSize= sizeof(uint8_t) + sizeof(uint32_t);
			const size_t	allocatableBytes= entireBlockSize > allocatedBlockHeaderSize ? entireBlockSize - allocatedBlockHeaderSize : 0;

			_blocks.push_back(_Block(block, free, allocatableBytes));
			if(block + entireBlockSize >= end) {
				break;
			}
			move(bytes);
		}
	}

}

#endif // __FileBlocks_h__
