MAKEFLAGS += --silent

.EXPORT_ALL_VARIABLES:
ASAN_OPTIONS="symbolize=1:color=always"
ASAN_SYMBOLIZER_PATH=$(bash which llvm-symbolizer)
GTEST_COLOR=1

.PHONY: default gen gen-release build test test-verbose test-valgrind
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
	@tmux split-window -v -l 80% "./build/src/Profiler/trace_collector --pipe /tmp/trace.pipe --output /tmp/trace.json; exec zsh"
	@tmux split-window -h "./build/src/Profiler/tests/profiler_pipe --pipe /tmp/trace.pipe; exec zsh"
test-trace-collector-cpp17-run:
	@make
	@tmux split-window -v -l 80% "./build/src/Profiler/trace_collector --pipe /tmp/trace_cpp17.pipe --output /tmp/trace_cpp17.json; exec zsh"
	@tmux split-window -h "./build/src/Profiler/tests/profiler_pipe --pipe /tmp/trace_cpp17.pipe; exec zsh"
