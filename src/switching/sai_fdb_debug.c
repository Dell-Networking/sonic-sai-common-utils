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
* @file sai_fdb_debug.c
*
* @brief This file contains Debug APIs for SAI FDB module
*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "saifdb.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_radix.h"
#include "sai_fdb_api.h"
#include "sai_fdb_common.h"
#include "sai_debug_utils.h"
#include "std_mac_utils.h"

static inline void print_fdb_header(void)
{
    SAI_DEBUG("%-20s %-5s %-20s %-5s %-5s","MAC","VLAN","Port","Type","Action");
    SAI_DEBUG("------------------------------------------------------------");
}

static inline void print_fdb_notification_header(void)
{
    SAI_DEBUG("%-20s %-5s %-20s %-5s %-5s","MAC","VLAN","Port","InCL","Event");
    SAI_DEBUG("------------------------------------------------------------");
}
void sai_dump_all_fdb_entry_nodes (bool print_all_entry_types,
                                   sai_fdb_flush_entry_type_t flush_entry_type)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t fdb_key;
    sai_fdb_entry_type_t entry_type = sai_get_sai_fdb_entry_type_for_flush(flush_entry_type);
    char mac_str[SAI_MAC_STR_LEN] = {0};
    std_rt_table *sai_global_fdb_tree = sai_fdb_cache_get();

    memset(&fdb_key, 0, sizeof(fdb_key));

    fdb_entry_node = (sai_fdb_entry_node_t *)std_radix_getnext(
                                                        sai_global_fdb_tree,
                                                        (u_char *)&fdb_key,
                                                        SAI_FDB_ENTRY_KEY_SIZE);

    print_fdb_header();
    while(fdb_entry_node != NULL) {
        memcpy(&fdb_key,&(fdb_entry_node->fdb_key),
               sizeof(sai_fdb_entry_key_t));
        if(print_all_entry_types == true || entry_type == fdb_entry_node->entry_type) {
            SAI_DEBUG("%-20s %-5d 0x%-20"PRIx64" %-5d %-5d",
                   std_mac_to_string((const sai_mac_t*)&(fdb_entry_node->fdb_key.mac_address),
                                     mac_str, sizeof(mac_str)),
                   fdb_entry_node->fdb_key.vlan_id,fdb_entry_node->port_id,
                   fdb_entry_node->entry_type, fdb_entry_node->action);
        }
        fdb_entry_node = (sai_fdb_entry_node_t *)std_radix_getnext(
                                                        sai_global_fdb_tree,
                                                        (u_char *)&fdb_key,
                                                        SAI_FDB_ENTRY_KEY_SIZE);
    }
}

void sai_dump_all_fdb_registered_nodes (void)
{
    sai_fdb_registered_node_t *fdb_registered_node = NULL;
    sai_fdb_entry_key_t fdb_key;
    char mac_str[SAI_MAC_STR_LEN] = {0};
    std_rt_table *sai_global_fdb_tree = sai_fdb_registered_entry_cache_get();

    memset(&fdb_key, 0, sizeof(fdb_key));

    fdb_registered_node = (sai_fdb_registered_node_t *)std_radix_getnext(
                                                        sai_global_fdb_tree,
                                                        (u_char *)&fdb_key,
                                                        SAI_FDB_ENTRY_KEY_SIZE);

    print_fdb_notification_header();
    while(fdb_registered_node != NULL) {
        memcpy(&fdb_key,&(fdb_registered_node->fdb_key),
               sizeof(sai_fdb_entry_key_t));
            SAI_DEBUG("%-20s %-5d 0x%-20"PRIx64" %-5d %-5d",
                   std_mac_to_string((const sai_mac_t*)&(fdb_registered_node->fdb_key.mac_address),
                                     mac_str, sizeof(mac_str)),
                   fdb_registered_node->fdb_key.vlan_id,fdb_registered_node->port_id,
                   fdb_registered_node->node_in_cl, fdb_registered_node->fdb_event);
        fdb_registered_node = (sai_fdb_registered_node_t *)std_radix_getnext(
                                                        sai_global_fdb_tree,
                                                        (u_char *)&fdb_key,
                                                        SAI_FDB_ENTRY_KEY_SIZE);
    }
}

