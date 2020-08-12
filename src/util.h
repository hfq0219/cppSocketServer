#pragma once

#include <string>

int readEagain(int fd,std::string &buff);
int writeEagain(int fd,std::string &buff);
int setSocketNonBlocking(int fd);
void ignoreSigpipe();