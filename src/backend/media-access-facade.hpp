/*
  MEDIA-ACCESS-FACADE.hpp  -  functions for querying media file and channels.

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


#ifndef BACKEND_INTERFACE_MEDIA_ACCESS_FACADE_H
#define BACKEND_INTERFACE_MEDIA_ACCESS_FACADE_H


#include "lib/error.hpp"
#include "lib/singleton.hpp"
#include "lib/time/timevalue.hpp"

#include <string>



namespace backend {
  
  using lib::time::Duration;
  using std::string;
  
  struct MediaDesc;
  struct ChanDesc;
  
  
  /******************************************************************
   * Interface to the backend layer:
   * provides functions for querying (opening) a media file,
   * detecting the channels or streams found within this file etc. 
   * Implementation delegating to the actual backend functions.
   * 
   * convention: data passed by pointer is owned by the originator;
   * it should be copied if needed beyond the control flow 
   * of the invoked function. 
   */
  class MediaAccessFacade
    {
    public:
      typedef void* ChanHandle;
      
      static Singleton<MediaAccessFacade> instance;
      
      /** request for testing the denoted files accessibility 
       *  @param name path and filename of the media file.
       *  @throw error::Invalid when passing empty filename,
       *          or in case the media file is inaccessible 
       *          or otherwise inappropriate. 
       *  @return opaque handle usable for querying channel
       *          information from this file, NULL if the
       *          file is not accessible.
       */
      virtual MediaDesc& queryFile (string const& name)  const;
      
      /** request for information about the n-th channel 
       *  of the file referred by FileHandle.
       *  @return ChanDesc which may contain \c NULL values if
       *          the file doesn't contain this much channels.
       *  @todo   throw or return NULL-ChanDesc if Filehandle is invalid?
       */  
      virtual ChanDesc queryChannel (MediaDesc&, uint chanNo)  const;
      
      virtual ~MediaAccessFacade () {}
    };
  
  
  /** 
   * Descriptor holding the global informations,
   * needed for further handling this media within Lumiera.
   */
  struct MediaDesc
    {
      /** effectively usable duration.
       *  A clip created from this media will have this
       *  maximum duration. We expect to get media stream data
       *  from all channels within this limit. 
       */ 
      Duration length;
      
      MediaDesc() : length(Duration::NIL) { }
    };
  
  
  
  
  /** 
   * Description of one channel found in a
   * media file; result of querying the channel.
   */
  struct ChanDesc
    {
      /** identifier which can be used to create a name
       *  for the media asset corresponding to this channel.
       *  May be NULL or empty and need not be unique.
       */
      const char* chanID;
      
      /** identifier characterising the access method (or codec)
       *  needed to get at the media data. This should be rather
       *  a high level description of the media stream type, 
       *  e.g. "H264" -- anyhow, it will be used to find a 
       *  codec asset for this channel. 
       */
      const char* codecID;
      
      /** opaque handle, which will be used later to open this
       *  channel and retrieve some frames from it
       */
      MediaAccessFacade::ChanHandle handle;
      
      ChanDesc (const char* chanName=0, const char* codec=0, 
                MediaAccessFacade::ChanHandle h=0)
        : chanID(chanName),
          codecID(codec),
          handle(h)
        { }
    };
  
} // namespace backend


namespace backend_interface {
  
  using backend::MediaAccessFacade;
  using backend::MediaDesc;
  using backend::ChanDesc;
  
} // namespace backend_interface
#endif