<?php

/**
 * Page callback for sif/%sif_object
 */
function sif_object_view_page($sif, $view_mode = 'details') {
  drupal_set_title(t('SIF Data Object') . ' ' . $sif->id);
  return sif_object_view($sif, $view_mode);
}

/**
 * View Sif Object
 */
function sif_object_view($sif, $view_mode = 'details') {

  if ($view_mode == 'details') {
    $build['type'] = array(
      '#type' => 'item',
      '#title' => t('Data Object Type'),
      '#markup' => $sif->type,
    );
    $build['id'] = array(
      '#type' => 'item',
      '#title' => t('Data Object ID'),
      '#markup' => $sif->id,
    );
  } elseif ($view_mode == 'xml') {
    $build['xml'] = array(
      '#type' => 'item',
      '#title' => t('XML'),
      '#markup' => clean_xml($sif->xml, TRUE),
    );
  }

  // Allow modules to modify the structured file.
  $type = 'sif_data_object';
  drupal_alter(array('sif_entity', 'entity_view'), $build, $type);
  return $build;
}
