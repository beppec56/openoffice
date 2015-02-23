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

#include <rtl/ustring.hxx>

#include "SerfPutReqProcImpl.hxx"

#include <serf/serf.h>

namespace http_dav_ucp
{

SerfPutReqProcImpl::SerfPutReqProcImpl( const char* inPath,
                                        const DAVRequestHeaders& inRequestHeaders,
                                        const char* inData,
                                        const char* inLockToken,
                                        apr_size_t inDataLen )
    : SerfRequestProcessorImpl( inPath, inRequestHeaders )
    , mpData( inData )
    , mpLockToken( inLockToken)
    , mnDataLen( inDataLen )
{
}

SerfPutReqProcImpl::~SerfPutReqProcImpl()
{
}

serf_bucket_t * SerfPutReqProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
    serf_bucket_alloc_t* pSerfBucketAlloc = serf_request_get_alloc( inSerfRequest );

    // create body bucket
    serf_bucket_t* body_bkt = 0;
    if ( mpData != 0 && mnDataLen > 0 )
    {
        body_bkt = SERF_BUCKET_SIMPLE_STRING_LEN( mpData, mnDataLen, pSerfBucketAlloc );
    }

    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest, 
                                                                 "PUT",
                                                                 getPathStr(),
                                                                 body_bkt,
                                                                 serf_request_get_alloc( inSerfRequest ) );
    handleChunkedEncoding(req_bkt, mnDataLen);

    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    // general header fields provided by caller
    setRequestHeaders( hdrs_bkt );
    if(mpLockToken)
    {
    // request specific header field
        serf_bucket_headers_set( hdrs_bkt, "if", mpLockToken );
    }

    return req_bkt;
}

void SerfPutReqProcImpl::processChunkOfResponseData( const char* /*data*/, 
                                                     apr_size_t /*len*/ )
{
    // nothing to do;
}

void SerfPutReqProcImpl::handleEndOfResponseData( serf_bucket_t * /*inSerfResponseBucket*/ )
{
    // nothing to do;
}

} // namespace http_dav_ucp
