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

/**
 * DELETE sif object form.
 */
function sif_object_delete_form($form, &$form_state, $sif) {
  $form_state['sif_object'] = $sif;

  $form['id'] = array(
    '#type' => 'value',
    '#value' => $sif->id,
  );

  $form['delete_from_server'] = array(
    '#type' => 'checkbox',
    '#title' => t('DELETE this object from the SIF server.'),
    '#default_value' => 1,
  );

  $description = t('This action cannot be undone.');

  return confirm_form($form,
    t('Are you sure you want to delete the SIF %type %id?', array(
      '%type' => $sif->type,
      '%id' => $sif->id,
  )),
    'sif/' . $sif->id,
    $description,
    t('Delete')
  );
}

/**
 * DELETE sif object form submission
 */
function sif_object_delete_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm'] && $sif = entity_load('sif_data_object', $form_state['values']['id'])) {

    // Delete the entity and possibly the object on the server.
    sif_data_object_delete($sif->id, $form_state['values']['delete_from_server']);

    $args = array(
      '@type' => $sif->type,
      '%title' => $sif->id,
    );
    watchdog('sif', 'SIF @type %title has been deleted from the local database.', $args);
    drupal_set_message(t('SIF @type %title has been deleted from the local database.', $args));
  }
  $form_state['redirect'] = '<front>';
}
