#include "../include/ci.hpp"

/** testingSequence
 *  Use this sequence to compile, test and return 
 *  aka this is the thing that does the stuff 
 *  --------MAKE BETTER COMMENT AS CODE IS DEVELOPED -----------------
 */
void testingSequence(std::string ref, std::string cloneUrl, std::string commitSHA, std::string branch, std::string apiUrl) {
    // git clone the branch at commit sha
    cloneFromGit(cloneUrl, commitSHA, branch);
    std::string projPath = "repos " + commitSHA;

    // make the cloned folder
    compileProject(projPath);

    // make test the cloned folder, return status
    int res = testProject(projPath);

    // Copy log to Results folder
    std::string log_command = "cp " + projPath + "/result.log logs/" + commitSHA + ".log";
    std::system(log_command.c_str());

    // message git with commit status
    notifyCommitStatus(apiUrl);

    // p+: save to database
}

int cloneFromGit(std::string cloneUrl, std::string commitSHA, std::string branch) {
    // Clone options
    std::string clone_command = "git clone --branch " + branch + " " + cloneUrl + " repos/" + commitSHA + " > /dev/null 2>&1";

    // Clone repository
    int res = std::system(clone_command.c_str());
    if (res != 0) {
        return 1;
    }

    // Go to the commitSHA
    std::string sha_command =  "cd repos/" + commitSHA + " && git reset --hard " + commitSHA + " > /dev/null 2>&1";
    res = std::system(sha_command.c_str());
    if (res != 0) {
        return 2;
    }    

    return 0;
}

int compileProject(std::string projPath) {
    // Make code 
    std::string make_command =  "cd " + projPath + " && make -s > /dev/null 2>&1";
    return std::system(make_command.c_str()); 
}

int testProject(std::string projPath) {
    std::string test_command =  "cd " + projPath + " && make -s test > result.log";
    return std::system(test_command.c_str());
}

int notifyCommitStatus(std::string apiUrl) {
    httplib::Client client("https://api.github.com");
    std::string apiPath = apiUrl.substr(22);

    std::string token = std::getenv("GITHUB_COMMIT_STATUS_TOKEN");
    nlohmann::json payload = {{"state", "success"}, {"context", "ci/testci"}};
    nlohmann::json headers =  {{"Authorization", "token " + token}, {"User-Agent", "My-CI-Server"}, {"Accept", "application/vnd.github.v3+json"}};

    auto res = client.Post(apiPath.c_str(), headers, payload.dump(), "application/json");
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
            std::string statusUrl = payload["repository"]["statuses_url"];

            #ifndef TESTING
                // do CI on recieved repository
                std::thread pr(testingSequence, ref, cloneUrl, commitSHA, branch, statusUrl);
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
                std::string statusUrl = payload["repository"]["statuses_url"];

                #ifndef TESTING
                    // do CI on recieved repository
                    std::thread pr(testingSequence, ref, cloneUrl, commitSHA, branch, statusUrl);
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