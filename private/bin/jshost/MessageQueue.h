//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class MessageBase
{
private:
    unsigned int m_time;
    unsigned int m_id;

    static unsigned int s_messageCount;

    MessageBase(const MessageBase&);

public:
    MessageBase(unsigned int time) : m_time(time), m_id(s_messageCount++) { }
    virtual ~MessageBase() { }

    void BeginTimer() { m_time += GetTickCount(); };
    unsigned int GetTime() { return m_time; };
    unsigned int GetId() { return m_id; };

    virtual HRESULT Call() = 0;

    struct Comparator
    {
        bool operator()(MessageBase const* const& lhs, MessageBase const* const& rhs) const
        {
            return lhs->m_time > rhs->m_time;
        }
    };
};

class MessageQueue
{
    std::multimap<unsigned int, MessageBase*> m_queue;

public:
    ~MessageQueue()
    {
        for (const auto& pair : m_queue)
        {
            delete pair.second;
        }
    }

    void Push(MessageBase *message)
    {
        message->BeginTimer();
        m_queue.insert(std::make_pair(message->GetTime(), message));
    }

    MessageBase* PopAndWait()
    {
        Assert(!m_queue.empty());

        auto it = m_queue.begin();
        MessageBase *tmp = it->second;
        
        m_queue.erase(it);

        int waitTime = tmp->GetTime() - GetTickCount();
        if(waitTime > 0)
        {
            Sleep(waitTime);
        }

        return tmp;
    }

    bool IsEmpty()
    {
        return m_queue.empty();
    }

    void RemoveById(unsigned int id)
    {
        // Search for the message with the correct id, and delete it.  Can be updated
        // to a hash to improve speed, if necessary.
        for(auto it = m_queue.begin(); it != m_queue.end(); ++it)
        {
            MessageBase *msg = it->second;
            if(msg->GetId() == id)
            {
                m_queue.erase(it);
                delete msg;
                break;
            }
        }
    }

    HRESULT ProcessAll()
    {
        while(!IsEmpty())
        {
            MessageBase *msg = PopAndWait();
            msg->Call();                     // Ommiting return value, its async function call it shouldn't affect others.
            delete msg;
        }
        return S_OK;
    }
};

//
// A custom message helper class to assist defining messages handled by callback functions.
//
template <class Func, class CustomBase>
class CustomMessage : public CustomBase
{
private:
    Func m_func;

public:
    CustomMessage(unsigned int time, const typename CustomBase::CustomArgType& customArg, const Func& func) :
        CustomBase(time, customArg), m_func(func)
    {}

    virtual HRESULT Call() override
    {
        return m_func(*this);
    }
};
