#ifndef WEBSERV_LOCATIONCONTEXT_HPP
#define WEBSERV_LOCATIONCONTEXT_HPP

#include "../utils/utils.hpp"
#include "Config.hpp"
#include "ServerContext.hpp"
#include <fstream>
#include <set>
#include <string>
#include <utility>

using std::make_pair;
using std::pair;
using std::set;
using std::string;
using std::vector;

const string LOCATION_CONTEXT_DIRECTIVES[]
	= { "allowed_method", "autoindex", "cgi_extension", "cgi_path", "client_max_body_size", "error_page", "index", "redirect", "root", "upload_path", "" };

class LocationContext {

public:
	LocationContext();
	LocationContext(const vector<string>& lineLocation, std::ifstream* fileSteam);
	virtual ~LocationContext();
	void parseAllowedMethods(vector<string> lineWords);
	bool isAutoIndex() const;
	void setAutoIndex(bool autoIndex);
	const string& getCgiExtension() const;
	void setCgiExtension(const string& cgiExtension);
	const string& getCgiPath() const;
	void setCgiPath(const string& cgiPath);
	void printConfig();
	const string& getLocation();
	const string& getModificator();

	size_t getClientMaxBodySize() const;
	void setClientMaxBodySize(size_t clientMaxBodySize);
	const vector<string>& getIndex() const;
	void setIndex(const vector<string>& index);
	const string& getRoot() const;
	void setRoot(const string& root);

	const set<string>& getAllowedMethods() const;
	void setErrorPagesFromServerContext(map<int, string>& serverErrorPages);
	string getErrorPage(int code);
	string getErrorPagePath(int code);

private:
	set<string> _allowedMethods;
	bool _autoIndex;
	string _cgiExtension;
	string _cgiPath;
	size_t _clientMaxBodySize;
	map<int, string> _errorPages;
	vector<string> _index;
	pair<int, string> _redirect;
	string _root;
	string _uploadPath;
	string _location;
	string _modificator;
	void checkDefaultValues();
};

#endif
