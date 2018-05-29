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
* @file sai_vlan_debug.c
*
* @brief This file contains debug APIs for SAI VLAN module
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "saivlan.h"
#include "saistatus.h"
#include "saitypes.h"
#include "sai_oid_utils.h"
#include "sai_vlan_common.h"
#include "sai_vlan_api.h"
#include "sai_debug_utils.h"

sai_status_t sai_dump_vlan(sai_vlan_id_t vlan_id)
{
    sai_vlan_port_node_t *vlan_port_node = NULL;
    std_dll *node = NULL;
    sai_vlan_global_cache_node_t *vlan_port_list = sai_vlan_portlist_cache_read(vlan_id);

    if(vlan_port_list == NULL) {
        SAI_DEBUG("Unable to get port list for vlan:%d",vlan_id);
        return SAI_STATUS_FAILURE;
    }

    for(node = std_dll_getfirst(&(vlan_port_list->port_list));
        node != NULL;
        node = std_dll_getnext(&(vlan_port_list->port_list),node)) {
        vlan_port_node = (sai_vlan_port_node_t *)node;
        SAI_DEBUG("port:%d tagging mode:%d",
               (int)sai_uoid_npu_obj_id_get(vlan_port_node->vlan_port.port_id),
               vlan_port_node->vlan_port.tagging_mode);
    }
    return SAI_STATUS_SUCCESS;
}
void sai_dump_all_vlans(void )
{
    sai_vlan_id_t vlan_id;

    for(vlan_id = SAI_MIN_VLAN_TAG_ID; vlan_id < SAI_MAX_VLAN_TAG_ID; vlan_id++) {
        if(sai_is_vlan_created(vlan_id)) {
            SAI_DEBUG("Dumping vlan:%d",vlan_id);
            sai_dump_vlan(vlan_id);
        }
    }
    SAI_DEBUG("* - Default VLAN Id");
}
