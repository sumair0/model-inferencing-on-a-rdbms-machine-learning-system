#ifndef K_MEANS_AGGREGATE_H
#define K_MEANS_AGGREGATE_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "ClusterAggregateComp.h"
#include "KMeansDoubleVector.h"
#include "limits.h"
#include "KMeansCentroid.h"
#include "KMeansAggregateOutputType.h"

/* 
 * This class computes the membership for each data point, 
 * and implements the aggregation for each cluster 
 */
using namespace pdb;

class KMeansAggregate : public ClusterAggregateComp<KMeansAggregateOutputType,
                                                    KMeansDoubleVector,
                                                    int,
                                                    KMeansCentroid> {

private:
    Vector<KMeansDoubleVector> model;

public:
    ENABLE_DEEP_COPY

    KMeansAggregate() {}

    KMeansAggregate(Handle<Vector<Handle<KMeansDoubleVector>>>& inputModel) {
        if (model.size() > 0) {
            model.clear();
        }
        for (int i = 0; i < inputModel->size(); i++) {
            model.push_back(*(*inputModel)[i]);
        }
    }

    Lambda<int> getKeyProjection(Handle<KMeansDoubleVector> aggMe) override {
        return makeLambda(aggMe, [&](Handle<KMeansDoubleVector>& aggMe) {
            return this->computeClusterMemberOptimized(*aggMe);
        });
    }

    Lambda<KMeansCentroid> getValueProjection(Handle<KMeansDoubleVector> aggMe) override {
        return makeLambda(
            aggMe, [](Handle<KMeansDoubleVector>& aggMe) { return KMeansCentroid(1, *aggMe); });
    }

    /* Compute the membership according to squared distance */
    int computeClusterMember(Handle<KMeansDoubleVector> data) {
        int closestDistance = INT_MAX;
        int cluster = 0;
        Vector<KMeansDoubleVector>& myModel = this->model;
        KMeansDoubleVector& myData = *data;
        size_t modelSize = myModel.size();
        for (int j = 0; j < modelSize; j++) {
            KMeansDoubleVector& mean = myModel[j];
            double distance = myData.getSquaredDistance(mean);

            if (distance < closestDistance) {
                closestDistance = distance;
                cluster = j;
            }
        }
        return cluster;
    }

    /* Another way to compute the membership */
    int computeClusterMemberOptimized(KMeansDoubleVector& data) {
        int closestDistance = INT_MAX;
        int cluster = 0;
        Vector<KMeansDoubleVector>& myModel = model;
        size_t modelSize = myModel.size();
        for (int i = 0; i < modelSize; i++) {
            KMeansDoubleVector& mean = myModel[i];
            double lowerBoundOfSqDist = mean.norm - data.norm;
            lowerBoundOfSqDist = lowerBoundOfSqDist * lowerBoundOfSqDist;
            if (lowerBoundOfSqDist < closestDistance) {
                double distance = data.getFastSquaredDistance(mean);
                if (distance < closestDistance) {
                    closestDistance = distance;
                    cluster = i;
                }
            }
        }
        return cluster;
    }
};


#endif
