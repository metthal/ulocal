#!/bin/bash

python3 -m pip install -r tests/integration/requirements.txt
SERVER_PATH=$(pwd)/build/tests/integration/server/server python3 -m pytest -vv tests/integration/test.py
