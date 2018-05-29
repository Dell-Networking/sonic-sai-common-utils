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

/***
 * \file    sai_vlan_api.h
 *
 * \brief Declaration of SAI VLAN related APIs
*/

#if !defined (__SAIVLANAPI_H_)
#define __SAIVLANAPI_H_
#include "saivlan.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sai_vlan_common.h"
/** SAI VLAN API - Init VLAN Module data structures
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_UNINITIALIZED
*/
sai_status_t sai_vlan_cache_init(void);

/** SAI VLAN API - Check whther if VLAN ID is valid
      \param[in] vlan_id VLAN Identifier
      \return Success: true
              Failure: false
*/
static inline bool sai_is_valid_vlan_id(sai_vlan_id_t vlan_id)
{
    if((vlan_id < SAI_MIN_VLAN_TAG_ID) ||
        (vlan_id > SAI_MAX_VLAN_TAG_ID)) {
        return false;
    }
    return true;
}

/** SAI VLAN API - Check whther if VLAN ID is created
      \param[in] vlan_id VLAN Identifier
      \return Success: true
              Failure: false
*/
bool sai_is_vlan_created(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Insert VLAN in Data cache
      \param[in] vlan_id VLAN Identifier
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_ITEM_ALREADY_EXISTS, SAI_STATUS_NO_MEMORY
*/
sai_status_t sai_insert_vlan_in_list(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Remove VLAN from Data cache
      \param[in] vlan_id VLAN Identifier
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_remove_vlan_from_list(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Insert VLAN Port node to VLAN Port list
      \param[in] vlan_id VLAN Identifier
      \param[in] vlan_port Port id along with tagging mode
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_NO_MEMORY
*/
sai_status_t sai_add_vlan_port_node(sai_vlan_id_t vlan_id,
                                    const sai_vlan_port_t *vlan_port);

/** SAI VLAN API - Find VLAN Port node from VLAN Port list
      \param[in] vlan_id VLAN Identifier
      \param[in] vlan_port Port id along with tagging mode
      \return Success: A valid pointer to VLAN Port node
              Failure: NULL
*/
sai_vlan_port_node_t* sai_find_vlan_port_node(sai_vlan_id_t vlan_id,
                                             const sai_vlan_port_t *vlan_port);

/** SAI VLAN API - Check if port is a valid member of a VLAN
      \param[in] vlan_id VLAN Identifier
      \param[in] vlan_port Port id along with tagging mode
      \return Success: true
              Failure: false
*/
bool sai_is_valid_vlan_port_member (sai_vlan_id_t vlan_id,
                                    const sai_vlan_port_t *vlan_port);

/** SAI VLAN API - Check if port is already tagged in different mode in same vlan
      \param[in] vlan_id VLAN Identifier
      \param[in] vlan_port Port id along with tagging mode
      \return Success: true
              Failure: false
*/
bool sai_is_port_in_different_tagging_mode (sai_vlan_id_t vlan_id,
                                    const sai_vlan_port_t *vlan_port);

/** SAI VLAN API - Remove VLAN Port node from VLAN Port list
      \param[in] vlan_id VLAN Identifier
      \param[in] vlan_port Port id along with tagging mode
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_INVALID_PORT_MEMBER
*/
sai_status_t sai_remove_vlan_port_node (sai_vlan_id_t vlan_id,
                                        const sai_vlan_port_t *vlan_port);

/** SAI VLAN API - Remove all VLAN Port nodes from VLAN Port list
      \param[in] vlan_id VLAN Identifier
*/
void sai_remove_all_vlan_port_nodes (sai_vlan_id_t vlan_id);

/** SAI VLAN API - Initialize internal vlan ID
      \param[in] vlan_id VLAN Identifier
*/
void sai_init_internal_vlan_id(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Check if VLAN ID is internal VLAN ID
      \param[in] vlan_id VLAN Identifier
      \return Success: true
              Failure: false
*/
bool sai_is_internal_vlan_id(sai_vlan_id_t vlan_id);

/** SAI VLAN API - check if internal VLAN ID is initialized.
      \return Success: true
              Failure: false
*/
bool sai_is_internal_vlan_id_initialized(void);

/** SAI VLAN API - Get the internal VLAN ID.
      \return internal vlan id.
*/
sai_vlan_id_t sai_internal_vlan_id_get(void);

/** SAI VLAN API - Get Port list for a VLAN
      \param[in] vlan_id VLAN Identifier
      \param[out] vlan_port_list List of vlan ports and their count
      \return Success: SAI_STATUS_SUCCESS
              Failure: SAI_STATUS_NO_MEMORY
*/
sai_status_t sai_vlan_port_list_get(sai_vlan_id_t vlan_id, sai_vlan_port_list_t *vlan_port_list);
/** SAI VLAN API - Lock VLAN for access
*/
void sai_vlan_lock(void);

/** SAI VLAN API - Unlock VLAN after access
*/
void sai_vlan_unlock(void);

/** SAI VLAN API - Get Port cache list for dump
      \param[in] vlan_id VLAN Identifier
      \return Success: A valid pointer to portlist for vlan in cache
              Failure: NULL
*/
sai_vlan_global_cache_node_t* sai_vlan_portlist_cache_read(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Check if tagging mode is valid
    \param[in] tagging_mode Tagging mode that needs to be checked
    \return Success: true
            Failure: false
*/
bool sai_is_valid_vlan_tagging_mode(sai_vlan_tagging_mode_t tagging_mode);

/** SAI VLAN API - Update vlan learn disable cache
    \param[in] vlan_id VLAN Identifier
    \param[in] disable if set disable
*/
void sai_vlan_learn_disable_cache_write(sai_vlan_id_t vlan_id, bool disable);

/** SAI VLAN API - Get vlan learn disable from cache
    \param[in] vlan_id VLAN Identifier
    \return  true if learn disable is set
             false otherwise
*/
bool sai_vlan_learn_disable_cache_read(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Update vlan max learn limit cache
    \param[in] vlan_id VLAN Identifier
    \param[in] val Max learn limit to be set
*/
void sai_vlan_max_learn_adddress_cache_write(sai_vlan_id_t vlan_id, unsigned int val);

/** SAI VLAN API - Get vlan max learn limit from cache
    \param[in] vlan_id VLAN Identifier
    \return  Max learn limit set on VLAN
             0 if not set
*/
unsigned int sai_vlan_max_learn_adddress_cache_read(sai_vlan_id_t vlan_id);

/** SAI VLAN API - Update vlan meta data cache
    \param[in] vlan_id VLAN Identifier
    \param[in] val Vlan Meta Data value
*/
void sai_vlan_meta_data_cache_write(sai_vlan_id_t vlan_id, unsigned int val);

/** SAI VLAN API - Get vlan meta data from cache
    \param[in] vlan_id VLAN Identifier
    \return  Vlan Meta Data value
*/
unsigned int sai_vlan_meta_data_cache_read(sai_vlan_id_t vlan_id);

#endif
