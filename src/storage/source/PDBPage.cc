
#ifndef PDB_Page_C
#define PDB_Page_C
#include "PDBPage.h"
#include <cstring>
#include <stdlib.h>
#include <iostream>

// create an empty page
PDBPage::PDBPage(char* dataIn,
                 NodeID dataNodeID,
                 DatabaseID dataDbID,
                 UserTypeID dataTypeID,
                 SetID dataSetID,
                 PageID dataPageID,
                 size_t dataSize,
                 size_t shmOffset,
                 int internalOffset,
                 int numObjectsIn,
		 unsigned int sharedCounts) {
    rawBytes = dataIn;
    nodeID = dataNodeID;
    dbID = dataDbID;
    typeID = dataTypeID;
    setID = dataSetID;
    pageID = dataPageID;
    size = dataSize;
    offset = shmOffset;
    numOwners = sharedCounts;
    this->numObjects = numObjectsIn;
    this->curAppendOffset = sizeof(NodeID) + sizeof(DatabaseID) + sizeof(UserTypeID) +
        sizeof(SetID) + sizeof(PageID) + sizeof(int) + sizeof(size_t)+ sizeof(unsigned int);
    this->refCount = 0;
    this->pinned = true;
    this->dirty = false;
    this->inFlush = false;
    this->partitionId = (FilePartitionID)(-1);
    this->pageSeqInPartition = (unsigned int)(-1);
    pthread_mutex_init(&(this->refCountMutex), nullptr);
    pthread_rwlock_init(&(this->flushLock), nullptr);
    this->internalOffset = internalOffset;
    char* refCountBytes = this->rawBytes +
        (sizeof(NodeID) + sizeof(DatabaseID) + sizeof(UserTypeID) + sizeof(SetID) + sizeof(PageID));
    *((int*)refCountBytes) = numObjectsIn;
    char* pageSizeBytes = refCountBytes + sizeof(int);
    *((size_t*)pageSizeBytes) = dataSize;
    char* sharedCountBytes = pageSizeBytes + sizeof(size_t);
    *((unsigned int*)sharedCountBytes) = numOwners;
}


// create a PDBPage instance from a non-empty page.
PDBPage::PDBPage(char* dataIn, size_t offset, int internalOffset) {
    this->rawBytes = dataIn;
    this->offset = offset;
    this->internalOffset = internalOffset;
    this->curAppendOffset = sizeof(NodeID) + sizeof(DatabaseID) + sizeof(UserTypeID) +
        sizeof(SetID) + sizeof(PageID) + sizeof(int) + sizeof(size_t) + sizeof(unsigned int);
    char* cur = this->rawBytes;
    this->nodeID = *((NodeID*)cur);
    cur = cur + sizeof(NodeID);
    this->dbID = *((DatabaseID*)cur);
    cur = cur + sizeof(DatabaseID);
    this->typeID = *((UserTypeID*)cur);
    cur = cur + sizeof(UserTypeID);
    this->setID = *((SetID*)cur);
    cur = cur + sizeof(SetID);
    this->pageID = *((PageID*)cur);
    cur = cur + sizeof(PageID);
    this->numObjects = *((int*)cur);
    cur = cur + sizeof(int);
    this->size = *((size_t*)cur);
    cur = cur + sizeof(unsigned int);
    this->numOwners = *((unsigned int*)cur);
    this->refCount = 0;
    this->pinned = true;
    this->dirty = false;
    this->inFlush = false;
    this->partitionId = (FilePartitionID)(-1);
    this->pageSeqInPartition = (unsigned int)(-1);
    pthread_mutex_init(&(this->refCountMutex), nullptr);
    pthread_rwlock_init(&(this->flushLock), nullptr);
}


PDBPage::~PDBPage() {
    freePage();
    pthread_mutex_destroy(&(this->refCountMutex));
    pthread_rwlock_destroy(&(this->flushLock));
}

/**
 * Prepare page head.
 */

void PDBPage::preparePage() {
    char* cur = this->rawBytes;
    *((NodeID*)cur) = nodeID;
    cur = cur + sizeof(NodeID);
    *((DatabaseID*)cur) = dbID;
    cur = cur + sizeof(DatabaseID);
    *((UserTypeID*)cur) = typeID;
    cur = cur + sizeof(UserTypeID);
    *((SetID*)cur) = setID;
    cur = cur + sizeof(SetID);
    *((PageID*)cur) = pageID;
    cur = cur + sizeof(PageID);
    *((int*)cur) = 0;
    cur = cur + sizeof(int);
    *((size_t*)cur) = this->size;
    cur = cur + sizeof(size_t);
    *((unsigned int *)cur) = this->numOwners;
    this->curAppendOffset = sizeof(NodeID) + sizeof(DatabaseID) + sizeof(UserTypeID) +
        sizeof(SetID) + sizeof(PageID) + sizeof(int) + sizeof(size_t)+sizeof(unsigned int);
    return;
}


void PDBPage::readLock() {
    pthread_rwlock_rdlock(&(this->flushLock));
}

void PDBPage::readUnlock() {
    pthread_rwlock_unlock(&(this->flushLock));
}

void PDBPage::writeLock() {
    pthread_rwlock_wrlock(&(this->flushLock));
}

void PDBPage::writeUnlock() {
    pthread_rwlock_unlock(&(this->flushLock));
}

void* PDBPage::getBytes() {

    return this->rawBytes + sizeof(NodeID) + sizeof(DatabaseID) + sizeof(UserTypeID) +
        sizeof(SetID) + sizeof(PageID) + sizeof(int) + sizeof(size_t) + sizeof(unsigned int);
}

size_t PDBPage::getSize() {

    return this->size - (sizeof(NodeID) + sizeof(DatabaseID) + sizeof(UserTypeID) + sizeof(SetID) +
                         sizeof(PageID) + sizeof(int) + sizeof(size_t) + sizeof(unsigned int));
}

void PDBPage::unpin() {
    this->decRefCount();
}


void PDBPage::freePage() {
    if (rawBytes != nullptr) {
        // we always free page data by SharedMem class when page is flushed or evicted, and we do
        // not free page data here
        // If it comes to here, there must be a problem. Shared memory should already be freed,
        // there could be memory leaks
        rawBytes = nullptr;
    }
    nodeID = -1;
    dbID = -1;
    typeID = -1;
    setID = -1;
    pageID = -1;
    offset = 0;
    size = 0;
    numObjects = 0;
    internalOffset = 0;
    numOwners = 0;
}

#endif
