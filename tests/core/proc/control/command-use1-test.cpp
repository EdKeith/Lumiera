/*
  CommandUse1(Test)  -  usage aspects I

  Copyright (C)         Lumiera.org
    2009,               Hermann Vosseler <Ichthyostega@web.de>

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


#include "lib/test/run.hpp"
#include "lib/test/test-helper.hpp"
#include "proc/control/command-invocation.hpp"
#include "proc/control/command-def.hpp"
#include "lib/format-util.hpp"
#include "lib/util.hpp"

#include "proc/control/test-dummy-commands.hpp"

#include <iostream>


namespace proc {
namespace control {
namespace test    {
  
  using util::isSameObject;
  using util::contains;
  using util::str;
  using std::cout;
  using std::endl;
  
  
  
  
  
  
  
  /***********************************************************************//**
   * @test command usage aspects I: defining commands in various ways,
   *       then re-accessing those definitions, create instances,
   *       invoke them and undo the effect. Clean up finally.
   * 
   * @see Command
   * @see command-basic-test.cpp (simple usage example)
   */
  class CommandUse1_test : public Test
    {
    
      int randVal;
      int random() { return randVal = 10 + (rand() % 40); }
      
      
      
      virtual void
      run (Arg) 
        {
          command1::check_ = 0;
          uint cnt_defs = Command::definition_count();
          uint cnt_inst = Command::instance_count();
          
          allInOneStep();
          standardUse();
          statePredicates();
          definePrototype();
          usePrototype();
          preventDuplicates();
          stringRepresentation();
          undef();
          
          CHECK (0 == command1::check_);
          CHECK (cnt_defs == Command::definition_count());
          CHECK (cnt_inst == Command::instance_count());
        }
      
      
      void
      allInOneStep()
        {
          
          CommandDef ("test.command1.1")
              .operation (command1::operate)
              .captureUndo (command1::capture)
              .undoOperation (command1::undoIt)
              .bind (random())
              .execSync()
              ;
          
          CHECK (randVal == command1::check_);
          
          Command::get("test.command1.1").undo();
          CHECK ( 0 == command1::check_);
        }
      
      
      void
      standardUse()
        {
          {
            CommandDef ("test.command1.2")
                 .operation (command1::operate)
                 .captureUndo (command1::capture)
                 .undoOperation (command1::undoIt)
                 ;
          }
          CHECK (CommandDef("test.command1.2"));
          
          Command com ("test.command1.2");
          CHECK (com);
          CHECK (com == Command::get("test.command1.2"));
          CHECK (contains (str(com), "test.command1.2"));
          CHECK (contains (str(com), "{def}"));
          CHECK (!com.canExec());
          VERIFY_ERROR (UNBOUND_ARGUMENTS, com() );
          CHECK ( 0 == command1::check_);
          
          VERIFY_ERROR (INVALID_ARGUMENTS, com.bind ("foo") );
          com.bind (random());              // note: run-time type check only
          CHECK ( com.canExec());
          CHECK (!com.canUndo());
          com();
          CHECK (randVal == command1::check_);
          com.undo();
          CHECK ( 0 == command1::check_);
          
          // the following shortcut does the same:
          invoke ("test.command1.2") (1234);
          CHECK ( 1234 == command1::check_);
          
          // another shortcut, with static type check:
//        invoke (command1::operate) (5678);                                  //////////////////TICKET #291  : unimplemented for now (9/09)
//        CHECK ( 1234+5678 == command1::check_);
          
          com.undo();
          CHECK ( 0 == command1::check_);
        }
      
      
      void
      statePredicates()
        {
          Command::remove("test.command1.2");
          VERIFY_ERROR (INVALID_COMMAND, Command::get("test.command1.2") );
          
          CommandDef def ("test.command1.2");
          CHECK (!def);
          
          def.operation (command1::operate)
             .captureUndo (command1::capture);
          CHECK (!def);                        // undo functor still missing
          VERIFY_ERROR (INVALID_COMMAND, Command::get("test.command1.2") );
          
          def.operation (command1::operate)
             .captureUndo (command1::capture)
             .undoOperation (command1::undoIt);
          CHECK (def);
          CHECK (CommandDef("test.command1.2"));
          CHECK (Command::get("test.command1.2"));
          
          CHECK ( Command::defined("test.command1.2"));
          CHECK (!Command::canExec("test.command1.2"));
          CHECK (!Command::canUndo("test.command1.2"));
          
          Command com = Command::get("test.command1.2");
          CHECK (com);
          CHECK (!com.canExec());
          CHECK (!com.canUndo());
          
          com.bind (11111);
          CHECK ( Command::defined("test.command1.2"));
          CHECK ( Command::canExec("test.command1.2"));
          CHECK (!Command::canUndo("test.command1.2"));
          
          com();
          CHECK ( Command::defined("test.command1.2"));
          CHECK ( Command::canExec("test.command1.2"));
          CHECK ( Command::canUndo("test.command1.2"));
          
          com.undo();
          CHECK ( Command::defined("test.command1.2"));
          CHECK ( Command::canExec("test.command1.2"));
          CHECK ( Command::canUndo("test.command1.2"));
        }
      
      
      void
      definePrototype()
        {
          CommandDef ("test.command1.3")
               .operation (command1::operate)
               .captureUndo (command1::capture)
               .undoOperation (command1::undoIt)
               .bind (random())
               ;
          
          CHECK (Command::get("test.command1.3").canExec());
        }
      
      
      void
      usePrototype()
        {
          Command c1 = Command::get("test.command1.3");
          CHECK (c1);
          CHECK (c1.canExec());
          CHECK (!c1.canUndo());
          
          Command c2 = c1.newInstance();
          CHECK (c2);
          CHECK (c2.canExec());
          CHECK (!c2.canUndo());
          
          CHECK (c1 == c2);
          CHECK (!isSameObject(c1, c2));
          
          CHECK (0 == command1::check_);
          
          c1();
          
          CHECK (randVal == command1::check_);
          CHECK ( c1.canUndo());
          CHECK (!c2.canUndo());
          CHECK (c1 != c2);
          
          c2();
          CHECK (randVal + randVal == command1::check_);
          CHECK (c2.canUndo());
          CHECK (c1 != c2);
          
          c1.undo();
          CHECK (0 == command1::check_);
          c2.undo();
          CHECK (randVal == command1::check_);
          
          c2.bind(23);
          c2();
          CHECK (randVal + 23 == command1::check_);
          
          // you should not use a command more than once (but it works...)
          c1();
          CHECK (randVal + 23 + randVal == command1::check_);
          c1.undo();
          CHECK (randVal + 23 == command1::check_);
          // note we've overwritten the previous undo state
          // and get the sate captured on the second invocation
          
          c2.undo();
          CHECK (randVal == command1::check_);
          c1.undo();
          CHECK (randVal + 23 == command1::check_);
          
          // use the current sate of c2 as Prototype for new command definition
          c2.storeDef ("test.command1.4");
          Command c4 = Command::get("test.command1.4");
          CHECK (c4);
          CHECK (c4.canExec());
          CHECK (c4.canUndo());
          CHECK (c4 == c2);
          CHECK (c4 != c1);
          c4();
          CHECK (c4 != c2); // now lives independently from the original
          CHECK (randVal + 2*23 == command1::check_);
          
          c4.bind(int(-command1::check_)); // new command argument binding
          c4();
          CHECK (0 == command1::check_);
          c2();
          CHECK (23 == command1::check_);
          c2.undo();
          CHECK (0 == command1::check_);
        }
      
      
      void
      preventDuplicates()
        {
          #define BUILD_NEW_COMMAND_DEF(_ID_) \
          CommandDef (_ID_)                    \
              .operation (command1::operate)    \
              .captureUndo (command1::capture)   \
              .undoOperation (command1::undoIt)
          
          CHECK (CommandDef ("test.command1.1"));
          VERIFY_ERROR (DUPLICATE_COMMAND, BUILD_NEW_COMMAND_DEF ("test.command1.1") );
          CHECK (CommandDef ("test.command1.2"));
          VERIFY_ERROR (DUPLICATE_COMMAND, BUILD_NEW_COMMAND_DEF ("test.command1.2") );
          CHECK (CommandDef ("test.command1.3"));
          VERIFY_ERROR (DUPLICATE_COMMAND, BUILD_NEW_COMMAND_DEF ("test.command1.3") );
          CHECK (CommandDef ("test.command1.4"));
          VERIFY_ERROR (DUPLICATE_COMMAND, BUILD_NEW_COMMAND_DEF ("test.command1.4") );
        }
      
      
      void
      stringRepresentation()
        {
          cout << string (Command::get("test.command1.1")) << endl;
          cout << string (Command::get("test.command1.2")) << endl;
          cout << string (Command::get("test.command1.3")) << endl;
          cout << string (Command::get("test.command1.4")) << endl;
          cout << string (Command()                      ) << endl;
          
          Command com
            = CommandDef ("test.command1.5")
                  .operation (command1::operate)
                  .captureUndo (command1::capture)
                  .undoOperation (command1::undoIt);
          
          cout << string (com) << endl;
          com.bind(123);
          cout << string (com) << endl;
          com();
          cout << string (com) << endl;
          com.undo();
          cout << string (com) << endl;
        }
      
      
      void
      undef()
        {
          CHECK (CommandDef ("test.command1.1"));
          CHECK (CommandDef ("test.command1.2"));
          CHECK (CommandDef ("test.command1.3"));
          CHECK (CommandDef ("test.command1.4"));
          
          CHECK (Command::get("test.command1.1"));
          CHECK (Command::get("test.command1.2"));
          CHECK (Command::get("test.command1.3"));
          CHECK (Command::get("test.command1.4"));
          
          VERIFY_ERROR (INVALID_COMMAND, Command::get("miracle"));
          VERIFY_ERROR (INVALID_COMMAND, invoke ("miracle") (1,2,3));
          
          CommandDef unbelievable ("miracle");
          CHECK (!unbelievable);
          
          Command miracle;
          // but because the miracle isn't yet defined, any use throws
          VERIFY_ERROR (INVALID_COMMAND, miracle.bind("abracadabra"));
          VERIFY_ERROR (INVALID_COMMAND, miracle.execSync());
          VERIFY_ERROR (INVALID_COMMAND, miracle.undo());
          VERIFY_ERROR (INVALID_COMMAND, miracle());
          CHECK (!miracle.canExec());
          CHECK (!miracle.canUndo());
          CHECK (!miracle);
          
          Command c5 (Command::get("test.command1.5"));
          
          CHECK (Command::remove("test.command1.1"));
          CHECK (Command::remove("test.command1.2"));
          CHECK (Command::remove("test.command1.3"));
          CHECK (Command::remove("test.command1.4"));
          CHECK (Command::remove("test.command1.5"));
          
          CHECK (!Command::remove("miracle")); // there is no such thing...
          
          VERIFY_ERROR (INVALID_COMMAND,   Command::get("test.command1.1"));
          VERIFY_ERROR (INVALID_COMMAND,   Command::get("test.command1.2"));
          VERIFY_ERROR (INVALID_COMMAND,   Command::get("test.command1.3"));
          VERIFY_ERROR (INVALID_COMMAND,   Command::get("test.command1.4"));
          VERIFY_ERROR (INVALID_COMMAND,   Command::get("test.command1.5"));
          VERIFY_ERROR (INVALID_COMMAND,   Command::get("miracle"));
          
          
          // note, removed the registered definitions,
          // but existing instances remain valid...
          // thus we're free to create new instances...
          CHECK (c5.isValid());
          CHECK (c5.canExec());
        }
    };
  
  
  
  /** Register this test class... */
  LAUNCHER (CommandUse1_test, "function controller");
  
  
}}} // namespace proc::control::test
