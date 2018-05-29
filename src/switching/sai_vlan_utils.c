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
* @file sai_vlan_utils.c
*
* @brief This file contains utility APIs for SAI VLAN module
*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "saivlan.h"
#include "saistatus.h"
#include "saitypes.h"
#include "std_llist.h"
#include "sai_vlan_api.h"
#include "sai_vlan_common.h"
#include "std_mutex_lock.h"
#include "std_assert.h"
#include "sai_switch_utils.h"
#include "sai_port_utils.h"
#include "sai_oid_utils.h"
#include "sai_gen_utils.h"

static sai_vlan_global_cache_node_t *global_vlan_list[SAI_MAX_VLAN_TAG_ID+1];
static std_mutex_lock_create_static_init_fast(vlan_lock);
static sai_vlan_id_t sai_internal_vlan_id = VLAN_UNDEF;
static std_dll_head global_port_vlan_count_list;

void sai_vlan_lock(void)
{
    std_mutex_lock(&vlan_lock);
}

void sai_vlan_unlock(void)
{
    std_mutex_unlock(&vlan_lock);
}

sai_vlan_global_cache_node_t* sai_vlan_portlist_cache_read(sai_vlan_id_t vlan_id)
{
    return global_vlan_list[vlan_id];
}

void sai_init_internal_vlan_id(sai_vlan_id_t vlan_id)
{
    sai_internal_vlan_id = vlan_id;
}

bool sai_is_internal_vlan_id_initialized (void)
{
    return (sai_internal_vlan_id != VLAN_UNDEF);
}

bool sai_is_internal_vlan_id(sai_vlan_id_t vlan_id)
{
    return (vlan_id == sai_internal_vlan_id) ? true : false;
}

sai_vlan_id_t sai_internal_vlan_id_get(void)
{
    return sai_internal_vlan_id;
}

