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

#include <precomp.h>
#include <idl/hi_display.hxx>


// NOT FULLY DEFINED SERVICES
#include <cosv/file.hxx>
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/getncast.hxx>
#include <toolkit/out_tree.hxx>
#include <cfrstd.hxx>
#include "hi_ary.hxx"
#include "hi_env.hxx"
#include "hi_main.hxx"


extern const String C_sCssFilename_Idl;


inline bool
HtmlDisplay_Idl::IsModule( const ary::idl::CodeEntity & i_ce ) const
{
 	return ary::is_type<ary::idl::Module>(i_ce);
}

inline const ary::idl::Module &
HtmlDisplay_Idl::Module_Cast( const ary::idl::CodeEntity & i_ce ) const
{
 	return ary::ary_cast<ary::idl::Module>(i_ce);
}




HtmlDisplay_Idl::HtmlDisplay_Idl()
    :   pCurPageEnv(),
        pMainDisplay()
{
}

HtmlDisplay_Idl::~HtmlDisplay_Idl()
{
}

void
HtmlDisplay_Idl::do_Run( const char *                       i_sOutputDirectory,
                         const ary::idl::Gate &             i_rAryGate,
                         const display::CorporateFrame &    i_rLayout )
{
    SetRunData( i_sOutputDirectory, i_rAryGate, i_rLayout );

    Create_StartFile();
    Create_CssFile();
    Create_FilesInNameTree();
    Create_IndexFiles();
    Create_FilesInProjectTree();
    Create_PackageList();
    Create_HelpFile();
}

void
HtmlDisplay_Idl::SetRunData( const char *                       i_sOutputDirectory,
                             const ary::idl::Gate &             i_rAryGate,
                             const display::CorporateFrame &    i_rLayout  )
{
    csv::ploc::Path aOutputDir( i_sOutputDirectory, true );
    pCurPageEnv = new HtmlEnvironment_Idl( aOutputDir, i_rAryGate, i_rLayout );
    pMainDisplay = new MainDisplay_Idl(*pCurPageEnv);
}

void
HtmlDisplay_Idl::Create_StartFile()
{
}

void
HtmlDisplay_Idl::Create_FilesInNameTree()
{
    Cout() << "\nCreate files in subtree namespaces ..." << Endl();

    const ary::idl::Module &
        rGlobalNamespace = pCurPageEnv->Data().GlobalNamespace();
    pCurPageEnv->Goto_Directory( pCurPageEnv->OutputTree().NamesRoot(), true );

    RecursiveDisplay_Module(rGlobalNamespace);

    Cout() << "... done." << Endl();
}

void
HtmlDisplay_Idl::Create_IndexFiles()
{
    Cout() << "\nCreate files in subtree index ..." << Endl();
    pCurPageEnv->Goto_Directory( pCurPageEnv->OutputTree().IndexRoot(), true );
    pMainDisplay->WriteGlobalIndices();
    Cout() << "... done.\n" << Endl();
}

typedef ary::Dyn_StdConstIterator<ary::idl::Ce_id>  Dyn_CeIterator;
typedef ary::StdConstIterator<ary::idl::Ce_id>      CeIterator;

void
HtmlDisplay_Idl::RecursiveDisplay_Module( const ary::idl::Module & i_module )
{
    i_module.Accept(*pMainDisplay);

    Dyn_CeIterator
        aMembers;
    i_module.Get_Names(aMembers);

    for ( CeIterator & iter = *aMembers;
          iter;
          ++iter )
    {
        const ary::idl::CodeEntity &
            rCe = pCurPageEnv->Data().Find_Ce(*iter);

        if ( NOT IsModule(rCe) )
            rCe.Accept(*pMainDisplay);
        else
        {
            pCurPageEnv->Goto_DirectoryLevelDown( rCe.LocalName(), true );
            RecursiveDisplay_Module( Module_Cast(rCe) );
            pCurPageEnv->Goto_DirectoryLevelUp();
        }
    }   // end for
}

void
HtmlDisplay_Idl::Create_FilesInProjectTree()
{
}

void
HtmlDisplay_Idl::Create_PackageList()
{
#if 0
    Cout() << "\nCreate package list ..." << std::flush;

    pCurPageEnv->CurPosition() = pCurPageEnv->OutputTree().Root();

    // KORR
    // ...

    Cout() << " done." << Endl();
#endif // 0
}

void
HtmlDisplay_Idl::Create_HelpFile()
{
}

void
HtmlDisplay_Idl::Create_CssFile()
{
    Cout() << "\nCreate css file ..." << Endl();

    pCurPageEnv->Goto_Directory( pCurPageEnv->OutputTree().Root(), true );
    pCurPageEnv->Set_CurFile( C_sCssFilename_Idl );

    StreamLock
        slCurFilePath(700);
    pCurPageEnv->Get_CurFilePath(slCurFilePath());

    csv::File
        aCssFile(slCurFilePath().c_str(), csv::CFM_CREATE);
    csv::OpenCloseGuard
        aOpenGuard(aCssFile);
    if (NOT aOpenGuard)
    {
        Cerr() << "Can't create file " << slCurFilePath().c_str() << "." << Endl();
        return;
    }

    aCssFile.write("/*      Autodoc css file for IDL documentation      */\n\n\n");
    aCssFile.write(pCurPageEnv->Layout().CssStyle());
    aCssFile.write("\n\n\n");
    aCssFile.write(pCurPageEnv->Layout().CssStylesExplanation());
}
