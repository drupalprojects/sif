<?php

/**
 * Data Object Types overview page.
 */
function sif_ui_data_object_types() {

  $objects = module_invoke_all('sif_data_object_types');
  $header = array(t('Name'));
  $rows = array();

  foreach ($objects as $key => $name) {
    $type = $objects[$key];

    $type_url_str = str_replace('_', '-', $key);
    $row = array($name);
    //$row[] = array('data' => l(t('Describe'), 'admin/sif/types/' . $type_url_str));
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
