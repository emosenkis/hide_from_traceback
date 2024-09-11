# hide\_from\_traceback

A simple library for preventing utility functions from appearing in tracebacks.
Compatible only with CPython 3. Bundles `set_exc_info` to avoid reliance on
`_testcapi`, which is not included in all Python distributions.

## Installation

```
pip install hide_from_traceback
```

## Usage

```
from hide_from_traceback import hide_from_traceback

@hide_from_traceback
def assert_something_complex(value: object) -> None:
    if not something_about_object(value):
        raise Exception("Failed ")

def test_foo():
    data = { ... }
    assert_something_complex(data)
```

## Disabling

Set the environment variable `NO_HIDE_FROM_TRACEBACK` to disable this module's
functionality completely, making `@hide_from_traceback` a no-op or disable the
traceback-editing functionality at runtime:

```
from hide_from_traceback import set_hide_from_traceback_enabled

set_hide_from_traceback_enabled(False)
```
