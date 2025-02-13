/**
 * @file ci.hpp
 * @brief Header file for Continuous Integration system functions.
 *
 * This file contains function declarations for handling webhooks, managing repositories,
 * compiling and testing code, and interacting with a database.
 */

#ifndef CI_H
#define CI_H

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <iostream>
#include <thread>
#include <array>
#include "../external/httplib.h"
#include "../external/json.hpp"
#include <sqlite3.h>
#include <filesystem>

using json = nlohmann::json;

/** @brief Port number for the server. */
const int port = 8012;

/** @brief Pointer to the SQLite database. */
extern sqlite3 *db;

/**
 * @brief Handles incoming webhooks.
 * @param req The incoming HTTP request.
 * @param res The HTTP response to be sent.
 */
void incomingWebhook(const httplib::Request &req, httplib::Response &res);

/**
 * @brief Lists the commits stored in the system.
 * @param req The incoming HTTP request.
 * @param res The HTTP response containing commit data.
 */
void listCommits(const httplib::Request &req, httplib::Response &res);

/**
 * @brief Sends commit information.
 * @param publicKey The public key associated with the commit.
 * @param res The HTTP response to be sent.
 */
void sendCommitInfo(std::string publicKey, httplib::Response &res);

/**
 * @brief Runs the testing sequence for a given repository.
 * @param cloneUrl The repository clone URL.
 * @param commitSHA The commit SHA to be tested.
 * @param branch The branch to be tested.
 * @param statusUrl The URL to send status updates.
 * @return Integer representing success (0) or failure.
 */
int testingSequence(std::string cloneUrl, std::string commitSHA, std::string branch, std::string statusUrl);

/**
 * @brief Clones a repository from Git.
 * @param cloneUrl The repository clone URL.
 * @param commitSHA The commit SHA to checkout.
 * @param branch The branch to be cloned.
 * @return Integer representing success (0) or failure.
 */
int cloneFromGit(std::string cloneUrl, std::string commitSHA, std::string branch);

/**
 * @brief Compiles the repository using a Makefile.
 * @param repoPath The path to the repository.
 * @return Integer representing success (0) or failure.
 */
int compile_Makefile(std::string repoPath);

/**
 * @brief Establishes a connection to the database.
 * @return Boolean indicating success or failure.
 */
bool connectDB();

/**
 * @brief Creates necessary tables in the database.
 * @return Boolean indicating success or failure.
 */
bool createTables();

/**
 * @brief Inserts commit data into the database.
 * @param commitSHA The commit SHA identifier.
 * @param buildLog The log of the build process.
 * @return Boolean indicating success or failure.
 */
bool insertToDB(std::string commitSHA, std::string buildLog);

/**
 * @brief Reads a file and returns its contents as a string.
 * @param filePath The path to the file.
 * @return The file contents as a string.
 */
std::string readFile(const std::string& filePath);

/**
 * @brief Runs tests on the repository.
 * @param repoPath The path to the repository.
 * @return Integer representing success (0) or failure.
 */
int makeTests(std::string repoPath);

/**
 * @brief Notifies the commit status to an external service.
 * @param commitSHA The commit SHA identifier.
 * @param status The status message.
 * @return Integer representing success (0) or failure.
 */
int notifyCommitStatus(std::string commitSHA, std::string status);

#endif // CI_H
