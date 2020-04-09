.PHONY:test docs

all:test docs lint

lint:bin/logs/lint.txt

OPENSSL_PATH=$(subst openssl=,-I,$(OS_OPTIONS))/include

KNOWN_ERRORS:= --suppress=unusedFunction \
				--suppress=knownConditionTrueFalse:Library.h:162 \
				--suppress=missingOverride:Library.h:75 \
				--suppress=missingOverride:Sqlite3Plus.h:74 \
				--suppress=missingOverride:ZCompression.h:19 \
				--suppress=missingOverride:CryptoHelpers.h:22 \
				--suppress=missingOverride:../os/CryptoHelpers.h:22 \
				-U_DEBUG_FILE

bin/logs/lint.txt: *.h
	@echo Linting ...
	@mkdir -p bin/logs
	@cppcheck --enable=all --force --std=c++11 $(KNOWN_ERRORS) --language=c++ $(OPENSSL_PATH) -I.. *.h &> $@
	@-cat $@ | grep performance: || true
	@-cat $@ | grep portability: || true
	@-cat $@ | grep style: || true
	@-cat $@ | grep warning: || true
	@-cat $@ | grep error: || true

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
