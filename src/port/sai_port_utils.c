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
* @file sai_port_utils.c
*
* @brief This file contains SAI Common Port Utility API's.
*        Common Port Utility API's can be used by other SAI components.
*
*************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "saiswitch.h"
#include "saitypes.h"
#include "saiport.h"
#include "saistatus.h"

#include "sai_port_common.h"
#include "sai_port_utils.h"
#include "sai_switch_utils.h"

#include "std_rbtree.h"
#include "std_assert.h"
#include "std_mutex_lock.h"
#include "std_struct_utils.h"

static std_mutex_lock_create_static_init_fast(port_lock);

void sai_port_lock(void)
{
    std_mutex_lock(&port_lock);
}

void sai_port_unlock(void)
{
    std_mutex_unlock(&port_lock);
}

/* Allocate memory for switch info table */
sai_status_t sai_port_info_init(void)
{
    sai_switch_info_t *switch_info = sai_switch_info_get();

    SAI_PORT_LOG_TRACE("Port info table initialization");

    switch_info->port_info_table =
        std_rbtree_create_simple("SAI Port info tree",
                                 STD_STR_OFFSET_OF(sai_port_info_t, sai_port_id),
                                 STD_STR_SIZE_OF(sai_port_info_t, sai_port_id));

    /* @todo replace STD_ASSERT with RELEASE_ASSERT */
    STD_ASSERT(!(switch_info->port_info_table == NULL));

    switch_info->port_applications_tree =
        std_rbtree_create_simple ("applications_tree_per_port",
                                  STD_STR_OFFSET_OF(sai_port_application_info_t, port_id),
                                  STD_STR_SIZE_OF(sai_port_application_info_t, port_id));

    /* @todo replace STD_ASSERT with RELEASE_ASSERT */
    STD_ASSERT(!(switch_info->port_applications_tree == NULL));

    return SAI_STATUS_SUCCESS;
}

/* Assumption is port info table will be allocated during Init,
 * and it can be accessed without failure checks
 */
sai_port_info_table_t sai_port_info_table_get(void)
{
    sai_switch_info_t *sai_switch_info_ptr = sai_switch_info_get();
    return sai_switch_info_ptr->port_info_table;
}

sai_port_info_t *sai_port_info_get(sai_object_id_t port)
{
    sai_port_info_table_t port_info_table = sai_port_info_table_get();
    sai_port_info_t port_info_t;

    memset(&port_info_t, 0, sizeof(sai_port_info_t));
    port_info_t.sai_port_id = port;

    /* Port validation check is expected to be done before calling this */
    return ((sai_port_info_t *) std_rbtree_getexact(port_info_table, &port_info_t));
}

static inline bool sai_is_logical_port_valid(sai_object_id_t port)
{
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(port_info_table == NULL) {
        return false;
    }

    return (port_info_table->port_valid);
}

bool sai_is_port_valid(sai_object_id_t port)
{
    if(!(sai_is_obj_id_cpu_port(port) || sai_is_obj_id_logical_port(port))) {
        return false;
    }

    /* CPU port is a valid sai port as well.
     * All public common sai port api's can pass cpu port as
     * a input for applicable set/get api's */

    if(port == sai_switch_cpu_port_obj_id_get()) {
        return true;
    }

    return sai_is_logical_port_valid(port);
}

