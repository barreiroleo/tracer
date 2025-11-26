-- TraceEvent TraceEvent:
--   name, cat, ph, ts, pid, tid, dur
-- Perfetto extra data:
--   slice.id, parent_id, depth, thread_dur, track_id, category, arg_set_id,

select * from slice;

select id, ts, dur, name, depth, thread_dur, category, correlation_id, arg_set_id, parent_id -- noqa: LT05
from (
    select
        slice.id, ts, dur, depth, name, thread_dur, track_id, category,
        extract_arg(arg_set_id, 'correlation_id') as correlation_id,
        arg_set_id, parent_id
    from slice
) where track_id in (0);
