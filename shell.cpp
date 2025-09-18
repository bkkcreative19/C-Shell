#include <iostream>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <bits/stdc++.h>
#include <sstream>
#include <string>
#include <map>


void SetColor(int textColor)
{
    std::cout << "\033[" << textColor << "m";
}

void ResetColor() {std::cout << "\033[0m";}

std::string expandVar(const std::string &token)
{
    if(!token.empty() && token[0] == '$')
    {
        const char* val = getenv(token.c_str() + 1);
        return val ? val : "";
    }

    return token;
}


std::vector<char*> getArgs(std::string command)
{
        std::vector<std::string> tokens{};
        std::stringstream check1(command);
        std::string intermediate{};

        while(std::getline(check1, intermediate, ' '))
        {
            tokens.push_back(intermediate);
        }

        std::vector<char*> args_c{};

        for (const auto& s : tokens)
        {
            
            if(s[0] == '$')
            {
                std::string yay = expandVar(std::string(s));
                args_c.push_back(const_cast<char*>(yay.c_str()));
            } else
            {
                args_c.push_back(const_cast<char*>(s.c_str()));
            }
        
        }

        args_c.push_back(NULL);

        return args_c;
}


int runCommand(std::string command)
{
    std::vector<char*> args{getArgs(command)};

    pid_t pid{fork()};

    if(pid < 0)
    {
        std::cerr << "Fork Failed." << std::endl;
        return 1;
    } else if(pid == 0)
    {
        
        int status_code = execvp(args[0], args.data());

        if(status_code == -1)
        {
            std::cout << args[0] << ": " << "command not found" << std::endl;
            // perror("failed");
            exit(1);
        }
    } else
    {
        // std::cout << "Parent process: PID = " << getpid() << "Child PID = " << getpid() << std::endl;
        wait(NULL);
    }

    return 0;
}

void builtIn(std::string command)
{
    std::vector<char*> args{getArgs(command)};

    if(std::string(args[0]) == "cd")
    {
        std::filesystem::path current_dir{std::filesystem::current_path()};

        std::string new_path{args[1]};
        std::filesystem::path new_dir = new_path;

        try
        {
            std::filesystem::current_path(new_dir);
        }
        catch(const std::filesystem::filesystem_error& e)
        {
            std::cerr << e.what() << '\n';
        }
        
        
    } else if(std::string(args[0]) == "pwd")
    {
        SetColor(88);
        std::cout << std::filesystem::current_path().string() << std::endl;

    } else if(std::string(args[0]) == "echo")
    {
        std::string ouput{};
        for(size_t i = 1; i < args.size(); ++i)
        {
            if(args[i] == NULL)
            {
                break;
            }
            ouput += args[i];
        }
        std::cout << ouput << std::endl;

    }
}


void printPrompt()
{
    SetColor(90);
    char hostname[50];
    char username[50];
    gethostname(hostname, 50);
    getlogin_r(username, 50);
    
    std::cout << username
        << "@"
        << hostname
        << ':';
    

    std::string home{std::getenv("HOME")};

    std::string path{std::filesystem::current_path().string()};
    
    SetColor(93);
    if(path.find("/home") != std::string::npos)
    {
        path.erase(0, home.length());
        std::cout << "~" 
        << path;
    } else 
    {
        std::cout << path;
    }
    std::cout << "$ ";
    ResetColor();
}

int main()
{
    while(true)
    {
        printPrompt();
        
        std::string input{};

        std::getline(std::cin, input);

        if(input == "exit")
        {
            break;
        } else if((input.find("cd") != std::string::npos) || (input.find("pwd") != std::string::npos) || (input.find("echo") != std::string::npos))
        {
            builtIn(input);
        } else
        {
            runCommand(input);
        }
        
    }
}