/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/**
* @file sai_stp_defs.h
*
* @brief This file contains the stp datastructures definitions and utility
*        functions
*
*************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_STP_DEFS_H__
#define __SAI_STP_DEFS_H__

#include "std_rbtree.h"

#include "saitypes.h"

/**
 * @brief Datastructure for STG instance
 */
typedef struct _dn_sai_stp_info_t {

    /** STP Instance Id */
    sai_object_id_t stp_inst_id;

    /** Vlans attached to this STG tree */
    rbtree_handle vlan_tree;
} dn_sai_stp_info_t;

#endif /* __SAI_STP_DEFS_H__ */
