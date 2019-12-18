//@<COPYRIGHT>@
//==================================================
//Copyright $2019.
//Siemens Product Lifecycle Management Software Inc.
//All Rights Reserved.
//==================================================
//@<COPYRIGHT>@

// 
//  @file
//  This file contains the implementation for the Business Object LM9_PartRevisionImpl
//

#include <LM9_Local/LM9_PartRevisionImpl.hxx>

#include <fclasses/tc_string.h>
#include <tc/tc.h>

using namespace lm9locals;

//----------------------------------------------------------------------------------
// LM9_PartRevisionImpl::LM9_PartRevisionImpl(LM9_PartRevision& busObj)
// Constructor for the class
//----------------------------------------------------------------------------------
LM9_PartRevisionImpl::LM9_PartRevisionImpl( LM9_PartRevision& busObj )
   : LM9_PartRevisionGenImpl( busObj )
{
}

//----------------------------------------------------------------------------------
// LM9_PartRevisionImpl::~LM9_PartRevisionImpl()
// Destructor for the class
//----------------------------------------------------------------------------------
LM9_PartRevisionImpl::~LM9_PartRevisionImpl()
{
}

//----------------------------------------------------------------------------------
// LM9_PartRevisionImpl::initializeClass
// This method is used to initialize this Class
//----------------------------------------------------------------------------------
int LM9_PartRevisionImpl::initializeClass()
{
    int ifail = ITK_ok;
    static bool initialized = false;

    if( !initialized )
    {
        ifail = LM9_PartRevisionGenImpl::initializeClass( );
        if ( ifail == ITK_ok )
        {
            initialized = true;
        }
    }
    return ifail;
}


///
/// Getter for an Integer Property
/// @param value - Parameter Value
/// @param isNull - Returns true if the Parameter value is null
/// @return - Status. 0 if successful
///
int  LM9_PartRevisionImpl::getLm9_prop1Base( int & value, bool & isNull ) const
{
	    int ifail = ITK_ok;
	    int n_rows = 0;
	    int actOrderedQty = 0;
	    tag_t obj = NULLTAG;
	    tag_t* newtable_rows = NULL;
	    isNull = false;
	    tag_t attr_tag = NULLTAG;
	    obj = getLM9_PartRevision()->getTag();
	    printf("inside function");


	    value = 0 ;
	    ifail = AOM_ask_table_rows(obj,PO_RECORDS_TABLE,&n_rows,&newtable_rows);
	    	if(n_rows>0)
	    	{
	    		for(int i=0;i<n_rows;i++)
	    		{
	    			ifail = AOM_ask_value_int(newtable_rows[i],ACT_ORDERED_QTY,&actOrderedQty);
	    			if(actOrderedQty!=NULL)
	    			{
	    				value=value+actOrderedQty;
	    			}
	    		}
	    	}

	    AOM_ask_value_tag(obj,"lm9_prop1",&attr_tag);
	     printf(" value==%d",value);
	    AttributeAccessor::setIntegerValue(obj,attr_tag,value,isNull);
	    return ifail;

}

