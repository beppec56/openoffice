/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include "SerfMoveReqProcImpl.hxx"

#include <serf/serf.h>

namespace http_dav_ucp
{

SerfMoveReqProcImpl::SerfMoveReqProcImpl( const char* inSourcePath,
                                          const DAVRequestHeaders& inRequestHeaders,
                                          const char* inDestinationPath,
                                          const bool inOverwrite,
                                          const char* inLockToken)
    : SerfRequestProcessorImpl( inSourcePath, inRequestHeaders )
    , mDestPathStr( inDestinationPath )
    , mbOverwrite( inOverwrite )
    , mpLockToken( inLockToken )
{
}

SerfMoveReqProcImpl::~SerfMoveReqProcImpl()
{
}

serf_bucket_t * SerfMoveReqProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest, 
                                                                 "MOVE",
                                                                 getPathStr(),
                                                                 0,
                                                                 serf_request_get_alloc( inSerfRequest ) );

    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    // general header fields provided by caller
    setRequestHeaders( hdrs_bkt );
    
    // MOVE specific header fields
    serf_bucket_headers_set( hdrs_bkt, "Destination", mDestPathStr );
    if ( mbOverwrite )
    {
        serf_bucket_headers_set( hdrs_bkt, "Overwrite", "T" );
    }
    else
    {
        serf_bucket_headers_set( hdrs_bkt, "Overwrite", "F" );
    }
    if(mpLockToken)
    {
        serf_bucket_headers_set( hdrs_bkt, "if", mpLockToken );
    }

    return req_bkt;
}

void SerfMoveReqProcImpl::processChunkOfResponseData( const char* /*data*/, 
                                                      apr_size_t /*len*/ )
{
    // nothing to do;
}

void SerfMoveReqProcImpl::handleEndOfResponseData( serf_bucket_t * /*inSerfResponseBucket*/ )
{
    // nothing to do;
}

} // namespace http_dav_ucp
