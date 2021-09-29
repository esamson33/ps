.PHONY:	debug release

NPROC = $(shell nproc)

all: debug release

debug:
	cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=.
	cmake --build build/debug/ -- -j$(NPROC)

release:
	cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=.
	cmake --build build/release/ -- -j$(NPROC)

clean:
	@rm -rf build
	@find . -name "*~" -delete
