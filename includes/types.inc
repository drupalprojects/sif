<?php

function sif_ui_data_object_types() {

  $objects = module_invoke_all('sif_data_object_types');
dsm($objects);
  $header = array(t('Name'), array(
    'data' => t('Operations'),
    'colspan' => '2',
  ));
  $rows = array();

  foreach ($objects as $key => $name) {
    $type = $objects[$key];

    $type_url_str = str_replace('_', '-', $key);
    $row = array($name);
    // Set the edit column.
    $row[] = array('data' => l(t('Describe'), 'admin/sif/types/' . $type_url_str));
    $rows[] = $row;
  }

  $build['sif_data_object_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No data object types available.'),
  );

  return $build;
}
