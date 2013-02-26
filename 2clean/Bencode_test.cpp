#include <stdio.h>
#include "os/Bencode.h"

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

void dump(const bencode::Item &item, const std::string &indent= "") {
	switch(item.type()) {
		case bencode::TypeString:
			fprintf(stderr, "%s '%s'\n", indent.c_str(), item.as<bencode::String>().value().c_str());
			break;
		case bencode::TypeInteger:
			fprintf(stderr, "%s %ld\n", indent.c_str(), static_cast<long>(item.as<bencode::Integer>().value()));
			break;
		case bencode::TypeList:
			fprintf(stderr, "%s List[%d]\n", indent.c_str(), item.as<bencode::List>().componentCount());
			for(uint32_t index= 0; index < item.as<bencode::List>().componentCount(); ++index) {
				dump(*item.as<bencode::List>().value(index), indent+"\t");
			}
			break;
		case bencode::TypeDictionary:
			fprintf(stderr, "%s Dictionary[%d]\n", indent.c_str(), item.as<bencode::List>().componentCount());
			for(bencode::Dictionary::key_iterator key= item.as<bencode::Dictionary>().keys(); key; ++key) {
				dump(*key, indent+"\t");
				dump(*item.as<bencode::Dictionary>()[&*key], indent+"\t\t");
			}
			break;
		default:
			fprintf(stderr, "%s Unknown\n", indent.c_str());
			break;
	}
}

bool equals(const bencode::Item &i1, const bencode::Item &i2) {
	if(i1.type() != i2.type()) {
		return false;
	}
	switch(i1.type()) {
		case bencode::TypeString:
			return i1.as<bencode::String>().value() == i2.as<bencode::String>().value();
		case bencode::TypeInteger:
			return i1.as<bencode::Integer>().value() == i2.as<bencode::Integer>().value();
		case bencode::TypeList:
			if(i1.as<bencode::List>().count() != i2.as<bencode::List>().count()) {
				return false;
			}
			for(uint32_t index= 0; index < i1.as<bencode::List>().count(); ++index) {
				if(!equals(*i1.as<bencode::List>().value(index), *i2.as<bencode::List>().value(index))) {
					return false;
				}
			}
			return true;
		case bencode::TypeDictionary: {
			bencode::Dictionary::key_iterator	k1= i1.as<bencode::Dictionary>().keys();
			bencode::Dictionary::key_iterator	k2= i2.as<bencode::Dictionary>().keys();

			while(k1 && k2) {
				if(!equals(*k1, *k2)) {
					return false;
				}
				if(!equals(*i1.as<bencode::Dictionary>()[&*k1], *i2.as<bencode::Dictionary>()[&*k2])) {
					return false;
				}
			}
			return true;
		}
		default:
			return false;
	}
}

int main(const int /*argc*/, const char * const /*argv*/[]) {
	std::string			buffer;
	bencode::Dictionary	dict;
	bencode::Item		*decoded;

	dict["test"]= "test";
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict["answer"]= 42;
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict["dict"]= new bencode::Dictionary();
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict["dict"]->as<bencode::Dictionary>()["marc"]= "page";
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict["dict"]->as<bencode::Dictionary>()["a"]= "z";
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict["list"]= new bencode::List();
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict["list"]->as<bencode::List>().push("item5");
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict["list"]->as<bencode::List>().push("item6");
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict["list"]->as<bencode::List>().push(7);
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict["list"]->as<bencode::List>().insert("item4");
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict["list"]->as<bencode::List>().insert(1);
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict["list"]->as<bencode::List>().insert("item3", 1);
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict["list"]->as<bencode::List>().insert(2, 1);
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict["remove"]= "gone";
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(dict.has_key("test"));
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(dict.has_key("answer"));
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(dict.has_key("dict"));
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(dict.has_key("list"));
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(dict.has_key("remove"));
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(dict["test"]->type() == bencode::TypeString);
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(dict["answer"]->type() == bencode::TypeInteger);
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(dict["dict"]->type() == bencode::TypeDictionary);
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(dict["list"]->type() == bencode::TypeList);
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(dict["remove"]->type() == bencode::TypeString);
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(*(dict.keys()+0) == bencode::String("answer"));
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(*(dict.keys()+1) == bencode::String("dict"));
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(*(dict.keys()+2) == bencode::String("list"));
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(*(dict.keys()+3) == bencode::String("remove"));
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dict.remove("remove");
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(!dict.has_key("remove"));
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(*(dict.keys()+0) == bencode::String("answer"));
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(*(dict.keys()+1) == bencode::String("dict"));
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);
	dotest(*(dict.keys()+2) == bencode::String("list"));
	printf("%s %d\n", dict.display(buffer).c_str(), __LINE__);

	buffer.clear();
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
	//dotest(equals(dict, *decoded));
	//dotest(equals(*decoded, dict));
	dump(dict);
	dump(*decoded);
	return 0;
}
