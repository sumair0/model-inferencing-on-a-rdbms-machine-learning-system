#include <iostream>
#include <cstring>

#include <PDBClient.h>
#include <RedditComment.h>
#include <RedditAuthor.h>
#include <RedditFeatures.h>
#include <RedditJoin.h>
#include <RedditPositiveLabelSelection.h>
#include <ScanUserSet.h>
#include <WriteUserSet.h>

using namespace pdb;


void run(PDBClient &pdbClient, bool whetherToAdaptiveJoin) {

  string errMsg;

    pdb::makeObjectAllocatorBlock(64 * 1024 * 1024, true);

    // clear the rankings
    pdbClient.clearSet("redditDB", "features", "reddit::Features", errMsg);

    // make a scan set
    Handle<Computation> input1 = makeObject<ScanUserSet<reddit::Comment>>("redditDB", "comments");
    Handle<Computation> input2 = makeObject<ScanUserSet<reddit::Author>>("redditDB", "authors");
    Handle<Computation> select = makeObject<reddit::PositiveLabelSelection>();
    select->setInput(input1);
    // join previous ranks with links
    Handle<Computation> join = makeObject<reddit::JoinAuthorsWithComments>();

    if (whetherToAdaptiveJoin) {
        join->setInput(0, select);
    } else {
        join->setInput(0, input1);
    }
    join->setInput(1, input2);


    // write it out
    Handle<Computation> myWriteSet = makeObject<WriteUserSet<reddit::Features>>("redditDB", "features");
    myWriteSet->setInput(join);

    // execute the computation
    auto begin = std::chrono::high_resolution_clock::now();
    pdbClient.executeComputations(errMsg, "reddit-a", false, myWriteSet);
    auto end = std::chrono::high_resolution_clock::now();
          std::cout << "Time Duration for Run: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

}

int main(int argc, char* argv[]) {

  // this is where we put an eventual error message
  string errMsg;

  // make sure we have the arguments
  if(argc < 4) {
    std::cout << "Usage : ./TestRedditJoin managerIP managerPort whetherToAdaptiveJoin whetherToRegisterLibraries\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "whetherToAdaptiveJoin - Y yes, N no\n";
    std::cout << "whetherToRegisterLibraries - Y yes, N no\n";
  }

  //  get the manager address
  std::string ip = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);
  bool whetherToAdaptiveJoin = false;
  if (strcmp(argv[3], "Y")==0) {
      whetherToAdaptiveJoin = true;
  }
  bool whetherToRegisterLibraries = true;
  if (strcmp(argv[4], "N")==0) {
     whetherToRegisterLibraries = false;
  }

  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, ip, clientLogger);
  PDBClient pdbClient(port, ip, clientLogger, false, true);
  
  if (whetherToRegisterLibraries) {
      pdbClient.registerType("libraries/libRedditComment.so", errMsg);
      pdbClient.registerType("libraries/libRedditAuthor.so", errMsg);
      pdbClient.registerType("libraries/libRedditFeatures.so", errMsg);
      pdbClient.registerType("libraries/libRedditJoin.so", errMsg);
      pdbClient.registerType("libraries/libRedditPositiveLabelSelection.so", errMsg);
  }
  
  pdbClient.removeSet("redditDB", "features", errMsg);
  pdbClient.createSet<reddit::Features>("redditDB", "features", errMsg,  (size_t)512*(size_t)1024*(size_t)1024);
  // run one iteration
  auto begin = std::chrono::high_resolution_clock::now();
  run(pdbClient, whetherToAdaptiveJoin);
  auto end = std::chrono::high_resolution_clock::now();
      std::cout << "End-to-End Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

  // print the results
  std::cout << "Features: \n";
  SetIterator<reddit::Features> result = pdbClient.getSetIterator<reddit::Features>("redditDB", "features");
  int count = 0;
  for (const auto &r : result) {
     count++;
  }
  std::cout << "count: " << count << std::endl;

  pdbClient.removeSet("redditDB", "features", errMsg);

}
