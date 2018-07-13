//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

struct LibrarySummary;
class RecyclerLibraryGraph
{
public:
    RecyclerLibraryGraph(RecyclerObjectGraph& objectGraph);
    ~RecyclerLibraryGraph();

    template <typename Fn>
    void ForEach(Fn fn)
    {
        for (size_t index = 0; index < libraryMap.size(); index++)
        {
            fn(*sortedArray.get()[index]);
        }
    }

    LibrarySummary const * GetLibrarySummary(ULONG64 library)
    {
        auto i = libraryMap.find(library);
        if (i != libraryMap.end())
        {
            return i->second;
        }
        return nullptr;
    }
private:
    template <typename Fn>
    uint WalkGraph(Fn process, bool includePredecessors)
    {
        // Go thru all node and do either a depth first walk (!includePredecessors)
        // Or visit all connect node (includePredecessors)
        // The callback is responsible for detect whether a node is visited or not
        uint iter = 1;
        ForEach([&](LibrarySummary& i)
        {
            if (!process(i, 0))
            {
                return;
            }

            std::stack<ULONG64> work;
            work.push(i.library);
            while (!work.empty())
            {
                ULONG64 curr = work.top();
                work.pop();
                LibrarySummary& currSummary = *libraryMap[curr];
                if (!process(currSummary, iter))
                {
                    continue;
                }

                if (includePredecessors)
                {
                    for (auto i : currSummary.predecessors)
                    {
                        work.push(i);
                    }
                }
                for (auto i : currSummary.successors)
                {
                    work.push(i);
                }
            }
            iter++;
        });
        return iter;
    }
    std::map<ULONG64, LibrarySummary *> libraryMap;
    std::auto_ptr<LibrarySummary *> sortedArray;
};