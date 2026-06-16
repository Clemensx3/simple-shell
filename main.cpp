#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <map>

enum class Codes
{
  SUCCESS_EXIT,
  FAILURE_EXIT,
  NO_EXIT
};

using func = Codes (*)(std::vector<std::string>&);

Codes commandCd(std::vector<std::string>& args);
Codes commandHelp(std::vector<std::string>& args);
Codes commandExit(std::vector<std::string>& args);

std::map<std::string, func> builtins
{
  {"cd", commandCd},
  {"help", commandHelp},
  {"exit", commandExit}
}; 


void splitLine(std::vector<std::string>& args, std::string line)
{
  std::stringstream ss(line);
  std::string curr_part;

  while(ss >> curr_part)
  {
    args.push_back(curr_part);
  }
}

Codes launch(std::vector<std::string>& args)
{
  pid_t pid;
  int status;

  //To convert string into const char* for execvp
  std::vector<char*> c_args;
  for(const auto& arg : args) {
    c_args.push_back(const_cast<char*>(arg.c_str()));
  }
  c_args.push_back(nullptr);

  pid = fork();
  if(pid == 0) {
    if(execvp(c_args.at(0), c_args.data()) == -1) {
      std::perror("shell");
    }
    std::exit(EXIT_FAILURE);
  }
  else if(pid < 0) {
    std::perror("shell");
  }
  else {
    waitpid(pid, &status, 0);
  }

  return Codes::NO_EXIT;
}

Codes execute(std::vector<std::string>& args)
{
  if(args.empty()) return Codes::NO_EXIT;

  std::string command = args.at(0);

  if(builtins.find(command) != builtins.end()) {
    return builtins[command](args);
  }

  return launch(args);
}


void command_loop()
{
  std::string line;
  Codes status;

  do 
  {
    std::vector<std::string> args;

    std::cout << "> ";
    if(!getline(std::cin ,line)) break;
    splitLine(args, line);
    status = execute(args);
  } while(status  == Codes::NO_EXIT);
}

Codes commandCd(std::vector<std::string>& args) {
  if(args.size() <= 1) {
    std::cout << stderr << "shell: expected argument to \"cd\"" << std::endl;
  }
  else {
    char* arg = const_cast<char*>(args.at(1).c_str());
    if(chdir(arg) != 0) {
      perror("shell");
    }
  }

  return Codes::NO_EXIT;
}

Codes commandHelp(std::vector<std::string>& args) {
  std::cout << "Type the program name and arguments and hit enter.\n";
  std::cout << "There are the following are built in: \n";

  for(auto& builtin : builtins) {
    std::cout << "  " << builtin.first << "\n";
  }

  std::cout << "Use man command for information on other programs." << std::endl;
  return Codes::NO_EXIT;
}

Codes commandExit(std::vector<std::string>& args) {
  return Codes::SUCCESS_EXIT;
}

int main()
{
  command_loop();

  return EXIT_SUCCESS;
}