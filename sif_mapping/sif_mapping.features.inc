<?php

/**
 * Implementation of hook_features_export_options().
 *
 * @return array
 *   A keyed array of items, suitable for use with a FormAPI select or
 *   checkboxes element.
 */
function sif_mapping_config_features_export_options() {
  // Grab the mapping objects
  $mappings = sif_mapping_get_mappings();
  $options = array();

  // For each object, add the SIF source as an exportable
  foreach ($mappings as $mapping) {
    $options[$mapping->source_type] = $mapping->source_type;
  }
  return $options;
}

/**
 * Implementation of hook_features_export().
 *
 * @param array $data
 *   Selected items to be exported
 * @param array &$export
 *   Array of items to be exported
 * @param string $module_name
 *   The name of the feature module to be generated.
 * @return array
 *   The pipe array of further processors that should be called
 */
function sif_mapping_config_features_export($data, &$export, $module_name) {
  // Make sure that sif_mapping is a dependency
  $export['dependencies']['sif_mapping'] = 'sif_mapping';

  // Create the exportable list
  foreach ($data as $component) {
    $export['features']['sif_mapping_config'][$component] = $component;
  }
  return array();
}

/**
 * Implementation of hook_features_export_render().
 *
 * @param string $module_name
 *   The name of the feature module to be exported.
 * @param array $data
 *   An array of machine name identifiers for the objects to be rendered.
 * @param array $export
 *   The full export array of the current feature being exported. This is only
 *   passed when hook_features_export_render() is invoked for an actual feature
 *   update or recreate, not during state checks or other operations.
 * @return array
 *   An associative array of rendered PHP code where the key is the name of the
 *   hook that should wrap the PHP code. The hook should not include the name
 *   of the module, e.g. the key for `hook_example` should simply be `example`.
 */
function sif_mapping_config_features_export_render($module_name, $data, $export = NULL) {
  // Start creating the callback
  $code = array();
  $code[] = '  $sif_mapping_configs = array();';
  $code[] = '';
  foreach ($data as $component) {
    // Grab the mapping object
    $item = sif_mapping_get_mappings($component);
    // Get rid of the mid, since we don't want to preserve it
    unset($item->mid);
    // Reset the array keys, since we don't want to preserve fids, either
    $item->fields = array_values($item->fields);

    // Add the mapping object
    if (is_object($item)) {
      $code[] = '  $sif_mapping_configs[] = '. features_var_export($item, '  ') .';';
    }
  }
  $code[] = '  return $sif_mapping_configs;';
  $code = implode("\n", $code);
  return array('sif_mapping_config_features_default_settings' => $code);
}

/**
 * Implementation of hook_features_rebuild().
 */
function sif_mapping_config_features_rebuild($module) {
  // Get the items using the callback
  $items = module_invoke($module, 'sif_mapping_config_features_default_settings');

  foreach ($items as $item) {
    $source = $item['source_type'];
    $destination = $item['destination_type'];
    $unique_id = $item['unique_id'];

    // Get the mid for the SIF source referenced
    $mid = db_select('sif_mapping_types', 't')
      ->fields('t', array('mid'))
      ->condition('source_type', $source, '=')
      ->execute()
      ->fetchField();

    // Delete the mappings for this mid
    db_delete('sif_mapping_types')
      ->condition('mid', $mid, '=')
      ->execute();

    db_delete('sif_mapping_fields')
      ->condition('mid', $mid, '=')
      ->execute();

    // Insert the mappings from code
    $mid = db_insert('sif_mapping_types')
      ->fields(array(
        'source_type' => $source,
        'destination_type' => $destination,
        'unique_id' => $unique_id,
      ))
      ->execute();

    foreach ($item['fields'] as $field) {
      db_insert('sif_mapping_fields')
        ->fields(array(
          'mid' => $mid,
          'source_field' => $field['source_field'],
          'destination_field' => $field['destination_field'],
        ))
        ->execute();
    }
  }
}

/**
 * Implementation of hook_features_revert().
 */
function sif_mapping_config_features_revert($module) {
  sif_mapping_config_features_rebuild($module);
}
