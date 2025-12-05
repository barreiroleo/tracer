MAKEFLAGS += --silent

.EXPORT_ALL_VARIABLES:
ASAN_OPTIONS="symbolize=1:color=always"
ASAN_SYMBOLIZER_PATH=$(bash which llvm-symbolizer)
GTEST_COLOR=1

.PHONY: default gen gen-release build build-gcc build-clang
.PHONY: test test-verbose test-valgrind
.PHONY: coverage graph format clean
default:
	@if [ ! -d build ]; then $(MAKE) gen; fi
	$(MAKE) build

gen:
	meson setup --reconfigure build --buildtype=debug

gen-release:
	meson setup --reconfigure build --buildtype=release

build:
	meson compile -C build

build-gcc:
	@CC=gcc CXX=g++ meson setup --reconfigure build-gcc
	@meson compile -C build-gcc

build-clang:
	@CC=clang CXX=clang++ meson setup --reconfigure build-clang
	@meson compile -C build-clang

test:
	meson test -C build

test-verbose:
	meson test -C build --verbose

test-valgrind:
	meson test -C build --wrap='valgrind'

coverage:
	@meson setup --reconfigure build -Db_coverage=true
	@meson test -C build
	@ninja -C build coverage
	@xdg-open ./build/meson-logs/coveragereport/index.html

graph:
	@ninja -C build -t graph all | dot -Tpng -o build_graph.png

format:
	@ninja -C build clang-format

clean:
	meson compile --clean -C build

test-server-client-run:
	@make
	@tmux split-window -v -l 80% "./build/src/IPC/client --pipe /tmp/tracer.pipe; exec zsh"
	@tmux split-window -h "./build/src/IPC/server --pipe /tmp/tracer.pipe; exec zsh"
test-trace-collector-run:
	@make
	@tmux split-window -v -l 80% "./build/src/TraceCollector/trace_collector --pipe /tmp/trace.pipe --output /tmp/trace.json; exec zsh"
	@tmux split-window -h "./build/src/TraceCollector/profiler_pipe --pipe /tmp/trace.pipe; exec zsh"
