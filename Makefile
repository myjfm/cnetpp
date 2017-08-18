.PHONY: all test clean

all:
	blade build . --verbose -p debug --bundle debug 2>&1 | tee build.log

test: all
	blade test . --verbos

clean:
	blade clean . -p debug
