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
* @file sai_lag_debug.c
*
* @brief This file contains debug APIs for SAI LAG module
*************************************************************************/

#include "saistatus.h"
#include "saitypes.h"
#include "std_llist.h"
#include "sai_lag_common.h"
#include "sai_lag_api.h"
#include "sai_debug_utils.h"
#include "sai_oid_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

sai_status_t sai_dump_lag_port_list(sai_object_id_t lag_id)
{
    sai_lag_node_t *lag_node = sai_lag_node_get(lag_id);
    sai_lag_port_node_t *lag_port_node = NULL;
    std_dll *node = NULL;

    if(lag_node == NULL) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    SAI_DEBUG("Lag ID:0x%"PRIx64" NPU ID:%d  port count:%d\r\n",lag_node->sai_lag_id,
            (int)sai_uoid_npu_obj_id_get(lag_node->sai_lag_id),
            lag_node->port_count);
    SAI_DEBUG("port list:");
    for(node = std_dll_getfirst(&(lag_node->port_list));
        node != NULL;
        node = std_dll_getnext(&(lag_node->port_list),node)) {
        lag_port_node = (sai_lag_port_node_t *)node;
        SAI_DEBUG("0x%"PRIx64"(%d) ",lag_port_node->port_id,
              (int)sai_uoid_npu_obj_id_get(lag_port_node->port_id));
    }
    SAI_DEBUG("\r\n");

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_dump_all_lags(void)
{
    sai_lag_node_t *lag_node = NULL;
    std_dll *node = NULL;
    std_dll_head *lag_list = sai_lag_list_get();

    for(node = std_dll_getfirst(lag_list);
        node != NULL;
        node = std_dll_getnext(lag_list,node)) {
        lag_node = (sai_lag_node_t *)node;
        sai_dump_lag_port_list(lag_node->sai_lag_id);
    }
    return SAI_STATUS_SUCCESS;
}
