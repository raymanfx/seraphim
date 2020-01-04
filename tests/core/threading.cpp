#include <catch2/catch.hpp>
#include <thread>

#include <seraphim/threading.hpp>

using namespace sph;

class Counter : public Synchronizeable<Counter> {
public:
    Counter() {}

    inline int val() const { return m_val; }

    void inc() {
        m_val++;

        if (!m_last_op_inc) {
            m_consecutive_incs = 0;
        }
        m_last_op_inc = true;
        m_consecutive_incs++;
    }

    void dec() {
        m_val--;

        if (m_last_op_inc) {
            m_consecutive_decs = 0;
        }
        m_last_op_inc = false;
        m_consecutive_decs++;
    }

    inline int consecutive_incs() const { return m_consecutive_incs; }
    inline int consecutive_decs() const { return m_consecutive_decs; }

private:
    int m_val = 0;

    bool m_last_op_inc = false;
    int m_consecutive_incs = 0;
    int m_consecutive_decs = 0;
};

TEST_CASE( "Synchronizeable runtime behavior", "[Synchronizeable]" ) {
    SECTION( "all members of the implementing class can be accessed" ) {
        Counter cnt;

        // this is a compile time test
        cnt.synchronized()->val();
        cnt.synchronized()->inc();
        cnt.synchronized()->dec();
        cnt.synchronized()->consecutive_incs();
        cnt.synchronized()->consecutive_decs();
    }
}

TEST_CASE( "Synchronized runtime behavior", "[Synchronized]" ) {
    SECTION( "all access to the wrapper instance is thread safe" ) {
        Counter cnt;

        std::thread inc_thread([&]() {
            Synchronized<Counter> scnt(cnt);
            for (auto i = 0; i < 1000; i++) {
                scnt->inc();
            }
        });

        std::thread dec_thread([&]() {
            Synchronized<Counter> scnt(cnt);
            scnt->dec();
            scnt->dec();
        });

        inc_thread.join();
        dec_thread.join();

        // verify that all ops occured in order
        REQUIRE( cnt.consecutive_incs() == 1000 );
        REQUIRE( cnt.consecutive_decs() == 2 );
    }
}
