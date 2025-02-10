#ifndef CI_H
#define CI_H

#include <iostream>
#include <thread>
#include <array>
#include "../external/httplib.h"
#include "../external/json.hpp"

using json = nlohmann::json;

const int port = 8012;

void incomingWebhook(const httplib::Request &req, httplib::Response &res);

int cloneFromGit(std::string cloneUrl, std::string commitSHA, std::string branch);

int compileProject(std::string projPath);

int testProject(std::string projPath);

void notifyCommitStatus(std::string apiUrl, std::string status);

#endif
