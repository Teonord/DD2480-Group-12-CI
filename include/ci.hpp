#ifndef CI_H
#define CI_H

#include <iostream>
#include <thread>
#include <array>
#include "../external/httplib.h"
#include "../external/json.hpp"
#include <sqlite3.h>

using json = nlohmann::json;

const int port = 8012;

void incomingWebhook(const httplib::Request &req, httplib::Response &res);
void listCommits(const httplib::Request &req, httplib::Response &res);
void sendCommitInfo(std::string publicKey, httplib::Response &res);
void testingSequence(std::string ref, std::string cloneUrl, std::string commitSHA, std::string branch);
int cloneFromGit(std::string cloneUrl, std::string commitSHA, std::string branch);
int compile_Makefile(std::string repoPath);
bool connectDB();
bool insertToDB(std::string commitSHA, std::string buildLog);

#endif
