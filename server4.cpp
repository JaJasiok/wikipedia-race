#include <iostream>
#include <vector>
#include <map>
#include <pthread.h>
#include <poll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <algorithm>
#include <string.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <fstream>
#include <ctime>
#include <cstdlib>

std::vector<std::string> lines;
std::string line;

const int MAX_CLIENTS = 100;

const int buffer_size = 1024;

std::map<std::string, int> usernames;
std::map<std::string, std::vector<std::string>> history;

const int minPlayers = 2;

int playerCount = 0;

bool started = false;

std::string start;
std::string destination;

std::vector<pollfd> clients;
pthread_mutex_t clientLock = PTHREAD_MUTEX_INITIALIZER;

void getUrls(){
    srand(time(0));
    int random_line = rand() % lines.size();
    start = lines[random_line];

    random_line = rand() % lines.size();
    destination = lines[random_line];
}

void closeSockets() {
    for (const auto& [name, fd] : usernames) {
        close(fd);
    }
}

void sigintHandler(int signum) {
    closeSockets();
    exit(0);
}

void* handleClient(void* arg) {
    int clientFd = *((int*)arg);
    char buffer[buffer_size];
    std::string username;
    bool unique = false;
    while (!unique) {
        int numBytes = read(clientFd, buffer, sizeof(buffer));
        if (numBytes <= 0) {
            break;
        }
        buffer[numBytes] = '\0';
        username = buffer;
        pthread_mutex_lock(&clientLock);
        if (usernames.find(username) == usernames.end()) {
            usernames[username] = clientFd;
            char message[buffer_size] = "OK";
            write(clientFd, message, buffer_size);
            unique = true;
        } else {
            char message[buffer_size] = "NO";
            write(clientFd, message, buffer_size);
        }
        pthread_mutex_unlock(&clientLock);
    }
    if (unique) {
        for (const auto& [name, fd] : usernames) {
            if (fd != clientFd) {
                char message1[buffer_size] = "Joined:";
                int i = 0;
                for(; i < username.length(); i++){
                    message1[i + 7] = username[i];
                }
                write(fd, message1, buffer_size);
                char message2[buffer_size] = "Joined:";
                i = 0;
                for(; i < name.length(); i++){
                    message2[i + 7] = name[i];
                }
                write(clientFd, message2, buffer_size);
            }    
        }
        if(started == true){
            char message1[buffer_size] = "Start:";
            int i = 0;
            for(; i < start.length(); i++){
                message1[i + 6] = start[i];
            }
            write(clientFd, message1, buffer_size);
            char message2[buffer_size] = "Dest:";
            i = 0;
            for(; i < destination.length(); i++){
                message2[i + 5] = destination[i];
            }
            write(clientFd, message2, buffer_size);
        }
        {
            pthread_mutex_lock(&clientLock);
            playerCount++;
            std::cout << playerCount << std::endl;
            if (playerCount >= minPlayers && started == false) {
                getUrls();
                started = true;
                for (const auto& [name, fd] : usernames) {
                    char message1[buffer_size] = "Start:";
                    int i = 0;
                    for(; i < start.length(); i++){
                        message1[i + 6] = start[i];
                    }
                    write(fd, message1, buffer_size);
                    char message2[buffer_size] = "Dest:";
                    i = 0;
                    for(; i < destination.length(); i++){
                        message2[i + 5] = destination[i];
                    }
                    write(fd, message2, buffer_size);
                }
            }
            pthread_mutex_unlock(&clientLock);
        }
        while (true) {
            int n = read(clientFd, buffer, sizeof(buffer));
            if (n == 0) {
                {
                    pthread_mutex_lock(&clientLock);
                    playerCount--;
                    // std::cout << playerCount << std::endl;
                    pthread_mutex_unlock(&clientLock);
                }
                for (const auto& [name, fd] : usernames) {
                    char message1[buffer_size] = "Quited:";
                    int i = 0;
                    for(; i < username.length(); i++){
                        message1[i + 7] = username[i];
                    }
                    write(fd, message1, buffer_size);
                }
                pthread_mutex_lock(&clientLock);
                usernames.erase(username);
                history.erase(username);
                pthread_mutex_unlock(&clientLock);
                close(clientFd);
                break;
            }
            std::string message(buffer, n);
            //std::cout << message << std::endl;
            // if(message.rfind("text/html",0) == 0)
            //{
                pthread_mutex_lock(&clientLock);
                std::string article = message;
                history[username].push_back(article.erase(0, 30));
                pthread_mutex_unlock(&clientLock);
                std::cout << message << std::endl;
                if(message == destination){
                    for (const auto& [name, fd] : usernames) {
                        char message1[buffer_size] = "Win:";
                        int i = 0;
                        for(; i < username.length(); i++){
                            message1[i + 4] = username[i];
                        }
                        message1[i + 4] = ' ';
                        i++;
                        for(auto& article : history.at(username)){
                            for(int j=0; j < article.length(); j++){
                                message1[i + 4] = article[j];
                                i++;
                            }
                            message1[i + 4] = ' ';
                            i++;
                        }
                        write(fd, message1, buffer_size);
                    }
                    started = false;
                    // for(auto &itr: history)
                    // {
                    //     std::cout << itr.first;
                    //     for (std::string i: itr.second)
                    //     {
                    //         std::cout << " " << i << std::endl;
                    //     }
                    // }                    
                }
            //}
        }
    }
    pthread_mutex_lock(&clientLock);
    usernames.erase(username);
    for (auto it = clients.begin(); it != clients.end(); it++) {
        if (it->fd == clientFd) {
            clients.erase(it);
            break;
        }
    }
    close(clientFd);
    pthread_mutex_unlock(&clientLock);
    return nullptr;
}

int main(int argc, char* argv[]) {
    std::ifstream myfile("pages.txt");
    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
            lines.push_back(line);
        }
        myfile.close();
    }
    int port = 12345;
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverFd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }
    if (listen(serverFd, MAX_CLIENTS) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        return 1;
    }
    clients.push_back({serverFd, POLLIN});

    signal(SIGINT, sigintHandler);

    while (true) {
        if (poll(&clients[0], clients.size(), -1) < 0) {
            std::cerr << "Error polling clients" << std::endl;
            return 1;
        }
        pthread_mutex_lock(&clientLock);
        for (auto& c : clients) {
            if (c.fd == serverFd && c.revents & POLLIN) {
                int clientFd = accept(serverFd, nullptr, nullptr);
                if (clientFd < 0) {
                    std::cerr << "Error accepting client" << std::endl;
                } else {
                    clients.push_back({clientFd, POLLIN});
                    pthread_t thread;
                    int* arg = new int(clientFd);
                    pthread_create(&thread, nullptr, handleClient, arg);
                }
            }
        }
        pthread_mutex_unlock(&clientLock);
    }
    closeSockets();
    return 0;
}