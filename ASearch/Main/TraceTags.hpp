/*

  © Vestris Inc., Geneva, Switzerland
  http://www.vestris.com, 1994-1999 All Rights Reserved
  ______________________________________________
  
  written by Daniel Doubrovkine - dblock@vestris.com
    
*/

#ifndef ALKALINE_TRACER_TAGS_HPP
#define ALKALINE_TRACER_TAGS_HPP
    
#include <platform/include.hpp>
#include <Object/Tracer.hpp>

#ifdef BASE_TRACE_ENABLED

typedef enum {
    tagAlkalineMin = tagBuiltinMax,
    tagAlkaline,
    tagMerge,
    tagExcludeWords,
    tagSearch,
    tagRemovePages,
    tagIndex,
    tagClient,
    tagRender,
    tagAlkalineMax
} CTraceAlkalineTags;

static CInternalTraceTagDesc s_TraceAlkalineDescriptions[] = {
    { tagAlkalineMin, "reserved tag" },
    { tagAlkaline, "generic engine tag" },
    { tagMerge, "merge" },
    { tagExcludeWords, "excludewords" },
    { tagSearch, "search" },
    { tagRemovePages, "remove" },
    { tagIndex, "index" },
    { tagClient, "client and admin" },
    { tagRender, "rendering results" },
    { tagAlkalineMax, "reserved tag" }
};

static inline CInternalTraceTagDesc * GetTraceAlkalineDescriptions(void) { return (CInternalTraceTagDesc *) s_TraceAlkalineDescriptions; }

#endif
#endif
