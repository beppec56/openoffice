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

#include "SerfLockReqProcImpl.hxx"
#include "SerfTypes.hxx"
#include "DAVProperties.hxx"

#include "webdavresponseparser.hxx"
#include <serf/serf.h>
#include <rtl/ustrbuf.hxx>

namespace http_dav_ucp
{

SerfLockReqProcImpl::SerfLockReqProcImpl( const char* inSourcePath,
					  const DAVRequestHeaders& inRequestHeaders, // on debug the header look empty
                                          const SerfLock & inLock ,
					  const char* inOwner,
					  const char* inTimeout )
    : SerfRequestProcessorImpl( inSourcePath, inRequestHeaders )
    , mLock( inLock )
    , xInputStream( new SerfInputStream() )
{
    switch ( inLock.eDepth )
    {
        case DAVZERO:
            mDepthStr = "0";
            break;
        case DAVONE:
            mDepthStr = "1";
            break;
        case DAVINFINITY:
            mDepthStr = "infinity";
            break;
    }

    switch ( inLock.eScope )
    {
        case EXCLUSIVE:
	  mLockScope = "<lockscope><exclusive/></lockscope>";
            break;
        case SHARED:
            mLockScope = "<lockscope><shared/></lockscope>";
            break;
    }

    mOwner = inOwner;
    mTimeout = inTimeout;
}

SerfLockReqProcImpl::~SerfLockReqProcImpl()
{
}

#define LOCK_HEADER "<?xml version=\"1.0\" encoding=\"utf-8\"?><lockinfo xmlns=\"DAV:\">"
#define LOCK_TYPE "<locktype><write/></locktype>"
#define LOCK_TRAILER "</lockinfo>"

serf_bucket_t * SerfLockReqProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
  //prepare body of request:
    serf_bucket_alloc_t* pSerfBucketAlloc = serf_request_get_alloc( inSerfRequest );
    serf_bucket_t* body_bkt = 0;
    rtl::OString aBodyText;
    {
       rtl::OUStringBuffer aBuffer;
        aBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( LOCK_HEADER ));
        aBuffer.appendAscii( mLockScope );
        aBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( LOCK_TYPE ));
        aBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( "<owner><href>" ));
        aBuffer.appendAscii( mOwner );
        aBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( "</href></owner>" ));
	aBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( LOCK_TRAILER ));
        aBodyText = rtl::OUStringToOString( aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
        body_bkt = serf_bucket_simple_copy_create( aBodyText.getStr(),
                                                   aBodyText.getLength(),
                                                   pSerfBucketAlloc );
    }

    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest, 
                                                                 "LOCK",
                                                                 getPathStr(),
                                                                 body_bkt,
                                                                 pSerfBucketAlloc );
    handleChunkedEncoding(req_bkt, aBodyText.getLength());

    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    if (hdrs_bkt != NULL)
    {
        // general header fields provided by caller
        setRequestHeaders( hdrs_bkt );

        // request specific header fields
        serf_bucket_headers_set( hdrs_bkt, "Timeout", mTimeout );
        serf_bucket_headers_set( hdrs_bkt, "Depth", mDepthStr );
        if (hdrs_bkt!=NULL && body_bkt != 0 && aBodyText.getLength() > 0 )
        {
            serf_bucket_headers_set( hdrs_bkt, "Content-Type", "application/xml" );
        }
    }
    else
    {
        OSL_ASSERT("Headers Bucket missing");
    }

    return req_bkt;
}

void SerfLockReqProcImpl::processChunkOfResponseData( const char* data, 
                                                      apr_size_t len )
{
  fprintf( stdout, "==\n=====>>>>> SerfLockReqProcImpl::processChunkOfResponseData %s %lu \n", data, len);
    if ( xInputStream.is() )
    {
        xInputStream->AddToStream( data, len );
    }
}

void SerfLockReqProcImpl::handleEndOfResponseData( serf_bucket_t * /*inSerfResponseBucket*/ )
{
  // we can use the propfind parser, for resource value
  fprintf( stdout, "==\n=====>>>>> SerfLockReqProcImpl::handleEndOfResponseData  \n");
    const DAVPropertyValue rResources( parseWebDAVLockResponse( xInputStream.get() ) );
// estract from returned resources the lock data and updatade the lock
    OSL_TRACE("=--> SerfLockReqProcImpl::handleEndOfResponseData - received %s \n",
        OUStringToOString( rResources.Name , RTL_TEXTENCODING_ISO_8859_1 ).getStr());
}

} // namespace http_dav_ucp
