set -e

if [ $1 != "now" ] || [ $# -ne 1 ]; then
	echo "running './install.sh now' will install the tool in this directory."
	echo "Consider adding this directory to the PATH."
	exit 1
fi
mkdir data
g++ src/workspace.cpp src/parser.cpp src/shell.cpp -std=c++17 -o sapphire
echo "Installation complete"

