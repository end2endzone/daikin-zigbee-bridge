#pragma once

#include <stdint.h>
#include "Arduino.h"
#include "zb_constants_helper.h"
#include "zb_helper.h"
#include "format_helper.h"
#include "esp_zigbee_core.h"
#include "zcl/esp_zigbee_zcl_common.h"
#include "zcl/esp_zigbee_zcl_core.h"

/**
 * @brief Escape pipe characters in a string so they don't break GFM table cells.
 */
static String markdown_escape_cell(const char* raw) {
  String s(raw);
  s.replace("|", "\\|");
  return s;
}

String zb_cluster_to_markdown_table_entry(int index, esp_zb_zcl_cluster_t * cluster) {
  String output;
  if (cluster == nullptr) {
    return output;
  }

  const char* cluster_name = zb_constants_cluster_id_to_string((esp_zb_zcl_cluster_id_t)cluster->cluster_id);
  const char* role_name = zb_constants_zcl_cluster_role_to_string((esp_zb_zcl_cluster_role_t)cluster->role_mask);
  
  output = format("| %d | %s | `0x%04x` | %s | %d | `0x%08x` | %s | `0x%04x` | `0x%08x` |",
    index,
    cluster_name,
    cluster->cluster_id,
    role_name,
    cluster->attr_count,
    (uintptr_t)cluster->attr_list,
    role_name,
    cluster->manuf_code,
    (uintptr_t)cluster->cluster_init);

  return output;
}

/**
 * @brief Print all cluster elements of a cluster list in a table.
 * One cluster per row.
 *
 * @param list  Pointer to the first node of the cluster linked list.
 */
static void zb_print_markdown_cluster_list_summary(esp_zb_cluster_list_t* list)
{
  logEntry("| # | Cluster Name | Cluster ID | Role | attr_count | attr_desc_list/attr_list | role_mask | manuf_code | cluster_init |");
  logEntry("|---|---|---|---|---|---|---|---|---|");

  esp_zb_cluster_list_t *element = list;

  // Skip sentinel/dummy cluster head node, if present
  if (element != nullptr && zb_zcl_cluster_is_sentinel(&element->cluster)) {
    element = element->next;  // Skip dummy cluster head
  }

  int index = 0;
  while (element != nullptr) {
    esp_zb_zcl_cluster_t& cluster = element->cluster;

    // cluster to markdown string
    String cluster_table_entry = zb_cluster_to_markdown_table_entry(index, &cluster);
    logEntry("%s", cluster_table_entry.c_str());

    index++;
    element = element->next;
  }
}

/**
 * @brief Print all attribute elements of a cluster as a table entry.
 * One attribute per row.
 *
 * @param cluster_id    The id of the cluster that owns the attribute list.
 * @param cluster_info  A text description of the cluster that owns the attribute list.
 * @param list          Pointer to the first node of the attribute linked list.
 */
static void zb_print_markdown_cluster_attributes_table_entry(uint16_t cluster_id, const char * cluster_info, esp_zb_attribute_list_t* list) {
  esp_zb_attribute_list_t *element = list;

  // Skip the sentinel/dummy head node
  if (element != nullptr && zb_zcl_attribute_is_sentinel(&element->attribute)) {
    element = element->next;  // Skip dummy attribute head
  }

  int index = 0;
  while (element != nullptr) {
    esp_zb_zcl_attr_t & attr = element->attribute;

    const char * attr_name = zb_constants_smart_cluster_attr_to_string((esp_zb_zcl_cluster_id_t)cluster_id, attr.id);
    const char * attr_type_name = zb_constants_zcl_attr_type_to_string((esp_zb_zcl_attr_type_t)attr.type);
    size_t attr_type_size = zb_constants_zcl_attr_type_size((esp_zb_zcl_attr_type_t)attr.type);
    const char * attr_access_name = zb_constants_zcl_attr_access_to_string((esp_zb_zcl_attr_access_t)attr.access);

    // Compute the value of the data as a string
    char data_str[DATA_VALUE_STRING_BUFFER_SIZE];
    bool success = zb_zcl_attribute_data_pointer_to_string(data_str, DATA_VALUE_STRING_BUFFER_SIZE, (esp_zb_zcl_attr_type_t)attr.type, attr.data_p);

    // Get more attribute info
    const char * unit_readable =  "-";
    const char * min_readable =   "-";
    const char * max_readable =   "-";
    const char * notes_readable = "";
    const zb_attr_more_info_t * attr_more = zb_get_attribute_more_info(cluster_id, attr.id);
    if (attr_more != nullptr) {
      if (attr_more->unit != nullptr)
      unit_readable = attr_more->unit;
      if (attr_more->scaled_unit != nullptr)
        unit_readable = attr_more->scaled_unit;
      if (attr_more->min != nullptr)
        min_readable = attr_more->min;
      if (attr_more->max != nullptr)
        max_readable = attr_more->max;
      if (attr_more->notes)
        notes_readable = attr_more->notes;
    }

    //                       info| idx|name (id)      |type (id)     |size|access (id)    | manuf    | data_p   | data |Unit|Min |Max |Notes|
    String output = format("| %s | %d | %s (`0x%04x`) | %s (`0x%04x`)| %u | %s (`0x%04x`) | `0x%04x` | `0x%08x` | `%s` | %s | %s | %s | %s |",
      cluster_info,
      index,
      attr_name,
      attr.id,
      attr_type_name,
      attr.type,
      attr_type_size,
      attr_access_name,
      attr.access,
      attr.manuf_code,
      (uintptr_t)attr.data_p,
      data_str,
      unit_readable,
      min_readable,
      max_readable,
      notes_readable);
    logEntry("%s", output.c_str());

    // Next attribute element in attribute list
    index++;
    element = element->next;
  }
}

/**
 * @brief Print all attribute elements of a cluster list in a table.
 * One attribute per row.
 *
 * @param list  Pointer to the first node of the cluster linked list.
 */
static void zb_print_markdown_attributes_summary(esp_zb_cluster_list_t* list)
{
  logEntry("| Cluster Info | # | Attr  | Type | Size | Access | manuf_code | data_p | data | Unit | Min | Max | Notes |");
  logEntry("|---|---|---|---|---|---|---|---|---|---|---|---|---|");

  esp_zb_cluster_list_t *element = list;

  // Skip sentinel/dummy cluster head node, if present
  if (element != nullptr && zb_zcl_cluster_is_sentinel(&element->cluster)) {
    element = element->next;  // Skip dummy cluster head
  }

  int index = 0;
  while (element != nullptr) {
    esp_zb_zcl_cluster_t& cluster = element->cluster;

    const char* cluster_name = zb_constants_cluster_id_to_string((esp_zb_zcl_cluster_id_t)cluster.cluster_id);

    // Build cluster info cell string
    static const size_t CLUSTER_INFO_SIZE = 128;
    char cluster_info[CLUSTER_INFO_SIZE] = {0};
    int result = snprintf(cluster_info, CLUSTER_INFO_SIZE, "#%d %s (`0x%04x`)", index, cluster_name, cluster.cluster_id);
    if (result < 0)
      cluster_info[0] = '\0';

    // For each attributes of this cluster.
    // Only handle clusters that use the linked-list attribute model
    esp_zb_attribute_list_t *attr_list = cluster.attr_list;
    if (cluster.attr_count == 0 && attr_list != nullptr) {
      zb_print_markdown_cluster_attributes_table_entry(cluster.cluster_id, cluster_info, attr_list);
    }

    index++;
    element = element->next;
  }
}
