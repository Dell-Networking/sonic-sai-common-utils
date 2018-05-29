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
* @file sai_port_attributes.c
*
* @brief This file contains SAI Port attributes default value initialization
* and sai_port_attr_info_t cache set and get APIs and few port attributes
* get implementation.
*
*************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "std_assert.h"

#include "saiswitch.h"
#include "saitypes.h"
#include "saiport.h"
#include "saistatus.h"

#include "sai_port_common.h"
#include "sai_port_utils.h"

/* CPU ports attribute info cache */
static sai_port_attr_info_t cpu_port_attr_info;

/* Port attributes default values */
static inline void sai_port_attr_info_defaults_get(sai_port_attr_info_t *port_attr_info)
{
    STD_ASSERT(!(port_attr_info == NULL));

    port_attr_info->oper_status = SAI_DFLT_OPER_STATUS;
    port_attr_info->speed = SAI_DFLT_SPEED;
    port_attr_info->duplex = SAI_DFLT_FULLDUPLEX;
    port_attr_info->admin_state = SAI_DFLT_ADMIN_STATE;
    port_attr_info->media_type = SAI_DFLT_MEDIA_TYPE;
    port_attr_info->default_vlan = SAI_DFLT_VLAN;
    port_attr_info->default_vlan_priority = SAI_DFLT_VLAN_PRIORITY;
    port_attr_info->ingress_filtering = SAI_DFLT_ING_FILTERING;
    port_attr_info->drop_untagged = SAI_DFLT_DROP_UNTAGGED;
    port_attr_info->drop_tagged = SAI_DFLT_DROP_TAGGED;
    port_attr_info->internal_loopback = SAI_DFLT_LOOPBACK_MODE;
    port_attr_info->fdb_learning = SAI_DFLT_FDB_LEARNING_MODE;
    port_attr_info->update_dscp = SAI_DFLT_UPDATE_DSCP;
    port_attr_info->mtu = SAI_DFLT_MTU;
    port_attr_info->max_learned_address = SAI_DFLT_MAX_LEARNED_ADDR;
    port_attr_info->fdb_learn_limit_violation = SAI_DFLT_FDB_LEARNED_LIMIT_VIOL;
    port_attr_info->flow_control_mode = SAI_DFLT_FLOW_CONTROL_MODE;
    port_attr_info->pfc_enabled_bitmap = SAI_DFLT_PFC_ENABLED_BITMAP;

}

/* Set the default port attribute values */
void sai_port_attr_defaults_init(void)
{
    sai_port_info_t *port_info = NULL;

    SAI_PORT_LOG_TRACE("Attributes default value init");

    for (port_info = sai_port_info_getfirst(); (port_info != NULL);
         port_info = sai_port_info_getnext(port_info)) {

        sai_port_attr_info_defaults_get(&port_info->port_attr_info);
    }

    /* Fill defaults for CPU port */
    memset(&cpu_port_attr_info, 0, sizeof(cpu_port_attr_info));
    sai_port_attr_info_defaults_get(&cpu_port_attr_info);
}

sai_status_t sai_port_attr_type_get(sai_object_id_t port_id,
                                    sai_attribute_value_t *value)
{
    STD_ASSERT(value != NULL);
    sai_port_type_t port_type = sai_port_type_get(port_id);

    switch(port_type)
    {
        case SAI_PORT_TYPE_CPU:
        case SAI_PORT_TYPE_LOGICAL:
            value->s32 = port_type;
            break;
        default:
            return SAI_STATUS_INVALID_OBJECT_ID;
    }

    return SAI_STATUS_SUCCESS;
}

sai_port_attr_info_t *sai_port_attr_info_get(sai_object_id_t port)
{
    if(sai_is_obj_id_cpu_port(port)) {
        return &cpu_port_attr_info;

    } else if(sai_is_obj_id_logical_port(port)) {

        sai_port_info_t *port_info_table = sai_port_info_get(port);
        if (port_info_table == NULL) {
            return NULL;
        }
        return &port_info_table->port_attr_info;
    }

    return NULL;
}

