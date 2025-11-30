#!/bin/bash
GREEN="\033[0;32m"
RED="\033[0;31m"
NC="\033[0m" # No Color

PROCESS="$2"
CWD="$(cd "$(dirname "$0")/.." && pwd)"
OUTPUT_DIR="${CWD}/build/perf-data-$(date +%Y%m%d-%H%M%S)"

while [[ $# -gt 0 ]]; do
    case $1 in
    --listen|--launch|--gprofng)
        MODE="${1#--}"
        shift
        ;;
    *)
        shift
        ;;
    esac
done

if [[ -z "$MODE" || -z "$PROCESS" ]]; then
    echo "Usage: $0 [mode] <process_name>"
    echo "Modes:"
    echo "  --listen        Listen for an existing process"
    echo "  --launch        Launch and profile a new process"
    echo "  --gprofng       Launch gprofng profiler"
    exit 1
fi

trace() {
    echo -e "${GREEN}[INFO]${NC} $1" >&2
}

tweak_kernel() {
    # kptr_restrict controls the visibility of kernel pointer addresses. Allow perf to resolve kernel symbols
    # perf_event_paranoid controls the level of access to perf events for unprivileged users.
    trace "Tweaking kernel..."
    sudo sh -c 'echo 0 > /proc/sys/kernel/kptr_restrict'
    sudo sh -c 'echo -1 > /proc/sys/kernel/perf_event_paranoid'
}

launch_and_record() {
    perf record -g -o ${OUTPUT_DIR}/perf.data --call-graph=dwarf ${PROCESS}
}

listen_and_record() {
    trace "Listening for process ${PROCESS}..."
    PID=""
    while [[ -z "$PID" ]]; do
        PID=$(pgrep ${PROCESS})
        if [[ -z "$PID" ]]; then
            sleep 0.5
        fi
    done
    trace "Recording process with ID ${PID}..."
    perf record -F 997 -e cpu-clock -e cs -a -g -o ${OUTPUT_DIR}/perf.data --call-graph=dwarf -p ${PID}
}

launch_gprofng() {
    trace "Launching gprofng profiler..."
    # From gprofng gui - Data collection parameters:
    #   Clock-profiling, interval = 500 microsecs.
    #   Synchronization tracing, threshold = 1 microsecs. (calibrated); Native- and Java-APIs
    #   Heap tracing
    #   HW counter-profiling; counters:
    #     cycles, tag 0, interval 1600500, memop 0
    #     insts, tag 1, interval 1600500, memop 0
    #     llm, tag 2, interval 5005, memop 0
    #     br_msp, tag 3, interval 50050, memop 0
    #     br_ins, tag 4, interval 500500, memop 0
    #   Periodic sampling, 1 secs.
    #   Follow descendant processes from: fork|exec|combo
    gprofng collect app -o ${OUTPUT_DIR}.er \
        -p 0.1 \
        -S on \
        -H on \
        -h cycles -h insts -h llm -h dcm -h br_msp -h br_ins \
        -s calibrate \
        -j off \
        /home/leonardo/develop/tracer/build/profiler
    gprofng display gui ${OUNPUT_DIR}.er
    trace "Results available in ${OUTPUT_DIR}"
}

process() {
    cd ${OUTPUT_DIR}
    trace "Dump raw trace data"
    perf script -D -i perf.data >perf.trace
    trace "Generating flamegraph..."
    perf script report flamegraph -- --allow-download
    trace "Generating gecko report..."
    perf script report gecko
}

main() {
    mkdir -p ${OUTPUT_DIR}

    tweak_kernel

    if [[ "$MODE" == "launch" ]]; then
        launch_and_record
    elif [[ "$MODE" == "listen" ]]; then
        listen_and_record
    elif [[ "$MODE" == "gprofng" ]]; then
        launch_gprofng
    fi

    process

    trace "Output saved in ${OUTPUT_DIR}"
}
main
