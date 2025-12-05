#pragma once

#include <Profiler/trace.hpp>

namespace Tracer {

class FileExporter;
class IPCExporter;

using Trace = TraceScope<FileExporter>;
using IPCTrace = TraceScope<IPCExporter>;

} // namespace Tracer

// Macros for file-based tracing (default)
#ifdef ENABLE_TRACING
#define TRACE_SETUP(file) Tracer::FileExporter::instance(file)
#define TRACE_SCOPE_CAT(name, cat) Tracer::Trace trace_##__LINE__(name, cat)
#define TRACE_SCOPE(name) Tracer::Trace trace_##__LINE__(name)
#define TRACE_FN_CAT(cat) TRACE_SCOPE_CAT(__FUNCTION__, cat)
#define TRACE_FN() TRACE_SCOPE(__FUNCTION__)
#else
#define TRACE_SETUP(file)
#define TRACE_SCOPE_CAT(name, cat)
#define TRACE_SCOPE(name)
#define TRACE_FN_CAT(cat)
#define TRACE_FN()
#endif // ENABLE_TRACING

// Macros for IPC-based tracing
#ifdef ENABLE_TRACING
#define IPC_TRACE_SETUP(pipe) Tracer::IPCExporter::instance(pipe)
#define IPC_TRACE_SCOPE_CAT(name, cat) Tracer::IPCTrace trace_##__LINE__(name, cat)
#define IPC_TRACE_SCOPE(name) Tracer::IPCTrace trace_##__LINE__(name)
#define IPC_TRACE_FN_CAT(cat) IPC_TRACE_SCOPE_CAT(__FUNCTION__, cat)
#define IPC_TRACE_FN() IPC_TRACE_SCOPE(__FUNCTION__)
#else
#define IPC_TRACE_SETUP(pipe)
#define IPC_TRACE_SCOPE_CAT(name, cat)
#define IPC_TRACE_SCOPE(name)
#define IPC_TRACE_FN_CAT(cat)
#define IPC_TRACE_FN()
#endif // ENABLE_TRACING
