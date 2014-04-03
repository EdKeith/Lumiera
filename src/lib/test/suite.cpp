/*
  Suite  -  helper class for running collections of tests

  Copyright (C)         Lumiera.org
    2008,               Hermann Vosseler <Ichthyostega@web.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of
  the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

* *****************************************************/



#include "include/logging.h"
#include "lib/cmdline.hpp"
#include "lib/test/test-helper.hpp"
#include "lib/test/suite.hpp"
#include "lib/test/run.hpp"
#include "lib/error.hpp"
#include "lib/util.hpp"

#include <boost/algorithm/string.hpp>
#include <memory>
#include <iostream>
#include <sstream>
#include <memory>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>


namespace test {
  
  using std::map;
  using std::cout;
  using std::cerr;
  using std::endl;
  using std::vector;
  using std::auto_ptr;
  using std::shared_ptr;
  using boost::algorithm::trim;
  
  using util::isnil;
  using util::contains;
  using lib::test::showType;
  
  typedef map<string, Launcher*> TestMap;
  typedef shared_ptr<TestMap>  PTestMap;
  typedef map<string,PTestMap> GroupMap;
  
  
  
  /** helper to collect and manage the test cases.
   *  Every testcase class should create a Launch instance
   *  which causes a call to Suite::enrol(), so we can add a
   *  pointer to this Launcher into a map indexed by the
   *  provided testIDs and groupIDs.
   *  This enables us to build a Suite instance for any 
   *  requested group and then instantiate and invoke
   *  individual testcases accordingly.
   */ 
  class Registry
    {
      auto_ptr<GroupMap> groups;
    public:
      Registry() : groups(new GroupMap ) {};
      PTestMap& getGroup (string grpID) { return (*groups)[grpID]; }; 
      void add2group (Launcher* test, string testID, string groupID);
    };
    
  void 
  Registry::add2group (Launcher* test, string testID, string groupID)
  {
    REQUIRE( test );
    REQUIRE( !isnil(testID) );
    REQUIRE( !isnil(groupID) );
    
    PTestMap& group = getGroup(groupID);
    if (!group)
      group.reset( new TestMap );
    (*group)[testID] = test;
  }
  
  Registry testcases;
  
  
  
  
  /** register the given test-launcher, so it can be later accessed
   *  either as a member of one of the specified groups, or directly
   *  by its testID. Any test is automatically added to the groupID
   *  #ALLGROUP
   *  @param test the Launcher object used to run this test
   *  @param testID unique ID to refer to this test (will be used as std::map key)
   *  @param groups List of group-IDs selected by whitespace
   * 
   */
  void 
  Suite::enrol (Launcher* test, string testID, string groups)
  {
    REQUIRE( test );
    REQUIRE( !isnil(testID) );
    
    std::istringstream ss(groups);
    string group;
    while (ss >> group )
      testcases.add2group(test, testID, group);
    
    // Magic: always add any testcase to groupID="ALL"
    testcases.add2group(test,testID, ALLGROUP);
  }
  
  /** "magic" groupID containing all registered testcases */
  const string Suite::ALLGROUP = "ALL";
  
  /** exit code returned when any individual test threw */
  const int Suite::EXCEPTION_THROWN = 5;
  const int Suite::TEST_OK = 0;

  
  
  /** create a suite comprised of all the testcases 
   *  previously @link #enrol() registered @endlink with this
   *  this group. 
   *  @see #run() running tests in a Suite 
   */
  Suite::Suite(string groupID) 
    : groupID_(groupID)
    , exitCode_(0)
  {
    REQUIRE( !isnil(groupID) );
    TRACE(test, "Test-Suite( groupID=%s )\n", groupID.c_str () );
    
    // Seed random number generator
    std::srand (std::time (NULL));
    
    if (!testcases.getGroup(groupID))
      throw lumiera::error::Invalid ("empty testsuite");
  }
  
  
  int
  Suite::getExitCode ()  const
    {
      return exitCode_;
    }
  
  
    
#define VALID(test,testID) \
  ASSERT ((test), "NULL testcase launcher for test '%s' found in testsuite '%s'", groupID_.c_str(),testID.c_str());
  
  
  namespace { // internal helper for launching with error logging
    
    int
    invokeTestCase (Test& theTest, Arg cmdline)
    {
      try 
        {
          INFO (test, "++------------------- invoking TEST: %s", showType(theTest).c());
          theTest.run (cmdline);
          return Suite::TEST_OK;
        }
      catch (lumiera::Error& failure)
        {
          lumiera_err errorID = lumiera_error(); // reset error flag
          cerr << "*** Test Failure " << showType(theTest) << endl;
          cerr << "***            : " << failure.what() << endl;
          ERROR (test,     "Error state %s", errorID);
          WARN  (progress, "Caught exception %s", failure.what());
          return Suite::EXCEPTION_THROWN;
    }   }
  }
  
  
  /** run all testcases contained in this Suite.
   *  The first argument in the commandline, if present, 
   *  will select one single testcase with a matching ID.
   *  In case of invoking a single testcase, the given cmdline
   *  will be forwarded to the testcase, after removing the
   *  testcaseID from cmdline[0]. Otherwise, every testcase 
   *  in this suite is invoked with a empty cmdline vector.
   *  @param cmdline ref to the vector of commandline tokens  
   */
  void 
  Suite::run (Arg cmdline)
  {
    PTestMap tests = testcases.getGroup(groupID_);
    if (!tests)
      throw lumiera::error::Invalid ("test group not found"); ///////// TODO: pass error description
    
    if (0 < cmdline.size())
      {
        string& testID (cmdline[0]);
        trim(testID);
        if ( contains (*tests, testID))
          {
            // first cmdline argument denotes a valid testcase registered in 
            // this group: invoke just this test with the remaining cmdline
            Launcher* test = (*tests)[testID];
            cmdline.erase (cmdline.begin());
            VALID (test,testID);
            exitCode_ |= invokeTestCase (*(*test)(), cmdline);  // TODO confusing statement, improve definition of test collection datatype Ticket #289
            return;
          }
        else
          throw lumiera::error::Invalid ("unknown test : "+testID);
      }
    
    // no test-ID was specified.
    // Instantiate all tests cases and execute them.
    for ( TestMap::iterator i=tests->begin(); i!=tests->end(); ++i )
      {
        std::cout << "\n  ----------"<< i->first<< "----------\n";
        Launcher* test = (i->second);
        VALID (test, i->first);
        exitCode_ |= invokeTestCase (*(*test)(), cmdline); // actually no cmdline arguments
      }
  }
  
  
  /** print to stdout an enumeration of all testcases in this suite,
   *  in a format suitable for use with Cehteh's ./test.sh 
   */
  void
  Suite::describe ()
  {
    lib::Cmdline noCmdline("");
    PTestMap tests = testcases.getGroup(groupID_);
    ASSERT (tests);
    
    cout << "TESTING \"Component Test Suite: " << groupID_ << "\" ./test-components\n\n";
    
    for ( TestMap::iterator i=tests->begin(); i!=tests->end(); ++i )
      {
        string key (i->first);
        cout << "\n\n";
        cout << "TEST \""<<key<<"\" "<<key<<" <<END\n";
        Launcher* test = (i->second);
        VALID (test, i->first);
        try
          {
            (*test)()->run(noCmdline); // run it to insert test generated output
          }
        catch (...) 
          {
            cout << "PLANNED ============= " << lumiera_error() << "\n";
          }
        cout << "END\n";
      }
  }
  
  
  
} // namespace test
