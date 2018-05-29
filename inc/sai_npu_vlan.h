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
 * \file    sai_npu_vlan.h
 *
 * \brief Declaration of SAI NPU VLAN APIs
*/

#if !defined (__SAINPUVLAN_H_)
#define __SAINPUVLAN_H_

#include "saivlan.h"
#include "saitypes.h"
#include "saistatus.h"

/** SAI NPU VLAN - -Perform VLAN NPU related initialization
  \return Success: SAI_STATUS_SUCCESS
          Failure: Appropriate failure error code
*/
typedef sai_status_t (*sai_npu_vlan_init_fn)(void);

/** SAI NPU VLAN - Create a VLAN
  \param[in] vlan_id VLAN Identifier
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_vlan_create_fn)(sai_vlan_id_t vlan_id);

/** SAI NPU VLAN - Delete a VLAN
  \param[in] vlan_id VLAN Identifier
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE
 */
typedef sai_status_t (*sai_npu_vlan_delete_fn)(sai_vlan_id_t vlan_id);

/** SAI NPU VLAN - Add Ports to a VLAN
  \param[in] vlan_id VLAN Identifier
  \param[in] port count The number of ports to be added
  \param[in] port_list Array containing the port list
  containing port number and tagging mode
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE,SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_add_ports_to_vlan_fn)(sai_vlan_id_t vlan_id,
                                                     unsigned int port_count,
                                                     const sai_vlan_port_t *port_list);

/** SAI NPU VLAN - Remove Ports from a VLAN
  \param[in] vlan_id VLAN Identifier
  \param[in] port count The number of ports to be added
  \param[in] port_list Array containing the port list
  containing port number and tagging mode
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_remove_ports_from_vlan_fn)(sai_vlan_id_t vlan_id,
                                                          unsigned int port_count,
                                                          const sai_vlan_port_t *port_list);

/** SAI NPU VLAN - Set VLAN Mac learning limit
  \param[in] vlan_id VLAN Identifier
  \param[in] value The MAC Learning limit
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_set_vlan_max_learned_address_fn)(sai_vlan_id_t vlan_id,
                                                                uint32_t value);

/** SAI NPU VLAN - Get VLAN Mac learning limit
  \param[in] vlan_id VLAN Identifier
  \param[out] value The MAC Learning limit
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_get_vlan_max_learned_address_fn)(sai_vlan_id_t vlan_id,
                                                                uint32_t* value);

/** SAI NPU VLAN - Get Counter collection on a VLAN
  \param[in] vlan_id VLAN Identifier
  \param[in] counter_ids Type of counter to enable
  \param[in] number_of_counters Number of counters to get
  \param[out] counters The value of the counters
  \return Success: SAI_STATUS_SUCCESS
Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_get_vlan_stats_fn)(sai_vlan_id_t vlan_id,
                                                     const sai_vlan_stat_counter_t *counter_ids,
                                                     unsigned int number_of_counters,
                                                     uint64_t* counters);

/** SAI NPU VLAN - Clear Counter collection on a VLAN
  \param[in] vlan_id VLAN Identifier
  \param[in] counter_ids Type of counter to enable
  \param[in] number_of_counters Number of counters to get
  \return Success: SAI_STATUS_SUCCESS
           Failure: SAI_STATUS_FAILURE, SAI_STATUS_NOT_SUPPORTED
 */
typedef sai_status_t (*sai_npu_clear_vlan_stats_fn) (sai_vlan_id_t vlan_id,
                                                     const sai_vlan_stat_counter_t *counter_ids,
                                                     unsigned int number_of_counters);

/** SAI NPU VLAN - Set disable learning on a VLAN
  \param[in] vlan_id VLAN Identifier
  \param[in] disable To disable
  \return Success: SAI_STATUS_SUCCESS
          Failure: Appropriate failure error code
 */
typedef sai_status_t (*sai_npu_disable_vlan_learn_set_fn)(sai_vlan_id_t vlan_id, bool disable);

/** SAI NPU VLAN - Get disable learning on a VLAN
  \param[in] vlan_id VLAN Identifier
  \param[out] disable true if VLAN learning is disabled else false.
  \return Success: SAI_STATUS_SUCCESS
          Failure: Appropriate failure error code
 */
typedef sai_status_t (*sai_npu_disable_vlan_learn_get_fn)(sai_vlan_id_t vlan_id, bool *disable);

/** SAI NPU VLAN - Set VLAN Meta Data
  \param[in] vlan_id VLAN Identifier
  \param[in] value Meta Data Value
  \return Success: SAI_STATUS_SUCCESS
          Failure: Appropriate failure error code
 */
typedef sai_status_t (*sai_npu_set_vlan_meta_data)(sai_vlan_id_t vlan_id,
                                                   uint_t value);

/** SAI NPU VLAN - Get VLAN Meta Data
  \param[in] vlan_id VLAN Identifier
  \param[out] value Meta Data Value
  \return Success: SAI_STATUS_SUCCESS
          Failure: Appropriate failure error code
 */
typedef sai_status_t (*sai_npu_get_vlan_meta_data)(sai_vlan_id_t vlan_id,
                                                   uint_t *value);

/**
 * @brief VLAN NPU API table.
 */
typedef struct _sai_npu_vlan_api_t {
    sai_npu_vlan_init_fn                      vlan_init;
    sai_npu_vlan_create_fn                    vlan_create;
    sai_npu_vlan_delete_fn                    vlan_delete;
    sai_npu_add_ports_to_vlan_fn              add_ports_to_vlan;
    sai_npu_remove_ports_from_vlan_fn         remove_ports_from_vlan;
    sai_npu_set_vlan_max_learned_address_fn   set_vlan_max_learned_address;
    sai_npu_get_vlan_max_learned_address_fn   get_vlan_max_learned_address;
    sai_npu_get_vlan_stats_fn                 get_vlan_stats;
    sai_npu_clear_vlan_stats_fn               clear_vlan_stats;
    sai_npu_disable_vlan_learn_set_fn         disable_vlan_learn_set;
    sai_npu_disable_vlan_learn_get_fn         disable_vlan_learn_get;
    sai_npu_set_vlan_meta_data                set_vlan_meta_data;
    sai_npu_get_vlan_meta_data                get_vlan_meta_data;
} sai_npu_vlan_api_t;

#endif
