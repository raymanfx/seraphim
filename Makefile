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
	$(MAKE) -C build -j4

install: build
	$(MAKE) -C build install

uninstall: install
	cat build/install_manifest.txt | xargs rm -f

clean:
	rm -rf build

format:
	clang-format -i -style=file $(SOURCES) $(HEADERS)
