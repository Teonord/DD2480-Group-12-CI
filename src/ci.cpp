#include "../include/ci.hpp"

sqlite3 *db = nullptr;


/** readFile
 *  Given a filepath reads the filecontent and returns it as a string.
 *  If no file was found or error opening file returns empty string.
 */
std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}




/** testingSequence
 *  Use this sequence to compile, test and return 
 *  aka this is the thing that does the stuff 
 *  -------- MAKE BETTER COMMENT AS CODE IS DEVELOPED -----------------
 */
int testingSequence(std::string cloneUrl, std::string commitSHA, std::string branch) {
    // git clone the branch at commit sha
    cloneFromGit(cloneUrl, commitSHA, branch);

    // make the cloned folder
    std::string repoPath = "repos/" + commitSHA;
    int res = compile_Makefile(repoPath);

    // make test the cloned folder, return status
    res = makeTests(repoPath);

    runTests(repoPath + "build/tests");

    // p+: save to database
    std::string filePath = "tests.log";
    std::string buildLog = readFile(filePath);
    insertToDB(commitSHA, buildLog);

    return 0;
}

/** runTests
 * runs the test binary and saves output to tests.log
 * 
 */

int runTests(std::string filePath) {
    if (std::getenv("INSIDE_TEST_BINARY")) {
        std::cerr << "Already inside test binary, skipping runTests() to prevent recursion.\n";
        return 0;
    }

    // Set the environment variable to indicate we're running tests
    setenv("INSIDE_TEST_BINARY", "1", 1);

    std::string runCmd = filePath + " > tests.log";
    int res = std::system(runCmd.c_str());

    return res;
}



/* compile_Makefile
 *
 * This function takes as input the location where the cloned repository is stored. 
 * Then it tries to make the cloned repository and returns 1 if unsuccesful. 
 */
int makeTests(std::string repoPath) {
    // Go to Makefile
    std::string make_command = "cd " + repoPath + " && make build/tests > build.log > 2&1";

    int res = std::system(make_command.c_str());
    if(res != 0) {
        return 1;
    }

    // Cleans compiled files
    std::string clean_command = "cd " + repoPath + " && make clean -s";
    std::system(clean_command.c_str());

    return 0;
}   

/* compile_Makefile
 *
 * This function takes as input the location where the cloned repository is stored. 
 * Then it tries to make the cloned repository and returns 1 if unsuccesful. 
 */
int compile_Makefile(std::string repoPath) {
    // Go to Makefile
    std::string make_command = "cd " + repoPath + " && make -s";

    int res = std::system(make_command.c_str());
    if(res != 0) {
        std::cout << "make command " << make_command << std::endl;
        std::cout << "returning 1" << std::endl;
        return 1;
    }

    // Cleans compiled files
    std::string clean_command = "cd " + repoPath + " && make clean -s";
    std::system(clean_command.c_str());

    return 0;
}   

/* cloneFromGit
 * 
 * This function constructs a git clone command and executes it.
 * It also constructs command to go to the specific commit at commitSHA.
 */
int cloneFromGit(std::string cloneUrl, std::string commitSHA, std::string branch) {

    if (std::filesystem::exists("repos/" + commitSHA)) {
        std::filesystem::remove_all("repos/" + commitSHA);
    }

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
                std::thread pr(testingSequence, cloneUrl, commitSHA, branch);
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
                    std::thread pr(testingSequence, cloneUrl, commitSHA, branch);
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


//returns true if connected to the database, else false
bool connectDB(){
    int rc = sqlite3_open("dd2480-ci.db", &db);
    if(rc){
        std::cout << "failed to connect to the database" << std::endl;
        return false;
    } 
    return true;
}

bool createTables(){
    const char* query = R"(
        CREATE TABLE IF NOT EXISTS ci_table (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            commit_id VARCHAR(40) NOT NULL,
            timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            build_log TEXT
        );
        )";
    const char* query2 = R"(
        CREATE TABLE IF NOT EXISTS ci_tests (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            commit_id VARCHAR(40) NOT NULL,
            timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            build_log TEXT
        );
        )";

    char *errMsg = nullptr;

    int exit = sqlite3_exec(db, query, nullptr, 0, &errMsg);

    if (exit != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false; // Failed to create table
    }

    exit = sqlite3_exec(db, query2, nullptr, 0, &errMsg);

    if (exit != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false; // Failed to create table
    }

    //std::cout << "Tables created successfully!" << std::endl;
    return true; 
}

