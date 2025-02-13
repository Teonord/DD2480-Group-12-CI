# CI: A simple C++ Continuous Integration server for GitHub
### Group 12's submission for Assignment 2 in DD2480 Software Engineering Fundamentals

C++ program that compiles and tests GitHub repositories, updates the status of the commit on completion. 

This program can be linked as a webhook to GitHub repositories, after which it will run on any push and pull request. The code will run the Makefile to both compile and test the program. The server will then send a mail to the tester with information about whether the program passed or not. 

The program also allows the user to go to `/list` to get a list of all previous runs, and these runs can be clicked to get further information. 

The program will most likely be running on [This Link](basically-sure-piranha.ngrok-free.app) for the presentation.

## Structure
- **external**
    - Contains external code developed by other developers, namely the header libraries used.
- **include**
    - Contains header files for our own developed code and program. 
- **src**
    - Contains the developed code, split into two files, one for atomic functions and one for the main function.
- **tests**
    - Contains the unit tests for all developed functions in the code. Uses the Catch2 framework from "external/"
- **Makefile**
	- Contains functions to easily compile and test the program, further usage explained below. 

## Running Code

This assumes you are running **Ubuntu** for your local computer. 

Clone the project

```bash
  git clone git@github.com:Teonord/DD2480-Group-12-CI.git
```

Go to the project directory

```bash
  cd DD2480-Group-12-CI
```

Install Dependencies

```bash
  sudo apt install make g++ libsqlite3-dev libssl-dev
```

Compile the files

```bash
  make 
```

Run the code

```bash
  ./build/ci
```

## Running Tests

Compile and run the tests

```bash
  make test
```

## Using Web Connections

The code automatically binds to Port 8012. To access this service, ngrok is recommended. 

To run ngrok for this program, open another command prompt and do

```bash
  ngrok http 8012 
```
If you have a URL from ngrok, you can append it here. For example

```bash
  ngrok http --url basically-sure-piranha.ngrok-free.app 8012 
```

Alternatively, you can open port 8012 in your router, but this will not be described here. 

`<link>` below will be the URL provided by ngrok (or `<ip>:8012` if port forwarding) 

To add as a CI server, go to your GitHub Repo > Settings > Webhooks > Add Webhook. Here, add `<link>/ci_webhook` as your Payload URL. Set Content type to `application/json`. Disable SSL verification. For events, select "Let me select individual events" and check `Pushes` and `Pull Requests`. Now the CI server should be good to go!

To get the list of all previous builds done, go to the following link:
```<link>/list```


## Clean build folder 

Clean the compiled code build 

```bash
  make clean 
```

## The Compilation
To compile the programs during runtime, we have opted to use the std::system() function. This function will call `make -s` in the folder of the downloaded repository, and `make test -s` is used for testing. The system call results in a 0 if everything passed smoothly, while if there are any errors a non-zero result is returned. Logs are saved into a Sqlite3 database.

To unit test this, we have three fake testing repositories in the testing folder, one with code that will compile well, one with code that will not compile, and one without a Makefile. Unittests are run with Catch2 to make sure that the expected behaviour is had when running `make` in all three of these folders. 

## The Testing
To test a GitHub repo, we have set up a HTTP server through httplib which listens to Post messages on the `/ci_webhook` link. The request information sent by GitHub is then used to extrapolate all important information such as Commit ID, branch name and Cloning URL. The repo is then cloned using `std::system` calls, which downloads the requested testing repo. Using the methods explained above, the project is then compiled and tested. After compilation and testing, the commit status is updated, as explained below. The results and other important information is also added to the Database for future storage.

To unit test our implementation, we have set up separate testing repos that run on the same CI server. There are several different ones, one that has a functional compilation and passes testing, one that doesn't complete testing, one that doesn't compile and one that doesn't have a Makefile. Through this we can unit test the functionality of the CI server. 

## Commit Status
To update the commit status, we needed to post a HTTPS message to the GitHub API. Luckily, the webhook included everything needed for us to get the URL and therefore the implementation could be done without too much hassle. In order to send the notification, we create a HTTPlib SSLClient for the github api, to which we make a payload with the status and a header with a GitHub authorization token. This is then posted to GitHub and the response is awaited, with the results being returned back. 

Unit testing of this was a bit difficult, but we ended up using a separate repo with a set commit as our testing grounds. We use the function notifyCommitStatus to change the commit status and await the response from github to see whether it has passed or not.

## Statement of Contributions

This has been an interesting exercise, with a lot more intricacy and complexity than the first Assignment. But, even with that, we made it work, and even has the properties for P+ - this is something to be proud of!

**All members** contributed with documentation and Pull Request checking/merging. 

**Rifat Kazi** - Database integration and Testing sequence.

**Teo Nordström** - Http interactions, README, GitHub status

**Linus Svensson** - Repository cloning and compiling with corresponding tests

**Syam Kumar Vemana** - Testing and evaluation of possible tools

## Licenses

This project includes Catch2, which is licensed under the Boost Software License 1.0.
The full license text can be found in `external/Catch2_LICENSE.txt`.

This project includes httplib, which is licensed under the MIT License.
The full license text can be found in `external/httplib_LICENSE.txt`.

This project includes nlohmann json, which is licensed under the MIT License.
The full license text can be found in `external/json_LICENSE.txt`.

The code developed in this project is licensed under the MIT License.
The full license text can be found in `LICENSE`.