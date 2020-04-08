.PHONY:test docs

all:test docs lint

lint:bin/logs/lint.txt

OPENSSL_PATH=$(subst openssl=,-I,$(OS_OPTIONS))/include

KNOWN_ERRORS:= --suppress=syntaxError:../os/Path.h:216 \
				--suppress=unusedFunction \
				--suppress=constParameter

bin/logs/lint.txt: *.h
	@mkdir -p bin/logs
	@cppcheck --enable=all --force --std=c++11 $(KNOWN_ERRORS) --language=c++ $(OPENSSL_PATH) -I/usr/include -I.. *.h &> $@
	@-cat $@ | grep performance:
	@-cat $@ | grep portability:
	@-cat $@ | grep style:
	@-cat $@ | grep warning:
	@-cat $@ | grep error:

documentation/index.html:
	@mkdir -p documentation
	@doxygen os.dox 2> bin/logs/doxygen.txt
	@if [ `cat bin/logs/doxygen.txt | wc -l` -ne "0" ]; then echo `cat bin/logs/doxygen.txt | wc -l` documentation messages; fi

docs:documentation/index.html

test:bin/test
	@bin/test $(OS_OPTIONS) $(COMPILER) $(TEST)

bin/test:tests/test.cpp *.h
	@mkdir -p bin
	@clang++ tests/test.cpp -o $@ -I..  -std=c++11 -lsqlite3 -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings

clean:
	@rm -Rf documentation bin/coverage bin/test bin/tests bin/logs/*.log bin/logs/*.txt
