<?php
/**
 * @file
 * Page that shows the available SIF object types
 */

/**
 * Data Object Types overview page.
 */
function sif_ui_data_object_types() {
  $objects = module_invoke_all('sif_data_object_types');

  $header = array(
    t('Name'),
    t('SIF type'),
    t('Properties'),
  );
  $rows = array();

  foreach ($objects as $type => $value) {
    if (is_array($value)) {
      $properties = implode(', ', $value['properties']);
      $row = array(
        $value['name'],
        $type,
        $properties,
      );
    }
    else {
      $row = array(
        $value,
        $type,
        '',
      );
    }
    $rows[] = $row;
  }

  $build['sif_data_object_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No data object types available.'),
    '#caption' => t('These are the available data objects that this site can store. Use hook_sif_data_object_types() to add more.'),
  );

  return $build;
}
