
/*
 * HashSet is an in-memory set for implementing Pangaea hash map.
 */


#ifndef HASHSET_H
#define HASHSET_H

#include "PageCache.h"
#include "PDBLogger.h"
#include "DataTypes.h"
#include "LocalitySet.h"
#include <memory>
#include <string>
using namespace std;

class HashSet;
typedef shared_ptr<HashSet> HashSetPtr;

class HashSet : public LocalitySet {
public:
    HashSet(PageCachePtr cache,
            PDBLoggerPtr logger,
            size_t pageSize,
            NodeID nodeId,
            DatabaseID dbId,
            UserTypeID typeId,
            SetID setId,
            string setName)
        : LocalitySet(HashPartitionData, LRU, Write, TryCache, Transient) {
        this->cache = cache;
        this->logger = logger;
        this->pageSize = pageSize;
        this->nodeId = nodeId;
        this->dbId = dbId;
        this->typeId = typeId;
        this->setId = setId;
        this->pageId = 0;
        this->setName = setName;
    }

    ~HashSet() {}

    void clear() {}

    DatabaseID getDatabaseID() {
        return this->dbId;
    }

    UserTypeID getUserTypeID() {
        return this->typeId;
    }

    SetID getSetID() {
        return this->setId;
    }

    string getSetName() {
        return this->setName;
    }

    PDBPagePtr addPage() {
        CacheKey key;
        key.dbId = this->dbId;
        key.typeId = this->typeId;
        key.setId = this->setId;
        key.pageId = this->pageId;
        this->pageId++;
        // PDBPagePtr page = this->cache->getNewPageNonBlocking(this->nodeId, key, this);
        PDBPagePtr page = this->cache->getNewPage(this->nodeId, key, this);
        return page;
    }

    void unpinPage(PageID pageId) {
        CacheKey key;
        key.dbId = this->dbId;
        key.typeId = this->typeId;
        key.setId = this->setId;
        key.pageId = pageId;
        this->cache->decPageRefCount(key);
    }

private:
    PDBLoggerPtr logger;
    PageCachePtr cache;
    size_t pageSize;
    NodeID nodeId;
    DatabaseID dbId;
    UserTypeID typeId;
    SetID setId;
    PageID pageId;
    string setName;
};

#endif
