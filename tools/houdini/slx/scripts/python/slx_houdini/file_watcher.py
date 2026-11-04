"""Recompiles SLX Material nodes when their source files change on disk.

Files are polled from Houdini's event loop rather than watched with Qt, which keeps this independent of
the PySide version a given Houdini ships.
"""

import os
import time

import hou

POLL_INTERVAL_SECONDS = 0.5

_watched_files = {}  # node session id -> ({path: modified time}, callback)
_last_poll_time = 0.0
_is_polling = False


def watch_files(node, paths, on_change):
    """Calls on_change(node) the next time any of paths is modified, created or deleted."""
    if not hou.isUIAvailable():
        return
    _watched_files[node.sessionId()] = ({p: get_modified_time(p) for p in paths}, on_change)
    start_polling()


def unwatch_files(node):
    _watched_files.pop(node.sessionId(), None)


def get_modified_time(path):
    try:
        return os.path.getmtime(path)
    except OSError:
        return None


def start_polling():
    global _is_polling
    if not _is_polling:
        hou.ui.addEventLoopCallback(poll_files)
        _is_polling = True


def stop_polling():
    global _is_polling
    if _is_polling:
        hou.ui.removeEventLoopCallback(poll_files)
        _is_polling = False


def poll_files():
    global _last_poll_time
    now = time.monotonic()
    if now - _last_poll_time < POLL_INTERVAL_SECONDS:
        return
    _last_poll_time = now

    for session_id, (modified_times, on_change) in list(_watched_files.items()):
        node = hou.nodeBySessionId(session_id)
        if node is None:
            del _watched_files[session_id]
            continue
        current_times = {p: get_modified_time(p) for p in modified_times}
        if current_times != modified_times:
            # Store the new times first, so a recompile that fails doesn't trigger again on the next poll.
            _watched_files[session_id] = (current_times, on_change)
            on_change(node)

    if not _watched_files:
        stop_polling()
