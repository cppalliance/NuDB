//
// Copyright (c) 2019 Miguel Portilla (miguelportilla64 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_IMPL_CONTEXT_IPP
#define NUDB_IMPL_CONTEXT_IPP

#include <nudb/detail/store_base.hpp>

namespace nudb {

context::
~context() noexcept (false)
{
    stop_all();
    std::lock_guard<std::mutex> lock(m_);
    if(! waiting_.empty() || ! flushing_.empty())
        throw std::logic_error("All databases haven't been closed");
}

void
context::
start()
{
    std::lock_guard<std::mutex> lock(m_);
    if(! stop_ && ! t_.joinable())
        t_ = std::thread(&context::run, this);
}

void
context::
stop_all()
{
    std::unique_lock<std::mutex> lock(m_);
    if(stop_)
        return;
    stop_ = true;
    cv_f_.notify_all();
    cv_w_.notify_all();
    cv_w_.wait(lock,
        [&]
        {
            return num_threads_ == 0;
        });
    if(t_.joinable())
    {
        lock.unlock();
        t_.join();
        lock.lock();
    }
    stop_ = false;
}

void
context::
run()
{
    bool const first = [&]
    {
        std::lock_guard<std::mutex> lock(m_);
        return ++num_threads_ == 1;
    }();

    auto when = clock_type::now();
    for(;;)
    {
        {
            std::unique_lock<std::mutex> lock(m_);
            if(stop_)
                break;
            if(first)
            {
                auto const status = cv_f_.wait_until(
                    lock, when + std::chrono::seconds{1});
                if(stop_)
                    break;
                if(status == std::cv_status::timeout && ! waiting_.empty())
                {
                    // Move everything in waiting_ to flushing_
                    for(auto store = waiting_.head_; store;
                        store = store->next_)
                        store->state_ = store_base::state::flushing;
                    flushing_.splice(waiting_);
                }

                when = clock_type::now();
                if(flushing_.empty())
                    continue;
                cv_f_.notify_all();
            }
            else
            {
                cv_f_.wait(lock,
                    [this]
                    {
                        return ! flushing_.empty() || stop_;
                    });
                if(stop_)
                    break;
            }
        }

        // process everything in flushing_
        for(;;)
            if(! flush_one())
                break;
    }

    std::lock_guard<std::mutex> lock(m_);
    --num_threads_;
    cv_w_.notify_all();
}

void
context::
insert(store_base& store)
{
    std::lock_guard<std::mutex> lock(m_);
    store.state_ = store_base::state::waiting;
    waiting_.push_back(&store);
    cv_w_.notify_all();
}

void
context::
erase(store_base& store)
{
    using state = store_base::state;
    std::unique_lock<std::mutex> lock(m_);
    switch(store.state_)
    {
    case state::waiting:
        waiting_.erase(&store);
        cv_w_.notify_all();
        break;
    case state::flushing:
        flushing_.erase(&store);
        break;
    case state::intermediate:
        cv_w_.wait(lock,
            [&]
            {
                return store.state_ != state::intermediate;
            });
        if(store.state_ == state::flushing)
        {
            flushing_.erase(&store);
        }
        else if(store.state_ == state::waiting)
        {
            waiting_.erase(&store);
            cv_w_.notify_all();
        }
        break;
    case state::none:
        return;
    }
    store.state_ = state::none;
}

bool
context::
flush_one()
{
    store_base* store;
    {
        std::lock_guard<std::mutex> lock(m_);
        if(flushing_.empty())
            return false;
        store = flushing_.head_;
        store->state_ = store_base::state::intermediate;
        flushing_.erase(store);
    }

    store->flush();

    std::lock_guard<std::mutex> lock(m_);
    store->state_ = store_base::state::waiting;
    waiting_.push_back(store);
    cv_w_.notify_all();
    return true;
}

void
context::list::
push_back(store_base* node)
{
    node->next_ = nullptr;
    node->prev_ = tail_;
    if(tail_)
        tail_->next_ = node;
    tail_ = node;
    if(!head_)
        head_ = node;
}

void
context::list::
erase(store_base* node)
{
    if(node->next_)
        node->next_->prev_ = node->prev_;
    else // node == tail_
        tail_ = node->prev_;

    if(node->prev_)
        node->prev_->next_ = node->next_;
    else // node == head_
        head_ = node->next_;

    node->next_ = nullptr;
    node->prev_ = nullptr;
}

void
context::list::
splice(list& right)
{
    if(right.empty())
        return;
    if(empty())
    {
        head_ = right.head_;
        tail_ = right.tail_;
    }
    else
    {
        right.head_->prev_ = tail_;
        tail_->next_ = right.head_;
        tail_ = right.tail_;
    }
    right.head_ = nullptr;
    right.tail_ = nullptr;
}

} // nudb

#endif
