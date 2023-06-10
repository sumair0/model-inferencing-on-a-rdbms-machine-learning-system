//
// Created by Joseph Hwang on 9/12/16.
//

#ifndef OBJECTQUERYMODEL_RANDOMPOLICY_H
#define OBJECTQUERYMODEL_RANDOMPOLICY_H

#include "PartitionPolicy.h"

#include <random>

namespace pdb {

class RandomPolicy;
typedef std::shared_ptr<RandomPolicy> RandomPolicyPtr;

/**
 * RandomPolicy simply selects a random node from its Storage Nodes List to send the entire Vector
 * of data to. We send
 * the entire Vector to a single node instead of partitioning it on an Object granularity to save
 * time.
 */
class RandomPolicy : public PartitionPolicy {
public:
    RandomPolicy();
    ~RandomPolicy();

    void updateStorageNodes(Handle<Vector<Handle<NodeDispatcherData>>> storageNodes);

    std::shared_ptr<std::unordered_map<NodeID, Handle<Vector<Handle<Object>>>>> partition(
        Handle<Vector<Handle<Object>>> toPartition);

private:
    // Seed used for the PRNG. It is configurable so that we can deterministically test
    // RandomPolicy's behavior.
    const int SEED = time(NULL);

    std::vector<NodePartitionDataPtr> createNodePartitionData(
        Handle<Vector<Handle<NodeDispatcherData>>> storageNodes);
    NodePartitionDataPtr updateExistingNode(NodePartitionDataPtr newNodeData,
                                            NodePartitionDataPtr oldNodeData);
    NodePartitionDataPtr updateNewNode(NodePartitionDataPtr newNode);
    NodePartitionDataPtr handleDeadNode(NodePartitionDataPtr deadNode);
};
}


#endif  // OBJECTQUERYMODEL_RandomPOLICY_H