sai_status_t sai_vlan_cache_init(void)
{
    sai_vlan_id_t vlan_id = 0;

    SAI_VLAN_LOG_TRACE("Performing VLAN Module Init");
    for(vlan_id = 0; vlan_id <= SAI_MAX_VLAN_TAG_ID; vlan_id++)
    {
        global_vlan_list[vlan_id] = NULL;
    }
    /*Allocating for maximum possible port range
      so that it can work for fanout too
     */
    std_dll_init_sort(&global_port_vlan_count_list,sai_port_node_compare,
                 SAI_PORTV_VLAN_COUNTER_OFFSET, SAI_PORTV_VLAN_COUNTER_SIZE);
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_add_port_vlan_counter_node(sai_object_id_t port_id)
{
    sai_port_vlan_counter_t *port_vlan_counter = NULL;

    port_vlan_counter = (sai_port_vlan_counter_t *)
                         calloc(1, sizeof(sai_port_vlan_counter_t));
    if(port_vlan_counter == NULL) {
        SAI_VLAN_LOG_CRIT("Unable to add port 0x%"PRIx64" memory %d unavailable",
                         port_id, sizeof(sai_port_vlan_counter_t));
        return SAI_STATUS_NO_MEMORY;

    }
    port_vlan_counter->port_id = port_id;
    std_dll_insert(&global_port_vlan_count_list,&(port_vlan_counter->node));
    return SAI_STATUS_SUCCESS;
}

static sai_port_vlan_counter_t* sai_find_port_vlan_counter(sai_object_id_t port_id)
{
    sai_port_vlan_counter_t *port_vlan_counter = NULL;
    std_dll *node = NULL;

    for(node = std_dll_getfirst(&global_port_vlan_count_list);
        node != NULL;
        node = std_dll_getnext(&global_port_vlan_count_list,node)) {
        port_vlan_counter = (sai_port_vlan_counter_t *)node;
        if(port_vlan_counter->port_id == port_id) {
            return port_vlan_counter;
        }else if (port_vlan_counter->port_id > port_id){
            break;
        }
    }
    return NULL;
}

static sai_status_t sai_remove_port_vlan_counter(sai_object_id_t port_id)
{
    sai_port_vlan_counter_t *port_vlan_counter = NULL;

    port_vlan_counter = sai_find_port_vlan_counter(port_id);
    if(port_vlan_counter != NULL) {
        std_dll_remove(&global_port_vlan_count_list,&(port_vlan_counter->node));
        free(port_vlan_counter);
        return SAI_STATUS_SUCCESS;
    }
    return SAI_STATUS_ITEM_NOT_FOUND;
}

bool sai_is_port_vlan_configured(sai_object_id_t port_id)
{
    sai_port_vlan_counter_t *port_vlan_counter = NULL;

    port_vlan_counter = sai_find_port_vlan_counter(port_id);
    if(port_vlan_counter == NULL) {
        return false;
    }
    return true;
}

static sai_status_t sai_increment_port_vlan_counter(sai_object_id_t port_id)
{
    sai_port_vlan_counter_t *port_vlan_counter = NULL;
    sai_status_t ret_val = SAI_STATUS_FAILURE;

    port_vlan_counter = sai_find_port_vlan_counter(port_id);
    if(port_vlan_counter == NULL) {
         ret_val = sai_add_port_vlan_counter_node(port_id);
         if(ret_val != SAI_STATUS_SUCCESS) {
             SAI_VLAN_LOG_WARN("Unable to create port vlan counter for port 0x%"PRIx64"",
                               port_id);
             return ret_val;
         }
         port_vlan_counter = sai_find_port_vlan_counter(port_id);
    }
    port_vlan_counter->vlan_count++;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_decrement_port_vlan_counter(sai_object_id_t port_id)
{
    sai_port_vlan_counter_t *port_vlan_counter = NULL;

    port_vlan_counter = sai_find_port_vlan_counter(port_id);
    if(port_vlan_counter == NULL) {
         SAI_VLAN_LOG_WARN("Unable to find port vlan counter for port 0x%"PRIx64"",
                            port_id);
         return SAI_STATUS_ITEM_NOT_FOUND;
    }
    port_vlan_counter->vlan_count--;
    if(port_vlan_counter->vlan_count == 0) {
        sai_remove_port_vlan_counter(port_id);
    }
    return SAI_STATUS_SUCCESS;
}

bool sai_is_vlan_created(sai_vlan_id_t vlan_id)
{
    if((vlan_id > SAI_MAX_VLAN_TAG_ID) ||
        (vlan_id < SAI_MIN_VLAN_TAG_ID)) {
        return false;
    }
    return ((global_vlan_list[vlan_id] != NULL) ? true:false);
}

sai_status_t sai_insert_vlan_in_list(sai_vlan_id_t vlan_id)
{
    sai_vlan_global_cache_node_t *vplist = NULL;

    if(sai_is_vlan_created(vlan_id)) {
        SAI_VLAN_LOG_INFO("Vlan Id %d already created",
                            vlan_id);
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }
    vplist=(sai_vlan_global_cache_node_t *)calloc(1, sizeof(sai_vlan_global_cache_node_t));
    if(vplist == NULL) {
        SAI_VLAN_LOG_CRIT("No memory  to create Vlan Id %d memory:%d",
                          vlan_id, sizeof(sai_vlan_global_cache_node_t));
        return SAI_STATUS_NO_MEMORY;
    }
    vplist->vlan_id=vlan_id;
    std_dll_init(&(vplist->port_list));
    global_vlan_list[vlan_id] = vplist;
    SAI_VLAN_LOG_TRACE("Vlan Id %d Inserted in cache",
                        vlan_id);
    return SAI_STATUS_SUCCESS;
}

void sai_remove_all_vlan_port_nodes (sai_vlan_id_t vlan_id)
{
    sai_vlan_port_node_t *vlan_port_node;
    std_dll *node = NULL;

    SAI_VLAN_LOG_TRACE("Removing all ports for Vlan Id %d",
                        vlan_id);
    for(node = std_dll_getfirst(&(global_vlan_list[vlan_id]->port_list));
        node != NULL;
        node = std_dll_getfirst(&(global_vlan_list[vlan_id]->port_list))) {
        /* Remvoing the first node here. Hence for loop should take first
         * node again which would have been replaced by remove operation
         * below
         */
        vlan_port_node = (sai_vlan_port_node_t *)node;
        std_dll_remove(&(global_vlan_list[vlan_id]->port_list),
                       &(vlan_port_node->node));
        sai_decrement_port_vlan_counter(vlan_port_node->vlan_port.port_id);
        free(vlan_port_node);
        global_vlan_list[vlan_id]->port_count--;
    }
}
sai_status_t sai_remove_vlan_from_list(sai_vlan_id_t vlan_id)
{
    if(!sai_is_vlan_created(vlan_id)) {
        SAI_VLAN_LOG_WARN("Vlan Id %d not found", vlan_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    sai_remove_all_vlan_port_nodes(vlan_id);
    free(global_vlan_list[vlan_id]);
    global_vlan_list[vlan_id] = NULL;
    SAI_VLAN_LOG_TRACE("Deleted Vlan Id %d", vlan_id);
    return SAI_STATUS_SUCCESS;
}
sai_status_t sai_add_vlan_port_node(sai_vlan_id_t vlan_id,
                                    const sai_vlan_port_t *vlan_port)
{
    sai_vlan_port_node_t *vlan_port_node = NULL;
    sai_vlan_port_t def_vlan_port;
    sai_port_fwd_mode_t fwd_mode;

    STD_ASSERT(vlan_port != NULL);
    memset(&def_vlan_port, 0, sizeof(def_vlan_port));
    vlan_port_node = (sai_vlan_port_node_t *)
                          calloc(1, sizeof(sai_vlan_port_node_t));

    if(vlan_port_node == NULL) {
        SAI_VLAN_LOG_CRIT("Unable to add  port 0x"PRIx64" Vlan Id %d \
                         memory %d unavailable",
                         vlan_port->port_id, vlan_id,
                         sizeof(sai_vlan_port_node_t));
        return SAI_STATUS_NO_MEMORY;
    }
    vlan_port_node->vlan_port = *vlan_port;
    std_dll_insertatback(&(global_vlan_list[vlan_id]->port_list),
                         &(vlan_port_node->node));
    SAI_VLAN_LOG_TRACE("Added  port 0x:%"PRIx64" Vlan Id %d",
                      vlan_port->port_id, vlan_id);
    global_vlan_list[vlan_id]->port_count++;
    if(!sai_is_port_vlan_configured(vlan_port->port_id)) {
        fwd_mode = SAI_PORT_FWD_MODE_SWITCHING;
        sai_port_forward_mode_info (vlan_port->port_id, &fwd_mode, true);
    }
    sai_increment_port_vlan_counter(vlan_port->port_id);
    return SAI_STATUS_SUCCESS;
}

sai_vlan_port_node_t* sai_find_vlan_port_node(sai_vlan_id_t vlan_id,
                                              const sai_vlan_port_t *vlan_port)
{
    sai_vlan_port_node_t *vlan_port_node = NULL;
    std_dll *node = NULL;
    STD_ASSERT(vlan_port != NULL);
    for(node = std_dll_getfirst(&(global_vlan_list[vlan_id]->port_list));
        node != NULL;
        node = std_dll_getnext(&(global_vlan_list[vlan_id]->port_list),node)) {
        vlan_port_node = (sai_vlan_port_node_t *)node;
        if((vlan_port_node->vlan_port.port_id == vlan_port->port_id) &&
           (vlan_port_node->vlan_port.tagging_mode == vlan_port->tagging_mode)){
            return vlan_port_node;
        }
    }
    SAI_VLAN_LOG_TRACE("port 0x%"PRIx64" tagging mode %d not found on Vlan Id %d",
                     vlan_port->port_id, vlan_port->tagging_mode, vlan_id);
    return NULL;
}

bool sai_is_valid_vlan_port_member (sai_vlan_id_t vlan_id,
                                    const sai_vlan_port_t *vlan_port)
{
    bool ret_val = false;
    STD_ASSERT(vlan_port != NULL);
    if(sai_find_vlan_port_node(vlan_id, vlan_port) != NULL) {
        ret_val = true;
    }
    return ret_val;
}
bool sai_is_port_in_different_tagging_mode(sai_vlan_id_t vlan_id,
                                              const sai_vlan_port_t *vlan_port)
{
    sai_vlan_port_node_t *vlan_port_node = NULL;
    std_dll *node = NULL;
    STD_ASSERT(vlan_port != NULL);
    for(node = std_dll_getfirst(&(global_vlan_list[vlan_id]->port_list));
        node != NULL;
        node = std_dll_getnext(&(global_vlan_list[vlan_id]->port_list),node)) {
        vlan_port_node = (sai_vlan_port_node_t *)node;
        if((vlan_port_node->vlan_port.port_id == vlan_port->port_id) &&
           (vlan_port_node->vlan_port.tagging_mode != vlan_port->tagging_mode)){
            return true;
        }
    }
    return false;
}
sai_status_t sai_remove_vlan_port_node (sai_vlan_id_t vlan_id,
                                        const sai_vlan_port_t *vlan_port)
{
    sai_vlan_port_node_t *vlan_port_node = NULL;
    sai_port_fwd_mode_t fwd_mode;

    STD_ASSERT(vlan_port != NULL);
    vlan_port_node = sai_find_vlan_port_node(vlan_id, vlan_port);
    if(vlan_port_node != NULL) {
        std_dll_remove(&(global_vlan_list[vlan_id]->port_list),
                       &(vlan_port_node->node));
        sai_decrement_port_vlan_counter(vlan_port_node->vlan_port.port_id);
        free(vlan_port_node);
        global_vlan_list[vlan_id]->port_count--;
        if(!sai_is_port_vlan_configured(vlan_port->port_id)) {
            fwd_mode = SAI_PORT_FWD_MODE_UNKNOWN;
            sai_port_forward_mode_info (vlan_port->port_id, &fwd_mode, true);
        }
        SAI_VLAN_LOG_TRACE("port 0x%"PRIx64" removed from Vlan Id %d",
                           vlan_port->port_id, vlan_id);
        return SAI_STATUS_SUCCESS;
    }
    return SAI_STATUS_INVALID_PORT_MEMBER;
}
sai_status_t sai_vlan_port_list_get(sai_vlan_id_t vlan_id, sai_vlan_port_list_t *vlan_port_list)
{
    sai_vlan_port_node_t *vlan_port_node = NULL;
    std_dll *node = NULL;
    unsigned int port_idx = 0;

    STD_ASSERT(vlan_port_list != NULL);
    if(vlan_port_list->count < global_vlan_list[vlan_id]->port_count) {
        vlan_port_list->count = global_vlan_list[vlan_id]->port_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    for(node = std_dll_getfirst(&(global_vlan_list[vlan_id]->port_list));
        node != NULL;
        node = std_dll_getnext(&(global_vlan_list[vlan_id]->port_list),node)) {
        vlan_port_node = (sai_vlan_port_node_t *)node;
        vlan_port_list->list[port_idx] = vlan_port_node->vlan_port;
        port_idx++;
    }
    vlan_port_list->count = port_idx;
    return SAI_STATUS_SUCCESS;
}
bool sai_is_valid_vlan_tagging_mode(sai_vlan_tagging_mode_t tagging_mode)
{
    switch(tagging_mode) {
        case SAI_VLAN_PORT_UNTAGGED:
        case SAI_VLAN_PORT_TAGGED:
        case SAI_VLAN_PORT_PRIORITY_TAGGED:
            return true;
    }
    return false;
}
void sai_vlan_learn_disable_cache_write(sai_vlan_id_t vlan_id, bool disable)
{
    global_vlan_list[vlan_id]->learn_disable = disable;
}
bool sai_vlan_learn_disable_cache_read(sai_vlan_id_t vlan_id)
{
    return global_vlan_list[vlan_id]->learn_disable;
}
void sai_vlan_max_learn_adddress_cache_write(sai_vlan_id_t vlan_id, unsigned int val)
{
    global_vlan_list[vlan_id]->max_learned_address = val;
}
unsigned int sai_vlan_max_learn_adddress_cache_read(sai_vlan_id_t vlan_id)
{
    return global_vlan_list[vlan_id]->max_learned_address;
}
void sai_vlan_meta_data_cache_write(sai_vlan_id_t vlan_id, unsigned int val)
{
    global_vlan_list[vlan_id]->meta_data = val;
}
unsigned int sai_vlan_meta_data_cache_read(sai_vlan_id_t vlan_id)
{
    return global_vlan_list[vlan_id]->meta_data;
}
