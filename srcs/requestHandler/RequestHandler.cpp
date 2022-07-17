#include "RequestHandler.hpp"

RequestHandler::RequestHandler(Config* config, Env& env)
	: config(config)
	, cgi(new CGI(env))
{
	fillTypes(_types);
}

RequestHandler::~RequestHandler() { }

const std::string& RequestHandler::mimeType(const std::string& uri)
{
	std::string::size_type found = uri.find_last_of('.');
	if (found == std::string::npos)
		return _types["txt"];
	std::string extension = uri.substr(found + 1);
	if (_types.find(extension) != _types.end())
		return _types[extension];
	return _types["bin"];
}

void RequestHandler::readfile(Response* response, const std::string& path)
{
	string extension = path.substr(path.find_last_of("/\\") + 1);
	string fileExtension = path.substr(path.find_last_of(".") + 1);

	response->setContentType(mimeType(extension));
	try {
		response->setBody(FileReader::readFile(path));
		response->setStatusLine("200 OK");
	} catch (FileReader::FileNotFoundException& ex) {
		response->setBody(getErrorPageBody("ДОБРО ПОЖАЛОВАТЬ НА СТРАНИЦУ 404!"));
		response->setStatusLine("404 NOT FOUND");
	}
}

bool RequestHandler::isBadRequest(Request* request) const { return request->getMethod().empty() || request->getUri().empty() || request->getProtocol().empty(); }

bool RequestHandler::isProtocolSupported(const string& protocol) const { return protocol == "HTTP/1.1"; }

void RequestHandler::formResponse(WebClient* client)
{
	Response* response = client->getResponse();
	Request* request = client->getRequest();
	LocationContext* location = config->getLocationContext(client->getIp(), client->getPort(), request->getHost(), request->getUri());
	cout << "*location info*" << endl;
	location->printConfig();
	response->setProtocol("HTTP/1.1");

	if (isBadRequest(request)) {
		response->setStatusCode(400);
	} else if (!isProtocolSupported(request->getProtocol())) {
		response->setStatusCode(505);
	} else if (location->getRedirect().first) {
		response->setStatusCode(location->getRedirect().first);
	} else if (!location->getAllowedMethods().empty() && !location->getAllowedMethods().count(request->getMethod())) {
		response->setStatusCode(405);
	} // TODO: GET метод нельзя запретить и он всегда должен быть в разрешенных методах!
	string pathToFile = getPathToFile(request, location);
	if (pathToFile.empty()) {
		response->setStatusCode(404);
	} else {
		if (cgi->isFileShouldBeHandleByCGI(pathToFile)) {
			cgi->setParameters(request, location, pathToFile);
			cout << "CGIIIIII" << endl;
			CGIModel cgiResult = cgi->getPathToFileWithResult();
			if (cgiResult.isSuccess) {
				readfile(response, cgiResult.pathToFile);
			} else {
				response->setStatusCode(500);
			}
		} else {
			if (request->getMethod() == "GET") {
				doGet(location, request, response, pathToFile);
			} else if (request->getMethod() == "POST" || request->getMethod() == "PUT") {
				doPost(location, request, response);
			} else if (request->getMethod() == "DELETE") {
				doDelete(location, request, response);
			} else {
				response->setStatusCode(501);
			}
		}
	}
	setStatusLine(response);
	if (response->getStatusCode()) {
		setBodyForStatusCode(response, location);
	}
	fillHeaders(response, location);
}

void RequestHandler::setStatusLine(Response* response)
{
	switch (response->getStatusCode()) {
	case 301:
		response->setStatusLine("301 Moved Permanently");
		break;
	case 302:
		response->setStatusLine("302 Found");
		break;
	case 307:
		response->setStatusLine("307 Temporary Redirect");
		break;
	case 308:
		response->setStatusLine("308 Permanent Redirect");
		break;
	case 400:
		response->setStatusLine("400 Bad Request");
		break;
	case 403:
		response->setStatusLine("403 Forbidden");
		break;
	case 404:
		response->setStatusLine("404 Not Found");
		break;
	case 405:
		response->setStatusLine("405 Method Not Allowed");
		break;
	case 500:
		response->setStatusLine("500 Server Error");
		break;
	case 501:
		response->setStatusLine("501 Not Implemented");
		break;
	case 505:
		response->setStatusLine("505 HTTP Version Not Supported");
		break;
	}
}

