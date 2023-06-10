
#ifndef STORAGE_CLIENT_H
#define STORAGE_CLIENT_H

namespace pdb {

class StorageClient;
}

#include "ServerFunctionality.h"
#include "CatalogClient.h"
#include "PDBLogger.h"
#include "PDBServer.h"

namespace pdb {


//this class encapsulates a StorageClient to talk with PangeaStorageServer

class StorageClient : public ServerFunctionality {

public:
    // these give us the port and the address of the catalog
    StorageClient(int port, std::string address, PDBLoggerPtr myLogger, bool usePangea = false);

    // function to register event handlers associated with this server functionality
    virtual void registerHandlers(PDBServer& forMe) override;

    // this registers a type with the system
    bool registerType(std::string fileContainingSharedLib, std::string& errMsg);

    // this returns the type of object in the specified set, as a type name... returns "" on error
    std::string getObjectType(std::string databaseName, std::string setName, std::string& errMsg);

    // this creates a new database... returns true on success
    bool createDatabase(std::string databaseName, std::string& errMsg);

    // shuts down the server that we are connected to... returns true on success
    bool shutDownServer(std::string& errMsg);

    // to flush buffered records into shared memory pages
    bool flushData(std::string& errMsg);


    // this creates a new set in a given database... returns true on success
    template <class DataType>
    bool createSet(std::string databaseName, std::string setName, std::string& errMsg);

    // this removes a set from a given database... returns true on success
    template <class DataType>
    bool removeSet(std::string databaseName, std::string setName, std::string& errMsg);


    // this stores data into a set... returns true on success
    template <class DataType>
    bool storeData(Handle<Vector<Handle<DataType>>> data,
                   std::string databaseName,
                   std::string setName,
                   std::string& errMsg,
                   bool typeCheck = true);

    bool storeData(Handle<Vector<Handle<Object>>> data,
                   std::string databaseName,
                   std::string setName,
                   std::string typeName,
                   std::string& errMsg);


    // this retrieves data into a set... returns true on success
    template <class DataType>
    bool retrieveData(std::string databaseName, std::string setName, std::string& errMsg);

    // this test set scan at backend ... returns true on success
    template <class DataType>
    bool scanData(std::string databaseName, std::string setName, std::string& errMsg);

    // this test set copy at backend ... returns true on success
    template <class DataType>
    bool copyData(std::string srcDatabaseName,
                  std::string srcSetName,
                  std::string destDatabaseName,
                  std::string destSetName,
                  std::string& errMsg);


private:
    CatalogClient myHelper;

    int port;
    std::string address;
    PDBLoggerPtr myLogger;

    bool usePangea;
};
}

#include "StorageClientTemplate.cc"

#endif
