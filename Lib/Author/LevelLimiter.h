#pragma once

namespace Authoring
{
    struct LevelLimiterContext
    {
    public:
        int level;
        bool treeTruncated;

        LevelLimiterContext()
            :level(0), treeTruncated(false)
        {
        }
    };

    struct LevelLimiter : public VisitorPolicyBase<LevelLimiterContext*>
    {
    protected:
        bool Preorder(ParseNode *pnode, LevelLimiterContext *context);
        void Postorder(ParseNode *pnode, LevelLimiterContext *context);
    };

}