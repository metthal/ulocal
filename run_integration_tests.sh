#!/bin/bash

pip3 install -r tests/integration/requirements.txt
SERVER_PATH=$(pwd)/build/tests/integration/server/cpp/server python3 -m pytest -vvv tests/integration/server
CLIENT_PATH=$(pwd)/build/tests/integration/client/cpp/client python3 -m pytest -vvv tests/integration/client