void sai_dump_fdb_entry_nodes_per_port (sai_object_id_t port_id, bool print_all_entry_types,
                                        sai_fdb_flush_entry_type_t flush_entry_type)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t fdb_key;
    sai_fdb_entry_type_t entry_type = sai_get_sai_fdb_entry_type_for_flush(flush_entry_type);
    char mac_str[SAI_MAC_STR_LEN] = {0};
    std_rt_table *sai_global_fdb_tree = sai_fdb_cache_get();

    memset(&fdb_key, 0, sizeof(fdb_key));
    fdb_entry_node = (sai_fdb_entry_node_t *)std_radix_getnext(
                                                        sai_global_fdb_tree,
                                                        (u_char *)&fdb_key,
                                                        SAI_FDB_ENTRY_KEY_SIZE);
    print_fdb_header();
    while(fdb_entry_node != NULL) {
        memcpy(&fdb_key,&(fdb_entry_node->fdb_key),
               sizeof(sai_fdb_entry_key_t));
        if(fdb_entry_node->port_id == port_id){
            if(print_all_entry_types == true || entry_type == fdb_entry_node->entry_type) {
                SAI_DEBUG("%-20s %-5d 0x%-20"PRIx64" %-5d %-5d",
                    std_mac_to_string((const sai_mac_t*)&(fdb_entry_node->fdb_key.mac_address),
                                      mac_str, sizeof(mac_str)),
                    fdb_entry_node->fdb_key.vlan_id,fdb_entry_node->port_id,
                    fdb_entry_node->entry_type, fdb_entry_node->action);
            }
        }
        fdb_entry_node = (sai_fdb_entry_node_t *)std_radix_getnext(
                                                        sai_global_fdb_tree,
                                                        (u_char *)&fdb_key,
                                                        SAI_FDB_ENTRY_KEY_SIZE);
    }
}

void sai_dump_fdb_entry_nodes_per_vlan (sai_vlan_id_t vlan_id,bool print_all_entry_types,
                                        sai_fdb_flush_entry_type_t flush_entry_type)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t fdb_key;
    sai_fdb_entry_type_t entry_type = sai_get_sai_fdb_entry_type_for_flush(flush_entry_type);
    char mac_str[SAI_MAC_STR_LEN] = {0};
    std_rt_table *sai_global_fdb_tree = sai_fdb_cache_get();

    memset(&fdb_key, 0, sizeof(fdb_key));
    fdb_key.vlan_id = vlan_id;

    fdb_entry_node = (sai_fdb_entry_node_t *)std_radix_getnext(
                                                        sai_global_fdb_tree,
                                                        (u_char *)&fdb_key,
                                                        SAI_FDB_ENTRY_KEY_SIZE);

    print_fdb_header();
    while(fdb_entry_node != NULL) {
        memcpy(&fdb_key,&(fdb_entry_node->fdb_key),
              sizeof(sai_fdb_entry_key_t));
        if(fdb_key.vlan_id != vlan_id){
            break;
        }
        if(print_all_entry_types == true || entry_type == fdb_entry_node->entry_type) {
            SAI_DEBUG("%-20s %-5d 0x%-20"PRIx64" %-5d %-5d",
                   std_mac_to_string((const sai_mac_t*)&(fdb_entry_node->fdb_key.mac_address),
                                     mac_str, sizeof(mac_str)),
                   fdb_entry_node->fdb_key.vlan_id,fdb_entry_node->port_id,
                   fdb_entry_node->entry_type, fdb_entry_node->action);
        }
        fdb_entry_node = (sai_fdb_entry_node_t *)std_radix_getnext(
                                                        sai_global_fdb_tree,
                                                        (u_char *)&fdb_key,
                                                        SAI_FDB_ENTRY_KEY_SIZE);
    }
}

void sai_dump_fdb_entry_nodes_per_port_vlan (sai_object_id_t port_id,
                                             sai_vlan_id_t vlan_id,
                                             bool print_all_entry_types,
                                             sai_fdb_flush_entry_type_t flush_entry_type)
{
    sai_fdb_entry_node_t *fdb_entry_node = NULL;
    sai_fdb_entry_key_t fdb_key;
    sai_fdb_entry_type_t entry_type = sai_get_sai_fdb_entry_type_for_flush(flush_entry_type);
    char mac_str[SAI_MAC_STR_LEN] = {0};
    std_rt_table *sai_global_fdb_tree = sai_fdb_cache_get();

    memset(&fdb_key, 0, sizeof(fdb_key));
    fdb_key.vlan_id = vlan_id;

    fdb_entry_node = (sai_fdb_entry_node_t *)std_radix_getnext(
                                                        sai_global_fdb_tree,
                                                        (u_char *)&fdb_key,
                                                        SAI_FDB_ENTRY_KEY_SIZE);
    print_fdb_header();
    while(fdb_entry_node != NULL) {
        memcpy(&fdb_key,&(fdb_entry_node->fdb_key),
               sizeof(sai_fdb_entry_key_t));
        if(fdb_key.vlan_id != vlan_id){
            break;
        }
        if((fdb_entry_node->port_id == port_id)){
            if(print_all_entry_types == true || entry_type == fdb_entry_node->entry_type) {
                SAI_DEBUG("%-20s %-5d 0x%-20"PRIx64" %-5d %-5d",
                       std_mac_to_string((const sai_mac_t*)&(fdb_entry_node->fdb_key.mac_address),
                                         mac_str, sizeof(mac_str)),
                       fdb_entry_node->fdb_key.vlan_id,fdb_entry_node->port_id,
                       fdb_entry_node->entry_type, fdb_entry_node->action);
            }
        }
        fdb_entry_node = (sai_fdb_entry_node_t *)std_radix_getnext(
                                                        sai_global_fdb_tree,
                                                        (u_char *)&fdb_key,
                                                        SAI_FDB_ENTRY_KEY_SIZE);
    }
}
