/*
  JOB.hpp  -  render job closure

  Copyright (C)         Lumiera.org
    2012,               Hermann Vosseler <Ichthyostega@web.de>

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


#ifndef PROC_ENGINE_JOB_H
#define PROC_ENGINE_JOB_H


#include <gavl/gavl.h>


typedef void* LList; ////////////////////////////////////TODO
typedef uint64_t InvocationInstanceID;  /////////////////TODO


enum JobState
  { 
    DONE,      ///< already done, nothing to do
    RUNNING,   ///< job is currently running
    WAITING,   ///< waits for some prerequisite resource
    REJECTED,  ///< sorry, can't do that Dave
    EXPIRED,   ///< deadline expired
    ABORTED    ///< got aborted
  };


/** 
 * closure representing the execution context of a job.
 * The information reachable through this closure is specific
 * for this kind of job, but static and typically shared among
 * all jobs for a given feed and segment of the timeline
 */ 
struct lumiera_jobClosure { /* placeholder */ };
typedef struct lumiera_jobClosure* LumieraJobClosure;


/** 
 * invocation parameter for the individual
 * frame calculation job. Embedded into the job descriptor
 * and passed to #lumiera_job_invoke when triggering
 */
struct lumiera_jobParameter_struct
  {
    InvocationInstanceID invoKey;
    gavl_time_t nominalTime;
    //////////////////////////////////////////////////////////////TODO: place an additional parameter value here, or make the instanceID globally unique?
  };
typedef struct lumiera_jobParameter_struct lumiera_jobParameter;
typedef lumiera_jobParameter* LumieraJobParameter;


/**
 * descriptor record used by the scheduler to organise job invocation.
 * The invocation parameter and job closure necessary to invoke this
 * job as a function is embedded into this descriptor.
 */
struct lumiera_jobDescriptor_struct
  {
    gavl_time_t when;
    
    /* == Job prerequisites == */
    LList waiting;
    LList failed;
    LList completed;
  
    JobState jobstate;
  
    LumieraJobClosure jobClosure;
    lumiera_jobParameter parameter;
  };
typedef struct lumiera_jobDescriptor_struct lumiera_jobDescriptor;
typedef lumiera_jobDescriptor* LumieraJobDescriptor;





#ifdef __cplusplus  /* ============== C++ Interface ================= */

#include "proc/common.hpp"
//#include "proc/state.hpp"
#include "lib/time/timevalue.hpp"
//#include "lib/time/timequant.hpp"



namespace proc {
namespace engine {
  
//using lib::time::TimeSpan;
//using lib::time::Duration;
//using lib::time::FSecs;
using lib::time::Time;
//  
//class ExitNode;
  
  /**
   * Frame rendering task, represented as closure.
   * This functor encodes all information necessary to actually
   * trigger and invoke the rendering operation. It will be embedded
   * by reference into a job descriptor and then enqueued with the scheduler
   * for invocation just in time. The job interface exposes everything necessary
   * to plan, handle, schedule and abort jobs. The implementation refers to the
   * concrete "execution plan" encoded into the corresponding engine::JobTicket.
   * The latter is embedded into the storage for segment of the low-level model
   * and thus is shared for all frames and channels within this part of the
   * timeline. Thus, the lumiera_jobParameter struct contains the "moving parts"
   * changing for each individual job.
   * 
   * @todo 1/12 WIP-WIP-WIP defining the invocation sequence and render jobs
   */
  class Job
    : public lumiera_jobClosure
    {
      
    public:
      //////////////////////////////TODO: value semantics or turn this into an interface?
      
      Job()
        {
          UNIMPLEMENTED ("job representation, planning and scheduling");
        }
      
      // using standard copy operations
      
      
      void triggerJob (lumiera_jobParameter)  const;
      void signalFailure (lumiera_jobParameter)  const;
      
      
      Time
      getNominalTime()  const
        {
          UNIMPLEMENTED ("job representation, planning and scheduling");
        }
      
      InvocationInstanceID
      getInvocationInstanceID()  const
        {
          UNIMPLEMENTED ("job representation, planning and scheduling");
        }
      
      bool
      isValid()  const
        {
          UNIMPLEMENTED ("validity self check");
        }
    };
  
  
  
}} // namespace proc::engine




extern "C" {
#endif /* =========================== CL Interface ===================== */


/** trigger execution of a specific job,
 *  assuming availability of all prerequisites */
void lumiera_job_invoke  (LumieraJobClosure, lumiera_jobParameter);

/** signal inability to invoke this job
 * @todo decide what and how to communicate details of the failure
 * @remarks the purpose of this function is to allow for reliable checkpoints
 *          within the network of dependent jobs invocations, even after
 *          missing deadlines or aborting a sequence of jobs */
void lumiera_job_failure (LumieraJobClosure, lumiera_jobParameter);



#ifdef __cplusplus
}
#endif
#endif
