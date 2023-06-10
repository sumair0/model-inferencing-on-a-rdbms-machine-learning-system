#ifndef FILESET_CC
#define FILESET_CC

#include "FileSet.h"
#include "FileSetPageIterator.h"
#include <stdio.h>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

FileSet::FileSet(PageCachePtr cache,
                 string filePath,
                 size_t pageSize,
                 NodeID nodeId,
                 DatabaseID dbId,
                 UserTypeID typeId,
                 SetID setId)
    : LocalitySet(PartialAggregationData, MRU, Write, TryCache, Transient) {
    this->cache = cache;
    this->filePath = filePath;
    this->pageSize = pageSize;
    this->fileSize = 0;
    this->nodeId = nodeId;
    this->dbId = dbId;
    this->typeId = typeId;
    this->setId = setId;
#ifdef __APPLE__
    this->handle = open(filePath.c_str(), O_RDWR | O_APPEND | O_CREAT, S_IRWXU | S_IRWXO);
#else
    this->handle =
        open(filePath.c_str(), O_RDWR | O_APPEND | O_CREAT | O_DIRECT, S_IRWXU | S_IRWXO);
#endif
    if (handle < 0) {
        cout << "file can't be open:" << this->filePath << "\n";
        exit(-1);
    }
}

FileSet::~FileSet() {
    if (this->handle >= 0) {
        this->clear();
    }
}

void FileSet::clear() {
    close(this->handle);
    this->handle = -1;
    remove(this->filePath.c_str());
}

int FileSet::writeData(void* data, size_t length) {
    size_t retSize = write(this->handle, data, length);
    if (retSize < length) {
        return -1;
    } else {
        this->fileSize = this->fileSize + length;
        return 0;
    }
}

PageIteratorPtr FileSet::getIterator() {
    PageIteratorPtr iter = make_shared<FileSetPageIterator>(this->cache,
                                                            this->handle,
                                                            this->fileSize,
                                                            this->pageSize,
                                                            this->nodeId,
                                                            this->dbId,
                                                            this->typeId,
                                                            this->setId);
    return iter;
}
#endif
