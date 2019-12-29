import http.server
import multiprocessing
import os
import pytest
import queue
import socket
import subprocess
import tempfile


class NoRequestError(Exception):
    pass


class UnixSocketHTTPServer(http.server.HTTPServer):
    address_family = socket.AF_UNIX

    def get_request(self):
        request, client_address = super(UnixSocketHTTPServer, self).get_request()
        return (request, ['local', 0])

    def server_bind(self):
        super(UnixSocketHTTPServer, self).server_bind()
        self.server_name = 'UnixSocketHTTPServer'
        self.server_port = 0


class MockServer:
    def __init__(self, name):
        self.name = name
        self.requests = multiprocessing.Queue()

    def start(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.socket_path = os.path.join(self.tempdir.name, self.name)
        if os.path.exists(self.socket_path):
            os.remove(self.socket_path)
        self.http_server = UnixSocketHTTPServer((self.socket_path), self.create_handler())
        self.process = multiprocessing.Process(target=self.http_server.serve_forever)
        self.process.start()

    def stop(self):
        self.process.terminate()
        if os.path.exists(self.socket_path):
            os.remove(self.socket_path)
        self.tempdir.cleanup()

    def get_request(self):
        try:
            return self.requests.get(timeout=2)
        except queue.Empty as err:
            raise NoRequestError('No HTTP request arrived on unix socket \'{}\''.format(self.socket_path)) from err


    def create_handler(self):
        class MockServerHTTPHandler(http.server.BaseHTTPRequestHandler):
            mock_server = self

            def do_GET(self):
                self.process_request()

            def do_POST(self):
                self.process_request()

            def do_HEAD(self):
                self.process_request()

            def do_PATCH(self):
                self.process_request()

            def do_DELETE(self):
                self.process_request()

            def do_PUT(self):
                self.process_request()

            def process_request(self):
                requestline_parts = self.requestline.split(' ')
                request = {
                    'method': requestline_parts[0],
                    'resource': requestline_parts[1],
                    'headers': dict(self.headers),
                    'content': None
                }
                content_length = int(self.headers.get('content-length', '0'))
                if content_length > 0:
                    request['content'] = self.rfile.read(content_length).decode('utf8')
                self.mock_server.requests.put(request)
                self.send_response(200)
                self.end_headers()

        return MockServerHTTPHandler



@pytest.fixture(scope='function')
def mock_server():
    test_name = os.environ['PYTEST_CURRENT_TEST'].split(':')[-1].split(' ')[0]
    server = MockServer(test_name)
    server.start()
    try:
        yield server
    finally:
        server.stop()


def send_request(socket_path, method, resource, headers=None, content=None):
    args = [os.environ['CLIENT_PATH'], socket_path, method, resource]
    if headers:
        for header_name, header_value in headers:
            args.extend([header_name, header_value])
    if content:
        args.append(content)
    client = subprocess.Popen(args)
    client.communicate()


def test_simple_get_request(mock_server):
    send_request(mock_server.socket_path, 'GET', '/')
    request = mock_server.get_request()
    assert request == {
        'method': 'GET',
        'resource': '/',
        'headers': {},
        'content': None
    }


def test_get_request_with_url_arguments(mock_server):
    send_request(mock_server.socket_path, 'GET', '/endpoint?arg1=value1&arg2=value2')
    request = mock_server.get_request()
    assert request == {
        'method': 'GET',
        'resource': '/endpoint?arg1=value1&arg2=value2',
        'headers': {},
        'content': None
    }


def test_get_request_with_encoded_arguments(mock_server):
    send_request(mock_server.socket_path, 'GET', '/endpoint?arg1=value%201&arg2=value%2A2')
    request = mock_server.get_request()
    assert request == {
        'method': 'GET',
        'resource': '/endpoint?arg1=value%201&arg2=value%2a2',
        'headers': {},
        'content': None
    }


def test_post_request_with_content(mock_server):
    send_request(mock_server.socket_path, 'POST', '/endpoint', content='Hello World!')
    request = mock_server.get_request()
    assert request == {
        'method': 'POST',
        'resource': '/endpoint',
        'headers': {
            'Content-Length': '12'
        },
        'content': 'Hello World!'
    }


def test_post_request_with_content_and_custom_headers(mock_server):
    send_request(mock_server.socket_path, 'POST', '/endpoint', content='Hello World!', headers=[('Custom-Header-Name', 'Custom Header Value')])
    request = mock_server.get_request()
    assert request == {
        'method': 'POST',
        'resource': '/endpoint',
        'headers': {
            'Content-Length': '12',
            'Custom-Header-Name': 'Custom Header Value'
        },
        'content': 'Hello World!'
    }
