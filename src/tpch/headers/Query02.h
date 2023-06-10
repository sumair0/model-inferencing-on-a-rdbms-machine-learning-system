#ifndef QUERY02_H
#define QUERY02_H


#include "TPCHSchema.h"
#include "PDBDebug.h"
#include "PDBString.h"
#include "Query.h"
#include "Lambda.h"
#include "PDBClient.h"
#include "DataTypes.h"
#include "InterfaceFunctions.h"
#include "Handle.h"
#include "LambdaCreationFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Pipeline.h"
#include "VectorSink.h"
#include "HashSink.h"
#include "MapTupleSetIterator.h"
#include "VectorTupleSetIterator.h"
#include "ComputePlan.h"
#include "QueryOutput.h"
#include "SelectionComp.h"
#include "JoinComp.h"
#include "ClusterAggregateComp.h"


using namespace pdb;

namespace tpch {

class Q02RegionSelection : public SelectionComp<Region, Region> {

private:

   String r_name;

public:

    ENABLE_DEEP_COPY

    Q02RegionSelection () {}

    Q02RegionSelection (std::string r_name) {
        this->r_name = r_name;
    }

    Lambda<bool> getSelection(Handle<Region> checkMe) override {
        return makeLambdaFromMember(checkMe, r_name ) == makeLambda(checkMe,
           [&](Handle<Region>& checkMe) { return this->r_name; });
    }

    Lambda<Handle<Region>> getProjection(Handle<Region> checkMe) override {
        return makeLambda(checkMe, [](Handle<Region>& checkMe) { return checkMe; });
    }


};



class Q02NationJoin : public JoinComp<Nation, Region, Nation> {

public:

    ENABLE_DEEP_COPY

    Q02NationJoin () {}

    Lambda<bool> getSelection (Handle<Region> in1, Handle<Nation> in2) override {
        return makeLambdaFromMember(in1, r_regionkey) == makeLambdaFromMember(in2,
           n_regionkey);
         
    }
    
    Lambda<Handle<Nation>> getProjection(Handle<Region> in1, Handle<Nation> in2) override {
        return makeLambda(in1, in2, [](Handle<Region>& in1, Handle<Nation>& in2) { return in2; });
    }


};


class Q02SupplierJoinOutput : public Object {

public:

    ENABLE_DEEP_COPY

    String n_name;
    int s_suppkey;
    double s_acctbal;
    String s_name;
    String s_address;
    String s_phone;
    String s_comment;

    Q02SupplierJoinOutput () {}

    Q02SupplierJoinOutput (String & n_name, Supplier & supplier) {
        this->n_name = n_name;
        this->s_suppkey = supplier.s_suppkey;
        this->s_acctbal = supplier.s_acctbal;
        this->s_name = supplier.s_name;
        this->s_address = supplier.s_address;
        this->s_phone = supplier.s_phone;
        this->s_comment = supplier.s_comment;
    }

};



class Q02SupplierJoin : public JoinComp<Q02SupplierJoinOutput, Nation, Supplier> {

public:

    ENABLE_DEEP_COPY

    Q02SupplierJoin () {}

    Lambda<bool> getSelection (Handle<Nation> in1, Handle<Supplier> in2) override {
        return makeLambdaFromMember(in1, n_nationkey) == makeLambdaFromMember(in2,
           s_nationkey);

    }

    Lambda<Handle<Q02SupplierJoinOutput>> getProjection(Handle<Nation> in1, Handle<Supplier> in2) override {
        return makeLambda(in1, in2, [](Handle<Nation>& in1, Handle<Supplier>& in2) { 

             Handle<Q02SupplierJoinOutput> ret = makeObject<Q02SupplierJoinOutput> (in1->n_name, *in2);
             return ret;

        });
    
}


};

class Q02PartSuppJoinOutput : public Object {

public:

    ENABLE_DEEP_COPY

    String n_name;
    double s_acctbal;
    String s_name;
    String s_address;
    String s_phone;
    String s_comment;
    int ps_partkey;
    double ps_supplycost;

    Q02PartSuppJoinOutput () {}

    Q02PartSuppJoinOutput (Q02SupplierJoinOutput& supplier, PartSupp& ps) {
        this->n_name = supplier.n_name;
        this->s_acctbal = supplier.s_acctbal;
        this->s_name = supplier.s_name;
        this->s_address = supplier.s_address;
        this->s_phone = supplier.s_phone;
        this->s_comment = supplier.s_comment;
        this->ps_partkey = ps.ps_partkey;
        this->ps_supplycost = ps.ps_supplycost;
    }    

};

class Q02PartSuppJoin : public JoinComp<Q02PartSuppJoinOutput, Q02SupplierJoinOutput, PartSupp> {

public:

    ENABLE_DEEP_COPY

    Q02PartSuppJoin () {}

    Lambda<bool> getSelection (Handle<Q02SupplierJoinOutput> in1, Handle<PartSupp> in2) override {
        return makeLambdaFromMember(in1, s_suppkey) == makeLambdaFromMember(in2,
           ps_suppkey);

    }

