#pragma once

#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "zb_markdown_helper.h"
    
static void zb_debug_print_attributes_in_attribute_list(esp_zb_attribute_list_t* list, uint16_t cluster_id)
{
  esp_zb_attribute_list_t *element = list;

  // Skip the sentinel/dummy head node
  if (element != nullptr && zb_zcl_attribute_is_sentinel(&element->attribute)) {
    element = element->next;  // Skip dummy attribute head
  }

  int index = 0;
  while (element != nullptr) {
    esp_zb_zcl_attr_t & attr = element->attribute;

    char str[256 + DATA_VALUE_STRING_BUFFER_SIZE] = {0};
    bool success = zb_zcl_attribute_to_string(str, sizeof(str), cluster_id, &attr);
    log_d("    attribute[%02d] %s", index, str);

    // Next attribute element in attribute list
    index++;
    element = element->next;
  }
}

static void zb_debug_print_attributes_in_cluster_list(esp_zb_cluster_list_t* list)
{
  esp_zb_cluster_list_t *element = list;

  // Skip sentinel/dummy cluster head node, if present
  if (element != nullptr && zb_zcl_cluster_is_sentinel(&element->cluster)) {
    element = element->next;  // Skip dummy cluster head
  }

  log_d("clusters {");

  int index = 0;
  while (element != nullptr) {
    esp_zb_zcl_cluster_t & cluster = element->cluster;

    const char * cluster_name = zb_constants_cluster_id_to_string((esp_zb_zcl_cluster_id_t)cluster.cluster_id);
    log_d("  cluster[%02d] id=0x%04x (%s), attr_count=%d, attr_desc_list=attr_list=0x%08x, role_mask=0x%02x, manuf_code=0x%04x, cluster_init=0x%08x {",
      index, cluster.cluster_id, cluster_name, cluster.attr_count, cluster.attr_list, cluster.role_mask, cluster.manuf_code, cluster.cluster_init);

    // Only handle clusters that use the linked-list attribute model
    esp_zb_attribute_list_t *attr_list = cluster.attr_list;
    if (attr_list != nullptr) {
        zb_debug_print_attributes_in_attribute_list(attr_list, cluster.cluster_id);
    }

    log_d("  }", index);

    // Next cluster element in cluster list
    index++;
    element = element->next;
  }

  log_d("}");
}
