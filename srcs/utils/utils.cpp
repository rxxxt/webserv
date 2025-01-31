#include "utils.hpp"

std::vector<string> ft_split(const std::string& str, const std::string& delim)
{
	std::vector<std::string> tokens;
	size_t prev = 0, pos;
	do {
		pos = str.find(delim, prev);
		if (pos == std::string::npos)
			pos = str.length();
		std::string token = str.substr(prev, pos - prev);
		if (!token.empty())
			tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

std::string ltrim(std::string str, const std::string& chars)
{
	str.erase(0, str.find_first_not_of(chars));
	return str;
}

std::string rtrim(std::string str, const std::string& chars)
{
	str.erase(str.find_last_not_of(chars) + 1);
	return str;
}

std::string trim(std::string str, const std::string& chars) { return ltrim(rtrim(str, chars), chars); }

struct pollfd fillPollfd(int sd, short events)
{
	struct pollfd fd;
	fd.fd = sd;
	fd.events = events;
	return fd;
}

// StarCompare() helper function
bool StarCmp(const char* str, const char* mask)
{
	if (*mask == '\0')
		return 1;
	if (*mask == '*') {
		if (*str) {
			// advance str and use the * again
			if (StarCmp(str + 1, mask))
				return 1;
		}
		// let * match nothing and advacne to the next pattern
		return StarCmp(str, mask + 1);
	}
	if (*mask == *str) {
		return StarCmp(str + 1, mask + 1);
	}
	return 0;
}

in_port_t getValidPort(const string& port)
{
	char* endPtr;
	long _port = std::strtol(port.c_str(), &endPtr, 10);

	if (*endPtr || _port <= 0 || _port > USHRT_MAX) {
		cout << "ERROR: Invalid port!" << endl;
		exit(EXIT_FAILURE);
	}
	return htons(_port);
}

void fatalError(const string& errorMessage, int errorCode)
{
	std::cerr << errorMessage << std::endl;
	exit(errorCode);
}

int getStringIndexFromArray(const string& str, const string* array)
{
	int i = 0;
	while (!array[i].empty()) {
		if (str == array[i])
			return i;
		++i;
	}
	return -1;
}

bool isOnlyDigits(const std::string& str) { return str.find_first_not_of("0123456789") == std::string::npos; }

bool isFileExists(const string& pathToFile)
{
	std::ifstream file(pathToFile);
	return file.is_open();
}

bool isDirectory(const string& pathToFile)
{
	struct stat file;
	return stat(pathToFile.c_str(), &file) != -1 && S_ISDIR(file.st_mode);
}

time_t getFileModificationDate(string& pathToFile)
{
	struct stat file;
	return (stat(pathToFile.c_str(), &file) != -1) ? file.st_mtime : -1;
}

string removeAfter(string s, char c)
{
	size_t pos = s.find(c);
	if (pos == string::npos) {
		return s;
	}
	return s.substr(0, pos);
}

bool isAccessRights(string& pathToFile)
{
	struct stat file;

	return stat(pathToFile.c_str(), &file) != -1 && file.st_mode & S_IROTH;
}

bool startsWith(const string& str, const string& pattern) { return str.find(pattern.c_str(), 0, pattern.length()) != string::npos; }

string getErrorPageBody(string errorMessage)
{
	return "<html>\n"
		   "<head>\n"
		   "    <title>Error "
		+ errorMessage
		+ "</title>\n"
		  "    <link href=\"https://fonts.googleapis.com/css2?family=Lato:wght@300&display=swap\" rel=\"stylesheet\">\n"
		  "    <link rel=\"stylesheet\" href=\"html/errorPages/style.css\">\n"
		  "</head>\n"
		  "<body>\n"
		  "<div id=\"main\">\n"
		  "    <div class=\"msg\">\n"
		  "        <h1>"
		+ errorMessage
		+ "</h1>\n"
		  "    </div>\n"
		  "</div>\n"
		  "</body>\n"
		  "</html>\n";
}

string getRedirectPageBody(std::pair<int, string> redirect)
{
	int redirectCode = redirect.first;
	if ((redirectCode > 300 && redirectCode < 303) || redirectCode == 307 || redirectCode == 308) {
		string redirectMsg = std::to_string(redirectCode);
		return "<html>\n"
			   "<head><title>"
			+ redirectMsg
			+ "</title></head>\n"
			  "<body>\n"
			  "<center><h1>"
			+ redirectMsg
			+ "</h1></center>\n"
			  "<hr><center>webserv</center>\n"
			  "</body>\n"
			  "</html>";
	} else
		return redirect.second;
}

string getStringAfterTarget(string source, string target)
{
	const size_t targetPos = source.find(target);
	return targetPos == string::npos ? "" : source.substr(targetPos + target.size());
}

char getLastSymbol(string s) { return s[s.length() - 1]; }

string getFileFormat(string& path)
{
	size_t pos = path.find_last_of('.');
	return (pos != string::npos) ? path.substr(pos + 1) : "";
}

string getFileName(const string& path)
{
	size_t pos = path.find_last_of('/');
	return (pos != string::npos) ? path.substr(pos + 1) : "";
}

int stringToInt(const string& str, int base)
{
	char* endPtr;
	long number = std::strtol(str.c_str(), &endPtr, base);

	if (*endPtr || number < INT32_MIN || number > INT32_MAX) {
		throw CastToIntException();
	}
	return number;
}

bool createFile(const string& pathToFile, const string& content)
{
	std::ofstream targetFile(pathToFile);
	if (!targetFile.is_open()) {
		return false;
	}

	targetFile << content;
	targetFile.close();
	return true;
}

string getParentFilePath(const string& pathToFile)
{
	size_t pos = pathToFile.find_last_of('/');
	return pathToFile.substr(0, pos);
}

const char* CastToIntException::what() const throw() { return "Conversion error to integer"; }

const char* BadChunkedRequestException::what() const throw() { return "Bad chunked request"; }

void printRequest(Request* request)
{
	string requestBuf;
	requestBuf.append("*REQUEST*\n");
	requestBuf.append(request->getMethod()).append(" ").append(request->getUri()).append(" ").append(request->getProtocol()).append("\n");

	map<string, string> headers = request->_headers;
	for (map<string, string>::iterator it = headers.begin(); it != headers.end(); ++it) {
		requestBuf.append(replace(it->first, "\r", "")).append(":").append(replace(it->second, "\r", "")).append("\n");
	}
	cout << requestBuf << endl;
}

void printResponse(Response* response)
{
	string responseBuf;
	responseBuf.append("*RESPONSE*\n");
	responseBuf.append(replace(response->getHeaders(), "\r", ""));
	cout << responseBuf << endl;
}

string replace(string input, const string& target, const string& replacement)
{
	for (size_t pos = input.find(target); pos < input.length() && pos != string::npos; pos = input.find(target)) {
		input.replace(pos, target.size(), replacement);
	}
	return input;
}

string removeTrailingSlashes(string s)
{
	return replace(s, "//", "/");
}

string createPath(const string& path1, const string& path2) {
	if (path1.back() == '/' && path2.front() == '/') {
		return path1 + path2.substr(1);
	} else if (path1.back() != '/' && path2.front() != '/') {
		return path1 + "/" + path2;
	}
	return path1 + path2;
}

string createPath(const string& path1, const string& path2, const string& path3) {
	string path = createPath(path1, path2);
	return createPath(path, path3);
}