void RequestHandler::setBodyForStatusCode(Response* response, LocationContext* location)
{
	if (response->getStatusCode() >= 300 && response->getStatusCode() < 400) {
		response->setBody(getRedirectPageBody(location->getRedirect()));
		response->setContentType(mimeType(".html"));
	} else {
		string pathToErrorPage = location->getErrorPage(response->getStatusCode());
		if (isFileExists(pathToErrorPage) && !isDirectory(pathToErrorPage)) {
			readfile(response, pathToErrorPage);
		} else {
			response->setBody(getErrorPageBody(response->getStatusLine()));
			response->setContentType(mimeType(".html"));
		}
	}
}

void RequestHandler::doPost(LocationContext* location, Request* request, Response* response) { (void)location, (void)request, (void)response; }

void RequestHandler::doGet(LocationContext* location, Request* request, Response* response, string& pathToFile)
{
	if (isDirectory(pathToFile)) {
		if (location->isAutoIndex()) {
			folderContents(response, pathToFile, request->getUri());
		} else {
			response->setStatusCode(403);
		}
	} else {
		readfile(response, pathToFile);
	}
}

void RequestHandler::folderContents(Response* response, const std::string& path, const string& uri)
{
	DIR* dp;
	struct dirent* di_struct;
	struct stat file_stats;
	time_t lastModified;
	string body;

	(void)path;
	string title = "Index of " + uri;
	body.append("<html>\n"
				"<head><title>"
		+ title
		+ "</title></head>\n"
		  "<body>\n"
		  "<h1>"
		+ title + "</h1><hr><pre><a href=\"../\">../</a>\n");
	if ((dp = opendir(path.c_str())) != nullptr) {
		while ((di_struct = readdir(dp)) != nullptr) {
			if (strcmp(di_struct->d_name, ".") && strcmp(di_struct->d_name, "..")) {

				string tmp_path = path + "/" + di_struct->d_name;
				stat(tmp_path.data(), &file_stats);

				body.append("<a href=\"" + uri + string(di_struct->d_name));
				if (S_ISDIR(file_stats.st_mode))
					body.append("/");
				body.append("\">" + string(di_struct->d_name));
				if (S_ISDIR(file_stats.st_mode))
					body.append("/");
				body.append("</a>                                               ");
				lastModified = getFileModificationDate(tmp_path);
				string date = string(ctime(&lastModified));
				date = date.substr(0, date.size() - 1);
				body.append(date + "                   ");
				if (S_ISDIR(file_stats.st_mode))
					body.append("-\n");
				else {
					body.append(std::to_string(static_cast<float>(file_stats.st_size)) + "\n");
				}
			}
		}
		closedir(dp);
	}
	body.append("</pre><hr></body>\n"
				"</html>\n");
	response->setBody(body);
	response->setStatusLine("200 OK");
}

void RequestHandler::doDelete(LocationContext* location, Request* request, Response* response) {
	string pathToFile = location->getRoot() + request->getUri();

	(void)request, (void)response;
}

void RequestHandler::fillHeaders(Response* response, LocationContext* location)
{
	time_t currentTime = time(0);
	char* time = ctime(&currentTime);
	pair<int, string> redirect = location->getRedirect();
	response->pushHeader(response->getProtocol() + " " + response->getStatusLine() + "\r\n");
	response->pushHeader("Server: webserv/2.0\r\n");
	response->pushHeader("Date: " + string(time, strlen(time) - 1) + "\r\n");
	response->pushHeader("Content-Type: " + response->getContentType() + "\r\n");
	response->pushHeader("Content-Length: " + std::to_string(response->getBody().size()) + "\r\n");
	if (redirect.first != 0) {
		response->pushHeader("Location: " + redirect.second + "\r\n");
	}
	response->pushHeader("Connection: keep-alive\r\n\r\n");
	response->setBuffer(response->getHeaders() + response->getBody());
}

string RequestHandler::getPathToFile(Request* request, LocationContext* location) const
{
	string path = location->getRoot() + request->getUri();
	if (isFileExists(path)) {
		if (isDirectory(path)) {
			if (path.back() != '/') {
				path.append("/");
			}
			for (vector<string>::const_iterator it = location->getIndex().begin(), ite = location->getIndex().end(); it != ite; ++it) {
				string pathToIndexFile;
				if ((*it).front() == '/') {
					pathToIndexFile = location->getRoot() + *it;
				} else {
					pathToIndexFile = path + *it;
				}
				if (isFileExists(pathToIndexFile) && !isDirectory(pathToIndexFile) && !access(pathToIndexFile.c_str(), W_OK)) {
					return pathToIndexFile;
				}
			}
		}
		return path;
	}
	return string();
}
