import os
import pytest
import requests
import requests_unixsocket
import subprocess
import urllib.parse


@pytest.fixture(scope='module')
def ulocal_server():
    socket_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'integration_tests.sock')
    try:
        os.remove(socket_path)
    except FileNotFoundError:
        pass
    server = subprocess.Popen([os.environ['SERVER_PATH'], socket_path])
    yield socket_path
    server.terminate()
    server.wait()


def send_json(ulocal_server, method, resource, data, args=None, session=None):
    session = requests_unixsocket.Session() if not session else session
    return session.request(method, 'http+unix://{}{}{}'.format(ulocal_server.replace('/', '%2F'), resource, ('?' + urllib.parse.urlencode(args)) if args else ''), json=data)


def test_root_endpoint_get(ulocal_server):
    response = send_json(
        ulocal_server,
        'GET',
        '/',
        {
            'key1': 'value1',
            'key2': 2
        },
        args={
            'arg1': '&value1',
            'arg2': '&value2'
        }
    )

    assert response.status_code == 200
    assert response.json() == {
        'endpoint': '/',
        'request': {
            'method': 'GET',
            'resource': '/',
            'args': {
                'arg1': '&value1',
                'arg2': '&value2'
            },
            'headers': {
                'Accept': '*/*',
                'Accept-Encoding': 'gzip, deflate',
                'Connection': 'keep-alive',
                'Content-Length': '29',
                'Content-Type': 'application/json',
                'Host': 'localhost',
                'User-Agent': 'python-requests/2.22.0'
            },
            'content': '{"key1": "value1", "key2": 2}'
        }
    }


def test_root_endpoint_post(ulocal_server):
    response = send_json(
        ulocal_server,
        'POST',
        '/',
        {
            'key1': 'value1',
            'key2': 2
        }
    )

    assert response.status_code == 200
    assert response.json() == {
        'endpoint': '/',
        'request': {
            'method': 'POST',
            'resource': '/',
            'args': {},
            'headers': {
                'Accept': '*/*',
                'Accept-Encoding': 'gzip, deflate',
                'Connection': 'keep-alive',
                'Content-Length': '29',
                'Content-Type': 'application/json',
                'Host': 'localhost',
                'User-Agent': 'python-requests/2.22.0'
            },
            'content': '{"key1": "value1", "key2": 2}'
        }
    }


def test_root_endpoint_put(ulocal_server):
    response = send_json(
        ulocal_server,
        'PUT',
        '/',
        {
            'key1': 'value1',
            'key2': 2
        }
    )

    assert response.status_code == 200
    assert response.json() == {
        'endpoint': '/',
        'request': {
            'method': 'PUT',
            'resource': '/',
            'args': {},
            'headers': {
                'Accept': '*/*',
                'Accept-Encoding': 'gzip, deflate',
                'Connection': 'keep-alive',
                'Content-Length': '29',
                'Content-Type': 'application/json',
                'Host': 'localhost',
                'User-Agent': 'python-requests/2.22.0'
            },
            'content': '{"key1": "value1", "key2": 2}'
        }
    }


def test_root_endpoint_delete(ulocal_server):
    response = send_json(
        ulocal_server,
        'DELETE',
        '/',
        {
            'key1': 'value1',
            'key2': 2
        }
    )

    assert response.status_code == 200
    assert response.json() == {
        'endpoint': '/',
        'request': {
            'method': 'DELETE',
            'resource': '/',
            'args': {},
            'headers': {
                'Accept': '*/*',
                'Accept-Encoding': 'gzip, deflate',
                'Connection': 'keep-alive',
                'Content-Length': '29',
                'Content-Type': 'application/json',
                'Host': 'localhost',
                'User-Agent': 'python-requests/2.22.0'
            },
            'content': '{"key1": "value1", "key2": 2}'
        }
    }