sai_status_t sai_port_phy_type_get(sai_object_id_t port, sai_port_phy_t *phy_type)
{
    STD_ASSERT(phy_type != NULL);

    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    *phy_type = port_info_table->phy_type;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_port_group_get(sai_object_id_t port, uint_t *port_group)
{
    STD_ASSERT(port_group != NULL);

    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_PORT_NUMBER;
    }

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    *port_group = port_info_table->port_group;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_ext_phy_addr_get(sai_object_id_t port,
                                       sai_npu_port_id_t *ext_phy_addr)
{
    sai_port_phy_t phy_type;
    STD_ASSERT(ext_phy_addr != NULL);

    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if(sai_port_phy_type_get(port, &phy_type) != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("Phy type get failed for port 0x%"PRIx64"", port);
        return SAI_STATUS_FAILURE;
    }

    if(phy_type != SAI_PORT_PHY_INTERNAL) {
        *ext_phy_addr = port_info_table->ext_phy_addr;
        return SAI_STATUS_SUCCESS;
    }

    return SAI_STATUS_FAILURE;
}

sai_status_t sai_port_to_npu_local_port(sai_object_id_t port,
                                        sai_npu_port_id_t *local_port_id)
{
    STD_ASSERT(local_port_id != NULL);

    if(!(sai_is_obj_id_cpu_port(port) || sai_is_obj_id_logical_port(port))) {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if(port == sai_switch_cpu_port_obj_id_get()) {
        *local_port_id = sai_switch_get_cpu_port();
        return SAI_STATUS_SUCCESS;
    }

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    *local_port_id = port_info_table->local_port_id;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_npu_local_port_to_sai_port(sai_npu_port_id_t local_port_id,
                                            sai_object_id_t *port)
{
    sai_port_info_t *port_info_table = NULL;

    STD_ASSERT(port != NULL);
    if(local_port_id == sai_switch_get_cpu_port()) {
        *port = sai_switch_cpu_port_obj_id_get();
        return SAI_STATUS_SUCCESS;
    }

    for (port_info_table = sai_port_info_getfirst(); (port_info_table != NULL);
         port_info_table = sai_port_info_getnext(port_info_table)) {

        if(port_info_table->local_port_id == local_port_id) {
            *port = port_info_table->sai_port_id;
            return SAI_STATUS_SUCCESS;
        }
    }

    SAI_PORT_LOG_ERR("Unable to find mapping for npu port:%d", local_port_id);
    return SAI_STATUS_INVALID_OBJECT_ID;
}

/* Conversion should be possible even for in-active ports */
sai_status_t sai_port_to_physical_port(sai_object_id_t port,
                                       sai_npu_port_id_t *phy_port_id)
{
    STD_ASSERT(phy_port_id != NULL);

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    *phy_port_id = port_info_table->phy_port_id;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_max_lanes_get(sai_object_id_t port,
                                    uint_t *max_lanes_per_port)
{
    STD_ASSERT(max_lanes_per_port != NULL);

    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    *max_lanes_per_port = port_info_table->max_lanes_per_port;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_lane_bmap_get(sai_object_id_t port,
                                    uint64_t *port_lane_bmap)
{
    STD_ASSERT(port_lane_bmap != NULL);

    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    *port_lane_bmap = port_info_table->port_lane_bmap;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_lane_bmap_set(sai_object_id_t port,
                                    uint64_t port_lane_bmap)
{
    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    port_info_table->port_lane_bmap = port_lane_bmap;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_speed_get(sai_object_id_t port, sai_port_speed_t *speed)
{
    STD_ASSERT(speed != NULL);

    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    *speed = port_info_table->port_speed;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_speed_set(sai_object_id_t port, sai_port_speed_t speed)
{
    if(speed > SAI_PORT_SPEED_MAX) {
        SAI_PORT_LOG_ERR("Invalid speed %d for port 0x%"PRIx64"", speed, port);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    port_info_table->port_speed = speed;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_attr_supported_speed_update(sai_object_id_t port,
                                                  uint_t speed_capb)
{
    uint_t lane = 0;
    uint_t max_lanes = 0;
    sai_status_t ret_code = SAI_STATUS_FAILURE;
    sai_port_info_t *port_info_table = sai_port_info_get(port);

    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    SAI_PORT_LOG_TRACE("Updating sai_port_info_t for control port 0x%"PRIx64" "
                       "with speed capb %d", port, speed_capb);

    ret_code = sai_port_max_lanes_get(port, &max_lanes);
    if(ret_code != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("Max port lane get failed for port 0x%"PRIx64" "
                         "with err %d", port, ret_code);
        return ret_code;
    }

    for (lane = 0; lane < max_lanes; lane++) {
        if(port_info_table != NULL) {
            port_info_table->port_speed_capb = speed_capb;
        }
        port_info_table = sai_port_info_getnext(port_info_table);
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_attr_supported_speed_get(sai_object_id_t port,
                                               sai_attribute_value_t *value)
{
    sai_s32_list_t mode_list;
    uint32_t count = 0;
    uint_t speed_capability = 0;
    sai_status_t ret_code = SAI_STATUS_SUCCESS;
    int32_t sai_port_supported_speed_list[SAI_PORT_CAP_SPEED_MAX] = {0};

    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_info_t *sai_port_info = sai_port_info_get(port);
    if(sai_port_info == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    mode_list.list = sai_port_supported_speed_list;

    speed_capability = sai_port_info->port_speed_capb;
    if (speed_capability == 0) {
        speed_capability = (1 << 3 | 1 << 5);
    }

    if (speed_capability & SAI_PORT_CAP_SPEED_TEN_MEG) {
        mode_list.list[count++] = SAI_PORT_SPEED_TEN_MEG;
    }
    if (speed_capability & SAI_PORT_CAP_SPEED_HUNDRED_MEG) {
        mode_list.list[count++] = SAI_PORT_SPEED_HUNDRED_MEG;
    }
    if (speed_capability & SAI_PORT_CAP_SPEED_GIG) {
        mode_list.list[count++] = SAI_PORT_SPEED_GIG;
    }
    if (speed_capability & SAI_PORT_CAP_SPEED_TEN_GIG) {
        mode_list.list[count++] = SAI_PORT_SPEED_TEN_GIG;
    }
    if (speed_capability & SAI_PORT_CAP_SPEED_TWENTY_FIVE_GIG) {
        mode_list.list[count++] = SAI_PORT_SPEED_TWENTY_FIVE_GIG;
    }
    if (speed_capability & SAI_PORT_CAP_SPEED_FORTY_GIG) {
        mode_list.list[count++] = SAI_PORT_SPEED_FORTY_GIG;
    }
    if (speed_capability & SAI_PORT_CAP_SPEED_FORTY_TWO_GIG) {
        mode_list.list[count++] = SAI_PORT_SPEED_FORTYTWO_GIG;
    }
    if (speed_capability & SAI_PORT_CAP_SPEED_HUNDRED_GIG) {
        mode_list.list[count++] = SAI_PORT_SPEED_HUNDRED_GIG;
    }

    mode_list.count = count;

    do {
        if(mode_list.count == 0) { /* Not likely */
            ret_code = SAI_STATUS_FAILURE;
            break;
        }
        if(value->s32list.count < mode_list.count) {
            SAI_PORT_LOG_ERR("Get supported speed list count %d is less than "
                             "actual speed count supported %d in for port 0x%"PRIx64"",
                             value->s32list.count, mode_list.count, port);

            value->s32list.count = mode_list.count;
            ret_code = SAI_STATUS_BUFFER_OVERFLOW;
            break;
        }

        memcpy(value->s32list.list, mode_list.list, (mode_list.count * sizeof(int32_t)));

        /* Update the attr value count with actual supported speed count */
        value->s32list.count = mode_list.count;

    } while(0);

    mode_list.list = NULL;

    SAI_PORT_LOG_TRACE("Supported speed get successful for port 0x%"PRIx64" count %d",
                       port, value->s32list.count);

    return ret_code;
}



sai_status_t sai_port_media_type_get(sai_object_id_t port,
                                     sai_port_media_type_t *media_type)
{
    STD_ASSERT(media_type != NULL);

    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    *media_type = port_info_table->media_type;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_media_type_set(sai_object_id_t port,
                                     sai_port_media_type_t media_type)
{
    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    port_info_table->media_type = media_type;

    return SAI_STATUS_SUCCESS;
}

/* Port Capabilities Enable/disable API's */
/* API's related to port supported capabilities */
sai_status_t sai_is_port_capb_supported(sai_object_id_t port,
                                        uint64_t capb_mask, bool *value)
{
    STD_ASSERT(value != NULL);
    *value = false;

    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_info_t *sai_port_info_ptr = sai_port_info_get(port);
    if(sai_port_info_ptr == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if(sai_port_info_ptr->port_supported_capb & capb_mask) {
        *value = true;
    }

    return SAI_STATUS_SUCCESS;
}

void sai_port_supported_capability_set(sai_object_id_t port,
                                       uint64_t capb_val)
{
    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return;
    }

    sai_port_info_t *sai_port_info_ptr = sai_port_info_get(port);
    if(sai_port_info_ptr == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return;
    }

    sai_port_info_ptr->port_supported_capb |= capb_val;

    SAI_PORT_LOG_INFO("Port capability val %d set for port 0x%"PRIx64"", capb_val, port);

}

/* API's related to enabled port Capabilities */
sai_status_t sai_is_port_capb_enabled(sai_object_id_t port,
                                      uint64_t capb_mask, bool *value)
{
    STD_ASSERT(value != NULL);
    *value = false;

    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_info_t *sai_port_info_ptr = sai_port_info_get(port);
    if(sai_port_info_ptr == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if(sai_port_info_ptr->port_enabled_capb & capb_mask) {
        *value = true;
    }

    return SAI_STATUS_SUCCESS;
}

void sai_port_capablility_enable(sai_object_id_t port, bool enable, uint64_t capb_val)
{
    if(!sai_is_logical_port_valid(port)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return;
    }

    sai_port_info_t *sai_port_info_ptr = sai_port_info_get(port);
    if(sai_port_info_ptr == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return;
    }

    /* Enable only the supported capabilities */
    capb_val = capb_val & (sai_port_info_ptr->port_supported_capb);

    if(enable) {
        sai_port_info_ptr->port_enabled_capb |= capb_val;

    } else {
        sai_port_info_ptr->port_enabled_capb &= ~capb_val;
    }

    SAI_PORT_LOG_INFO("Port capability val %d %s for port 0x%"PRIx64"",
                       capb_val, (enable) ? "enable" : "disable", port);

}

sai_status_t sai_port_set_forwarding_mode(sai_object_id_t port,
                                          sai_port_fwd_mode_t fwd_mode)
{

    sai_port_info_t *sai_port_info_ptr = sai_port_info_get(port);
    if(sai_port_info_ptr == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    sai_port_info_ptr->fwd_mode = fwd_mode;

    SAI_PORT_LOG_TRACE("Port forwarding mode set to %d\r\n",fwd_mode);
    return SAI_STATUS_SUCCESS;
}


sai_status_t sai_port_get_forwarding_mode(sai_object_id_t port,
                                          sai_port_fwd_mode_t *fwd_mode)
{
    STD_ASSERT(fwd_mode != NULL);

    sai_port_info_t *sai_port_info_ptr = sai_port_info_get(port);
    if(sai_port_info_ptr == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    *fwd_mode = sai_port_info_ptr->fwd_mode;

    SAI_PORT_LOG_TRACE("Port forwarding mode is %d\r\n",*fwd_mode);
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_forward_mode_info(sai_object_id_t port,
                                        sai_port_fwd_mode_t *fwd_mode,
                                        bool update)
{
    sai_status_t ret_val = SAI_STATUS_FAILURE;
    STD_ASSERT(fwd_mode != NULL);
    sai_port_lock();
    if(update) {
        ret_val = sai_port_set_forwarding_mode(port,*fwd_mode);
    } else {
        ret_val = sai_port_get_forwarding_mode(port, fwd_mode);
    }
    sai_port_unlock();
    return ret_val;
}
const char *sai_port_forwarding_mode_to_str (sai_port_fwd_mode_t fwd_mode)
{
    if (fwd_mode == SAI_PORT_FWD_MODE_UNKNOWN) {
        return "Unknown";

    } else if (fwd_mode == SAI_PORT_FWD_MODE_SWITCHING) {
        return "Switching";

    } else if (fwd_mode == SAI_PORT_FWD_MODE_ROUTING) {
        return "Routing";

    } else {
        return "Invalid";
    }
}

sai_port_application_info_t* sai_port_application_info_create_and_get (sai_object_id_t port_id)
{
    sai_port_application_info_t *p_port_node = NULL;
    sai_port_application_info_t tmp_port_node;
    rbtree_handle               ports_applications_tree = NULL;

    memset (&tmp_port_node, 0, sizeof(sai_port_application_info_t));

    ports_applications_tree = sai_switch_info_get()->port_applications_tree;

    if (ports_applications_tree == NULL) {
        SAI_PORT_LOG_ERR ("Port applications tree is not created, Could be because of"
                          "switch initialization is not been completed");
        return NULL;
    }

    tmp_port_node.port_id = port_id;

    p_port_node = std_rbtree_getexact (ports_applications_tree, (void *) &tmp_port_node);

    if (p_port_node == NULL) {
        p_port_node = (sai_port_application_info_t *)
                        calloc (1, sizeof(sai_port_application_info_t));

        if (p_port_node == NULL) {
            SAI_PORT_LOG_ERR ("Could not allocate memory of size %d for application specific"
                              "info on port 0x%"PRIx64"", sizeof(sai_port_application_info_t),
                              port_id);
            return NULL;
        }

        p_port_node->port_id = port_id;

        if (std_rbtree_insert(ports_applications_tree, (void *) p_port_node) != STD_ERR_OK) {
            SAI_PORT_LOG_ERR ("Port Node insertion failed for port %u",p_port_node->port_id);
            free ((void *)p_port_node);
            return NULL;
        }

    }

    return p_port_node;
}

sai_status_t sai_port_application_info_remove (sai_port_application_info_t *p_port_node)
{
    rbtree_handle               ports_applications_tree = NULL;

    STD_ASSERT (p_port_node != NULL);
    ports_applications_tree = sai_switch_info_get()->port_applications_tree;

    if (ports_applications_tree == NULL) {
        SAI_PORT_LOG_ERR ("Port applications tree is not created, Could be because of"
                          "switch initialization is not been completed");
        return SAI_STATUS_FAILURE;
    }

    /* All the applications running on the port should add a check here */

    if (p_port_node->mirror_sessions_tree != NULL) {
        SAI_PORT_LOG_TRACE ("Mirror Applications still running on the port %u",
                            p_port_node->port_id);
        return SAI_STATUS_SUCCESS;
    }

    if (p_port_node->qos_port_db != NULL) {
        SAI_PORT_LOG_TRACE ("Qos Applications still running on the port %u",
                            p_port_node->port_id);
        return SAI_STATUS_SUCCESS;
    }

    if (std_rbtree_remove (ports_applications_tree, (void *)p_port_node) != p_port_node) {
        SAI_PORT_LOG_ERR ("Port Node remove failed for port %u",p_port_node->port_id);
        return SAI_STATUS_FAILURE;
    }

    free ((void *)p_port_node);

    return SAI_STATUS_SUCCESS;
}

sai_port_application_info_t* sai_port_application_info_get (sai_object_id_t port_id)
{
    sai_port_application_info_t *p_port_node = NULL;
    sai_port_application_info_t tmp_port_node;
    rbtree_handle               ports_applications_tree = NULL;

    memset (&tmp_port_node, 0, sizeof(sai_port_application_info_t));

    ports_applications_tree = sai_switch_info_get()->port_applications_tree;

    if ( NULL == ports_applications_tree) {
        SAI_PORT_LOG_ERR ("Port applications tree is not created, Could be because of"
                          "switch initialization is not been completed");
        return NULL;
    }

    tmp_port_node.port_id = port_id;

    p_port_node = std_rbtree_getexact (ports_applications_tree, (void *) &tmp_port_node);

    return p_port_node;
}

sai_port_application_info_t *sai_port_first_application_node_get (void)
{
    rbtree_handle port_applications_tree = sai_switch_info_get()->port_applications_tree;

    if (NULL == port_applications_tree)
        return NULL;

    return ((sai_port_application_info_t *)std_rbtree_getfirst(port_applications_tree));
}

sai_port_application_info_t *sai_port_next_application_node_get (
                                              sai_port_application_info_t *p_port_node)
{
    rbtree_handle port_applications_tree = sai_switch_info_get()->port_applications_tree;

    if (NULL == port_applications_tree)
        return NULL;

    return ((sai_port_application_info_t *)std_rbtree_getnext(port_applications_tree, p_port_node));
}

/* Get the active breakout mode for a given port */
sai_port_breakout_mode_type_t sai_port_current_breakout_mode_get(sai_object_id_t port)
{
    bool mode_enabled = false;

    sai_is_port_capb_enabled(port, SAI_PORT_CAP_BREAKOUT_MODE_2X, &mode_enabled);
    if(mode_enabled) {
        return sai_port_break_mode_from_capb(SAI_PORT_CAP_BREAKOUT_MODE_2X);
    }

    sai_is_port_capb_enabled(port, SAI_PORT_CAP_BREAKOUT_MODE_4X, &mode_enabled);
    if(mode_enabled) {
        return sai_port_break_mode_from_capb(SAI_PORT_CAP_BREAKOUT_MODE_4X);
    }

    /* A valid breakout port should be part of one of the possible breakout modes;
     * Default mode is SAI_PORT_CAP_BREAKOUT_MODE_1X */
    return sai_port_break_mode_from_capb(SAI_PORT_CAP_BREAKOUT_MODE_1X);
}

/* Note: port validation is expected to be done before invoking this call */
static void sai_port_break_mode_list(sai_object_id_t port, sai_s32_list_t *mode_list)
{
    uint32_t count = 0;
    bool mode_supported = false;

    STD_ASSERT(mode_list != NULL);

    sai_is_port_capb_supported(port, SAI_PORT_CAP_BREAKOUT_MODE_1X, &mode_supported);
    if(mode_supported) {
        mode_list->list[count] = sai_port_break_mode_from_capb(SAI_PORT_CAP_BREAKOUT_MODE_1X);
        count++;
    }

    sai_is_port_capb_supported(port, SAI_PORT_CAP_BREAKOUT_MODE_2X, &mode_supported);
    if(mode_supported) {
        mode_list->list[count] = sai_port_break_mode_from_capb(SAI_PORT_CAP_BREAKOUT_MODE_2X);
        count++;
    }

    sai_is_port_capb_supported(port, SAI_PORT_CAP_BREAKOUT_MODE_4X, &mode_supported);
    if(mode_supported) {
        mode_list->list[count] = sai_port_break_mode_from_capb(SAI_PORT_CAP_BREAKOUT_MODE_4X);
        count++;
    }

    mode_list->count = count;
}

sai_status_t sai_port_attr_supported_breakout_mode_get(sai_object_id_t port_id,
                                                       sai_attribute_value_t *value)
{
    sai_s32_list_t mode_list;
    sai_status_t ret_code = SAI_STATUS_SUCCESS;

    STD_ASSERT(value != NULL);
    STD_ASSERT(value->s32list.list != NULL);

    mode_list.list = (int32_t *) calloc(SAI_PORT_BREAKOUT_MODE_MAX,
                                        sizeof(int32_t));
    if(mode_list.list == NULL) {
        SAI_PORT_LOG_ERR("Allocation of Memory failed for breakout "
                         "mode lane list of port 0x%"PRIx64"", port_id);
        return SAI_STATUS_NO_MEMORY;
    }

    sai_port_break_mode_list(port_id, &mode_list);

    do {
        if(mode_list.count == 0) { /* Not likely */
            ret_code = SAI_STATUS_FAILURE;
            break;
        }

        if(value->s32list.count < mode_list.count) {
            SAI_PORT_LOG_ERR("Get supported breakout mode list count %d is less than "
                             "actual mode supported %d in for port 0x%"PRIx64"",
                             value->s32list.count, mode_list.count, port_id);

            value->s32list.count = mode_list.count;
            ret_code = SAI_STATUS_BUFFER_OVERFLOW;
            break;
        }

        memcpy(value->s32list.list, mode_list.list,
               (mode_list.count * sizeof(sai_port_breakout_mode_type_t)));

        /* Update the attr value count with actual breakout modes count */
        value->s32list.count = mode_list.count;
    } while(0);

    free(mode_list.list);
    mode_list.list = NULL;

    SAI_PORT_LOG_TRACE("Breakout mode get successful for port 0x%"PRIx64" count %d",
                       port_id, value->s32list.count);

    return ret_code;
}

static sai_status_t sai_port_hw_lane_list_get(sai_object_id_t port, uint32_t lane_count,
                                              uint32_t *lane_list)
{
    uint_t cur_lane = 0, count = 0, serdes_port = 0;
    sai_status_t ret_code = SAI_STATUS_FAILURE;

    STD_ASSERT(lane_list != NULL);

    sai_port_info_t *port_info = sai_port_info_get(port);
    if(port_info == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    ret_code = sai_port_to_physical_port(port, &serdes_port);
    if(ret_code != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("Phy port id get failed for port 0x%"PRIx64" with err %d",
                         port, ret_code);
        return ret_code;
    }

    lane_list[count] = serdes_port;
    count++;

    /* @todo lanes may not be sequential, it should be based on actual
     * hardware lane list. Also, currently for non-breakout ports,
     * physical id's of all lanes are not stored */

    for (cur_lane = 1; cur_lane < lane_count; cur_lane++) {
        lane_list[count] = ++serdes_port;
        count++;
    }

    return ret_code;
}

/* Get the HW lane list for a given SAI valid Logical port and CPU port is not supported */
sai_status_t sai_port_attr_hw_lane_list_get(sai_object_id_t port_id,
                                            sai_attribute_value_t *value)
{
    uint32_t *lane_list = NULL, lane_count = 0;
    sai_status_t ret_code = SAI_STATUS_FAILURE;

    STD_ASSERT(value != NULL);
    STD_ASSERT(value->u32list.list != NULL);

    ret_code = sai_port_max_lanes_get(port_id, &lane_count);
    if(ret_code != SAI_STATUS_SUCCESS) {
        SAI_SWITCH_LOG_ERR("Max port lane get failed for port 0x%"PRIx64" with err %d",
                           port_id, ret_code);
        return ret_code;
    }

    if(lane_count == 0) { /* Not likely */
        return SAI_STATUS_FAILURE;
    }

    if(value->u32list.count < lane_count) {
        SAI_PORT_LOG_ERR("Get hw lane list count %d is less than "
                         "actual lanes count %d in for port 0x%"PRIx64"",
                         value->u32list.count, lane_count, port_id);

        value->u32list.count = lane_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    lane_list = (uint32_t *) calloc(lane_count, sizeof(uint32_t));
    if(lane_list == NULL) {
        SAI_PORT_LOG_ERR ("Allocation of Memory failed "
                          "for hw port lane list ");
        return SAI_STATUS_NO_MEMORY;
    }

    do {
        ret_code = sai_port_hw_lane_list_get(port_id, lane_count, lane_list);
        if(ret_code != SAI_STATUS_SUCCESS) {
            SAI_PORT_LOG_ERR("Port hw lane list get port 0x%"PRIx64" ret %d",
                             port_id, ret_code);
            break;
        }

        memcpy(value->u32list.list, lane_list, (lane_count * sizeof(uint32_t)));

        /* Update the attr value count with actual HW lane count */
        value->u32list.count = lane_count;
    } while(0);

    free(lane_list);
    lane_list = NULL;

    SAI_PORT_LOG_TRACE("HW lane list get successful for port 0x%"PRIx64" count %d",
                       port_id, lane_count);

    return ret_code;
}

sai_status_t sai_port_breakout_mode_update(sai_object_id_t port,
                                           sai_port_speed_t speed,
                                           sai_port_breakout_mode_type_t new_mode,
                                           sai_port_breakout_mode_type_t prev_mode)
{
    uint_t lane = 0, max_lanes = 0, cap_val =0;
    sai_status_t ret_code = SAI_STATUS_FAILURE;
    sai_port_media_type_t media_type = SAI_PORT_MEDIA_TYPE_NOT_PRESENT;
    uint_t prev_cap_val = sai_port_capb_from_break_mode(prev_mode);
    sai_port_info_t *port_info_table = sai_port_info_get(port);
    if(port_info_table == NULL) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not a valid logical port", port);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    SAI_PORT_LOG_TRACE("Updating sai_port_info_t for control port 0x%"PRIx64" "
                       "with breakout mode %d", port, new_mode);

    ret_code = sai_port_max_lanes_get(port, &max_lanes);
    if(ret_code != SAI_STATUS_SUCCESS) {
        SAI_PORT_LOG_ERR("Max port lane get failed for port 0x%"PRIx64" "
                         "with err %d", port, ret_code);
        return ret_code;
    }

    /* @todo handle SAI_PORT_BREAKOUT_MODE_2_LANE */
    if(new_mode == SAI_PORT_BREAKOUT_MODE_1_LANE) { /* Single lane mode */

        /* Update control port's speed, lane bitmap and port capabilities */
        port_info_table->port_speed = speed;
        /* @todo lane bitmap should be based on max_lanes */
        port_info_table->port_lane_bmap = SAI_FOUR_LANE_BITMAP;

        cap_val = (SAI_PORT_CAP_BREAKOUT_MODE | prev_cap_val);
        sai_port_capablility_enable(port, false, cap_val);
        sai_port_capablility_enable(port, true, SAI_PORT_CAP_BREAKOUT_MODE_1X);

        /* Disable the subsidiary ports and set appropriate capability flags */
        for (lane = 1; lane < max_lanes; lane++) {
            port_info_table = sai_port_info_getnext(port_info_table);
            sai_port_capablility_enable(port_info_table->sai_port_id, false, cap_val);
            sai_port_capablility_enable(port_info_table->sai_port_id,
                                        true, SAI_PORT_CAP_BREAKOUT_MODE_1X);

            port_info_table->port_valid = false;
            port_info_table->media_type = media_type;
        }
    } else if(new_mode == SAI_PORT_BREAKOUT_MODE_4_LANE) { /* 4 lane Mode */

        /* Update control port's speed, lane bitmap and port capabilities */
        port_info_table->port_lane_bmap = SAI_ONE_LANE_BITMAP;
        port_info_table->port_speed = speed;

        sai_port_capablility_enable(port, false, prev_cap_val);
        cap_val = (SAI_PORT_CAP_BREAKOUT_MODE | SAI_PORT_CAP_BREAKOUT_MODE_4X);
        sai_port_capablility_enable(port, true, cap_val);
        media_type = port_info_table->media_type;

        /* Update the subsidiary ports with speed, media type, capabilities */
        for (lane = 1; lane < max_lanes; lane++) {
            port_info_table = sai_port_info_getnext(port_info_table);

            /* Set the port_valid bit and appropriate port speed */
            port_info_table->port_valid = true;
            port_info_table->port_speed = speed;

            /* update with control port's media type
             * @todo remove this as it should be set from adapter host */
            port_info_table->media_type = media_type;

            /* Clear the previous capability and enable the new port capability */
            sai_port_capablility_enable(port_info_table->sai_port_id, false, prev_cap_val);
            sai_port_capablility_enable(port_info_table->sai_port_id, true, cap_val);
        }
    }

    return ret_code;
}

void sai_port_logical_list_get(sai_object_list_t *port_list)
{
    uint_t count = 0;
    STD_ASSERT(port_list != NULL);
    STD_ASSERT(port_list->list != NULL);
    sai_port_info_t *port_info = NULL;

    for (port_info = sai_port_info_getfirst(); (port_info != NULL);
         port_info = sai_port_info_getnext(port_info)) {

        if(!sai_is_port_valid(port_info->sai_port_id)) {
            continue;
        }

        port_list->list[count] = port_info->sai_port_id;
        count++;
    }

    port_list->count = count;
}