/* Cache the port attributes for VM and Dump */
sai_status_t sai_port_attr_info_cache_set(sai_object_id_t port_id,
                                          const sai_attribute_t *attr)
{
    STD_ASSERT(!(attr == NULL));

    if(!sai_is_port_valid(port_id)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not valid port", port_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_attr_info_t *port_attr_info = sai_port_attr_info_get(port_id);
    STD_ASSERT(!(port_attr_info == NULL));

    SAI_PORT_LOG_TRACE("Attribute %d cache update for port 0x%"PRIx64"", attr->id, port_id);

    switch(attr->id) {
        case SAI_PORT_ATTR_OPER_STATUS:
            port_attr_info->oper_status = attr->value.s32;
            break;

        case SAI_PORT_ATTR_SPEED:
            port_attr_info->speed = attr->value.u32;
            break;

        case SAI_PORT_ATTR_FULL_DUPLEX_MODE:
            port_attr_info->duplex = attr->value.booldata;
            break;

        case SAI_PORT_ATTR_AUTO_NEG_MODE:
            port_attr_info->autoneg = attr->value.booldata;
            break;

        case SAI_PORT_ATTR_ADMIN_STATE:
            port_attr_info->admin_state = attr->value.booldata;
            break;

        case SAI_PORT_ATTR_MEDIA_TYPE:
            port_attr_info->media_type = attr->value.s32;
            break;

        case SAI_PORT_ATTR_PORT_VLAN_ID:
            port_attr_info->default_vlan = attr->value.u16;
            break;

        case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
            port_attr_info->default_vlan_priority = attr->value.u8;
            break;

        case SAI_PORT_ATTR_INGRESS_FILTERING:
            port_attr_info->ingress_filtering = attr->value.booldata;
            break;

        case SAI_PORT_ATTR_DROP_UNTAGGED:
            port_attr_info->drop_untagged = attr->value.booldata;
            break;

        case SAI_PORT_ATTR_DROP_TAGGED:
            port_attr_info->drop_tagged = attr->value.booldata;
            break;

        case SAI_PORT_ATTR_INTERNAL_LOOPBACK:
            port_attr_info->internal_loopback = attr->value.s32;
            break;

        case SAI_PORT_ATTR_FDB_LEARNING:
            port_attr_info->fdb_learning = attr->value.s32;
            break;

        case SAI_PORT_ATTR_UPDATE_DSCP:
            port_attr_info->update_dscp = attr->value.booldata;
            break;

        case SAI_PORT_ATTR_MTU:
            port_attr_info->mtu = attr->value.u32;
            break;

        case SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES:
            port_attr_info->max_learned_address = attr->value.u32;
            break;

        case SAI_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION:
            port_attr_info->fdb_learn_limit_violation = attr->value.s32;
            break;

        case SAI_PORT_ATTR_META_DATA:
            port_attr_info->meta_data = attr->value.u32;
            break;

        case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL:
            port_attr_info->flow_control_mode = attr->value.s32;
            break;

        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL:
            port_attr_info->pfc_enabled_bitmap = attr->value.u8;
            break;

        default:
            SAI_PORT_LOG_ERR("Attribute %d not in cache list for port 0x%"PRIx64"",
                             attr->id, port_id);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_port_attr_info_cache_get(sai_object_id_t port_id,
                                          sai_attribute_t *attr)
{
    STD_ASSERT(!(attr == NULL));

    if(!sai_is_port_valid(port_id)) {
        SAI_PORT_LOG_ERR("Port 0x%"PRIx64" is not valid port", port_id);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    sai_port_attr_info_t *port_attr_info = sai_port_attr_info_get(port_id);
    STD_ASSERT(!(port_attr_info == NULL));

    switch(attr->id) {
        case SAI_PORT_ATTR_OPER_STATUS:
            attr->value.s32 = port_attr_info->oper_status;
            break;

        case SAI_PORT_ATTR_SPEED:
            attr->value.u32 = port_attr_info->speed;
            break;

        case SAI_PORT_ATTR_FULL_DUPLEX_MODE:
            attr->value.booldata = port_attr_info->duplex;
            break;

        case SAI_PORT_ATTR_AUTO_NEG_MODE:
            attr->value.booldata = port_attr_info->autoneg;
            break;

        case SAI_PORT_ATTR_ADMIN_STATE:
            attr->value.booldata = port_attr_info->admin_state;
            break;

        case SAI_PORT_ATTR_MEDIA_TYPE:
            attr->value.s32 = port_attr_info->media_type;
            break;

        case SAI_PORT_ATTR_PORT_VLAN_ID:
            attr->value.u16 = port_attr_info->default_vlan;
            break;

        case SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY:
            attr->value.u8 = port_attr_info->default_vlan_priority;
            break;

        case SAI_PORT_ATTR_INGRESS_FILTERING:
            attr->value.booldata = port_attr_info->ingress_filtering;
            break;

        case SAI_PORT_ATTR_DROP_UNTAGGED:
            attr->value.booldata = port_attr_info->drop_untagged;
            break;

        case SAI_PORT_ATTR_DROP_TAGGED:
            attr->value.booldata = port_attr_info->drop_tagged;
            break;

        case SAI_PORT_ATTR_INTERNAL_LOOPBACK:
            attr->value.s32 = port_attr_info->internal_loopback;
            break;

        case SAI_PORT_ATTR_FDB_LEARNING:
            attr->value.s32 = port_attr_info->fdb_learning;
            break;

        case SAI_PORT_ATTR_UPDATE_DSCP:
            attr->value.booldata = port_attr_info->update_dscp;
            break;

        case SAI_PORT_ATTR_MTU:
            attr->value.u32 = port_attr_info->mtu;
            break;

        case SAI_PORT_ATTR_MAX_LEARNED_ADDRESSES:
            attr->value.u32 = port_attr_info->max_learned_address;
            break;

        case SAI_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION:
            attr->value.s32 = port_attr_info->fdb_learn_limit_violation;
            break;

        case SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL:
            attr->value.s32 = port_attr_info->flow_control_mode;
            break;

        case SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL:
            attr->value.u8 = port_attr_info->pfc_enabled_bitmap;
            break;

        default:
            SAI_PORT_LOG_ERR("Attribute %d not in cache list for port 0x%"PRIx64"",
                             attr->id, port_id);
            return SAI_STATUS_INVALID_ATTRIBUTE_0;
    }

    return SAI_STATUS_SUCCESS;
}

