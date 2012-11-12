#include "os/Bencode.h"

#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(const int /*argc*/, const char * const /*argv*/[]) {
	std::string			buffer;
	bencode::Dictionary	dict;

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
	dotest(**(dict.keys()+0) == bencode::String("answer"));
	dotest(**(dict.keys()+1) == bencode::String("dict"));
	dotest(**(dict.keys()+2) == bencode::String("list"));
	dotest(**(dict.keys()+3) == bencode::String("remove"));
	dict.remove("remove");
	dotest(!dict.has_key("remove"));
	dotest(**(dict.keys()+0) == bencode::String("answer"));
	dotest(**(dict.keys()+1) == bencode::String("dict"));
	dotest(**(dict.keys()+2) == bencode::String("list"));
	return 0;
}
