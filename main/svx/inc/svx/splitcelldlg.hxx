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


#ifndef _SVX_SPLITCELLDLG_HXX
#define _SVX_SPLITCELLDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/button.hxx>

#include <svx/stddlg.hxx>

class SvxSplitTableDlg : public SvxStandardDialog
{
	FixedText 			maCountLbl;
	NumericField 		maCountEdit;
    FixedLine           maCountFL;
	ImageRadioButton 	maHorzBox;
    ImageRadioButton    maVertBox;
    CheckBox            maPropCB;
    FixedLine           maDirFL;
	OKButton 			maOKBtn;
	CancelButton 		maCancelBtn;
	HelpButton 			maHelpBtn;

protected:
	virtual void Apply();

public:
	SvxSplitTableDlg(Window *pParent );
    DECL_LINK( ClickHdl, Button * );

    bool IsHorizontal() const;
    bool IsProportional() const;
    long GetCount() const;
};

#endif