    Lambda<Handle<Q02PartSuppJoinOutput>> getProjection(Handle<Q02SupplierJoinOutput> in1, Handle<PartSupp> in2) override {
        return makeLambda(in1, in2, [](Handle<Q02SupplierJoinOutput>& in1, Handle<PartSupp>& in2) { 
            Handle<Q02PartSuppJoinOutput> ret = makeObject<Q02PartSuppJoinOutput> (*in1, *in2);
            return ret; 
        });
    }

};



class Q02PartSelection : public SelectionComp<Part, Part> {

private:
    int size;
    String type;

public:

    ENABLE_DEEP_COPY

    Q02PartSelection () {}

    Q02PartSelection (int size, std::string type) {
        this->size = size;
        this->type = type;
    }

    Lambda<bool> getSelection(Handle<Part> checkMe) override {
        return makeLambda(checkMe,
           [&](Handle<Part>& checkMe) { return (checkMe->p_size == this->size); }) && makeLambda(checkMe,
           [&](Handle<Part>& checkMe) { 
               return checkMe->p_type.endsWith(this->type);
               
           });
    }

    Lambda<Handle<Part>> getProjection(Handle<Part> checkMe) override {
        return makeLambda(checkMe, [](Handle<Part>& checkMe) { return checkMe; });
    }


};



class Q02PartJoinOutput : public Object {

public:
    
    ENABLE_DEEP_COPY
    
    int p_partkey;
    String p_mfgr;
    String n_name;
    double s_acctbal;
    String s_name;
    String s_address;
    String s_phone;
    String s_comment;
    double ps_supplycost;

    Q02PartJoinOutput () {}

    Q02PartJoinOutput (Part& part, Q02PartSuppJoinOutput& supplier) {
        this->n_name = supplier.n_name;
        this->s_acctbal = supplier.s_acctbal;
        this->s_name = supplier.s_name;
        this->s_address = supplier.s_address;
        this->s_phone = supplier.s_phone;
        this->s_comment = supplier.s_comment;
        this->ps_supplycost = supplier.ps_supplycost;
        this->p_partkey = part.p_partkey;
        this->p_mfgr = part.p_mfgr;
    }

};


class Q02PartJoin : public JoinComp<Q02PartJoinOutput, Part, Q02PartSuppJoinOutput> {

public:

    ENABLE_DEEP_COPY

    Q02PartJoin () {}

    Lambda<bool> getSelection (Handle<Part> in1, Handle<Q02PartSuppJoinOutput> in2) override {
        return makeLambdaFromMember(in1, p_partkey) == makeLambdaFromMember(in2,
           ps_partkey);

    }

    Lambda<Handle<Q02PartJoinOutput>> getProjection(Handle<Part> in1, Handle<Q02PartSuppJoinOutput> in2) override {
        return makeLambda(in1, in2, [](Handle<Part>& in1, Handle<Q02PartSuppJoinOutput>& in2) { 
             Handle<Q02PartJoinOutput> ret = makeObject<Q02PartJoinOutput> (*in1, *in2);
             return ret;
        });
    }

};

class Q02PartJoinOutputIdentitySelection : public SelectionComp<Q02PartJoinOutput, Q02PartJoinOutput> {

public:

    ENABLE_DEEP_COPY

    Q02PartJoinOutputIdentitySelection () {}

    Lambda<bool> getSelection (Handle<Q02PartJoinOutput> checkMe) override {
        return makeLambda(checkMe, [](Handle<Q02PartJoinOutput> & checkMe) {
             return true;
         });
    }

    Lambda<Handle<Q02PartJoinOutput>> getProjection (Handle<Q02PartJoinOutput> checkMe) override {
        return makeLambda(checkMe,  [](Handle<Q02PartJoinOutput> & checkMe) {
             return checkMe;
        });
    }

};




class MinDouble : public Object {

public:

    double min;

    ENABLE_DEEP_COPY

    MinDouble () {}

    void setValue (double min) {
        this->min = min;
    }

    MinDouble& operator+ (MinDouble& addMeIn) {
        if(addMeIn.min < min) {
            min = addMeIn.min;
        }
        return *this;
    }

};


class Q02MinCostPerPart : public Object {

public:

    int ps_partkey;
    MinDouble min;

    ENABLE_DEEP_COPY

    Q02MinCostPerPart () {}

    int & getKey() {
        return ps_partkey;
    }

    MinDouble & getValue() {
        return min;
    }

};


class Q02MinAgg : public ClusterAggregateComp<Q02MinCostPerPart,
                                               Q02PartJoinOutput,
                                               int,
                                               MinDouble> {

public:

    ENABLE_DEEP_COPY


    Q02MinAgg () {}

    Lambda<int> getKeyProjection(Handle<Q02PartJoinOutput> aggMe) override {
         return makeLambdaFromMember(aggMe, p_partkey);
    }

    Lambda<MinDouble> getValueProjection (Handle<Q02PartJoinOutput> aggMe) override {
         return makeLambda(aggMe, [](Handle<Q02PartJoinOutput>& aggMe) {
             MinDouble ret;
             ret.setValue (aggMe->ps_supplycost);
             return ret;
         });
    }


};


class Q02MinCostJoinOutput : public Object {

public:

