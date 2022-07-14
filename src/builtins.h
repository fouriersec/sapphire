#include <string>
#include <vector>

// File builtins.h

class Target {
	public:
		Target();
		std::string getIP();
		std::string getNotes();
		void setIP(std::string IPaddr);
		void makeNote(std::string note);
		std::string noteFile;
	private:
		std::string IP;
		std::string notes;
};

class Workspace {
	public:
		Workspace();
		void parse(std::string file);
		void setName(std::string hostname);
		void addTarget(std::string IP);
		int getTargetNo();
		std::vector<Target> getTargets();
		int MAX_TARGETS;
		std::string getName();
		Target load(int targetID);
		void deleteTarget(int id);
		
	private:
		void updateConf();
		std::string name;
		int targetNo{};
		std::string targetList;
		std::vector<Target> targets;
};

			

std::vector<std::string> parseConf(std::string file);
