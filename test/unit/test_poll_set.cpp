/// Unit tests for PollSet

#include <PollSet.h>
#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <unistd.h>


namespace {


struct PipePair
{
    int readFd = -1;
    int writeFd = -1;

    PipePair()
    {
        int fds[2];
        if (::pipe(fds) == 0) {
            readFd = fds[0];
            writeFd = fds[1];
        }
    }

    ~PipePair()
    {
        if (readFd >= 0)
            ::close(readFd);
        if (writeFd >= 0)
            ::close(writeFd);
    }

    PipePair(const PipePair &) = delete;
    PipePair & operator=(const PipePair &) = delete;
};


}  // namespace


TEST_CASE("PollSet starts empty and reports size 0", "[PollSet]")
{
    PollSet set;
    REQUIRE(set.size() == 0);

    PollSet::t_FileDescList list;
    REQUIRE(set.getFdList(list));
    REQUIRE(list.empty());
}


TEST_CASE("PollSet add increases size", "[PollSet]")
{
    PipePair pipe;
    REQUIRE(pipe.readFd >= 0);

    PollSet set;
    REQUIRE(set.add(pipe.readFd));
    REQUIRE(set.size() == 1);

    PollSet::t_FileDescList list;
    REQUIRE(set.getFdList(list));
    REQUIRE(list.size() == 1);
    REQUIRE(list[0] == pipe.readFd);
}


TEST_CASE("PollSet clear empties the set", "[PollSet]")
{
    PipePair p1;
    PipePair p2;
    REQUIRE(p1.readFd >= 0);
    REQUIRE(p2.readFd >= 0);

    PollSet set;
    REQUIRE(set.add(p1.readFd));
    REQUIRE(set.add(p2.readFd));
    REQUIRE(set.size() == 2);

    set.clear();
    REQUIRE(set.size() == 0);

    PollSet::t_FileDescList list;
    REQUIRE(set.getFdList(list));
    REQUIRE(list.empty());
}


TEST_CASE("PollSet poll with no ready FDs returns empty list", "[PollSet]")
{
    PipePair pipe;
    REQUIRE(pipe.readFd >= 0);

    PollSet set;
    REQUIRE(set.add(pipe.readFd));

    PollSet::t_FileDescList active;
    // Pipe has no data written — poll with zero timeout should return
    // true (poll succeeded) with an empty active list.
    REQUIRE(set.poll(0, active));
    REQUIRE(active.empty());
}


TEST_CASE("PollSet poll on empty set returns empty list", "[PollSet]")
{
    PollSet set;

    PollSet::t_FileDescList active;
    REQUIRE(set.poll(0, active));
    REQUIRE(active.empty());
}


TEST_CASE("PollSet poll returns ready FD when pipe has data", "[PollSet]")
{
    PipePair pipe;
    REQUIRE(pipe.readFd >= 0);

    PollSet set;
    REQUIRE(set.add(pipe.readFd));

    const char msg = 'x';
    REQUIRE(::write(pipe.writeFd, &msg, 1) == 1);

    PollSet::t_FileDescList active;
    // short timeout — but data is already written, so poll() should
    // return immediately with the read end active.
    REQUIRE(set.poll(100, active));
    REQUIRE(active.size() == 1);
    REQUIRE(active[0] == pipe.readFd);

    // Drain so destructor close doesn't leave data pending
    char buf;
    ::read(pipe.readFd, &buf, 1);
}


TEST_CASE("PollSet poll with multiple FDs returns only the ready one", "[PollSet]")
{
    PipePair ready;
    PipePair idle;
    REQUIRE(ready.readFd >= 0);
    REQUIRE(idle.readFd >= 0);

    PollSet set;
    REQUIRE(set.add(ready.readFd));
    REQUIRE(set.add(idle.readFd));
    REQUIRE(set.size() == 2);

    const char msg = 'y';
    REQUIRE(::write(ready.writeFd, &msg, 1) == 1);

    PollSet::t_FileDescList active;
    REQUIRE(set.poll(100, active));

    REQUIRE(active.size() == 1);
    REQUIRE(active[0] == ready.readFd);

    char buf;
    ::read(ready.readFd, &buf, 1);
}


TEST_CASE("PollSet add via FD list adds all", "[PollSet]")
{
    PipePair p1;
    PipePair p2;
    PipePair p3;
    REQUIRE(p1.readFd >= 0);
    REQUIRE(p2.readFd >= 0);
    REQUIRE(p3.readFd >= 0);

    PollSet::t_FileDescList toAdd = {p1.readFd, p2.readFd, p3.readFd};

    PollSet set;
    REQUIRE(set.add(toAdd));
    REQUIRE(set.size() == 3);
}


TEST_CASE("PollSet duplicate add is allowed (non-idempotent)", "[PollSet]")
{
    // PollSet::add does not deduplicate — each add() increments the
    // FD count. Document that behavior so callers are aware.
    //
    PipePair pipe;
    REQUIRE(pipe.readFd >= 0);

    PollSet set;
    REQUIRE(set.add(pipe.readFd));
    REQUIRE(set.add(pipe.readFd));
    REQUIRE(set.size() == 2);

    PollSet::t_FileDescList list;
    REQUIRE(set.getFdList(list));
    REQUIRE(list.size() == 2);
    REQUIRE(list[0] == pipe.readFd);
    REQUIRE(list[1] == pipe.readFd);
}


TEST_CASE("PollSet setEvents updates requested event mask", "[PollSet]")
{
    PipePair pipe;
    REQUIRE(pipe.readFd >= 0);

    PollSet set(POLLIN);
    REQUIRE(set.add(pipe.readFd));
    REQUIRE(set.setEvents(POLLIN | POLLOUT));

    // After changing events, empty poll still works cleanly
    PollSet::t_FileDescList active;
    REQUIRE(set.poll(0, active));
}


TEST_CASE("PollSet resize keeps existing FDs", "[PollSet]")
{
    // defaultPollFds is 128; adding more than that should trigger a resize
    // and preserve the earlier entries.
    //
    PollSet set;

    // Use pipe read ends (held open) for at least one real entry and fill the
    // remainder with the same FD value since PollSet doesn't validate.
    PipePair pipe;
    REQUIRE(pipe.readFd >= 0);

    for (int i = 0; i < 200; ++i) {
        REQUIRE(set.add(pipe.readFd));
    }
    REQUIRE(set.size() == 200);

    PollSet::t_FileDescList list;
    REQUIRE(set.getFdList(list));
    REQUIRE(list.size() == 200);
    for (int fd : list) {
        REQUIRE(fd == pipe.readFd);
    }
}
