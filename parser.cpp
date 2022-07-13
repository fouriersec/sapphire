#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "builtins.h"


std::vector<std::string> parseConf(std::string file) {

	std::string config;

	std::ifstream f(file);
	std::string str;
	while (std::getline(f, str))
		config = config + str + '\n';


	std::vector<std::string> results;
	std::istringstream is_file(config);

	std::string line;
	while( std::getline(is_file, line) ){

		std::istringstream is_line(line);
		std::string key;
		if( std::getline(is_line, key, '=') ){
			std::string value;
			if( std::getline(is_line, value) ) 
			results.push_back(value);
		}
	}
	return results;
}
