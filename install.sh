set -e

if [ $# -eq 0 ]; then
	echo "running './install.sh now' will install the tool in this directory."
	echo "Consider adding this directory to the PATH."
	exit 1
fi
mkdir notes
mkdir workspaces
g++ workspace.cpp parser.cpp shell.cpp -std=c++17 -o sapphire
echo "Installation complete"

