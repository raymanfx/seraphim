#
# Proxy makefile that performs housekeeping tasks (code formatting, etc) and
# delegates 'real' building tasks to cmake.
#

SOURCES = $(shell find src -name *.c -o -name *.cc -o -name *.cpp -o -name *.cxx)
HEADERS = $(shell find src -name *.h -o -name *.hpp)

all: build

cmake:
	mkdir -p build
	cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

build: cmake
	$(MAKE) -C build -j$(shell echo $$(($(shell getconf _NPROCESSORS_ONLN) + 1)))

install: build
	$(MAKE) -C build install

uninstall: install
	cat build/install_manifest.txt | xargs rm -f

clean:
	rm -rf build

format:
	@clang-format -i -style=file $(SOURCES) $(HEADERS)
	$(eval MODIFIED := $(shell git ls-files -m))
	$(if $(strip $(MODIFIED)), @echo $(MODIFIED))

lint: format
	$(eval MODIFIED := $(shell git ls-files -m))
	$(if $(strip $(MODIFIED)), @exit 1, @exit 0)

test: build
	find build/tests -type f -executable | xargs -L 1 sh -c

tidy: build
	python3 run-clang-tidy.py -p build

docs:
	make -C doc

rpm: build
	cd build && cpack -G RPM
