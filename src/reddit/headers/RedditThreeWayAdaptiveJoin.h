#pragma once

#include <JoinComp.h>
#include <LambdaCreationFunctions.h>
#include "RedditComment.h"
#include "RedditAuthor.h"
#include "RedditSub.h"
#include "RedditFullFeatures.h"

namespace reddit {

class ThreeWayAdaptiveJoin : public JoinComp<FullFeatures, Comment, Author, Sub> {

 public:

  ENABLE_DEEP_COPY

  ThreeWayAdaptiveJoin() = default;

  Lambda<bool> getSelection(Handle<Comment> in1, Handle<Author> in2, Handle<Sub> in3) override {
    return makeLambda(in1, in2, in3, [](Handle<Comment>& in1, Handle<Author>& in2, Handle<Sub>& in3) {
        if (in1->label == 1) 
           return in1->author == in2->author;
        else
           return in1->subreddit_id == in3->name;
    });
  }

  Lambda<Handle<FullFeatures>> getProjection(Handle<Comment> in1, Handle<Author> in2, Handle<Sub> in3) override {
    return makeLambda(in1, in2, in3, [](Handle<Comment>& in1, Handle<Author>& in2, Handle<Sub>& in3) {
      Handle<FullFeatures> tmp = makeObject<FullFeatures>(in1, in2, in3);
      return tmp;
    });
  }
};

}
