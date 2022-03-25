#ifndef WEBSERV_LOCATIONCONTEXT_HPP
#define WEBSERV_LOCATIONCONTEXT_HPP

#include "../utils/utils.hpp"
#include "Config.hpp"
#include "ServerContext.hpp"
#include <fstream>
#include <string>
#include <utility>

using std::make_pair;
using std::pair;
using std::string;
using std::vector;

const string LOCATION_CONTEXT_DIRECTIVES[]
	= { "allowed_method", "autoindex", "cgi_extension", "cgi_path", "client_max_body_size", "error_page", "index", "redirect", "root", "upload_path", "" };

class LocationContext {

public:
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

private:
	vector<string> _allowedMethods;
	bool _autoIndex;
	string _cgiExtension;
	string _cgiPath;
	size_t _clientMaxBodySize;
	vector<pair<int, string> > _errorPages;
	vector<string> _index;
	pair<int, string> _redirect;
	string _root;
	string _uploadPath;
	string _location;
	string _modificator;
	void checkDefaultValues();
};

#endif
