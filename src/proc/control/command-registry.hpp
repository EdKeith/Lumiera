/*
  COMMAND-REGISTRY.hpp  -  proc-Command object registration and storage management 
 
  Copyright (C)         Lumiera.org
    2009,               Hermann Vosseler <Ichthyostega@web.de>
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
*/


/** @file command-registry.hpp
 ** Managing command definitions and the storage of individual command objects.
 ** @todo WIP WIP.
 ** 
 ** \par Lifecycle
 ** 
 ** @see Command
 ** @see ProcDispatcher
 **
 */



#ifndef CONTROL_COMMAND_REGISTRY_H
#define CONTROL_COMMAND_REGISTRY_H

//#include "pre.hpp"
#include "lib/error.hpp"
#include "lib/singleton.hpp"
#include "lib/sync.hpp"
#include "lib/format.hpp"
#include "include/logging.h"
//#include "lib/bool-checkable.hpp"

#include "proc/control/command.hpp"
#include "proc/control/command-signature.hpp"
#include "proc/control/command-argument-holder.hpp"
//#include "proc/control/memento-tie.hpp"

#include <tr1/memory>
//#include <iostream>
#include <string>



namespace control {
  
  using std::tr1::shared_ptr;
//  using std::ostream;
  using std::string;
  
  
  
  
  /**
   * TODO type comment
   */
  class CommandRegistry
    : public lib::Sync<>
    {
      
    public:
      static lumiera::Singleton<CommandRegistry> instance;
      
      
      /** register a command (Frontend) under the given ID
       *  @return either the new command, or an already existing
       *          command registered under the given ID*/ 
      Command
      track (Symbol cmdID, Command const& commandHandle)
        {
          Lock sync(this);
          UNIMPLEMENTED ("place a commandHandle into the command index, or return the command already registered there");
        }
      
      
      /** query the command index by ID
       *  @return the registered command,
       *          or an "invalid" token */
      Command
      queryIndex (Symbol cmdID)
        {
          Lock sync(this);
          UNIMPLEMENTED ("retrieve the command registered under the given ID, maybe return an »empty« command");
          // if index.contains(cmdID)
          //   return index[cmdID]
          // else
          return Command();
        }
      
      
      /** search the command index for a definition
       *  @param cmdInstance using the definition to look up
       *  @return the ID used to register this definition 
       *          or \c NULL in case of an "anonymous" command */
      const char*
      findDefinition (Command const& cmdInstance)
        {
          UNIMPLEMENTED ("try to find a registration in the index for a given command instance");
        }
      
      
      size_t
      index_size()
        {
          UNIMPLEMENTED ("number of defs in the index");
        }
      
      
      size_t
      instance_count()
        {
          UNIMPLEMENTED ("number of active command impl instances");
        }
      
      
      /** set up a new command implementation frame
       *  @return pointer to a newly created CommandImpl, allocated 
       *          through the registry. The caller is responsible for
       *          deallocating this frame by calling #killCommandImpl 
       */
      template< typename SIG_OPER    ///< signature of the command operation
              , typename SIG_CAPT    ///< signature for capturing undo state
              , typename SIG_UNDO    ///< signature to undo the command
              >
      CommandImpl*
      newCommandImpl (function<SIG_OPER>& operFunctor
                     ,function<SIG_CAPT>& captFunctor
                     ,function<SIG_UNDO>& undoFunctor)
        {
          Lock sync(this);
          
          // derive the storage type necessary
          // to hold the command arguments and UNDO memento
          typedef typename UndoSignature<SIG_CAPT>::Memento Mem;
          typedef ArgumentHolder<SIG_OPER,Mem> Arguments;
          
          shared_ptr<Arguments> pArg ( new (allocateSlot<Arguments>()) Arguments()
                                     , &kill<Arguments>
                                     );
          void* implStorage = 0;
          try
            {
              implStorage = allocateSlot<CommandImpl>();
              ASSERT (implStorage);
              
              return new(implStorage) CommandImpl (pArg, operFunctor,captFunctor,undoFunctor);
            }
          catch(...)
            {
              if (implStorage)
                releaseSlot<CommandImpl> (implStorage);
              throw;
        }   }
      
      
      /** delete the command implementation and free the corresponding allocation */
      static void killCommandImpl (CommandImpl* entry) { kill(entry); }
      
      
      template<class IMP>
      static void
      kill (IMP* entry)
        {
                                  ///////////////////////////////////////////////TODO: clean behaviour while in App shutdown (Ticket #196)
          instance().destroyImpl(entry);
        }
      
      
    private:
      template<class IMP>
      void*
      allocateSlot ()
        {
          TODO ("redirect to the corresponding pool allocator");
        }
      
      template<class IMP>
      void
      releaseSlot (void* entry)
        {
          TODO ("redirect to the corresponding pool allocator");
        }
      
      
      template<class IMP>
      void
      destroyImpl (IMP* entry)
        {
          if (!entry) return;
          try
            {
              entry->~IMP();
            }
          catch(...)
            {
              WARN (command_dbg, "dtor of %s failed: %s", util::tyStr(entry).c_str()
                                                        , lumiera_error() );
            }
          releaseSlot<IMP> (entry);
        }
      
    };
  
  
} // namespace control
#endif
