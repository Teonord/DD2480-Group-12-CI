#ifndef CI_H
#define CI_H

#include <iostream>
#include <thread>
#include <array>
#include "../external/httplib.h"
#include "../external/json.hpp"
#include <sqlite3.h>
#include <filesystem>


using json = nlohmann::json;

const int port = 8012;

void incomingWebhook(const httplib::Request &req, httplib::Response &res);
int testingSequence(std::string cloneUrl, std::string commitSHA, std::string branch);
int cloneFromGit(std::string cloneUrl, std::string commitSHA, std::string branch);
int compile_Makefile(std::string repoPath);
bool connectDB();
bool insertToDB(std::string commitSHA, std::string buildLog);
std::string readFile(const std::string& filePath);
int makeTests(std::string repoPath);
int runTests(std::string repoPath);

#endif
