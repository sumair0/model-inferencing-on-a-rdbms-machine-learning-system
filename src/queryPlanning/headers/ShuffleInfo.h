#ifndef SHUFFLE_INFO_H
#define SHUFFLE_INFO_H

#include "DataTypes.h"
#include "StandardResourceInfo.h"

namespace pdb {

// this class wraps shuffle information for job stages that needs repartitioning
// data

class ShuffleInfo {

private:
  // number of nodes in this cluster
  int numNodes;

  // number of CPU cores allocated for hash aggregation and hash join in this
  // cluster
  int numHashPartitions;

  // hash partition ids allocated for each node
  std::vector<std::vector<HashPartitionID>> partitionIds;

  // address for each node
  std::vector<std::string> addresses;

public:
  // constructor
  ShuffleInfo(std::vector<StandardResourceInfoPtr> *clusterResources,
              double partitionToCoreRatio) {

    this->numNodes = clusterResources->size();
    this->numHashPartitions = 0;
    int i, j;
    HashPartitionID id = 0;
    partitionIds.resize(this->numNodes);
    for (i = 0; i < this->numNodes; i++) {
      StandardResourceInfoPtr node = clusterResources->at(i);
      int numCoresOnThisNodeForHashing =
          (int)((double)(node->getNumCores()) * partitionToCoreRatio);
      if (numCoresOnThisNodeForHashing == 0) {
        numCoresOnThisNodeForHashing = 1;
      }
      for (j = 0; j < numCoresOnThisNodeForHashing; j++) {
        partitionIds[i].push_back(id);
        id++;
      }
      std::string curAddress =
          node->getAddress() + ":" + std::to_string(node->getPort());
      this->addresses.push_back(curAddress);
      this->numHashPartitions += numCoresOnThisNodeForHashing;
    }
  }

  // destructor
  ~ShuffleInfo() {}

  // to return number of nodes in the system
  int getNumNodes() { return this->numNodes; }

  // to return number of hash partitions in the system
  int getNumHashPartitions() { return this->numHashPartitions; }

  // to return partitioning scheme, so each node will have a set of partitionIds
  std::vector<std::vector<HashPartitionID>> &getPartitionIds() {
    return this->partitionIds;
  }

  // to return the address of nodes in the system
  std::vector<std::string> &getAddresses() { return this->addresses; }
};
}
#endif
