#include "os/Bencode.h"

#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

void dump(const bencode::Item &item, const std::string &indent= "") {
	switch(item.type()) {
		case bencode::TypeString:
			printf("%s '%s'\n", indent.c_str(), item.as<bencode::String>().value().c_str());
			break;
		case bencode::TypeInteger:
			printf("%s %ld\n", indent.c_str(), item.as<bencode::Integer>().value());
			break;
		case bencode::TypeList:
			printf("%s List[%d]\n", indent.c_str(), item.as<bencode::List>().componentCount());
			for(uint32_t index= 0; index < item.as<bencode::List>().componentCount(); ++index) {
				dump(*item.as<bencode::List>().value(index), indent+"\t");
			}
			break;
		case bencode::TypeDictionary:
			printf("%s Dictionary[%d]\n", indent.c_str(), item.as<bencode::List>().componentCount());
			for(bencode::Dictionary::key_iterator key= item.as<bencode::Dictionary>().keys(); key; ++key) {
				dump(*key, indent+"\t");
				dump(*item.as<bencode::Dictionary>()[&*key], indent+"\t\t");
			}
			break;
		default:
			printf("%s Unknown\n", indent.c_str());
			break;
	}
}

int main(const int /*argc*/, const char * const /*argv*/[]) {
	std::string			buffer;
	bencode::Dictionary	dict;
	bencode::Item		*decoded;

	dict["test"]= "test";
	dict["answer"]= 42;
	dict["dict"]= new bencode::Dictionary();
	dict["dict"]->as<bencode::Dictionary>()["marc"]= "page";
	dict["dict"]->as<bencode::Dictionary>()["a"]= "z";
	dict["list"]= new bencode::List();
	dict["list"]->as<bencode::List>().push("item5");
	dict["list"]->as<bencode::List>().push("item6");
	dict["list"]->as<bencode::List>().push(7);
	dict["list"]->as<bencode::List>().insert("item4");
	dict["list"]->as<bencode::List>().insert(1);
	dict["list"]->as<bencode::List>().insert("item3", 1);
	dict["list"]->as<bencode::List>().insert(2, 1);
	dict["remove"]= "gone";
	dotest(dict.has_key("test"));
	dotest(dict.has_key("answer"));
	dotest(dict.has_key("dict"));
	dotest(dict.has_key("list"));
	dotest(dict.has_key("remove"));
	dotest(dict["test"]->type() == bencode::TypeString);
	dotest(dict["answer"]->type() == bencode::TypeInteger);
	dotest(dict["dict"]->type() == bencode::TypeDictionary);
	dotest(dict["list"]->type() == bencode::TypeList);
	dotest(dict["remove"]->type() == bencode::TypeString);
	dotest(*(dict.keys()+0) == bencode::String("answer"));
	dotest(*(dict.keys()+1) == bencode::String("dict"));
	dotest(*(dict.keys()+2) == bencode::String("list"));
	dotest(*(dict.keys()+3) == bencode::String("remove"));
	dict.remove("remove");
	dotest(!dict.has_key("remove"));
	dotest(*(dict.keys()+0) == bencode::String("answer"));
	dotest(*(dict.keys()+1) == bencode::String("dict"));
	dotest(*(dict.keys()+2) == bencode::String("list"));

	bencode::ReferencedStringOutput	out(buffer);

	dict.write(out);
	dotest(buffer == "d6:answeri42e4:dictd1:a1:z4:marc4:pagee4:listli1ei2e5:item35:item45:item55:item6i7ee4:test4:teste");

	bencode::ReferencedStringInput	in(buffer);

	decoded= bencode::Item::read(in);
	dotest(NULL != decoded);
	dotest(!decoded->as<bencode::Dictionary>().has_key("remove"));
	dotest(*(decoded->as<bencode::Dictionary>().keys()+0) == bencode::String("answer"));
	dotest(*(decoded->as<bencode::Dictionary>().keys()+1) == bencode::String("dict"));
	dotest(*(decoded->as<bencode::Dictionary>().keys()+2) == bencode::String("list"));
	dotest(*decoded == dict);
	dotest(dict == *decoded);

	//printf("original\n");
	//dump(dict);

	//printf("\ncopy\n");
	//dump(*decoded);
	return 0;
}
