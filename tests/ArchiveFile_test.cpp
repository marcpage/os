#include "os/ArchiveFile.h"
#include <stdio.h>

// clang++ ArchiveFile_test.cpp -I .. -o /tmp/test -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings
// /tmp/test | grep ArchiveFile.h | sort | uniq | wc -l
// rm /tmp/*.archive
// 133

int blockCount(io::ArchiveFile::Block begin, io::ArchiveFile::Block end) {
	int		count= 0;
	int64_t	size= 0;

	while(begin != end) {
		size+= begin.size();
		count++;
		begin++;
	}
	return count;
}

void printBlocks(io::ArchiveFile::Block begin, io::ArchiveFile::Block end) {
	while(begin != end) {
		printf("%d[%d] %s '%s'\n",static_cast<int>(begin.identifier()),
			static_cast<int>(begin.size(false)),
			begin.free() ? "free" : "aloc",
			begin.free() ? "" : begin.read().c_str()
		);
		begin++;
	}
}

#define testBlocks(file, count) \
	if(blockCount(file.begin(), file.end()) != static_cast<int>(count)) { \
		printf("Unexpected blockcount, %d instead of %d (line %d)\n", \
			blockCount(file.begin(), file.end()), \
			static_cast<int>(count),\
			__LINE__ \
		); \
	}

int main(int,const char*[]) {
	try	{
		struct {
			uint64_t	identifier;
			const char	*value;
		} positions[]= {
			{0, "test1"},
			{0, "test2"},
			{0, "help"},
		};
		{
			io::ArchiveFile	file("/tmp/file1.archive");
			int64_t			lastHeaderSize= -1;

			testBlocks(file, 1);
			for(unsigned int i= 0; i < sizeof(positions)/sizeof(positions[0]); ++i) {
				io::ArchiveFile::Block	b= file.allocate(strlen(positions[i].value), i);
				int64_t					headerSize1, headerSize2;

				testBlocks(file, 2 + i);
				headerSize1= b.size(false) - b.size(true);
				headerSize2= b.offset(true) - b.offset(false);
				if(headerSize1 != headerSize2) {
					printf("Headersize calulcations mismatched\n");
				}
				if(lastHeaderSize != -1) {
					if(lastHeaderSize != headerSize1) {
						printf("Headersize varies from block to block\n");
					}
				}
				lastHeaderSize= headerSize1;
				file.write(positions[i].value, strlen(positions[i].value), b, b);
				positions[0].identifier= b.identifier();
			}
		}
		{
			io::ArchiveFile	file("/tmp/file1.archive");
			std::string		value;

			testBlocks(file, sizeof(positions)/sizeof(positions[0])+1);
			for(io::ArchiveFile::Block b= file.begin(); b; ++b) {
				if(!b.free()) {
					file.read(value, b.size(), b, b);
					if(value != positions[b.flags()].value) {
						printf("Block mismatch\n");
					}
				}
			}
		}
		{
			io::File	file("/tmp/short.archive", io::File::Binary, io::File::WriteIfPossible);

			file.write("Test");
		}
		try	{
			io::ArchiveFile	file("/tmp/short.archive");

			printf("File was too short and we still opened it\n");
		} catch(const posix::err::ERANGE_Errno &error) {
			// expected
		}
		{
			io::File	file("/tmp/big.archive", io::File::Binary, io::File::WriteIfPossible);

			file.write("This is a test of how reading an archive file that is big enough but has a bad header");
		}
		try	{
			io::ArchiveFile	file("/tmp/big.archive");

			printf("File had wrong signature and we still opened it\n");
		} catch(const posix::err::EILSEQ_Errno &error) {
			// expected
		}
		try	{
			io::ArchiveFile	file("/tmp/file1.archive", io::File::WriteIfPossible, 2);

			printf("File had wrong version and we still opened it\n");
		} catch(const posix::err::EILSEQ_Errno &error) {
			// expected
		}
		{
			io::ArchiveFile			file("/tmp/file2.archive");
			io::ArchiveFile::Block	blocks[]= {
				file.allocate("1", 1),
				file.allocate("10", 10),
				file.allocate("123", 123),
				file.allocate("5678", 56),
			};
			testBlocks(file, sizeof(blocks)/sizeof(blocks[0])+1);
			blocks[1].dispose();
			testBlocks(file, sizeof(blocks)/sizeof(blocks[0])+1);
			blocks[1]= io::ArchiveFile::Block();
			if(blocks[0].resize(13)) {
				printf("We shouldn't have been able to resize 13\n");
			}
			if(blocks[0].resize(12)) {
				file.write("123456789012", blocks[0].size(), blocks[0], blocks[0]);
				if(blocks[0].read() != "123456789012") {
					printf("New 12 block didn't match\n");
				}
				testBlocks(file, sizeof(blocks)/sizeof(blocks[0]));
			} else {
					printf("Couldn't resize to exact size\n");
			}
			blocks[1]= file.allocate("Hello World");
			testBlocks(file, sizeof(blocks)/sizeof(blocks[0])+1);

			#define test(x) if(x) {printf("%s failed\n", #x);}
			test(blocks[0] >= blocks[1]);
			test(blocks[1] <= blocks[2]);
			test(blocks[2] > blocks[3]);
			test(blocks[1] < blocks[3]);
			test(!(blocks[0] < blocks[1]));
			test(!(blocks[1] > blocks[2]));
			test(!(blocks[2] <= blocks[3]));
			test(!(blocks[3] >= blocks[0]));

			io::ArchiveFile::Block tmp= blocks[1];
			blocks[1]= blocks[2];
			blocks[2]= blocks[3];
			blocks[3]= tmp;

			blocks[2].dispose();
			testBlocks(file, 5);
			blocks[0].dispose();
			testBlocks(file, 5);
			//printBlocks(file.begin(), file.end());
			blocks[3].dispose();
			testBlocks(file, 4);
			blocks[1].dispose();
			testBlocks(file, 2);
			blocks[0].merge();
			testBlocks(file, 1);
		}
	} catch(const std::exception &exception) {
		printf("EXCEPTION: %s\n", exception.what());
	}
	return 0;
}
