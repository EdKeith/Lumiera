/*
  BUFFHANDLE.hpp  -  Buffer handling support for the render engine

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

*/

/** @file buffhandle.hpp
 ** Various bits needed to support the buffer management within the render nodes.
 ** When pulling data from predecessor nodes and calculating new data, each render node
 ** needs several input and output buffers. These may be allocated and provided by several
 ** different "buffer providers" (for example the frame cache). Typically, the real buffers
 ** will be passed as parameters to the actual job instance when scheduled, drawing on the
 ** results of prerequisite jobs. Yet the actual job implementation remains agnostic with
 ** respect to the way actual buffers are provided; the invocation just pushes BuffHandle
 ** objects around. The actual render function gets an array of C-pointers to the actual
 ** buffers, and for accessing those buffers, the node needs to keep a table of buffer
 ** pointers, and for releasing the buffers later on, we utilise the buffer handles.
 ** The usage pattern of those buffer pointer tables is stack-like, thus the actual
 ** implementation utilises a single large buffer pointer array per pull() call
 ** sequence and dynamically claims small chunks for each node.
 ** 
 ** @see nodewiring-def.hpp
 ** @see nodeoperation.hpp
 ** @see bufftable.hpp       storage for the buffer table
 ** @see engine::RenderInvocation
 */

#ifndef ENGINE_BUFFHANDLE_H
#define ENGINE_BUFFHANDLE_H


#include "lib/error.hpp"
#include "lib/streamtype.hpp"
#include "lib/bool-checkable.hpp"


namespace engine {
  
  typedef size_t HashVal;           ////////////TICKET #722
  
  class BuffHandle;
  class BufferProvider;
  
  
  
  /**
   * An opaque descriptor to identify the type and further properties of a data buffer.
   * For each kind of buffer, there is somewhere a BufferProvider responsible for the
   * actual storage management. This provider may "lock" a buffer for actual use,
   * returning a BuffHandle.
   * @note this descriptor and especially the #subClassification_ is really owned
   *       by the BufferProvider, which may use (and even change) the opaque contents
   *       to organise the internal buffer management.
   * 
   * @todo try to move that definition into buffer-provider.hpp   ////////////////////////////////////TICKET #249
   */
  class BufferDescriptor
    {
      BufferProvider* provider_;
      HashVal subClassification_;
      
      BufferDescriptor(BufferProvider& manager, HashVal detail)
        : provider_(&manager)
        , subClassification_(detail)
      { }
      
      friend class BufferProvider;
      
    public:
      // using standard copy operations
      
      bool verifyValidity()  const;
      
      void emit   (BuffHandle const&)  const;
      void release (BuffHandle const&)  const;
      
      operator HashVal()  const { return subClassification_; }
    };
  
  
  
  class ProcNode;
  typedef ProcNode* PNode;
  
  
  struct ChannelDescriptor  ///////TODO really need to define that here? it is needed for node wiring only
    {
      const lumiera::StreamType * bufferType;                /////////////////////////////////////////TICKET #828
    };
  
  struct InChanDescriptor : ChannelDescriptor
    {
      PNode dataSrc;    ///< the ProcNode to pull this input from
      uint srcChannel; ///<  output channel to use on the predecessor node
    };
  
  
  
  
  
  /**
   * Handle for a buffer for processing data, abstracting away the actual implementation.
   * The real buffer pointer can be retrieved by dereferencing this smart-handle class.
   * 
   * @todo as of 6/2011 it isn't clear how buffer handles are actually created
   *       and how the lifecycle (and memory) management works                  //////////////////////TICKET #249 rework BuffHandle creation and usage
   */
  class BuffHandle
    : public lib::BoolCheckable<BuffHandle>
    {
      typedef lumiera::StreamType::ImplFacade::DataBuffer Buff;
      
      BufferDescriptor descriptor_;
      Buff* pBuffer_; 
      
      
    public:
      typedef Buff* PBuff;
      
      /** @internal a buffer handle may be obtained by "locking"
       *  a buffer from the corresponding BufferProvider */
      BuffHandle(BufferDescriptor const& typeInfo, void* storage = 0)
        : descriptor_(typeInfo)
        , pBuffer_(static_cast<PBuff>(storage))
        { }
      
      // using standard copy operations
      
      
      
      void
      emit()
        {
          REQUIRE (pBuffer_);
          descriptor_.emit (*this);
        }
      
      void release()
        {
          REQUIRE (pBuffer_);
          descriptor_.release (*this);
        }
      
      
      template<typename BU>
      BU& create();
      
      template<typename BU>
      BU& accessAs();
      
      
      //////////////////////////////////////////TICKET #249 this operator looks obsolete. The Buff type is a placeholder type,
      //////////////////////////////////////////TODO         it should never be accessed directly from within Lumiera engine code
      Buff&
      operator* ()  const
        {
          ENSURE (pBuffer_);
          return *pBuffer_;
        }
      
      bool
      isValid()  const
        {
          return bool(pBuffer_)
              && descriptor_.verifyValidity();
        }
      
      HashVal
      entryID()  const
        {
          return descriptor_;
        }
      
      size_t
      size()  const
        {
          UNIMPLEMENTED ("forward to the buffer provider for storage size diagnostics");
        }
      
    };
  
  
  /* === Implementation details === */
  
  /** convenience shortcut: place and maintain an object within the buffer.
   *  This operation performs the necessary steps to attach an object;
   *  if the buffer isn't locked yet, it will do so. Moreover, the created
   *  object will be owned by the buffer management facilities, i.e. the
   *  destructor is registered as cleanup function.   
   */
  template<typename BU>
  BU&
  BuffHandle::create()
  {
    UNIMPLEMENTED ("convenience shortcut to attach/place an object in one sway");
  }
  
  
  /** convenience shortcut: access the buffer contents in a typesafe fashion.
   *  This is equivalent to a plain dereferentiation with additional metadata check
   * @throw error::Logic in case of type mismatch \c LUMIERA_ERROR_WRONG_TYPE
   */
  template<typename BU>
  BU&
  BuffHandle::accessAs()
  {
    UNIMPLEMENTED ("convenience shortcut to access buffer contents typesafe");
  }
  
  
  
} // namespace engine
#endif
