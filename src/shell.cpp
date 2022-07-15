#include <vector>
#include <algorithm>
#include <iostream>
#include "builtins.h"
#include "boost/format.hpp"
#include <regex>
#include <sys/stat.h>
#include <filesystem>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <limits.h>
#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define YELLOW  "\033[1;33m"

Workspace Ws;
Target target;
int targetID{999};
int counter{};
std::string noSelectionError = "Target has not been selected. run 'info' for target list, then select a target with 'load <id>'\n";
std::string WORK_AREA;

void printUsage(){
	using boost::format;
	std::cout << "\nUsage: sapphire <workspace>\n\nThis will either load a workspace or create a new one."
		".\nConsider adding sapphire/ to the PATH."
		"\nTo delete a workspace, simply delete its folder from the data/ directory.\n\n";
	std::cout << format("%-40s %-40s %-40s\n") % "Commands" % "Usage" % "Description";
	std::cout << format("%-40s %-40s %-40s\n") % "info" % "" % "list all targets and their IDs, as well as workspace name";
	std::cout << format("%-40s %-40s %-40s\n") % "add-target" % "add-target <IP>" % "add a target machine to your workspace (maximum 3)";
	std::cout << format("%-40s %-40s %-40s\n") % "load" % "load <target-id>" % "load a target to work with";
	std::cout << format("%-40s %-40s %-40s\n") % "notes" % "" % "display all notes made for the loaded target";
	std::cout << format("%-40s %-40s %-40s\n") % "make-note" % "make-note <note>" % "add a note for the loaded target to its note file.";
	std::cout << format("%-40s %-40s %-40s\n") % "back" % "" % "return to the workspace directory (see !cd)";
	std::cout << format("%-40s %-40s %-40s\n") % "set" % "set <ENV_VAR> <value>" % "set environment variables for use when using exec";
	std::cout << "\nNote: You may use 'set current' to set the environment variable RHOST to the IP of the currently loaded target\n\n";
	std::cout << format("%-40s %-40s %-40s\n") % "clear" % "" % "clear the terminal";
	std::cout << format("%-40s %-40s %-40s\n") % "unload" % "" % "unload the current target";
	std::cout << format("%-40s %-40s %-40s\n") % "visit" % "visit <protocol> <port>" % "visit the webpage of the loaded target";
	std::cout << format("%-40s %-40s %-40s\n") % "help" % "" % "display this help message";
	std::cout << "\nDangerous commands - these must be prefixed with a '!' to be run\n\n";
	std::cout << format("%-40s %-40s %-40s\n") % "cd" % "cd <dir>" % "change current working directory - MAY CAUSE ERRORS, USE CAUTIOUSLY";
	std::cout << format("%-40s %-40s %-40s\n") % "exec" % "exec <cmd>" % "execute standard shell commands - you may wish to spawn a shell instead";
	std::cout << format("%-40s %-40s %-40s\n") % "" % "" % "and then 'exit' after use";
	std::cout << format("%-40s %-40s %-40s\n") % "delete" % "delete <target-id>" % "delete a target and everything associated with it from workspace\n";
}

std::vector<std::string> get_tokens(std::string cmd) {
	std::vector<std::string> tokens;
	std::string word;
	for (std::string::size_type i = 0; i < cmd.size(); i++) {	
		if (cmd[i] != ' ') {
			word = word + cmd[i];
		}
		else {
			tokens.push_back(word);
			word = "";
		}
	}
	tokens.push_back(word);
	return tokens;
}

void info() {
	using boost::format;
	std::cout << "\n";
	std::cout << format("%-20s %-20s\n") % "Workspace Name:" % Ws.getName();
	std::cout << format("%-20s %-20s\n") % "Number of Targets:" % Ws.getTargetNo();
	counter = 0;
	for (Target i : Ws.getTargets()) {
		std::string targetName = "target " + std::to_string(counter) + ":";
		std::cout << format("%-20s %-20s\n") % targetName % i.getIP();
		counter += 1;
	}
	std::cout << "\n";
}

std::string incorrectUsage(std::string cmd){
	std::string message = "sapphire: incorrect use of ";
	return message + cmd + "\n";
}

Workspace init_workspace(std::string name) {
	std::string data_dir 	= "data/" + name;
	std::string note_dir	= "notes";
	std::string configFile 	= name + ".conf";
	if (!std::filesystem::exists(data_dir))
		std::filesystem::create_directory(data_dir);

	std::filesystem::current_path(data_dir);

	if (!std::filesystem::exists(note_dir))
		std::filesystem::create_directory(note_dir); 

	if (std::filesystem::exists(configFile)) {
		std::cout << "Loading " << name << ".conf ...\n";
		Ws.parse(configFile); //Parse configfile and update class values with it.
		std::cout << "Loaded workspace successfully\n";
	}
	else {
		Ws.setName(name);
		std::cout << "Workspace " << Ws.getName() << " has been initialised.\n";
	}
	WORK_AREA = std::filesystem::current_path();
	info();
	return Ws;
}

bool passTargetCheck(){
	return targetID != 999;
}

void chdir (std::string path) {
	try{
		std::filesystem::current_path(path);
	}
	catch(std::filesystem::filesystem_error const& ex) {
		std::cout << "sapphire: " << ex.what();
	}
}

int set (std::string var, std::string value) {
	const char *var_c = var.c_str();	
	const char *val_c = value.c_str();	
	return setenv(var_c,val_c,0);
}

