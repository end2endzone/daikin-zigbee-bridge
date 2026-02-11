#pragma once

#include "Zigbee.h"
#include "zb_uint8_t.h"

String debugIntegerToString(int value) {
  String output;
  output += "0x";
  output += String(value, HEX);
  output += " (";
  output += String(value);
  output += ")";
  return output;
}

inline String debugMakeIndentString(int indent) {
  String s;
  for(int i=0; i<indent; i++) {
    s += ' ';
  }
  return s;
}

void debugPrintAttributeList(esp_zb_attribute_list_t * attr_list, int indent_size) {
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

void debugPrintClusterList(esp_zb_cluster_list_t* cluster_list, int indent_size) {
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

void debugPrintClusterList(esp_zb_cluster_list_t* cluster_list) {
  debugPrintClusterList(cluster_list, 0);
}
