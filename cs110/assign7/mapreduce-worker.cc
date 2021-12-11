/**
 * File: mapreduce-worker.cc
 * -------------------------
 * Presents the implementation of the MapReduceWorker class.
 */

#include "mapreduce-worker.h"
#include <cassert>
#include <sstream>
#include "mr-messages.h"
#include "string-utils.h"
#include "client-socket.h"
#include "socket++/sockstream.h"
using namespace std;

MapReduceWorker::MapReduceWorker(const string& serverHost, unsigned short serverPort,
                                 const string& executablePath, const string& executable,
                                 const string& outputPath) :
  serverHost(serverHost), serverPort(serverPort), executablePath(executablePath), 
  executable(executable), outputPath(outputPath) {}

int MapReduceWorker::work() {
  while (true) {
    string inputFile;
    if (!requestInputFile(inputFile)) break;
    sendProgressReportToServer(inputFile);
  }

  return 0;
}

bool MapReduceWorker::requestInputFile(string& inputFile) {
  int clientSocket = getClientSocket();
  sockbuf sb(clientSocket);
  iosockstream ss(&sb);
  sendWorkerReady(ss);
  MRMessage message;
  string payload;
  receiveMessage(ss, message, payload);
  if (message == kServerDone) return false;
  inputFile = trim(payload);
  return true;
}

string getFilenameFromPath(const string &path) {
  size_t found = path.rfind("/") + 1;
  return path.substr(found) + ".out";
}

void MapReduceWorker::sendProgressReportToServer(const string& inputFile) {
  int clientSocket = getClientSocket();
  sockbuf sb(clientSocket);
  iosockstream ss(&sb);

  string outputFile = outputPath + "/" + getFilenameFromPath(inputFile);
  string exec = executablePath + "/" + executable;
  string cmdLine = exec + " " + inputFile + " " + outputFile;

  int retVal = system(cmdLine.c_str());
  if (retVal == -1 || (WIFEXITED(retVal) && WEXITSTATUS(retVal) != 0)) {
    sendJobFailed(ss, inputFile);
    return;
  }

  sendJobSucceeded(ss, inputFile);
}

static const int kServerInaccessible = 2;
int MapReduceWorker::getClientSocket() {
  int clientSocket = createClientSocket(serverHost, serverPort);
  if (clientSocket == kClientSocketError) {
    exit(kServerInaccessible);
  }
  
  return clientSocket;
}