void handler(std::vector<std::string> cmd) {
	const std::string workspace_cmd[]	= {"add-target","info","load","make-note","notes","!exec","!cd","back","set","!delete","unload","help","clear","visit"};
	const std::string dangerous[]		= {"exec","cd","delete"};
	int no_of_dangerous			= sizeof(dangerous)/sizeof(dangerous[0]);
	std::string subTool			= cmd[0];
	int no_of_tools			= sizeof(workspace_cmd)/sizeof(workspace_cmd[0]);
	auto iter			= std::find(workspace_cmd, workspace_cmd + no_of_tools, subTool);
	std::string system_cmd;
	bool first = true;
	int returnVal{};
	std::string confirm;
	std::string site_str = "xdg-open ";

	switch(std::distance(workspace_cmd, iter)){ //takes the index of the builtin function as a parameter
		case 0:
			for (auto i : cmd) {
				if (first) {first=false; continue; }
				Ws.addTarget(i);
			}		
			break;
		case 1:
			info();
			break;
		case 2:
			if (cmd.size() < 2){
				std::cout << "please specify target id(s): 'load <id>'\n";
				return;
			}
			targetID = std::stoi(cmd[1]);
			target = Ws.load(targetID);
			std::cout << "Successfully loaded target " << cmd[1] << "\n";
			break;
		case 3:
			if (!passTargetCheck()){
				std::cout << noSelectionError;
				return;
			}
			for (std::string word : cmd){
				if (first) {first = false; continue; }
				target.makeNote(word);
			}
			target.makeNote("\n\n");
			std::cout << "Successfully noted\n";
			break;
		case 4:
			if (!passTargetCheck()){
				std::cout << noSelectionError;
				return;
			}

			std::cout << "\n" << target.getNotes();
			break;
		case 5:
			for (std::string token : cmd){
				if (first) { first = false; continue; }
				if (token == "cd") {std::cout << "cd can be used without the exec function.\n"; return;}
				system_cmd += token + ' ';	
			}

			system(system_cmd.c_str());
			break;
		case 6:
			if (cmd.size() != 2){std::cout << incorrectUsage(cmd[0]);return;}
			chdir(cmd[1]);
			std::cout << RED << "\nWarning: You have left your workspace - use 'back' to return\n\n" << RESET;
			break;
		case 7:
			if (cmd.size() != 1){std::cout << incorrectUsage(cmd[0]);return;}
			chdir(WORK_AREA);
			break;
		case 8:	
			if (cmd.size() > 3 || cmd.size() < 2){std::cout << incorrectUsage(cmd[0]); return;}
			else if (!passTargetCheck()) 
				std::cout << noSelectionError;
			else if (cmd.size() == 2 && cmd[1] == "current"){
				set( "RHOST", target.getIP() );
				std::cout << "Environment variable RHOST set to " << target.getIP() << "\n";
			}
			else {
				returnVal = set(cmd[1],cmd[2]);
				if (returnVal == 0)
					std::cout << "Enviroment variable " << cmd[1] << " set to value " << cmd[2] << "\n";
				else
					std::cout << "Setting environment variable " << cmd[1] << " failed - it may already exist\n";
				
			}
			break;
		case 9:
			if (cmd.size() != 2) {std::cout << incorrectUsage(cmd[0]);return;}
			std::cout << YELLOW << "are you sure you want to delete target " << cmd[1] << "? [y/n] " << RESET;
			std::cin >> confirm;
			if (confirm == "y"){
				Ws.deleteTarget(std::stoi(cmd[1]));
				std::cout << "target " << cmd[1] << " successfully deleted\n";
			}
			std::cin.clear();
			std::cin.ignore();
			break;
		case 10:
			if (cmd.size() != 1) {std::cout << incorrectUsage(cmd[0]);return;}
			if (targetID == 999){std::cout << "no targets to unload\n";return;}
			target = Target();
			std::cout << "unloaded target " << targetID << "\n";
			break;
		case 11:
			if (cmd.size() != 1) {std::cout << incorrectUsage(cmd[0]);return;}
			printUsage();
			break;
		case 12:
			if (cmd.size() != 1) {std::cout << incorrectUsage(cmd[0]);return;}
			system("clear");
			break;
		case 13:
			if (cmd.size() != 2) {std::cout << incorrectUsage(cmd[0]);return;}
			else if (!passTargetCheck())
				std::cout << noSelectionError;
			else
				site_str += cmd[1] + "://" + target.getIP() + ":" + cmd[2];
			system(site_str.c_str());
			break;
		default:
			for (int i = 0; i < no_of_dangerous; i++){
				if (cmd[0] == dangerous[i]){
					std::cout << "sapphire: this is 'dangerous' command - try prefixing with '!'\n";
					return;
				}
			}
			std::cout << "sapphire: " << subTool << ": command not found\n";
	}	
	return;
}

std::string get_selfpath() {
        char buff[1024];
        ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);
        if (len != -1) {
                buff[len] = '\0';
                return std::string(buff);
    }
        else {
                return "Cannot get binary path";
    }  
}

int main(int argc, char *argv[]) {
	using namespace std;
	std::string selfpath = get_selfpath();
	selfpath.erase(selfpath.length()-8);
	std::cout << selfpath << "\n";
	string username = getenv("USER");
	string cmd;
	if (argc != 2){
		printUsage();
		return 1;
	}
	filesystem::current_path(selfpath);
	Workspace Ws = init_workspace(argv[1]);

	while (1){
		cout << "(" << RED << username << "@sapphire" << RESET << ")~$ ";
		getline(cin, cmd);
		cmd = regex_replace(cmd, regex("^ +| +$|( ) +"), "$1"); //remove trailing, leading and extra whitespaces
		if (cmd == "exit"){
			cout << "\nGoodbye";
			break;
		}
		std::vector<std::string> tokens	= get_tokens(cmd);
		handler(tokens);
	}
	return 0;
}

