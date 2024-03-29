#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string memTotalStr;
  string memFreeStr;
  long memTotal;
  long memFree;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if(stream.is_open()){
    std::getline(stream, memTotalStr);
    std::getline(stream, memFreeStr);
    // To seperate the number from the rest of the string
    memTotal = SeperateMemInfoNumberFromStringHelperMethod(memTotalStr);
    memFree = SeperateMemInfoNumberFromStringHelperMethod(memFreeStr);
    return (float)(memTotal - memFree) / (float)memTotal;
  }
  return -1.0; 
}

long LinuxParser::SeperateMemInfoNumberFromStringHelperMethod(string str){
	int i = str.find_first_of("0123456789");
	str = str.substr(i, str.size());
	i = str.find(" ");
	str = str.substr(0, i);
    return atol(str.c_str());
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() { 
  string line;
  string systemUpTime;
  string idleTime;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if(stream.is_open()){
    // int i = line.find(' ');
    // systemUpTime = line.substr(0, i);
    std::getline(stream, systemUpTime, ' ');
  }
  return atol(systemUpTime.c_str());
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
	return ActiveJiffies() + IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  long upTime = Jiffies();
  string str;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if(stream.is_open()){
    std::getline(stream, str);
    for(int i=0; i<13; i++){
      std::size_t found = str.find(" ");
      if (found != std::string::npos){
        str = str.substr(found+1, str.size());
      }
    }
    vector<long> vect;
    for(int i=0; i<4; i++){
      std::size_t found = str.find(" ");
      if (found != std::string::npos){
        string str2 = str.substr(0, found);
        vect.push_back(atol(str2.c_str()));
        str = str.substr(found+1 , str.size());
      }
    }
    for(int i=0; i<4; i++){
      std::size_t found = str.find(" ");
      if (found != std::string::npos){
        str = str.substr(found+1 , str.size());
      }
    }
    std::size_t found = str.find(" ");
    if (found != std::string::npos){
      string str2 = str.substr(0, found);
      vect.push_back(atol(str2.c_str()));
    }
    long sum = 0;
    for(int i=0; i<4; i++){
      sum += vect[i];
    }
    long rtn = upTime - vect[4];
    return sum;
  }
  return -1;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> vect = CpuUtilization();
  if(vect.size() > 0){
    long active = 0;
    active += atol(vect[0].c_str()) + atol(vect[1].c_str()) + atol(vect[2].c_str()) + atol(vect[5].c_str()) + atol(vect[6].c_str()) + atol(vect[7].c_str());
    return active;
  }
  return -1;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> vect = CpuUtilization();
  if(vect.size() > 0){
    long active = 0;
    active += atol(vect[3].c_str()) + atol(vect[4].c_str());
    return active;
  }
  return -1;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()){
    std::getline(stream, line);
    std::size_t i = line.find(' ');
    line = line.substr(i+1, line.size());
    i = line.find(' ');
    vector<string> vect;
    while(i != string::npos){
      vect.push_back(line.substr(0,i));
      line = line.substr(i+1, line.size());
      i = line.find(' ');
    }
    return vect;
  }
  return {};
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  string str;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()){
    while(true){
      std::getline(stream, str);
      std::size_t found = str.find("processes");
      if (found != std::string::npos){
        int i = str.find_first_of("0123456789");
        str = str.substr(i, str.size());
        int num = atol(str.c_str());
        return num;
      }
  	}
  }
  return -1; 
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
    string str;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()){
    while(true){
      std::getline(stream, str);
      std::size_t found = str.find("procs_running");
      if (found != std::string::npos){
        int i = str.find_first_of("0123456789");
        str = str.substr(i, str.size());
        int num = atol(str.c_str());
        return num;
      }
  	}
  }
  return -1; 
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) { 
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if(stream.is_open()){
    std::getline(stream, line);
    return line;
  }
  return "-1";
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) { 
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if(stream.is_open()){
    for(int i=0; i<18; i++){
      std::getline(stream, line);
    }
    int j = line.find(' ');
    line = line.substr(j, line.size());
    long num = atol(line.c_str());
    num /= 1000;
    return to_string(num);
  }
  return "-1";
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string str;
  std::ifstream stream(kProcDirectory + to_string(pid) +kStatusFilename);
  if(stream.is_open()){
    while(stream.is_open()){
      std::getline(stream, str);
      std::size_t found = str.find("Uid");
      if (found != std::string::npos){
        int i = str.find_first_of("0123456789");
        string rest = str.substr(i, str.size());
        i = rest.find_first_of("\t");
        str = rest.substr(0, i);
        return str;
      }
  	}
  }
  return "-1"; 
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string str;
  std::ifstream stream(kPasswordPath);
  string uid = Uid(pid);
  if(stream.is_open()){
    while(std::getline(stream, str)){
      std::size_t found = str.find(":x:");
      if (found != std::string::npos){
        int i = str.find_first_of("0123456789");
        string uidLine = str.substr(i, i+uid.size()-1);
        return str.substr(0, found);
      }
  	}
  }
  return "-1"; 
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) { 
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if(stream.is_open()){
    std::getline(stream, line);
    for(int i=0; i<21; i++){
      int j = line.find(' ');
      line = line.substr(j+1, line.size());
    }
    int j = line.find(' ');
    line = line.substr(0, j);
    return atol(line.c_str());
  }
  return -1;
}