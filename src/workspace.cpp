#include "builtins.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdio>
#include <exception>
#include <vector>
#include <sstream>
#include <filesystem>
#include <regex>


Target::Target() {
	;
}

std::string Target::getNotes(){
	bool notesExist = std::filesystem::exists(noteFile);
	if (!notesExist)
		return "No notes have been made for this target\n";
	notes = "";
	std::ifstream input(noteFile);
	for( std::string line; getline( input, line ); ){
		line = std::regex_replace(line, std::regex("^ +| +$|( ) +"), "$1"); //remove trailing leading extra whitespace
		notes += line + "\n";
	}
	return notes;
}


	   

void Target::makeNote(std::string note) {
	noteFile = "notes/" + getIP() + ".note";
	if (std::filesystem::exists(noteFile)){
		std::ofstream notefile;
		notefile.open(noteFile, std::ios_base::app);
		notefile << ' ' << note;
	}
	else {
		std::ofstream notefile(noteFile);
		notefile << ' ' << note;
		notefile.close();
	}		
}


std::string Target::getIP(){
	return IP;
}

void Target::setIP(std::string IPaddr){
	IP = IPaddr;
	noteFile = "notes/" + IP + ".note";
}
	

Workspace::Workspace() {
	MAX_TARGETS=3;
	targetList = "-";
}

void Workspace::updateConf() {
	std::ofstream configFile("/tmp/"+name+".conf");
	configFile << "name="+name+'\n';
	configFile << "target_number=" << targetNo << "\n";
	configFile << "targets=" << targetList;
	configFile.close();
	try {
		std::string path = name + ".conf";
		const char *p = path.c_str();
		std::remove(p);
		std::filesystem::rename("/tmp/"+name+".conf",path);	//Replaces config file in workspace dir
	}
	catch (std::filesystem::filesystem_error& e) {
		std::cout << e.what();
	}
}

Target Workspace::load(int targetID){
	return targets[targetID];
}

void Workspace::parse(std::string file) {
	std::vector<std::string> values = parseConf(file);
	name = values[0];
	targetNo = std::stoi(values[1]); // values[1] is the target number in config file
	targetList = values[2];
	if (targetList == "-")
		return;
	std::stringstream ss(values[2]); // values[2] is the list of targets in form a,b,c,d
				 // splitting list of targets into array
	while(ss.good()){
		std::string substr;
		std::getline( ss, substr, ',' );
		Target tgt;
		tgt.setIP(substr);
		targets.push_back(tgt);
	}
}



std::string Workspace::getName() {
	return name;
}

void Workspace::addTarget(std::string IP) {
	if (targetNo == MAX_TARGETS) {
		std::cout << "sapphire: maximum number of targets has been reached.\n";
		return;
	}
	for (auto t : targets){
		if (t.getIP() == IP){
			std::cout << "sapphire: a target already exists with this IP address.\n";
			return;
		}
	}
	targetNo = targetNo + 1;
	if (targetList == "-") { 				//If we are at the first target to be added
		targetList = IP;
	}
	else {
		targetList = targetList + "," + IP;
	}
	Target tgt;
	tgt.setIP(IP);
	targets.push_back(tgt);
	Workspace::updateConf();
	std::cout << IP << " has been added as target " << targetNo-1 << "\n";
}

void Workspace::deleteTarget(int id){
	std::string noteFile = "notes/" + targets[id].getIP() + ".note";
	if (std::filesystem::exists(noteFile)){
		const char *noteFile_c = noteFile.c_str();
		remove(noteFile_c);
	}
	targets.erase(targets.begin()+id);
	targetNo = targets.size();
	targetList = "";
	if (targetNo == 0)
		targetList = "-";
	else{
		for (Target i : targets){
			if (i.getIP() == targets.back().getIP())
				targetList += i.getIP();
			else
				targetList += i.getIP() + ",";
		}
	}
	
	updateConf();
}
			

	
int Workspace::getTargetNo() {
	return targetNo;
}

std::vector<Target> Workspace::getTargets() {
	return targets;
}

void Workspace::setName(std::string hostname) {
	std::ofstream configFile(hostname+".conf");
	name = hostname;
	updateConf();
	configFile.close();
}
