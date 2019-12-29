import os
import pytest


def pytest_collectreport(report):
    if 'CLIENT_PATH' not in os.environ:
        raise pytest.UsageError('You need to set CLIENT_PATH environment variable')
