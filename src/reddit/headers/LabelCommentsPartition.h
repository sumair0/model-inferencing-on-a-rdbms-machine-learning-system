#ifndef LABEL_COMMENTS_PARTITION_H
#define LABEL_COMMENTS_PARTITION_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "RedditComment.h"
#include "PartitionComp.h"

using namespace pdb;

class LabelCommentsPartition : public PartitionComp<pdb::String, reddit::Comment> {
public:
  ENABLE_DEEP_COPY

  LabelCommentsPartition() {}

  LabelCommentsPartition(std::string dbname, std::string setname) {
    this->setOutput(dbname, setname);
  }

  Lambda<pdb::String> getProjection(Handle<reddit::Comment> row) override {
    return makeLambda(row, [this](Handle<reddit::Comment> &row) {
      if(row->label == 1){
        return row->author;
      }else{
        return row->subreddit_id;
      }
    });
  }
};

#endif