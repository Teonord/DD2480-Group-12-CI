#define CATCH_CONFIG_MAIN

#include "../external/catch.hpp"
#include "../include/ci.hpp"

TEST_CASE("push events handled correctly", "[incomingWebhook]") {
    httplib::Request req;
    httplib::Response res;

    json payload = {
        {"ref", "refs/heads/test1"},
        {"action", "opened"},
        {"repository", {{"clone_url", "https://test.test"}}},
        {"head_commit", {{"id", "testshaa"}}}
    };

    req.body = payload.dump();
    req.headers.insert({"X-GitHub-Event", "push"});

    incomingWebhook(req, res);

    REQUIRE(res.body == "push recv");
    REQUIRE(res.status == 200);
}

TEST_CASE("usable pull request events handled correctly", "[incomingWebhook]") {
    httplib::Request req;
    httplib::Response res;

    json payload = {
        {"ref", "refs/heads/test2"},
        {"action", "opened"},
        {"repository", {{"clone_url", "https://test.test"}}},
        {"pull_request", {
            {"head", {
                {"sha", "testsha"},
                {"ref", "branch"}
            }}
        }}
    };

    req.body = payload.dump();
    req.headers.insert({"X-GitHub-Event", "pull_request"});

    incomingWebhook(req, res);

    REQUIRE(res.body == "pr recv");
    REQUIRE(res.status == 201);
}

TEST_CASE("trash pull request events handled correctly", "[incomingWebhook]") {
    httplib::Request req;
    httplib::Response res;

    json payload = {
        {"ref", "refs/heads/test3"},
        {"action", "reviewed"},
        {"repository", {{"clone_url", "https://test.test"}}},
        {"pull_request", {
            {"head", {
                {"sha", "testsha"},
                {"ref", "branch"}
            }}
        }}
    };

    req.body = payload.dump();
    req.headers.insert({"X-GitHub-Event", "pull_request"});

    incomingWebhook(req, res);

    REQUIRE(res.body == "non-used pr recv");
    REQUIRE(res.status == 202);
}

TEST_CASE("other events handled correctly", "[incomingWebhook]") {
    httplib::Request req;
    httplib::Response res;

    req.body = R"({"ref": "refs/heads/test4"})";
    req.headers.insert({"X-GitHub-Event", "not_push"});

    incomingWebhook(req, res);

    REQUIRE(res.body == "non-used recv");
    REQUIRE(res.status == 203);
}

TEST_CASE("error handled with 400", "[incomingWebhook]") {
    httplib::Request req;
    httplib::Response res;

    req.body = "({broken_json_test5";
    req.headers.insert({"X-GitHub-Event", "push"});

    incomingWebhook(req, res);

    REQUIRE(res.body == "err");
    REQUIRE(res.status == 400);
}