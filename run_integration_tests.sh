#!/bin/bash

pip3 install -r tests/integration/requirements.txt
SERVER_PATH=$(pwd)/build/tests/integration/server/server python3 -m pytest -vv tests/integration/test.py
