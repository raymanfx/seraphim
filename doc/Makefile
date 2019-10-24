all: doxy exhale

venv:
	mkdir -p ../build/doc
	python3 -m venv ../build/doc/env
	./../build/doc/env/bin/pip3 install breathe exhale sphinx sphinx_rtd_theme

doxy:
	mkdir -p ../build/doc/doxygen
	# path must be relative to current directory
	DOXYGEN_INPUT=../src/lib DOXYGEN_OUTPUT=../build/doc/doxygen doxygen ./doxygen/Doxyfile

exhale: doxy venv
	mkdir -p ../build/doc/sphinx
	# path must be relative to sphinx conf.py
	./../build/doc/env/bin/sphinx-build -b html ./sphinx ../build/doc/sphinx
	rm -rf ./sphinx/api
