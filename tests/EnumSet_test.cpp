#include "EnumSet.h"

// g++ -o /tmp/test EnumSet_test.cpp -I.. -g -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings -Wno-non-template-friend
// /tmp/test | grep EnumSet.h | sort | uniq | wc -l
// max = 133

/**
	@todo set the return value for main
	@todo update dotest and main in all tests so far
*/
#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

enum Bits {
	Bit0,
	Bit1,
	Bit2,
	Bit3,
	Bit4,
	Bit5,
	Bit6,
	Bit7,
	Bit8
};

typedef EnumSet<Bits, Bit8> SafeBits;

int main(int,const char*const[]) {
	SafeBits	bits1, bits2(Bit2);

	bits1= bits2;
	bits1.set(Bit0).set(Bit1).set(Bit3).set(Bit6);
	const int	expected= 1 * 1 | 1 * 2 | 1 * 4 | 1 * 8 | 0 * 16 | 0 * 32 | 1 * 64 | 0 * 128 | 0 * 256;
	dotest(bits1.get<int>() == expected);
	dotest(!bits1.empty());
	dotest(!bits2.empty());
	dotest((SafeBits().set(Bit5)^=Bit5).empty());
	dotest( (bits1 | bits2) == bits1 );
	dotest( (bits1 ^ bits2) != bits1 );
	dotest( ((bits1 ^ bits2) & bits2) == SafeBits() );
	dotest( (SafeBits().set(Bit2).set(Bit3)&=Bit2) == bits2 );
	dotest( (SafeBits()|=Bit2) == bits2 );
	dotest( ((SafeBits().set(Bit3)^=Bit2)^=Bit3) == bits2 );
	dotest( ~bits1 == SafeBits().set(Bit4).set(Bit5).set(Bit7).set(Bit8) );
	dotest(bits1 == SafeBits().set(expected));
	dotest(bits2 == Bit2);
	dotest(!(bits1 < bits2));
	dotest(bits1 > bits2);
	dotest(!(bits1 <= bits2));
	dotest(bits1 >= bits2);
	dotest(bits1 != bits2);
	dotest(bits1 != bits2);
	dotest(Bit2 == bits2);
	dotest(!(bits2 != Bit2));
	dotest(!(Bit2 != bits2));
	dotest(!(bits2 < Bit2));
	dotest(!(bits2 < Bit1));
	dotest(bits2 < Bit3);
	dotest(!(Bit2 > bits2));
	dotest(!(Bit1 > bits2));
	dotest(Bit3 > bits2);
	dotest(!(Bit2 < bits2));
	dotest(Bit2 <= bits2);
	dotest(Bit1 < bits2);
	dotest(!(Bit3 < bits2));
	dotest(!(Bit3 <= bits2));
	dotest(bits2 >= Bit2);
	dotest(bits2 >= Bit1);
	dotest(!(bits2 >= Bit3));
	dotest(Bit2 >= bits2);
	dotest(!(Bit1 >= bits2));
	dotest(Bit3 >= bits2);
	dotest( (bits2&bits1) == bits2 );
	dotest( (bits1&Bit2) == bits2 );
	dotest( (Bit2&bits1) == bits2 );
	dotest( (SafeBits().set(Bit3)|Bit5) == SafeBits().set(Bit5).set(Bit3) );
	dotest( (Bit5 | SafeBits().set(Bit3)) == SafeBits().set(Bit5).set(Bit3) );
	dotest( (Bit5 ^ SafeBits().set(Bit3)) == SafeBits().set(Bit5).set(Bit3) );
	dotest( (Bit5 ^ SafeBits().set(Bit3).set(Bit5)) == Bit3 );
	dotest(256 + 255 <= bits1.max<int>());
	try	{
		bits1.set(256 + 255);
	} catch(const std::exception &exception) {
		const bool exceptionNotThrown= false;
		dotest(exceptionNotThrown);
	}
	try	{
		bits1.set(256 + 255 + 1);
		const bool exceptionThrown= false;
		dotest(exceptionThrown);
	} catch(const std::exception &exception) {
	}
	return 0;
}
