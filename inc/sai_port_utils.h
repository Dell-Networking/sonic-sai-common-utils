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
* @file sai_port_utils.h
*
* @brief This file contains SAI Common Port Utility API signatures.
*  Common Port utility API's can be used by other SAI components.
*
*  SAI Port Id used in the below APIs refers to the actual NPU switch ports
*  and doesn't include virtual ports like LAG and Tunnel ports and not
*  even CPU port. CPU port info is available as part of switch information.
*
*  Below API's are used to get the switch port related information like
*  port capabilities such as Breakout mode, port speed, PHY device type.
*
*************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_PORT_UTILS_H__
#define __SAI_PORT_UTILS_H__

#include "saitypes.h"
#include "saiswitch.h"
#include "saiport.h"
#include "saistatus.h"

#include "std_type_defs.h"
#include "std_rbtree.h"
#include "sai_switch_common.h"
#include "sai_port_common.h"
#include "sai_switch_utils.h"

/** \defgroup SAIPORTAPI SAI - Port Utility
 *  Common Utility functions for SAI Port component
 *
 *  \ingroup SAIAPI
 *  \{
 */

/** Logging utility for SAI Virtual Router API */
#define SAI_PORT_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_PORT, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_PORT, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI Virtual Router API */
#define SAI_PORT_LOG_TRACE(msg, ...) \
        SAI_PORT_LOG (SAI_LOG_DEBUG, msg, ##__VA_ARGS__)

#define SAI_PORT_LOG_CRIT(msg, ...) \
        SAI_PORT_LOG (SAI_LOG_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_PORT_LOG_ERR(msg, ...) \
        SAI_PORT_LOG (SAI_LOG_ERROR, msg, ##__VA_ARGS__)

#define SAI_PORT_LOG_INFO(msg, ...) \
        SAI_PORT_LOG (SAI_LOG_INFO, msg, ##__VA_ARGS__)

#define SAI_PORT_LOG_WARN(msg, ...) \
        SAI_PORT_LOG (SAI_LOG_WARN, msg, ##__VA_ARGS__)

#define SAI_PORT_LOG_NTC(msg, ...) \
        SAI_PORT_LOG (SAI_LOG_NOTICE, msg, ##__VA_ARGS__)

/**
 * @brief Initialize the Port information table.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_info_init(void);

/**
 * @brief Initialize the port attributes to its default value
 */
void sai_port_attr_defaults_init(void);

/**
 * @brief Get the entire port information table.
 *
 * @return pointer to global port information table.
 */
sai_port_info_table_t sai_port_info_table_get(void);

/**
 * @brief Get the port info for a given switch port number
 *
 * @param[in] port  switch port id to index the port table
 * @return pointer to port info for the given port id
 */
sai_port_info_t *sai_port_info_get(sai_object_id_t port);

/**
 * @brief Get the first node in the port info table
 *
 * @return pointer to first port info node in the table
 */
static inline sai_port_info_t *sai_port_info_getfirst(void)
{
    return ((sai_port_info_t *)std_rbtree_getfirst(sai_port_info_table_get()));
}

/**
 * @brief Get the next node in port info table for the given port info node
 *
 * @param[in] port_info  current port info node
 * @return pointer to next port info node in the table
 */
static inline sai_port_info_t *sai_port_info_getnext(sai_port_info_t *port_info)
{
    return (sai_port_info_t *)std_rbtree_getnext(sai_port_info_table_get(), port_info);
}

/**
 * @brief Check if a given switch port is valid
 *
 * @param[in] port  switch port id to be validated
 * @return Success - true if valid port
 *         Failure - false if not a valid port
 */
bool sai_is_port_valid(sai_object_id_t port);

/**
 * @brief Get all the port attributes info for a given logical or CPU port
 *
 * @param[in] port  sai switch port number
 * @return pointer to port attribute info corresponding to a given
 *  valid port id else NULL for an invalid port
 */
sai_port_attr_info_t *sai_port_attr_info_get(sai_object_id_t port);

/**
 * @brief Update a specific port attribute info for a given logical port
 *
 * @param[in] port  sai switch port number
 * @param[in] attr pointer to the port attribute with attr id and value
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_attr_info_cache_set(sai_object_id_t port_id,
                                          const sai_attribute_t *attr);

/**
 * @brief Get a specific port attribute info for a given logical port
 *
 * @param[in] port  sai switch port number
 * @param[inout] attr pointer to the port attribute with attr id as input and
 *  value as output
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_attr_info_cache_get(sai_object_id_t port_id,
                                          sai_attribute_t *attr);

/**
 * @brief Get the phy device type of a given switch port
 *
 * @param[in] port  sai switch port number
 * @param[out] phy_type  phy device type of the port
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_phy_type_get(sai_object_id_t port, sai_port_phy_t *phy_type);


/**
 * @brief Get the port group of a given switch port.
 *  In a multi-lane port, all its lanes are part of the same port group.
 *  In a single-lane port, each port is part of individual port group.
 *
 * @param[in] port  sai switch port number
 * @param[out] port_group port group it is part of
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_port_group_get(sai_object_id_t port, uint_t *port_group);

/**
 * @brief Get the external physical address of a given switch port
 *
 * @param[in] port  sai switch port number
 * @param[out] ext_phy_addr  external physical address of the port
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_ext_phy_addr_get(sai_object_id_t port,
                                       sai_npu_port_id_t *ext_phy_addr);

/**
 * @brief Get the local/logical port for the given sai switch port;
 * local/Logical port is used to index the vendor SDK APIs
 *
 * @param[in] port  sai switch port number
 * @param[out] local_port_id  local port id for the given sai switch port id
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_to_npu_local_port(sai_object_id_t port,
                                        sai_npu_port_id_t *local_port_id);

/**
 * @brief Get the switch port for the given local/logical port;
 * Local/Logical port is used to index the vendor SDK APIs
 *
 * @param[in] local_port_id  local port id for the given sai switch port id
 * @param[out] port  sai switch port number
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_npu_local_port_to_sai_port(sai_npu_port_id_t local_port_id,
                                            sai_object_id_t *port);
/**
 * @brief Get the physical port number for the given switch port
 *
 * @param[in] port  sai switch port number
 * @param[out] phy_port_id  physical port number for the given switch port
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_to_physical_port(sai_object_id_t port,
                                       sai_npu_port_id_t *phy_port_id);

/**
 * @brief Get the maximum SerDess lanes for the given sai port
 *
 * @param[in] port  sai switch port number
 * @param[out] max_lanes_per_port  max SerDes lanes for the given port
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_max_lanes_get(sai_object_id_t port,
                                    uint_t *max_lanes_per_port);

/**
 * @brief Get the active lane bitmap for a given sai port
 *
 * @param[in] port  sai switch port number
 * @param[out] port_lane_bmap  active lane bitmap value
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_lane_bmap_get(sai_object_id_t port,
                                    uint64_t *port_lane_bmap);

/**
 * @brief Set the active lane bitmap for a given sai port
 *
 * @param[in] port  sai switch port number
 * @param[in] port_lane_bmap  active lane bitmap value
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_lane_bmap_set(sai_object_id_t port,
                                    uint64_t port_lane_bmap);

/**
 * @brief Get the speed of the switch port
 *
 * @param[in] port  sai switch port number
 * @param[out] speed  port speed in Gbps
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_speed_get(sai_object_id_t port, sai_port_speed_t *speed);

/**
 * @brief Set the speed of the switch port
 *
 * @param[in] port  sai switch port number
 * @param[in] speed  port speed in Gbps
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_speed_set(sai_object_id_t port, sai_port_speed_t speed);

/**
 * @brief Get the optics media type inserted in the switch port
 *
 * @param[in] port  sai switch port number
 * @param[out] media_type  media type inserted
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_media_type_get(sai_object_id_t port,
                                     sai_port_media_type_t *media_type);

/**
 * @brief Set the optics media type inserted in the switch port
 *
 * @param[in] port  sai switch port number
 * @param[in] media_type  media type inserted
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_media_type_set(sai_object_id_t port,
                                     sai_port_media_type_t media_type);

/**
 * @brief Check if a given port capability is supported.
 *
 * @param[in] port  sai switch port number
 * @param[in] capb_mask  capability bit mask in sai_port_capability_t
 * @param[out] value  true or false
 * @sa sai_port_capability_t
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_is_port_capb_supported(sai_object_id_t port,
                                        uint64_t capb_mask, bool *value);

/**
 * @brief Set the given port capability supported flags
 *
 * @param[in] port  sai switch port number
 * @param[in] capb_val  capability bit mask in sai_port_capability_t
 * @sa sai_port_capability_t
 */
void sai_port_supported_capability_set(sai_object_id_t port,
                                       uint64_t capb_val);

/**
 * @brief Check if breakout mode is supported in the port
 *
 * @param[in] port  sai switch port number
 * @param[in] breakout_type  breakout mode types in sai_port_capability_t
 * @param[out] value  true or false
 * @sa sai_port_capability_t
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
static inline sai_status_t sai_port_is_breakout_type_supported(sai_object_id_t port,
                                                               uint64_t breakout_type, bool *value)
{
    return (sai_is_port_capb_supported(port, breakout_type, value));
}

/**
 * @brief Check if a given port capability is enabled.
 *
 * @param[in] port  sai switch port number
 * @param[in] capb_mask  capability bit mask in sai_port_capability_t
 * @param[out] value  true or false
 * @sa sai_port_capability_t
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_is_port_capb_enabled(sai_object_id_t port,
                                      uint64_t capb_mask, bool *value);

/**
 * @brief Enable the given port capabilities
 *
 * @param[in] port  sai switch port number
 * @param[in] enable  flag to enable or disable the capb_val
 * @param[in] capb_val  capability bit mask in sai_port_capability_t
 * @sa sai_port_capability_t
 */
void sai_port_capablility_enable(sai_object_id_t port, bool enable, uint64_t capb_val);

/**
 * @brief Check if a given breakout type is enabled in the port
 *
 * @param[in] port  sai switch port number
 * @param[in] breakout_type  breakout mode types in sai_port_capability_t
 * @param[out] value  true or false
 * @sa sai_port_capability_t
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
static inline sai_status_t sai_port_is_breakout_type_enabled(sai_object_id_t port,
                                                             uint64_t breakout_type, bool *value)
{
    return (sai_is_port_capb_enabled(port, breakout_type, value));
}

/**
 * @brief Set the port forwarding mode
 *
 * @param[in] port  sai switch port number
 * @param[in] fwd_mode  port forwarding mode - switching or routing
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_set_forwarding_mode(sai_object_id_t port,
                                          sai_port_fwd_mode_t fwd_mode);

/**
 * @brief Get the port forwarding mode
 *
 * @param[in] port  sai switch port number
 * @param[out] fwd_mode  port forwarding mode - switching or routing
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_get_forwarding_mode(sai_object_id_t port,
                                          sai_port_fwd_mode_t *fwd_mode);

/**
 * @brief Get/set the port forwarding mode. This API is to be used by modules other than port
 *        to get and update the port forwarding mode.
 *
 * @param[in] port  sai switch port number
 * @param[in][out] fwd_mode  port forwarding mode - switching or routing
 * @param[in] update If true set else get
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_forward_mode_info(sai_object_id_t port,
                                        sai_port_fwd_mode_t *fwd_mode, bool update);
/**
 * @brief Get the string for the port forwarding mode
 *
 * @param[in] fwd_mode port forwarding mode - unknown or switching or routing
 * @return String for the specified port forwarding mode.
 */
const char *sai_port_forwarding_mode_to_str (sai_port_fwd_mode_t fwd_mode);

/**
 * @brief Mutex lock port for access
 */
void sai_port_lock(void);

/**
 * @brief Mutex unlock port after access
 */
void sai_port_unlock(void);

/**
 * @brief Retrieve/Create the port node for the applications running on the port
 *
 * @param[in] port_id  sai switch port number
 * @return port node in the port_applications_tree if create/get successful otherwise NULL
 */
sai_port_application_info_t* sai_port_application_info_create_and_get (sai_object_id_t port_id);

/**
 * @brief Checks all the application running on the port and removes the node
 *
 * @param[in] p_port_node Node to be removed
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_application_info_remove (sai_port_application_info_t *p_port_node);

/**
 * @brief Retrieve the port node for the applications running on the port
 *
 * @param[in] port_id  sai switch port number
 * @return port node in the port_applications_tree if get successful otherwise NULL
 */
sai_port_application_info_t* sai_port_application_info_get (sai_object_id_t port_id);

/**
 * @brief Get first application port node from tree.
 *
 * @return Pointer to the first application port node.
 */
sai_port_application_info_t *sai_port_first_application_node_get (void);

/**
 * @brief Get next application port node from tree.
 *
 * @param[in] p_port_node   Pointer to the Qos port node
 * @return Pointer to the next port node in tree.
 */
sai_port_application_info_t *sai_port_next_application_node_get (
                                          sai_port_application_info_t *p_port_node);

/**
 * @brief Get breakout mode port capability value from breakout mode
 *
 * @param[in] mode breakout mode type
 * @return breakout mode port capability of type sai_port_capability_t
 */
static inline sai_port_capability_t sai_port_capb_from_break_mode(sai_port_breakout_mode_type_t mode)
{
    if(mode == SAI_PORT_BREAKOUT_MODE_2_LANE) {
        return SAI_PORT_CAP_BREAKOUT_MODE_2X;

    } else if(mode == SAI_PORT_BREAKOUT_MODE_4_LANE) {
        return SAI_PORT_CAP_BREAKOUT_MODE_4X;
    }

    return SAI_PORT_CAP_BREAKOUT_MODE_1X;
}

/**
 * @brief Get breakout mode from breakout mode port capability value
 *
 * @param[in] capb port capability
 * @return breakout mode of type sai_port_breakout_mode_type_t
 */
static inline sai_port_breakout_mode_type_t sai_port_break_mode_from_capb(sai_port_capability_t capb)
{
    if(capb == SAI_PORT_CAP_BREAKOUT_MODE_2X) {
        return SAI_PORT_BREAKOUT_MODE_2_LANE;

    } else if(capb == SAI_PORT_CAP_BREAKOUT_MODE_4X) {
        return SAI_PORT_BREAKOUT_MODE_4_LANE;
    }

    return SAI_PORT_BREAKOUT_MODE_1_LANE;
}

/**
 * @brief Get the port lane count needed for a specific breakout mode
 *
 * @param[in] mode breakout mode type
 * @return port lane count corresponding to the breakout mode
 */
static inline sai_port_lane_count_t sai_port_breakout_lane_count_get(sai_port_breakout_mode_type_t mode)
{
    if(mode == SAI_PORT_BREAKOUT_MODE_4_LANE) {
        return SAI_PORT_LANE_COUNT_ONE;

    } else if(mode == SAI_PORT_BREAKOUT_MODE_2_LANE) {
        return SAI_PORT_LANE_COUNT_TWO;
    }

    return SAI_PORT_LANE_COUNT_FOUR;
}

/**
 * @brief Get the HW lane list for a given SAI logical port.
 * CPU port is not supported by thie API.
 *
 * @param[in] port_id  sai switch port number
 * @param[inout] list of hardware lanes for the port
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_attr_hw_lane_list_get(sai_object_id_t port_id, sai_attribute_value_t *value);

/**
 * @brief Get the supported breakout mode(s) for a given SAI logical port.
 * CPU port is not supported by thie API.
 *
 * @param[in] port_id  sai switch port number
 * @param[inout] value list of supported breakout mode(s) for the port
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_attr_supported_breakout_mode_get(sai_object_id_t port_id,
                                                       sai_attribute_value_t *value);

/**
 * @brief Get the current breakout mode for a given SAI logical port.
 * CPU port is not supported by thie API.
 *
 * @param[in] port_id  sai switch port number
 * @return Current breakout mode of the port
 */
sai_port_breakout_mode_type_t sai_port_current_breakout_mode_get(sai_object_id_t port);

/**
 * @brief Updates the port info before applying breakout mode
 *
 * @param[in] port_id  sai switch port number
 * @param[in] speed  port speed in Gbps
 * @param[in] new_mode  new breakout mode to be configured
 * @param[in] prev_mode  previous/current breakout mode
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_breakout_mode_update(sai_object_id_t port,
                                           sai_port_speed_t speed,
                                           sai_port_breakout_mode_type_t new_mode,
                                           sai_port_breakout_mode_type_t prev_mode);

/**
 * @brief Get the list of  SAI logical port and it doesn't include CPU port
 *
 * @param[out] port_list port list to be filled. Memory allocated by caller.
 */
void sai_port_logical_list_get(sai_object_list_t *port_list);

/**
 * @brief Get the port type for a given sai port
 *
 * @param[in] port_id  sai switch port number - can be Logical or CPU port
 * @param[out] value  pointer to get the port type
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_attr_type_get(sai_object_id_t port_id,
                                    sai_attribute_value_t *value);

/**
 * @brief Get the supported speed values for the given sai port
 *
 * @param[in] port  sai switch port number
 * @param[out] value  pointer to list of supported speed
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_attr_supported_speed_get(sai_object_id_t port,
                                               sai_attribute_value_t *value);


/**
 * @brief Update the supported speed values for the given sai port
 *
 * @param[in] port  sai switch port number
 * @param[in] speed_capb Supported speed bitmap
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
sai_status_t sai_port_attr_supported_speed_update(sai_object_id_t port,
                                                  uint_t speed_capb);
/**
 * \}
 */

#endif /* __SAI_PORT_UTILS_H__ */
