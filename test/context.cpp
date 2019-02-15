//
// Copyright (c) 2019 Miguel Portilla (miguelportilla64 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Test that header file is self-contained
#include <nudb/context.hpp>
#include <nudb/_experimental/test/temp_dir.hpp>
#include <nudb/nudb.hpp>

#include "suite.hpp"

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <boost/filesystem.hpp>

namespace nudb {
namespace test {

class context_test : public boost::beast::unit_test::suite
{
public:
    class tmp_store : public store
    {
    public:
        tmp_store(context& ctx, std::function<void(std::thread::id)>& f,
            error_code& ec)
            : store(ctx)
            , td_(boost::filesystem::path{})
            , f_(f)
        {
            nudb::create<nudb::xxhasher>(
                td_.file("nudb.dat"),
                td_.file("nudb.key"),
                td_.file("nudb.log"),
                1,
                nudb::make_salt(),
                4,
                4096,
                0.5f,
                ec);
        }

        ~tmp_store()
        {
            error_code ec;
            close(ec);
        }

        void
        flush() override
        {
            store::flush();
            f_(std::this_thread::get_id());
        }

        temp_dir td_;
        std::function<void(std::thread::id)>& f_;
    };

    std::size_t
    size(context::list const& v)
    {
        std::size_t size = 0;
        for (auto node = v.head_; node; node = node->next_)
            ++size;
        return size;
    }

    void
    test_list()
    {
        context::list v[2];
        store s[2];

        v[0].push_back(&s[0]);
        v[1].push_back(&s[1]);
        BEAST_EXPECT(size(v[0]) == 1);

        v[1].splice(v[0]);
        BEAST_EXPECT(size(v[1]) == 2);
        BEAST_EXPECT(v[1].head_ == &s[1]);
        BEAST_EXPECT(v[0].empty());

        v[1].splice(v[0]);
        BEAST_EXPECT(size(v[1]) == 2);
        BEAST_EXPECT(v[0].empty());

        v[0].splice(v[1]);
        BEAST_EXPECT(size(v[0]) == 2);
        BEAST_EXPECT(v[0].head_ == &s[1]);
        BEAST_EXPECT(v[1].empty());

        v[0].erase(&s[1]);
        BEAST_EXPECT(size(v[0]) == 1);
        BEAST_EXPECT(v[0].head_ == &s[0]);

        v[0].push_back(&s[1]);
        v[0].erase(&s[1]);
        BEAST_EXPECT(size(v[0]) == 1);

        v[0].erase(&s[0]);
        BEAST_EXPECT(v[0].empty());
        BEAST_EXPECT(v[0].head_ == nullptr);
    }

    void
    test_context()
    {
        using state = detail::store_base::state;

        context ctx;
        BEAST_EXPECT(ctx.num_threads_ == 0);
        BEAST_EXPECT(! ctx.t_.joinable());
        BEAST_EXPECT(! ctx.stop_);

        error_code ec;
        std::vector<std::unique_ptr<tmp_store>> dbs;
        std::set<std::thread::id> ids;
        std::mutex ids_mutex;
        std::function<void(std::thread::id)> f = [&](std::thread::id id) {
            std::lock_guard<std::mutex> lock(ids_mutex);
            ids.erase(id);
        };
        for (int i = 0; i < 3; ++i)
        {
            dbs.emplace_back(std::make_unique<tmp_store>(ctx, f, ec));
            if(! BEAST_EXPECTS(! ec, ec.message()))
                return;
        }

        BEAST_EXPECT(ctx.waiting_.empty());
        BEAST_EXPECT(ctx.flushing_.empty());
        for (auto& db : dbs)
        {
            BEAST_EXPECT(db->state_ ==  state::none);
            db->open(
                db->td_.file("nudb.dat"),
                db->td_.file("nudb.key"),
                db->td_.file("nudb.log"),
                ec);
            if(! BEAST_EXPECTS(! ec, ec.message()))
                return;
            BEAST_EXPECT(db->state_ == state::waiting);
        }
        BEAST_EXPECT(size(ctx.waiting_) == dbs.size());

        for (auto s = ctx.waiting_.head_; s; s = s->next_)
            s->state_ = state::flushing;
        ctx.flushing_.splice(ctx.waiting_);
        BEAST_EXPECT(size(ctx.flushing_) == dbs.size());

        while (ctx.flush_one()) {}
        BEAST_EXPECT(size(ctx.waiting_) == dbs.size());
        for (auto s = ctx.waiting_.head_; s; s = s->next_)
            BEAST_EXPECT(s->state_ == state::waiting);
        BEAST_EXPECT(ctx.flushing_.empty());

        // context store erase
        {
            // state waiting
            auto s = ctx.waiting_.head_;
            ctx.erase(*s);
            BEAST_EXPECT(s->state_ == state::none);

            // state flushing
            ctx.flushing_.push_back(s);
            s->state_ = state::flushing;
            ctx.erase(*s);
            BEAST_EXPECT(s->state_ == state::none);

            // state intermediate flush
            ctx.flushing_.push_back(s);
            s->state_ = state::intermediate;
            {
                std::thread t([&]() {
                    ctx.erase(*s);
                });
                s->state_ = state::flushing;
                ctx.cv_w_.notify_all();
                t.join();
            }
            BEAST_EXPECT(s->state_ == state::none);

            // state intermediate waiting
            ctx.waiting_.push_back(s);
            s->state_ = state::intermediate;
            {
                std::thread t([&]() {
                    ctx.erase(*s);
                });
                s->state_ = state::waiting;
                ctx.cv_w_.notify_all();
                t.join();
            }
            BEAST_EXPECT(s->state_ == state::none);

            ctx.waiting_.push_back(s);
            s->state_ = state::waiting;
        }

        // thread function
        {
            ctx.start();
            {
                std::unique_lock<std::mutex> lock(ctx.m_);
                BEAST_EXPECT(ctx.t_.joinable());
                ctx.cv_f_.wait(lock,
                    [&]
                    {
                        return ctx.num_threads_ != 0;
                    });
                ids.emplace(ctx.t_.get_id());

                for (int i = 0; i < 3; ++i)
                {
                    auto t = std::thread([&] {
                        ctx.run();
                    });
                    ids.emplace(t.get_id());
                    t.detach();
                }

                ctx.cv_f_.wait(lock,
                    [&]
                    {
                        std::lock_guard<std::mutex> lock(ids_mutex);
                        return ids.empty();
                    });
            }
            BEAST_EXPECT(ctx.num_threads_ == 4);
            ctx.stop_all();
            BEAST_EXPECT(ctx.num_threads_ == 0);
            BEAST_EXPECT(! ctx.t_.joinable());
            BEAST_EXPECT(! ctx.stop_);
        }

        dbs.clear();
        BEAST_EXPECT(ctx.waiting_.empty());
        BEAST_EXPECT(ctx.flushing_.empty());
    }

    void
    run() override
    {
        BOOST_STATIC_ASSERT(! std::is_copy_constructible<context>{});
        BOOST_STATIC_ASSERT(! std::is_copy_assignable<context>{});
        BOOST_STATIC_ASSERT(! std::is_move_constructible<context>{});
        BOOST_STATIC_ASSERT(! std::is_move_assignable<context>{});

        test_list();
        test_context();
    }
};

DEFINE_TESTSUITE(nudb,test,context);

} // test
} // nudb

