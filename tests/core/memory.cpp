#include <catch2/catch.hpp>

#include <seraphim/memory.hpp>

using namespace sph;

class Base {};
class Derived : public Base {};

TEST_CASE( "Unique pointer conversion", "[convert_unique]" ) {
    SECTION( "derived class pointers can be converted to base class pointers" ) {
        std::unique_ptr<Derived> derived_ptr(new Derived);

        // this is a compile time test
        std::unique_ptr<Base> base_ptr = convert_unique<Base>(derived_ptr);
    }
}

TEST_CASE( "Shared pointer conversion", "[convert_shared]" ) {
    SECTION( "derived class pointers can be converted to base class pointers" ) {
        std::shared_ptr<Derived> derived_ptr(new Derived);

        // this is a compile time test
        std::shared_ptr<Base> base_ptr = convert_shared<Base>(derived_ptr);
    }
    SECTION( "derived unique class pointers can be converted to shared base class pointers" ) {
        std::unique_ptr<Derived> derived_ptr(new Derived);

        // this is a compile time test
        std::shared_ptr<Base> base_ptr = convert_shared<Base>(derived_ptr);
    }
}
