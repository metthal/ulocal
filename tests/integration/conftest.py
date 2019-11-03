import os
import pytest


def pytest_collectreport(report):
    if 'SERVER_PATH' not in os.environ:
        raise pytest.UsageError('You need to set SERVER_PATH environment variable')
