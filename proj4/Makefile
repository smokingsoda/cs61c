CC = gcc-14
CFLAGS = -g -Wall -std=c99 -fopenmp -pthread
LDFLAGS = -fopenmp
CUNIT = -L/opt/homebrew/opt/cunit/lib -I/opt/homebrew/opt/cunit/include -lcunit
PYTHON = -L/opt/homebrew/opt/python@3.12/Frameworks/Python.framework/Versions/3.12/lib/ -lpython3.12

install:
	if [ ! -f files.txt ]; then touch files.txt; fi
	rm -rf build
	xargs rm -rf < files.txt
	python3 setup.py install --record files.txt

uninstall:
	if [ ! -f files.txt ]; then touch files.txt; fi
	rm -rf build
	xargs rm -rf < files.txt

clean:
	rm -f *.o
	rm -f test
	rm -rf build
	rm -rf __pycache__

test:
	rm -f test
	$(CC) $(CFLAGS) mat_test.c matrix.c -o test $(LDFLAGS) $(CUNIT) $(PYTHON)
	./test

.PHONY: test