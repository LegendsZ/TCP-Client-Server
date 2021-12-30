#pragma once

#include <iostream>
#include "client.h"
#include "server.h"

void FprintStuffC(bool* updateScreenG, client* clientG);
void FprintStuffS(bool* updateScreenG, server* clientG);

class printStuffS { //singleton
public:
    static bool updateScreen;
    static std::thread updateScreenThread;
    static bool init(client* clientOBJ) {
        updateScreen = true;
        updateScreenThread = std::thread(FprintStuffC, &printStuffS::updateScreen, clientOBJ);
        return true;
    }
    static bool init(server* serverOBJ) {
        updateScreen = true;
        updateScreenThread = std::thread(FprintStuffS, &printStuffS::updateScreen, serverOBJ);
        return true;
    }
};

bool printStuffS::updateScreen;
std::thread printStuffS::updateScreenThread;

int main()
{
    system("title TCP_CS");

    client* clientOBJ = nullptr;
    server* serverOBJ = nullptr;

    while (true) {
        std::string userinput;
        std::cout << "C/c for connect\nS/s for server\nE/e for exit\n\n";
        std::getline(std::cin, userinput);
        if (userinput == "C" or userinput == "c") {
            std::string ipG;
            std::cout << "IP: ";
            std::getline(std::cin, ipG);

            std::string portG;
            std::cout << "PORT: ";
            std::getline(std::cin, portG);

            clientOBJ = new client(ipG, std::stoi(portG));
            break;
        }
        else if (userinput == "S" or userinput == "s") {
            std::string portG;
            std::cout << "PORT: ";
            std::getline(std::cin, portG);
            serverOBJ = new server(std::stoi(portG));
            break;
        }
        else if (userinput == "E" or userinput == "e") {
            break;
        }
        else {
            std::cout << "Error - Unrecognized command!\n";
        }
    }

    
    if (clientOBJ != nullptr) {
        if (!clientOBJ->autoStart()) {
            return 0;
        }
        printStuffS::init(clientOBJ);
        std::cout << "\n\n";
    }
    if (serverOBJ != nullptr) {
        if (!serverOBJ->autoStart()) {
            return 0;
        }
        std::cout << "Once you are done accepting, ";
        system("pause");
        std::cout << "Stopped accepting clients!\n\n";
        serverOBJ->endFindConnections();
        serverOBJ->startListening();
        printStuffS::init(serverOBJ);
    }
    while (clientOBJ != nullptr) {
        std::cout << "> ";
        std::string userInput;
        std::getline(std::cin, userInput);
        if (userInput == "~exit") {
            break;
        }
        if (!clientOBJ->sendData(userInput)) {
            std::cout << "Failed to send message!";
            return 0; //?
        }
    }
    while (serverOBJ != nullptr) {
        std::cout << "Pick a phrase to guess: ";
        std::string word;
        std::getline(std::cin, word);
        std::cout << "Provide a hint: ";
        std::string hint;
        std::getline(std::cin, hint); //work here

        std::cout << "> ";
        std::string userInput;
        std::getline(std::cin, userInput);
        if (userInput == "~exit") {
            break;
        }
        if (!serverOBJ->sendData(1,userInput)) {
            std::cout << "Failed to send message!";
            return 0; //?
        }
    }

    //terminate / cleanup
    if (clientOBJ != nullptr) {
        printStuffS::updateScreen = false;
        printStuffS::updateScreenThread.detach();
        clientOBJ->Terminate();
        delete clientOBJ;
    }
    if (serverOBJ != nullptr) {
        printStuffS::updateScreen = false;
        printStuffS::updateScreenThread.detach();
        serverOBJ->Terminate();
        delete serverOBJ;
    }
    std::cout << "Successfully exited.\n"; //problem terminating -- potentially with wsacleanup()
    system("pause");
}

void FprintStuffC(bool* updateScreenG, client* clientG) {
    while (*updateScreenG) {
        if (clientG->recieved.size() != 0) {
            std::cout << "Console: " << clientG->recieved.front() << "\n> ";
            clientG->recieved.erase(clientG->recieved.begin());
        }
    }
}

void FprintStuffS(bool* updateScreenG, server* serverG) {
    while (*updateScreenG) {
        if (serverG->recieved.size() != 0) {
            std::cout << "Console: " << serverG->recieved.front() << "\n> ";
            serverG->recieved.erase(serverG->recieved.begin());
        }
    }
}