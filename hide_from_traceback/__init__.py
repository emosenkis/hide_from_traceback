import functools
import os
import sys
from typing import Callable, TypeVar

if sys.version_info < (3, 10):
    from typing_extensions import ParamSpec
else:
    from typing import ParamSpec


P = ParamSpec("P")
R = TypeVar("R")

enabled = True

def set_hide_from_traceback_enabled(value: bool) -> None:
    global enabled
    enabled = value


def hide_from_traceback(f: Callable[P, R]) -> Callable[P, R]:
    r"""Decorator that hides the decorated function from tracebacks.
    
    >>> import re
    >>> import sys
    >>> import traceback

    >>> def print_traceback():
    ...     tb = traceback.format_exc()
    ...     tb = tb.replace(__file__, "hide_from_traceback.py")
    ...     tb = re.sub(r"line \d+", "line xx", tb)
    ...     tb = re.sub(r"\[\d+\]", "[xx]", tb)
    ...     tb = re.sub(r"\n\s*\^+\s*\n", "\n", tb)
    ...     print(tb.rstrip())

    >>> def not_hidden():
    ...     raise Exception("foo")

    >>> @hide_from_traceback
    ... def hidden():
    ...     raise Exception("bar")

    >>> set_hide_from_traceback_enabled(True)

    >>> try:
    ...     not_hidden()
    ... except:
    ...     print_traceback()
    Traceback (most recent call last):
      File "<doctest __init__.hide_from_traceback[xx]>", line xx, in <module>
        not_hidden()
      File "<doctest __init__.hide_from_traceback[xx]>", line xx, in not_hidden
        raise Exception("foo")
    Exception: foo

    >>> try:
    ...     hidden()
    ... except:
    ...     print_traceback()
    Traceback (most recent call last):
      File "<doctest __init__.hide_from_traceback[xx]>", line xx, in <module>
        hidden()
    Exception: bar

    >>> set_hide_from_traceback_enabled(False)
    >>> try:
    ...     hidden()
    ... except:
    ...     print_traceback()
    Traceback (most recent call last):
      File "<doctest __init__.hide_from_traceback[xx]>", line xx, in <module>
        hidden()
      File "hide_from_traceback.py", line xx, in wrapper
        return f(*args, **kwargs)
      File "<doctest __init__.hide_from_traceback[xx]>", line xx, in hidden
        raise Exception("bar")
    Exception: bar
    """

    import _hide_from_traceback

    if sys.version_info < (3, 11):
        @functools.wraps(f)
        def wrapper(*args: P.args, **kwargs: P.kwargs) -> R:
            try:
                return f(*args, **kwargs)
            except:
                if not enabled:
                    raise
                tp, exc, tb = sys.exc_info()
                if tb:  # Remove decorated function
                    tb = tb.tb_next
                    if tb:  # Remove wrapper
                        tb = tb.tb_next
                _hide_from_traceback.set_exc_info(tp, exc, tb)
                del tp, exc, tb
                raise
    else:
        @functools.wraps(f)
        def wrapper(*args: P.args, **kwargs: P.kwargs) -> R:
            try:
                return f(*args, **kwargs)
            except Exception as e:
                if not enabled:
                    raise
                tb = e.__traceback__
                # Remove one extra frame in 3.11+
                for i in range(3):
                    if tb:
                        tb = tb.tb_next
                e.__traceback__ = tb
                del e, tb
                raise

    return wrapper


if os.environ.get("NO_HIDE_FROM_TRACEBACK") is not None:
    def hide_from_traceback(f: Callable[P, R]) -> Callable[P, R]:
        return f
