#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>

enum class Codes
{
  SUCCESS_EXIT,
  FAILURE_EXIT,
  NO_EXIT
};

void split_line(std::vector<std::string>& args, std::string line)
{
  std::stringstream ss(line);
  std::string curr_part;

  while(ss >> curr_part)
  {
    args.push_back(curr_part);
  }
}

Codes execute(std::vector<std::string>& args)
{
  if(args.empty()) return Codes::NO_EXIT;
  else if(args.at(0) == "quit") return Codes::SUCCESS_EXIT;

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
      std::perror("lsh");
    }
    std::exit(EXIT_FAILURE);
  }
  else if(pid < 0) {
    std::perror("lsh");
  }
  else {
    waitpid(pid, &status, 0);
  }

  return Codes::NO_EXIT;
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
    split_line(args, line);
    status = execute(args);
  } while(status  == Codes::NO_EXIT);
}

int main()
{
  command_loop();

  return EXIT_SUCCESS;
}