def test_get_endpoint_get(ulocal_server):
    response = send_json(
        ulocal_server,
        'GET',
        '/get',
        {
            'key1': 'value1',
            'key2': 2
        }
    )

    assert response.status_code == 200
    assert response.json() == {
        'endpoint': '/',
        'request': {
            'method': 'GET',
            'resource': '/get',
            'args': {},
            'headers': {
                'Accept': '*/*',
                'Accept-Encoding': 'gzip, deflate',
                'Connection': 'keep-alive',
                'Content-Length': '29',
                'Content-Type': 'application/json',
                'Host': 'localhost',
                'User-Agent': 'python-requests/2.22.0'
            },
            'content': '{"key1": "value1", "key2": 2}'
        }
    }


def test_get_endpoint_post(ulocal_server):
    response = send_json(
        ulocal_server,
        'POST',
        '/get',
        {
            'key1': 'value1',
            'key2': 2
        }
    )

    assert response.status_code == 405


def test_post_endpoint_get(ulocal_server):
    response = send_json(
        ulocal_server,
        'GET',
        '/post',
        {
            'key1': 'value1',
            'key2': 2
        }
    )

    assert response.status_code == 405


def test_post_endpoint_post(ulocal_server):
    response = send_json(
        ulocal_server,
        'POST',
        '/post',
        {
            'key1': 'value1',
            'key2': 2
        }
    )

    assert response.status_code == 200
    assert response.json() == {
        'endpoint': '/',
        'request': {
            'method': 'POST',
            'resource': '/post',
            'args': {},
            'headers': {
                'Accept': '*/*',
                'Accept-Encoding': 'gzip, deflate',
                'Connection': 'keep-alive',
                'Content-Length': '29',
                'Content-Type': 'application/json',
                'Host': 'localhost',
                'User-Agent': 'python-requests/2.22.0'
            },
            'content': '{"key1": "value1", "key2": 2}'
        }
    }


def test_error_endpoint(ulocal_server):
    response = send_json(
        ulocal_server,
        'GET',
        '/error/500',
        {
            'key1': 'value1',
            'key2': 2
        }
    )

    assert response.status_code == 500


def test_non_existing_endpoint(ulocal_server):
    response = send_json(
        ulocal_server,
        'GET',
        '/non_existing',
        {
            'key1': 'value1',
            'key2': 2
        }
    )

    assert response.status_code == 404


def test_multiple_requests_one_session(ulocal_server):
    session = requests_unixsocket.Session()

    response1 = send_json(
        ulocal_server,
        'GET',
        '/',
        {
            'key1': 'value1',
            'key2': 2
        },
        args={
            'arg1': '&value1',
            'arg2': '&value2'
        },
        session=session
    )

    assert response1.status_code == 200
    assert response1.json() == {
        'endpoint': '/',
        'request': {
            'method': 'GET',
            'resource': '/',
            'args': {
                'arg1': '&value1',
                'arg2': '&value2'
            },
            'headers': {
                'Accept': '*/*',
                'Accept-Encoding': 'gzip, deflate',
                'Connection': 'keep-alive',
                'Content-Length': '29',
                'Content-Type': 'application/json',
                'Host': 'localhost',
                'User-Agent': 'python-requests/2.22.0'
            },
            'content': '{"key1": "value1", "key2": 2}'
        }
    }

    response2 = send_json(
        ulocal_server,
        'GET',
        '/',
        {
            'key3': 'value3',
            'key4': 4
        },
        args={
            'arg3': '&value3',
            'arg4': '&value4'
        },
        session=session
    )

    assert response2.status_code == 200
    assert response2.json() == {
        'endpoint': '/',
        'request': {
            'method': 'GET',
            'resource': '/',
            'args': {
                'arg3': '&value3',
                'arg4': '&value4'
            },
            'headers': {
                'Accept': '*/*',
                'Accept-Encoding': 'gzip, deflate',
                'Connection': 'keep-alive',
                'Content-Length': '29',
                'Content-Type': 'application/json',
                'Host': 'localhost',
                'User-Agent': 'python-requests/2.22.0'
            },
            'content': '{"key3": "value3", "key4": 4}'
        }
    }
