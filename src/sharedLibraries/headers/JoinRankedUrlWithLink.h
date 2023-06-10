#pragma once

#include <JoinComp.h>
#include <LambdaCreationFunctions.h>
#include "URLURLsRank.h"
#include "Link.h"
#include "RankedUrl.h"

namespace pdb {

class JoinRankedUrlWithLink : public JoinComp<URLURLsRank, Link, RankedUrl> {

 public:

  ENABLE_DEEP_COPY

  JoinRankedUrlWithLink() = default;

  Lambda<bool> getSelection(Handle<Link> in1, Handle<RankedUrl> in2) override {
    return (makeLambdaFromMember(in1, url) == makeLambdaFromMember(in2, url));
  }

  Lambda<Handle<URLURLsRank>> getProjection(Handle<Link> in1, Handle<RankedUrl> in2) override {
    return makeLambda(in1, in2, [](Handle<Link>& in1, Handle<RankedUrl>& in2) {
      Handle<URLURLsRank> tmp = makeObject<URLURLsRank>(in1->url, in1->neighbors, in2->rank);
      return tmp;
    });
  }
};

}
