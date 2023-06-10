#ifndef COMMENT_PARTITION_H
#define COMMENT_PARTITION_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "CommentPartition.h"
#include "PartitionComp.h"
#include "RedditComment.h"

#include <iostream>

using namespace pdb;

namespace reddit {
class CommentPartition : public PartitionComp<int, Comment> {
public:
  ENABLE_DEEP_COPY
  int range;

  CommentPartition() {}

  CommentPartition(int range, std::string dbname, std::string setname) : range(range) {
    this->setOutput(dbname, setname);
  }

  Lambda<int> getProjection(Handle<Comment> checkMe) override {
    return makeLambda(
        checkMe, [this](Handle<Comment> &checkMe) { return (int) checkMe->index / range; });
  }
};
} // namespace reddit

#endif