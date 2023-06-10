#ifndef RESOURCE_MANAGER_SERVER_H
#define RESOURCE_MANAGER_SERVER_H


#include "ServerFunctionality.h"
#include "ResourceInfo.h"
#include "Handle.h"
#include "PDBVector.h"
#include "NodeDispatcherData.h"


namespace pdb {


// ResourceManagerServer MUST co-locate with QuerySchedulerServer and DispatcherServer
class ResourceManagerServer : public ServerFunctionality {

public:
    // destructor
    ~ResourceManagerServer();

    // constructor, initialize from a server list file
    ResourceManagerServer(std::string pathToServerList,
                          int port,
                          bool pseudoClusterMode = false,
                          std::string pemFile = "conf/pdb.key");

    Handle<Vector<Handle<ResourceInfo>>> getAllResources();

    Handle<Vector<Handle<NodeDispatcherData>>> getAllNodes();

    // from the serverFunctionality interface... register the resource manager handlers
    void registerHandlers(PDBServer& forMe) override;

    void cleanup() override;

protected:
    // initialize from a serverlist file, by invoking shell scripts to connect to each server, and
    // got that server's resource list
    // now all resources shall be initialized when the cluster is started
    void initialize(std::string pathToServerList);

    void analyzeResources(std::string resourceFileName);

    void analyzeNodes(std::string serverList);

private:
    Handle<Vector<Handle<ResourceInfo>>> resources = nullptr;

    Handle<Vector<Handle<NodeDispatcherData>>> nodes = nullptr;

    int port;

    bool pseudoClusterMode;

    std::string pemFile;
};
}


#endif
