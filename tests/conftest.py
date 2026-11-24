"""Pause faulthandler while the emulator tests run.

Unicorn 2.x on Windows raises access violations inside its own code and
handles them itself. Python's faulthandler sees each one before that handler
does and prints "Windows fatal exception: access violation" plus a stack, so a
full suite run logged ~115,000 such dumps (~500 MB) without any real fault. A
five-byte `mov eax, 42` run shows the same reports, and the same correct result
with faulthandler off.

Only the tests that emulate in-process skip faulthandler; every other test keeps
pytest's crash reports.
"""
import faulthandler
import sys

import pytest

EMULATING = {'test_emu_diff.py', 'test_static_initializers.py'}


@pytest.hookimpl(hookwrapper=True)
def pytest_runtest_call(item):
    if item.path.name not in EMULATING or not faulthandler.is_enabled():
        yield
        return
    faulthandler.disable()
    yield
    try:
        from _pytest.faulthandler import fault_handler_stderr_key
        stream = item.config.stash[fault_handler_stderr_key]
    except (ImportError, KeyError):
        stream = sys.__stderr__
    faulthandler.enable(file=stream)