    ENABLE_DEEP_COPY

    int p_partkey;
    String p_mfgr;
    String n_name;
    double s_acctbal;
    String s_name;
    String s_address;
    String s_phone;
    String s_comment;
    double ps_supplycost;
    double min;

    Q02MinCostJoinOutput () {}

    Q02MinCostJoinOutput (Q02MinCostPerPart& minCost, Q02PartJoinOutput& supplier) {
        this->n_name = supplier.n_name;
        this->s_acctbal = supplier.s_acctbal;
        this->s_name = supplier.s_name;
        this->s_address = supplier.s_address;
        this->s_phone = supplier.s_phone;
        this->s_comment = supplier.s_comment;
        this->p_partkey = supplier.p_partkey;
        this->p_mfgr = supplier.p_mfgr;
        this->ps_supplycost = supplier.ps_supplycost;
        this->min = minCost.min.min;
    }

};




class Q02MinCostJoin : public JoinComp<Q02MinCostJoinOutput, Q02MinCostPerPart, Q02PartJoinOutput> {

public:

    ENABLE_DEEP_COPY


    Q02MinCostJoin () {}

    Lambda<bool> getSelection (Handle<Q02MinCostPerPart> in1, Handle<Q02PartJoinOutput> in2) override {
        return makeLambdaFromMember(in1, ps_partkey) == makeLambdaFromMember(in2, p_partkey);
    }

    Lambda<Handle<Q02MinCostJoinOutput>> getProjection(Handle<Q02MinCostPerPart> in1, Handle<Q02PartJoinOutput> in2) override {
        return makeLambda(in1, in2, [](Handle<Q02MinCostPerPart>& in1, Handle<Q02PartJoinOutput>& in2) {
             Handle<Q02MinCostJoinOutput> ret = makeObject<Q02MinCostJoinOutput> (*in1, *in2);
             return ret;
        });
    }

};


class Q02MinCostSelectionOutput : public Object {

public:

    ENABLE_DEEP_COPY

    int p_partkey;
    String p_mfgr;
    String n_name;
    double s_acctbal;
    String s_name;
    String s_address;
    String s_phone;
    String s_comment;

    Q02MinCostSelectionOutput () {}

    Q02MinCostSelectionOutput (Q02MinCostJoinOutput& minCost) {
        this->n_name = minCost.n_name;
        this->s_acctbal = minCost.s_acctbal;
        this->s_name = minCost.s_name;
        this->s_address = minCost.s_address;
        this->s_phone = minCost.s_phone;
        this->s_comment = minCost.s_comment;
        this->p_partkey = minCost.p_partkey;
        this->p_mfgr = minCost.p_mfgr;
    }

};



class Q02MinCostSelection : public SelectionComp<Q02MinCostSelectionOutput, Q02MinCostJoinOutput> {


public:

    ENABLE_DEEP_COPY

    Q02MinCostSelection () {}

    Lambda<bool> getSelection(Handle<Q02MinCostJoinOutput> checkMe) override {
        return makeLambdaFromMember(checkMe, ps_supplycost) == 
                  makeLambdaFromMember(checkMe, min);
    }

    Lambda<Handle<Q02MinCostSelectionOutput>> getProjection(Handle<Q02MinCostJoinOutput> checkMe) override {
        return makeLambda(checkMe, [](Handle<Q02MinCostJoinOutput>& checkMe) { 
           Handle<Q02MinCostSelectionOutput> ret = makeObject<Q02MinCostSelectionOutput> (*checkMe);
           return ret; 
        });
    }


};

/*
    val europe = region.filter($"r_name" === "EUROPE")
      .join(nation, $"r_regionkey" === nation("n_regionkey"))
      .join(supplier, $"n_nationkey" === supplier("s_nationkey"))
      .join(partsupp, supplier("s_suppkey") === partsupp("ps_suppkey"))
    //.select($"r_regionkey", $"n_regionkey", $"s_suppkey", $"n_nationkey", $"s_nationkey", $"p_partkey", $"p_mfgr", $"ps_partkey", $"ps_supplycost", $"s_acctbal", $"s_name", $"n_name", $"s_address", $"s_phone", $"s_comment")

    val brass = part.filter(part("p_size") === 15 && part("p_type").endsWith("BRASS"))
      .join(europe, europe("ps_partkey") === $"p_partkey")
    //.cache

    val minCost = brass.groupBy(brass("ps_partkey"))
      .agg(min("ps_supplycost").as("min"))

    brass.join(minCost, brass("ps_partkey") === minCost("ps_partkey"))
      .filter(brass("ps_supplycost") === minCost("min"))
      .select("s_acctbal", "s_name", "n_name", "p_partkey", "p_mfgr", "s_address", "s_phone", "s_comment")
      .sort($"s_acctbal".desc, $"n_name", $"s_name", $"p_partkey")
      .limit(100)
*/

}

#endif
