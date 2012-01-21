/*
  ENGINE-SERVICE.hpp  -  primary service access point for using the renderengine

  Copyright (C)         Lumiera.org
    2011,               Hermann Vosseler <Ichthyostega@web.de>

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

*/

/** @file engine-service.hpp
 ** Access point for the (core) calculation service of the render engine.
 ** This Proc-Layer internal service is provided for use by the Player subsystem.
 ** The actual implementation is forwarded to backend services (especially the scheduler).
 ** The EngineService singleton has no state beyond the jobs currently managed by the
 ** scheduler; when the latter isn't available, any invocation will throw.
 ** 
 ** The central concept provided through this facade interface is the <i>calculation stream</i>.
 ** This represents a series of calculations, expected to happen in a timely fashion and in order
 ** to deliver a frame data stream onto an opened output connection. On the implementation side,
 ** a calculation stream will be translated into a series of jobs invoking render nodes,
 ** to be executed through the scheduler in the backend layer.
 ** 
 ** While the individual CalcStram is simple, linear and unmodifiable, any CalcStream may be
 ** \em superseded by a new definition. In this case, the engine will care for a seamless
 ** switch and continuation; under the hood, there is a mechanism to discard resources
 ** tied to the original CalcStream, once the switch to the new definition is complete.
 ** 
 ** @see EngineInterface_test
 ** @see CalcStream_test
 ** @see proc::play::PlayerService
 */


#ifndef PROC_ENGINE_ENGINE_SERVICE_H
#define PROC_ENGINE_ENGINE_SERVICE_H


#include "lib/error.hpp"
//#include "include/dummy-player-facade.h"
//#include "include/display-facade.h"
#include "proc/engine/calc-stream.hpp"
#include "proc/mobject/model-port.hpp"
#include "proc/play/timings.hpp"
#include "proc/play/output-slot.hpp"
//#include "common/instancehandle.hpp"
//#include "lib/singleton-ref.hpp"
#include "lib/polymorphic-value.hpp"
#include "lib/singleton.hpp"
//
#include <boost/noncopyable.hpp>
//#include <boost/scoped_ptr.hpp>
//#include <string>


namespace proc  {
namespace engine{

//    using std::string;
//    using lumiera::Subsys;
//    using lumiera::Display;
//    using lumiera::DummyPlayer;
  using mobject::ModelPort;
  using proc::play::Timings;
  
  typedef proc::play::OutputSlot::Allocation OutputConnection;
  
  
  
  
  
  /******************************************************
   * A service to schedule series of calculations,
   * delivering the rendered data into an external output
   * sink in a timely fashion. Actually the CalculationStream
   * instances provided through this (facade) interface are
   * backed by jobs executed through the scheduler in the
   * backend layer. The implementation of this service
   * cares to create the right job entries in the correct
   * order and to enqueue these into the scheduler.
   */
  class EngineService
    : boost::noncopyable
    {
      
      
      /* The following typedefs allow to hand out predefined
       * Quality-of-Service strategy definitions as value objects,
       * without disclosing implementation details here in this header.
       */
      enum{ QoS_IMPL_SIZE = sizeof(size_t) };  /////////////////////////////////////////TODO is this correct??
      
      
    public:
      /*************************************************************
       * Quality-of-Service definition for an Render Engine usage.
       * This strategy defines how to decide between conflicting goals like
       * - timely delivery
       * - image quality
       * - niceness and resource usage
       */
      class Quality
        {
          public:
            virtual ~Quality();  ///< this is an Interface
        };
      
      
      typedef lib::polyvalue::CloneValueSupport<Quality> _Clonable_QoS_Strategy;
      typedef lib::PolymorphicValue<Quality, QoS_IMPL_SIZE, _Clonable_QoS_Strategy> QoS_Definition;
      
      static QoS_Definition  QoS_DEFAULT;
      static QoS_Definition  QoS_BACKGROUND;
      static QoS_Definition  QoS_SYNC_PRIORITY;
      static QoS_Definition  QoS_PERFECT_RESULT;
      static QoS_Definition  QoS_COMPROMISE;
      
      
      /** access point to the Engine Interface.
       * @internal this is an facade interface for internal use
       *           by the player. Client code should use the Player.
       */
      static lib::Singleton<EngineService> instance;
      
      
      EngineService();
     ~EngineService() { }
      
      CalcStreams
      calculate(ModelPort mPort,
                Timings nominalTimings,
                OutputConnection& output,
                Quality serviceQuality =QoS_DEFAULT);
      
      CalcStreams
      calculateBackground(ModelPort mPort,
                          Timings nominalTimings,
                          Quality serviceQuality =QoS_BACKGROUND);
      
      
    private:
      CalcStream activateCalculation (RenderEnvironmentClosure&);
      
    protected:
      virtual CalcStream configureCalculation ();
      
      void activateTracing();
      void disableTracing(); ///< EX_FREE
      
      friend class EngineDiagnostics;
    };
  
  
  
  
  
  
} // namespace engine
} // namespace proc
#endif
