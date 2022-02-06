
#include "WebClient.hpp"

WebClient::WebClient(int fd, int port)
		:_fd(fd),
		 _port(port),
		 _status(READABLE),
		 _request(nullptr),
		 _response(nullptr) {}

WebClient::WebClient(const WebClient& other)
		:_fd(other._fd),
		 _port(other._port),
         _status(other._status),
		 _request(other._request),
		 _response(other._response) {}

WebClient& WebClient::operator=(const WebClient& other) {
	if (this != &other) {
        _fd = other._fd;
		_port = other._port;
		_status = other._status;
		_request = other._request;
		_response = other._response;
	}
	return *this;
}

WebClient::~WebClient() {
    delete _request;
    delete _response;
}

void WebClient::setStatus(Status status) {
    this->_status = status;
}

void WebClient::setRequest(Request* request) {
	this->_request = request;
}

void WebClient::setResponse(Response* response) {
	this->_response = response;
}

int WebClient::getFd() const {
	return _fd;
}

int WebClient::getPort() const {
	return _port;
}

WebClient::Status WebClient::getStatus() const {
	return _status;
}

Request* WebClient::getRequest() const {
	return _request;
}

Response* WebClient::getResponse() const {
    return _response;
}

void WebClient::update() {
    delete _request;
    delete _response;
    _request = new Request();
    _response = new Response();
    setStatus(READABLE);
}