//returns true if insert into db was succ
//returns false if not
bool insertToDB(std::string commitSHA, std::string buildLog){
    connectDB();
    createTables();

    #ifndef TESTING
    const char *query = "INSERT INTO ci_table (commit_id, build_log) VALUES (?, ?)";
    #else 
    const char *query = "INSERT INTO ci_tests (commit_id, build_log) VALUES (?, ?)";
    #endif

    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    } 

    sqlite3_bind_text(stmt, 1, commitSHA.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, buildLog.c_str(), -1, SQLITE_STATIC);
    

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    return true;
    
}

/** listCommits
 *  Function to handle HTTP get messages to grab a list of all builds done by the CI
 *  server in the past. req does not matter as function is only called when going to
 *  a specific / of the url. Begins by grabbing all the ids and commit numbers from
 *  the database, adds them to the HTML as links.
 * 
 *  @param req required argument for httplib callback function
 *  @param res response with HTML to the caller.
 */
void listCommits(const httplib::Request &req, httplib::Response &res) {
    try
    {

        #ifndef TESTING
        const char *query = "SELECT id, commit_id FROM ci_table";
        #else 
        const char *query = "SELECT id, commit_id FROM ci_tests";
        #endif

        sqlite3_stmt *stmt;

        int rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            if (stmt) sqlite3_finalize(stmt);
            res.status = 401;
            res.set_content("sql err", "text/plain");
            return;
        } 

        std::string html;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string publicKey = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string commit = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

            html += "<a href='/id/" + publicKey + "'>" + commit + "<br>";
        }

        sqlite3_finalize(stmt);

        res.status = 200;
        res.set_content(html, "text/html");
    }
    catch(std::exception &e)
    {
        res.status = 400;
        res.set_content("err", "text/plain");
    }
}

/** sendCommitInfo
 *  Function to get data from the database and send it back as HTML for
 *  a specific CI execution. First grabs neccessary columns from publicKey row
 *  then sets these ordered in a HTML document and sends back through res.
 * 
 *  @param publicKey which publicKey row to grab from. 
 *  @param res response with HTML to the caller.
 */
void sendCommitInfo(std::string publicKey, httplib::Response &res) {
    try
    {
        #ifndef TESTING
        const char *query = "SELECT commit_id, timestamp, build_log FROM ci_table WHERE id = ?";
        #else 
        const char *query = "SELECT commit_id, timestamp, build_log FROM ci_tests WHERE id = ?";
        #endif

        sqlite3_stmt *stmt;

        int rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            if (stmt) sqlite3_finalize(stmt);
            res.status = 401;
            res.set_content("sql err", "text/plain");
            return;
        } 


        sqlite3_bind_text(stmt, 1, publicKey.c_str(), -1, SQLITE_STATIC);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            res.status = 404;
            res.set_content("sql err 404", "text/plain");
            sqlite3_finalize(stmt);
            return;
        }

        std::string commit = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string buildLog = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        std::string html;

        html += "Test ID: " + publicKey;
        html += "<br>Commit SHA: " + commit;
        html += "<br>Timestamp: " + timestamp;
        html += "<br>Log:<br>" + buildLog;

        sqlite3_finalize(stmt);

        res.status = 200;
        res.set_content(html, "text/html");
    }
    catch(std::exception &e)
    {
        res.status = 400;
        res.set_content("err", "text/plain");
    }
}