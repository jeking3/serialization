/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// test_variant.cpp
// test of non-intrusive serialization of variant types
//
// copyright (c) 2005   
// troy d. straszheim <troy@resophonic.com>
// http://www.resophonic.com
//
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org for updates, documentation, and revision history.
//
// thanks to Robert Ramey and Peter Dimov.
//

#include <fstream>
#include <cstdio> // remove
#include <cmath> // for fabs()
#include <boost/config.hpp>
#if defined(BOOST_NO_STDC_NAMESPACE)
namespace std{ 
    using ::remove;
}
#endif

#if defined(_MSC_VER) && (_MSC_VER <= 1020)
#  pragma warning (disable : 4786) // too long name, harmless warning
#endif

#include "test_tools.hpp"
#include <boost/preprocessor/stringize.hpp>
#include BOOST_PP_STRINGIZE(BOOST_ARCHIVE_TEST)

#include "throw_exception.hpp"
#include <boost/archive/archive_exception.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/variant.hpp>

#include "A.hpp"

class are_equal
    : public boost::static_visitor<bool>
{
public:
    template <typename T, typename U>
    bool operator()( const T &, const U & ) const 
    {
        return false; // cannot compare different types
    }
    template <typename T>
    bool operator()( const T & lhs, const T & rhs ) const 
    {
        return lhs == rhs;
    }
    bool operator()( const float & lhs, const float & rhs ) const
    {
        return std::fabs(lhs- rhs) < std::numeric_limits<float>::round_error();
    }
    bool operator()( const double & lhs, const double & rhs ) const
    {
        return std::fabs(lhs - rhs) < std::numeric_limits<float>::round_error();
    }
};

template <class T>
void test_type(const T& gets_written){
   const char * testfile = boost::archive::tmpnam(NULL);
   BOOST_REQUIRE(testfile != NULL);
   {
      test_ostream os(testfile, TEST_STREAM_FLAGS);
      test_oarchive oa(os);
      oa << boost::serialization::make_nvp("written", gets_written);
   }

   T got_read;
   {
      test_istream is(testfile, TEST_STREAM_FLAGS);
      test_iarchive ia(is);
      ia >> boost::serialization::make_nvp("written", got_read);
   }
   BOOST_CHECK(boost::apply_visitor(are_equal(), gets_written, got_read));

   std::remove(testfile);
}

//
// this verifies that if you try to read in a variant from a file
// whose "which" is illegal for the one in memory (that is, you're
// reading in to a different variant than you wrote out to) the load()
// operation will throw.  One could concievably add checking for
// sequence length as well, but this would add size to the archive for
// dubious benefit.
//
void do_bad_read()
{
  boost::variant<bool, float, int, std::string> big_variant;
  big_variant = std::string("adrenochrome");

  const char * testfile = boost::archive::tmpnam(NULL);
  BOOST_REQUIRE(testfile != NULL);
  {
    test_ostream os(testfile, TEST_STREAM_FLAGS);
    test_oarchive oa(os);
    oa << BOOST_SERIALIZATION_NVP(big_variant);
  }
  boost::variant<bool, float, int> little_variant;
  {
    test_istream is(testfile, TEST_STREAM_FLAGS);
    test_iarchive ia(is);
    bool exception_invoked = false;
    BOOST_TRY {
      ia >> BOOST_SERIALIZATION_NVP(little_variant);
    } BOOST_CATCH (boost::archive::archive_exception e) {
      BOOST_CHECK(boost::archive::archive_exception::stream_error == e.code);
      exception_invoked = true;
    }
    BOOST_CHECK(exception_invoked);
  }
}

int test_main( int /* argc */, char* /* argv */[] )
{
   {
      boost::variant<bool, int, float, double, A, std::string> v;
      v = false;
      test_type(v);
      v = 1;
      test_type(v);
      v = (float) 2.3;
      test_type(v);
      v = (double) 6.4;
      test_type(v);
      v = std::string("we can't stop here, this is Bat Country");
      test_type(v);
      v = A();
      test_type(v);
   }
   do_bad_read();
   return EXIT_SUCCESS;
}

// EOF