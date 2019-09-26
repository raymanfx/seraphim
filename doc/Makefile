all: doxy exhale

doxy:
	mkdir -p ../build/doc/doxygen
	# path must be relative to current directory
	DOXYGEN_INPUT=../src/lib DOXYGEN_OUTPUT=../build/doc/doxygen doxygen ./doxygen/Doxyfile

exhale: doxy
	mkdir -p ../build/doc/sphinx
	# path must be relative to sphinx conf.py
	sphinx-build-3 -b html ./sphinx ../build/doc/sphinx
	rm -rf ./sphinx/api
