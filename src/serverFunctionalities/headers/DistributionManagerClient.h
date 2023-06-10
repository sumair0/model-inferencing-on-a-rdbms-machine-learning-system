
#ifndef Distribution_Manager_CLIENT_H
#define Distribution_Manager_CLIENT_H

#include "ServerFunctionality.h"
#include "PDBLogger.h"
#include "PDBServer.h"
#include "PDBString.h"


#include "NodeInfo.h"
#include "Ack.h"
#include "QueryPermit.h"
#include "QueryPermitResponse.h"
#include "QueryDone.h"
#include "PlaceOfQueryPlanner.h"
#include "PDBVector.h"
#include "ListOfNodes.h"

namespace pdb {

class DistributionManagerClient;
typedef shared_ptr<DistributionManagerClient> DistributionManagerClientPtr;

class DistributionManagerClient : public ServerFunctionality {

public:
    DistributionManagerClient(PDBLoggerPtr logger);

    DistributionManagerClient(pdb::String hostnameIn, int portIn, PDBLoggerPtr logger);

    ~DistributionManagerClient();

    // function to register event handlers associated with this server functionality
    virtual void registerHandlers(PDBServer& forMe) override;


    // TODO: Not sure if we need a shutDown method for the distribution manager.
    //	// shuts down the server that we are connected to... returns true on success
    //	bool shutDownServer (std :: string &errMsg);

    // send the HeartBeat info to the server.
    void sendHeartBeat(string& masterHostName, int masterNodePort, bool& wasError, string& errMsg);

    // send the HeartBeat info to the server.
    Handle<ListOfNodes> getCurrentNodes(string& masterHostName,
                                        int masterNodePort,
                                        bool& wasError,
                                        string& errMsg);

    // send a request for running a query
    Handle<QueryPermitResponse> sendQueryPermitt(string& hostName,
                                                 int masterNodePort,
                                                 pdb::Handle<QueryPermit> m_queryPermit,
                                                 bool& wasError,
                                                 string& errMsg);

    // informs the server that running of a specific query is done
    Handle<Ack> sendQueryDone(string& hostName,
                              int masterNodePort,
                              Handle<QueryDone> m_queryDone,
                              bool& wasError,
                              string& errMsg);

    // sends the place of query planner node to the user-client
    Handle<Ack> sendGetPlaceOfQueryPlanner(string& hostName,
                                           int masterNodePort,
                                           Handle<PlaceOfQueryPlanner> m_PlaceOfQueryPlanner,
                                           bool& wasError,
                                           string& errMsg);

    // Executes a vector of queries on the cluster on a specific set of cluster nodes.
    //	Handle <Vector <Handle <Ack>>> executeQueriesOnCluster(Handle <Vector<Handle <QueryBase>>>
    //queries, Handle <Vector <Handle <String>>> hostNames, Handle <Vector <Handle <int>>>
    //hostPorts);

    // Executes a single queries on a single remote node.
    //	Handle<Ack> executeQueryOnSingleNode(Handle <Vector<Handle <QueryBase>>> queries, Handle
    //<String> hostNames, Handle <int> hostPorts,  string& errMsg);

    // get the logger
    PDBLoggerPtr getLogger() {
        return this->logger;
    }

private:
    pdb::String hostname;
    int port;
    PDBLoggerPtr logger;
};
}

#endif
