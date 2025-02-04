#include "../include/ci.hpp"

int main(int argc, char** argv) {
  httplib::Server server;

  server.Post("/ci_webhook", incomingWebhook);

  std::cout << "Server starting on port " << port << std::endl;
  
  server.listen("0.0.0.0", port);
  return 0;
}
