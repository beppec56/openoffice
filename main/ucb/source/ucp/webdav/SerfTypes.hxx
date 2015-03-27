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



#ifndef INCLUDED_SERFTYPES_HXX
#define INCLUDED_SERFTYPES_HXX

#include <serf/serf.h>
#include <com/sun/star/ucb/Lock.hpp>
#include "DAVTypes.hxx"

using namespace com::sun::star;

namespace http_dav_ucp
{

typedef serf_connection_t SerfConnection;

// TODO, figure out type of <SerfLock>
class SerfLock
{
    rtl::OUString   sessionURI;
    ucb::Lock   aLock;
    SerfLock() {};
 };

// TODO, check if we need it later on
typedef struct { const char *nspace, *name; } SerfPropName;

};
#endif // INCLUDED_SERFTYPES_HXX
