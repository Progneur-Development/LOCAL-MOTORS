//@<COPYRIGHT>@
//==================================================
//Copyright $2019.
//Siemens Product Lifecycle Management Software Inc.
//All Rights Reserved.
//==================================================
//@<COPYRIGHT>@

// 
//  @file
//  This file contains the declaration for the Business Object LM9_PartRevisionImpl
//

#ifndef LM9LOCALS__LM9_PARTREVISIONIMPL_HXX
#define LM9LOCALS__LM9_PARTREVISIONIMPL_HXX

#include <LM9_Local/LM9_PartRevisionGenImpl.hxx>
#include<tc/tc.h>
#include<stdio.h>
#include<conio.h>
#include<tccore/aom.h>
#include<tccore/aom_prop.h>
#include <LM9_Local/liblm9_local_exports.h>

#define PO_RECORDS_TABLE "lm9_PORecordsTable"
#define ACT_ORDERED_QTY "lm9_ActualOrderQty"
#define QUANTITY "lm9_Quantity"
#define TOTAL_QUANTITY_ORDERED "lm9_TotalQuantityOrdered"
#define TOTAL_QUANTITY_RECEIVED "lm9_TotalQuantityReceived"
namespace lm9locals
{
    class LM9_PartRevisionImpl; 
    class LM9_PartRevisionDelegate;
}

class  LM9_LOCAL_API lm9locals::LM9_PartRevisionImpl
    : public lm9locals::LM9_PartRevisionGenImpl
{
public:

    ///
    /// Getter for an Integer Property
    /// @param value - Parameter Value
    /// @param isNull - Returns true if the Parameter value is null
    /// @return - Status. 0 if successful
    ///
    int  getLm9_prop1Base( int &value, bool &isNull ) const;


protected:
    ///
    /// Constructor for a LM9_PartRevision
    explicit LM9_PartRevisionImpl( LM9_PartRevision& busObj );

    ///
    /// Destructor
    virtual ~LM9_PartRevisionImpl();

private:
    ///
    /// Default Constructor for the class
    LM9_PartRevisionImpl();
    
    ///
    /// Private default constructor. We do not want this class instantiated without the business object passed in.
    LM9_PartRevisionImpl( const LM9_PartRevisionImpl& );

    ///
    /// Copy constructor
    LM9_PartRevisionImpl& operator=( const LM9_PartRevisionImpl& );

    ///
    /// Method to initialize this Class
    static int initializeClass();

    ///
    ///static data
    friend class lm9locals::LM9_PartRevisionDelegate;

};

#include <LM9_Local/liblm9_local_undef.h>
#endif // LM9LOCALS__LM9_PARTREVISIONIMPL_HXX
