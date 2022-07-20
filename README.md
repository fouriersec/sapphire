# Sapphire
## A CTF workspace organisation tool  

View the full demo for Sapphire [here](https://fouriersec.github.io/2022/07/15/sapphire-project).

### Why?
I first made a (very) simple prototype of Sapphire in python to allow me to automatically set up my work area each time I played some [HackTheBox](https://hackthebox.com).  
The idea grew, and eventually I decided to create a full-fledged version of the tool in C++17, due to the speed benefits that come with it. There is still room for improvement in terms of the source code, but I hope you find Sapphire useful.  

### Features
- Use the custom shell to easily set up and manage a workspace environment for penetration testing / CTFs 
- Add up to three targets per workspace
- Make notes for each target
- view all notes taken for each target 
- visit target websites with one command
- set temporary environment variables to use when executing external commands

### Usage
Sapphire is pretty simple to install, just run `./install.sh`. To view the usage, run `./sapphire`. You can always view the usage by running 'help' within the software.

Built and tested on Kali Linux.
