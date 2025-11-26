MAKEFLAGS += --silent

.EXPORT_ALL_VARIABLES:
ASAN_OPTIONS="symbolize=1:color=always"
ASAN_SYMBOLIZER_PATH=$(bash which llvm-symbolizer)
GTEST_COLOR=1

.PHONY: default gen build test test-valgrind coverage graph
default:
	@if [ ! -d build ]; then $(MAKE) gen; fi
	$(MAKE) build

gen:
	meson setup --reconfigure build --buildtype=debug -Db_coverage=true

build:
	meson compile -C build

test:
	meson test -C build

test-verbose:
	meson test -C build --verbose

test-valgrind:
	meson test -C build --wrap='valgrind'

coverage:
	@ninja -C build coverage > /dev/null 2>&1
	@cat build/meson-logs/coverage.txt
	@echo "HTML coverage report generated in ${PWD}/build/meson-logs/coveragereport/index.html"

graph:
	@ninja -C build -t graph all | dot -Tpng -o build_graph.png

format:
	@ninja -C build clang-format
