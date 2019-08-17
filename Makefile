#
# Proxy makefile that performs housekeeping tasks (code formatting, etc) and
# delegates 'real' building tasks to cmake.
#

SOURCES = $(shell find . -name *.c -o -name *.cc -o -name *.cpp -o -name *.cxx)
HEADERS = $(shell find . -name *.h -o -name *.hpp)

all: build

cmake:
	mkdir -p build
	cd build && cmake ..

build: cmake
	$(MAKE) -C build -j$(shell getconf _NPROCESSORS_ONLN)

install: build
	$(MAKE) -C build install

uninstall: install
	cat build/install_manifest.txt | xargs rm -f

clean:
	rm -rf build

format:
	clang-format -i -style=file $(SOURCES) $(HEADERS)

tidy:
	mkdir -p build
	cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
	$(MAKE) -C build -j4
	python3 run-clang-tidy.py -p build

docs:
	mkdir -p build/docs
	doxygen .doxyfile
