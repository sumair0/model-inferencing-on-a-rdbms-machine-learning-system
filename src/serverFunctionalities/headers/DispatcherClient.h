
#ifndef OBJECTQUERYMODEL_DISPATCHERCLIENT_H
#define OBJECTQUERYMODEL_DISPATCHERCLIENT_H

#include "ServerFunctionality.h"
#include "Handle.h"
#include "PDBVector.h"
#include "PDBObject.h"
#include "PartitionPolicy.h"
#include "CatalogClient.h"

namespace pdb {

// this class serves as a dispatcher client to talk with the DispatcherServer
// to send Vector<Objects> from clients to the distributed storage server. 


class DispatcherClient : public ServerFunctionality {

public:
    DispatcherClient();
    DispatcherClient(int portIn, std::string addressIn, PDBLoggerPtr myLoggerIn);
    ~DispatcherClient();

    /**
     *
     * @param forMe
     */
    void registerHandlers(PDBServer& forMe) override;  // no-op

    /**
     *
     * @param setAndDatabase
     * @return
     */
    bool registerSet(std::pair<std::string, std::string> setAndDatabase,
                     PartitionPolicy::Policy policy,
                     std::string& errMsg);

    /**
     *
     * @param setAndDatabase
     * @return
     */
    template <class DataType>
    bool sendData(std::pair<std::string, std::string> setAndDatabase,
                  Handle<Vector<Handle<DataType>>> dataToSend,
                  std::string& errMsg);

    template <class DataType>
    bool sendBytes(std::pair<std::string, std::string> setAndDatabase,
                   char* bytes,
                   size_t numBytes,
                   std::string& errMsg);

private:
    CatalogClient myHelper;
    int port;
    std::string address;
    PDBLoggerPtr logger;
};
}

#include "DispatcherClientTemplate.cc"

#endif  // OBJECTQUERYMODEL_DISPATCHERCLIENT_H
