#include "../include/ci.hpp"


/** testingSequence
 *  Use this sequence to compile, test and return 
 *  aka this is the thing that does the stuff 
 *  -------- MAKE BETTER COMMENT AS CODE IS DEVELOPED -----------------
 */
void testingSequence(std::string ref, std::string cloneUrl, std::string commitSHA, std::string branch) {
    // git clone the branch at commit sha
    cloneFromGit(cloneUrl, commitSHA, branch);

    // make the cloned folder

    // make test the cloned folder, return status

    // message git with commit status

    // p+: save to database
}
/* cloneFromGit
 * 
 * This function constructs a git clone command and executes it.
 * It also constructs command to go to the specific commit at commitSHA.
 */
int cloneFromGit(std::string cloneUrl, std::string commitSHA, std::string branch) {
    // Clone options
    std::string clone_command = "git clone --branch " + branch + " " + cloneUrl + " repos/" + commitSHA;

    // Clone repository
    int res = std::system(clone_command.c_str());
    if (res != 0) {
        return 1;
    }

    // Go to the commitSHA
    std::string sha_command =  "cd repos/" + commitSHA + " && git reset --hard " + commitSHA;
    res = std::system(sha_command.c_str());
    if (res != 0) {
        return 2;
    }    

    return 0;
}


/** incomingWebhook
 *  Function to handle HTTP messages sent to the webhook from GitHub. When a request (and response object)
 *  is recieved from the server, parse the body and check the event header. If event is to be processed, 
 *  extract the required parameters to be used for the CI. If not used, return a successfull response but
 *  don't use it. If error in packet, return a 400 status.
 * 
 *  @param req a httplib request containing information from GitHub
 *  @param res a response to send back to GitHub for it to know that the
 *             request is being serviced. 
 */
void incomingWebhook(const httplib::Request &req, httplib::Response &res) {
    try {
        auto payload = json::parse(req.body);

        std::string githubEvent = req.get_header_value("X-GitHub-Event");

        if (githubEvent == "push") {

            res.set_content("push recv", "text/plain");
            res.status = 200;

            std::string ref = payload["ref"];
            std::string cloneUrl = payload["repository"]["clone_url"];
            std::string commitSHA = payload["head_commit"]["id"];
            std::string branch = ref.substr(11);

            #ifndef TESTING
                // do CI on recieved repository
                std::thread pr(testingSequence, ref, cloneUrl, commitSHA, branch);
                pr.detach();
            #endif
            
        } else if (githubEvent == "pull_request") {
            std::string action = payload["action"];

            if (action == "opened" || action == "synchronize" || action == "reopened") {
                res.set_content("pr recv", "text/plain");
                res.status = 201;

                std::string ref = payload["ref"];
                std::string cloneUrl = payload["repository"]["clone_url"];
                std::string commitSHA = payload["pull_request"]["head"]["sha"];
                std::string branch = payload["pull_request"]["head"]["ref"];

                #ifndef TESTING
                    // do CI on recieved repository
                    std::thread pr(testingSequence, ref, cloneUrl, commitSHA, branch);
                    pr.detach();
                #endif

            } else {
                res.set_content("non-used pr recv", "text/plain");
                res.status = 202;
            }
            
        } else {

            res.set_content("non-used recv", "text/plain");
            res.status = 203;

            #ifndef TESTING
                std::cout << "[Warning]: Non-push or pull_request event recieved" << std::endl;
            #endif
        }

    } catch (std::exception &e) {
        res.status = 400;
        res.set_content("err", "text/plain");
    }
} 