#include "../include/ci.hpp"
/** main
 *  Main function to run the CI server. Uses httplib with developed callbacks
 *  to get data from a webhook to process the CI execution, but also to run
 *  an extra listing service which shows links to all previous runs, links 
 *  which lets you see extra information about each commit. 
 * 
 *  Server runs on port 8012
 * 
 *  @param argc unused parameter
 *  @param argv unused parameter
 */
int main(int argc, char** argv) {
  httplib::Server server;

  server.Post("/ci_webhook", incomingWebhook);
  server.Get("/list", listCommits);
  server.Get("/id/:pk", [](const httplib::Request &req, httplib::Response &res) {
    std::string publicKey = req.path_params.at("pk");
    sendCommitInfo(publicKey, res);
  });

  std::cout << "Server starting on port " << port << std::endl;
  
  server.listen("0.0.0.0", port);
  return 0;
}
