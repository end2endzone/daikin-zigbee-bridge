#pragma once

#include "Zigbee.h"
#include "zb_uint8_t.h"
#include "zb_markdown_helper.h"
    
static String debugIntegerToString(int value) {
  String output;
  output += "0x";
  output += String(value, HEX);
  output += " (";
  output += String(value);
  output += ")";
  return output;
}

static inline String debugMakeIndentString(int indent) {
  String s;
  for(int i=0; i<indent; i++) {
    s += ' ';
  }
  return s;
}

static void debugPrintAttributeList(esp_zb_attribute_list_t * attr_list, int indent_size) {
  String indent_str = debugMakeIndentString(indent_size);

  esp_zb_attribute_list_t *current_attr_list_ptr = attr_list;
  int attr_index = 0;

  while (current_attr_list_ptr != nullptr) {
    esp_zb_zcl_attr_t &attr = current_attr_list_ptr->attribute;

    String output = "";
    output += indent_str + "------------------------------\n";
    output += indent_str + "Attribute #" + String(attr_index) + "\n";
    output += indent_str + "ID: " + debugIntegerToString(attr.id) + ", " + zb_constants_smart_cluster_attr_to_string((esp_zb_zcl_cluster_id_t)current_attr_list_ptr->cluster_id, attr.id) + "\n";
    output += indent_str + "Type: " + debugIntegerToString(attr.type) + ", " + zb_constants_zcl_attr_type_to_string((esp_zb_zcl_attr_type_t)attr.type) + "\n";
    output += indent_str + "Access: " + debugIntegerToString(attr.access) + ", " + zb_constants_zcl_attr_access_to_string((esp_zb_zcl_attr_access_t)attr.access) + "\n";
    output += indent_str + "Manufacturer Code: " + debugIntegerToString(attr.manuf_code) + "\n";
    output += indent_str + "Data Pointer: 0x" + String((uintptr_t)attr.data_p, HEX) + "\n";

    // Print the whole block at once
    Serial.print(output);
    output.clear();

    // next
    attr_index++;
    current_attr_list_ptr = current_attr_list_ptr->next;
  }
}

static void debugPrintClusterList(esp_zb_cluster_list_t* cluster_list, int indent_size) {
  String indent_str = debugMakeIndentString(indent_size);

  esp_zb_cluster_list_t *current_cluster_list_ptr = cluster_list;
  int cluster_index = 0;

  Serial.println("Printing cluster list:");

  while (current_cluster_list_ptr != nullptr) {
    esp_zb_zcl_cluster_t &cluster = current_cluster_list_ptr->cluster;

    String output = "";
    output += indent_str + "Cluster #" + String(cluster_index) + " {\n";
    output += indent_str + "  Cluster ID: " + debugIntegerToString(cluster.cluster_id) + ", " + zb_constants_cluster_id_to_string((esp_zb_zcl_cluster_id_t)cluster.cluster_id) + "\n";
    output += indent_str + "  Attribute Count: " + String(cluster.attr_count) + "\n";
    output += indent_str + "  Role Mask: " + debugIntegerToString(cluster.role_mask) + "\n";
    output += indent_str + "  Manufacturer Code: " + debugIntegerToString(cluster.manuf_code) + "\n";
    output += indent_str + "  Cluster Init Callback: 0x" + String((uintptr_t)cluster.cluster_init, HEX) + "\n";

    esp_zb_attribute_list_t * attr_list = esp_zb_cluster_list_get_cluster(cluster_list, cluster.cluster_id, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    output += indent_str + "  attr_list: 0x" + String((uintptr_t)attr_list, HEX) + " {\n";

    // Print the whole block at once
    Serial.print(output);
    output.clear();

    // print attributes (1 indent level down)
    if (attr_list != nullptr) {
      debugPrintAttributeList(attr_list, indent_size + 4);
    }

    // close attr_list
    output += indent_str + "  }\n";

    // close cluster
    output += indent_str + "}\n";

    // Print the whole block at once
    Serial.print(output);
    output.clear();

    cluster_index++;
    current_cluster_list_ptr = current_cluster_list_ptr->next;
  }
}

static void debugPrintClusterList(esp_zb_cluster_list_t* cluster_list) {
  debugPrintClusterList(cluster_list, 0);
}

static void debugPrintStackWaterMark() {
  // Get the remaining stack space for the current task (the loop task)
  UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(NULL);
  Serial.print("Remaining Stack (Bytes): ");
  Serial.println(highWaterMark);
}

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
    logEntry("    attribute[%02d] %s", index, str);

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

  logEntry("clusters {");

  int index = 0;
  while (element != nullptr) {
    esp_zb_zcl_cluster_t & cluster = element->cluster;

    const char * cluster_name = zb_constants_cluster_id_to_string((esp_zb_zcl_cluster_id_t)cluster.cluster_id);
    logEntry("  cluster[%02d] id=0x%04x (%s), attr_count=%d, attr_desc_list=attr_list=0x%08x, role_mask=0x%02x, manuf_code=0x%04x, cluster_init=0x%08x {",
      index, cluster.cluster_id, cluster_name, cluster.attr_count, cluster.attr_list, cluster.role_mask, cluster.manuf_code, cluster.cluster_init);

    // Only handle clusters that use the linked-list attribute model
    esp_zb_attribute_list_t *attr_list = cluster.attr_list;
    if (attr_list != nullptr) {
        zb_debug_print_attributes_in_attribute_list(attr_list, cluster.cluster_id);
    }

    logEntry("  }", index);

    // Next cluster element in cluster list
    index++;
    element = element->next;
  }

  logEntry("}");
}
