
PREFIX ?= $(CURDIR)/build/install

.PHONY: all build python test clean install

all: build

build:
	@mkdir -p build \
	&& cd build \
	&& cmake .. \
	&& cmake --build . --config Release

python:
	@mkdir -p build && cd build \
	&& cmake .. -DBUILD_SWIG_PYTHON=ON \
	&& cmake --build . --config Release \
	&& cmake --install . --prefix $(PREFIX)

test:
	@mkdir -p build && cd build \
	&& cmake .. -DBUILD_TESTS=ON \
	&& cmake --build . --config Debug \
	&& ctest

install: build
	@cd build \
	&& cmake --install . --prefix $(PREFIX)

clean:
	@rm -rf build
