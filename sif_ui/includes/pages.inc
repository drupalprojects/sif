<?php
/**
 * @file
 * Pages that constitute the basic SIF UI
 */

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
  }
  elseif ($view_mode == 'xml') {
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
 * DELETE sif object form validator.
 */
function sif_object_delete_form_validate($form, &$form_state) {
  if ($form_state['values']['confirm'] && $form_state['values']['delete_from_server']) {
    // Delete the object on the server.
    $default = variable_get('sif_default_environment_id');
    $sif_id = $form_state['values']['id'];
    $sif_object = $form_state['sif_object'];
    $type = $sif_object->type;

    $sif_env = NULL;
    if ($type == 'environment') {
      $sif_env = $sif_id;
    }

    if ($response = sif_delete($type, NULL, array('id' => $sif_id, 'multi' => TRUE), $sif_env)) {
      if ($type == 'environment') {
        // Unset the default environment if we are deleting the current default.
        if ($default == $sif_id) {
          variable_set('sif_default_environment_id', '');
        }
      }
      drupal_set_message(t('Successfully deleted from server'));
    }
    else {
      form_set_error('delete_from_server', t('Unable to delete from server. See above error'));
    }
  }
}

/**
 * DELETE sif object form submission
 */
function sif_object_delete_form_submit($form, &$form_state) {
  $sif_id = $form_state['values']['id'];
  $sif_object = $form_state['sif_object'];
  if ($form_state['values']['confirm']) {
    // Delete the entity
    sif_local_delete($sif_object->type, $sif_id);
  }
  $form_state['redirect'] = '<front>';
}

/**
 * Service viewer
 */
function sif_service_view($sif, $name) {
  $entity = entity_load('sif_data_object', array($sif));
  $url = $entity[$sif]->infrastructureServices[$name];

  $options = array(
    'method' => 'GET',
    'headers' => array(
      'Authorization' => $entity[$sif]->authorizationToken,
      'Accept' => 'application/xml',
      'Content-Type' => 'application/xml',
    ),
  );
  $response = drupal_http_request($url, $options);
  if ($response->code != "200") {
    if (property_exists($response, 'data')) {
      drupal_set_message(t('POST unsuccessful: @message', array('@message' => strip_tags($response->data))));
    }
    else {
      drupal_set_message(t('POST unsuccessful: code: @code, error: @error', array('@code' => strip_tags($response->code), '@error' => strip_tags($response->error))));
    }
  }
  else {
    return clean_xml($response->data, TRUE);
  }
  return t('Problem viewing sevice');;
}
