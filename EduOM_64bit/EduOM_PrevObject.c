/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/EduCOSMOS Educational-Purpose Object Storage System            */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Database and Multimedia Laboratory                                      */
/*                                                                            */
/*    Computer Science Department and                                         */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: kywhang@cs.kaist.ac.kr                                          */
/*    phone: +82-42-350-7722                                                  */
/*    fax: +82-42-350-8380                                                    */
/*                                                                            */
/*    Copyright (c) 1995-2013 by Kyu-Young Whang                              */
/*                                                                            */
/*    All rights reserved. No part of this software may be reproduced,        */
/*    stored in a retrieval system, or transmitted, in any form or by any     */
/*    means, electronic, mechanical, photocopying, recording, or otherwise,   */
/*    without prior written permission of the copyright owner.                */
/*                                                                            */
/******************************************************************************/
/*
 * Module: EduOM_PrevObject.c
 *
 * Description: 
 *  Return the previous object of the given current object.
 *
 * Exports:
 *  Four EduOM_PrevObject(ObjectID*, ObjectID*, ObjectID*, ObjectHdr*)
 */


#include "EduOM_common.h"
#include "BfM.h"
#include "EduOM_Internal.h"

/*@================================
 * EduOM_PrevObject()
 *================================*/
/*
 * Function: Four EduOM_PrevObject(ObjectID*, ObjectID*, ObjectID*, ObjectHdr*)
 *
 * Description: 
 * (Following description is for original ODYSSEUS/COSMOS OM.
 *  For ODYSSEUS/EduCOSMOS EduOM, refer to the EduOM project manual.)
 *
 *  Return the previous object of the given current object. Find the object in
 *  the same page which has the current object and  if there  is no previous
 *  object in the same page, find it from the previous page.
 *  If the current object is NULL, return the last object of the file.
 *
 * Returns:
 *  error code
 *    eBADCATALOGOBJECT_OM
 *    eBADOBJECTID_OM
 *    some errors caused by function calls
 *
 * Side effect:
 *  1) parameter prevOID
 *     prevOID is filled with the previous object's identifier
 *  2) parameter objHdr
 *     objHdr is filled with the previous object's header
 */
Four EduOM_PrevObject(
    ObjectID *catObjForFile,	/* IN informations about a data file */
    ObjectID *curOID,		/* IN a ObjectID of the current object */
    ObjectID *prevOID,		/* OUT the previous object of a current object */
    ObjectHdr*objHdr)		/* OUT the object header of previous object */
{
    Four e;			/* error */
    Two  i;			/* index */
    Four offset;		/* starting offset of object within a page */
    PageID pid;			/* a page identifier */
    PageNo pageNo;		/* a temporary var for previous page's PageNo */
    SlottedPage *apage;		/* a pointer to the data page */
    Object *obj;		/* a pointer to the Object */
    SlottedPage *catPage;	/* buffer page containing the catalog object */
    sm_CatOverlayForData *catEntry; /* overlay structure for catalog object access */
    PhysicalFileID pFid;


    /*@ parameter checking */
    if (catObjForFile == NULL) ERR(eBADCATALOGOBJECT_OM);
    
    if (prevOID == NULL) ERR(eBADOBJECTID_OM);

    // Read
    e = BfM_GetTrain((TrainID*)catObjForFile, (char**)&catPage, PAGE_BUF);
    if (e < 0) ERR(e);
    GET_PTR_TO_CATENTRY_FOR_DATA(catObjForFile, catPage, catEntry);
    MAKE_PHYSICALFILEID(pFid, catEntry->fid.volNo, catEntry->lastPage);

    // curOID is NULL, return the last object in the slot array 
    if (curOID == NULL) {
        pageNo = catEntry->lastPage;
        if (pageNo != NULL) {
            MAKE_PAGEID(pid, pFid.volNo, pageNo);
            e = BfM_GetTrain((TrainID*)&pid, (char**)&apage, PAGE_BUF);
            if(e < 0) ERR(e);

            prevOID->pageNo = pageNo;
            prevOID->volNo = pid.volNo;
            prevOID->slotNo = apage->header.nSlots - 1;
            prevOID->unique = apage->slot[-prevOID->slotNo].unique;
        }
    }
    else { // curOID is not NULL, find the the corresponding object and return the previous one.
        MAKE_PAGEID(pid, curOID->volNo, curOID->pageNo);
        e = BfM_GetTrain((TrainID*)&pid, (char**)&apage, PAGE_BUF);
        if (e < 0) ERR(e);

        if (curOID->slotNo == 0) {
            if (apage->header.pid.pageNo != catEntry->firstPage) {
                pageNo = apage->header.prevPage;
                e = BfM_FreeTrain((TrainID*)&pid, PAGE_BUF);
                if (e < 0) ERR(e);
                pid.pageNo = pageNo;
                e = BfM_GetTrain((TrainID*)&pid, (char**)&apage, PAGE_BUF);
                if (e < 0) ERR(e);
                prevOID->pageNo = pageNo;
                prevOID->slotNo = apage->header.nSlots - 1;
                prevOID->volNo = curOID->volNo;
                prevOID->unique = apage->slot[-prevOID->slotNo].unique;
            }
        }
        else {
            prevOID->pageNo = curOID->pageNo;
            prevOID->volNo = curOID->volNo;
            prevOID->slotNo = curOID->slotNo - 1;
            prevOID->unique = apage->slot[-prevOID->slotNo].unique;
        }
    }

    e = BfM_FreeTrain((TrainID*)&pid, PAGE_BUF);
    if(e < 0) ERR(e);

    e = BfM_FreeTrain((TrainID*)catObjForFile, PAGE_BUF);
    if(e < 0) ERR(e);

    return(EOS);
    
} /* EduOM_PrevObject() */
