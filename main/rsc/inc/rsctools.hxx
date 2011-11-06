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


struct RSHEADER_TYPE;
class  RscPtrPtr;

#ifndef _RSCTOOLS_HXX
#define _RSCTOOLS_HXX

#ifdef UNX
#include <stdlib.h>
#endif
#include <stdio.h>
#include <tools/string.hxx>
#include <tools/list.hxx>

/******************* T y p e s *******************************************/
// Zeichensatz
enum COMPARE { LESS = -1, EQUAL = 0, GREATER = 1 };

enum RSCBYTEORDER_TYPE { RSC_BIGENDIAN, RSC_LITTLEENDIAN, RSC_SYSTEMENDIAN };

/******************* M A K R O S *****************************************/
#define ALIGNED_SIZE( nSize )								\
			(nSize + sizeof( void * ) -1) / sizeof( void * ) * sizeof( void * )
/******************* F u n c t i o n   F o r w a r d s *******************/
ByteString GetTmpFileName();
sal_Bool Append( ByteString aDestFile, ByteString aSourceFile );
sal_Bool Append( FILE * fDest, ByteString aSourceFile );
ByteString InputFile ( const char * pInput, const char * pExt );
ByteString OutputFile( ByteString aInput, const char * ext );
char * ResponseFile( RscPtrPtr * ppCmd, char ** ppArgv,
					 sal_uInt32 nArgc );
void RscExit( sal_uInt32 nExit );

/********* A n s i - F u n c t i o n   F o r w a r d s *******************/
int rsc_strnicmp( const char *string1, const char *string2, size_t count );
int rsc_stricmp( const char *string1, const char *string2 );
char* rsc_strdup( const char* );

/****************** C L A S S E S ****************************************/
DECLARE_LIST( RscStrList, ByteString * )
/*********** R s c C h a r ***********************************************/
class RscChar
{
public:
	static char * MakeUTF8( char * pStr, sal_uInt16 nTextEncoding );
};

/****************** R s c P t r P t r ************************************/
class RscPtrPtr
{
	sal_uInt32	nCount;
	void ** 		pMem;
public:
					RscPtrPtr();
					~RscPtrPtr();
	void			Reset();
	sal_uInt32	Append( void * );
	sal_uInt32	Append( char * pStr ){
						return( Append( (void *)pStr ) );
					};
	sal_uInt32	GetCount(){ return( nCount ); };
	void *			GetEntry( sal_uInt32 nEle );
	void ** 		GetBlock(){ return( pMem ); };
};

/****************** R s c W r i t e R c **********************************/
class RscWriteRc
{
	sal_uInt32				nLen;
	sal_Bool				bSwap;
	RSCBYTEORDER_TYPE	nByteOrder;
	char *				pMem;
	char *				GetPointer( sal_uInt32 nSize );
public:
				RscWriteRc( RSCBYTEORDER_TYPE nOrder = RSC_SYSTEMENDIAN );
				~RscWriteRc();
	sal_uInt32		IncSize( sal_uInt32 nSize );// gibt die vorherige Groesse
	void *		GetBuffer()
				{
					return GetPointer( 0 );
				}
	sal_uInt16	GetShort( sal_uInt32 nPos )
				{
                    sal_uInt16 nVal = 0;
                    char* pFrom = GetPointer(nPos);
                    char* pTo = (char*)&nVal;
                    *pTo++ = *pFrom++;
                    *pTo++ = *pFrom++;
					return bSwap ? SWAPSHORT( nVal ) : nVal;
				}
	sal_uInt32	GetLong( sal_uInt32 nPos )
				{
                    sal_uInt32 nVal = 0;
                    char* pFrom = GetPointer(nPos);
                    char* pTo = (char*)&nVal;
                    *pTo++ = *pFrom++;
                    *pTo++ = *pFrom++;
                    *pTo++ = *pFrom++;
                    *pTo++ = *pFrom++;
					return bSwap ? SWAPLONG( nVal ) : nVal;
				}
	char *		GetUTF8( sal_uInt32 nPos )
				{
					return GetPointer( nPos );
				}


	RSCBYTEORDER_TYPE GetByteOrder() const { return nByteOrder; }
	sal_uInt32		Size(){ return( nLen ); };
    void        Put( sal_uInt64 lVal )
                {
                    union
                    {
                        sal_uInt64 lVal64;
                        sal_uInt32 aVal32[2];
                    };
                    lVal64 = lVal;
                    if( bSwap )
                    {
                        Put( aVal32[1] );
                        Put( aVal32[0] );
                    }
                    else
                    {
                        Put( aVal32[0] );
                        Put( aVal32[1] );
                    }
                }
	void		Put( sal_Int32 lVal )
				{
                    union
                    {
                        sal_uInt32 lVal32;
                        sal_uInt16 aVal16[2];
                    };
                    lVal32 = lVal;

					if( bSwap )
					{
						Put( aVal16[1] );
						Put( aVal16[0] );
					}
					else
					{
						Put( aVal16[0] );
						Put( aVal16[1] );
					}
				}
	void		Put( sal_uInt32 nValue )
				{ Put( (sal_Int32)nValue ); }
	void		Put( sal_uInt16 nValue );
	void		Put( sal_Int16 nValue )
				{ Put( (sal_uInt16)nValue ); }
	void		PutUTF8( char * pData );

	void		PutAt( sal_uInt32 nPos, sal_Int32 lVal )
				{
                    union
                    {
                        sal_uInt32 lVal32;
                        sal_uInt16 aVal16[2];
                    };
                    lVal32 = lVal;

					if( bSwap )
					{
						PutAt( nPos, aVal16[1] );
						PutAt( nPos + 2, aVal16[0] );
					}
					else
					{
						PutAt( nPos, aVal16[0] );
						PutAt( nPos + 2, aVal16[1] );
					}
				}
    void        PutAt( sal_uInt32 nPos, sal_uInt32 lVal )
                {
                    PutAt( nPos, (sal_Int32)lVal);
                }
	void		PutAt( sal_uInt32 nPos, short nVal )
				{
					PutAt( nPos, (sal_uInt16)nVal );
				}
	void		PutAt( sal_uInt32 nPos, sal_uInt16 nVal )
				{
                    if( bSwap )
                        nVal = SWAPSHORT( nVal );
                    char* pTo = GetPointer( nPos );
                    char* pFrom = (char*)&nVal;
                    *pTo++ = *pFrom++;
                    *pTo++ = *pFrom++;
				}
};

#endif // _RSCTOOLS_HXX
