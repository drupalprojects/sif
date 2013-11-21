<?php

/**
 * @file environments.inc
 */

/**
 * Environments management page.
 */
function sif_ui_environments(){

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'sif_data_object')
    ->entityCondition('bundle', 'environment');
  $result = $query->execute();

  $rows = array();

  if (isset($result['sif_data_object'])) {
    $object_ids = array_keys($result['sif_data_object']);
    $environments = entity_load('sif_data_object', $object_ids);
    $default_env = variable_get('sif_default_environment_id');
    foreach ($environments as $environment){
      $row = array();
      $row[] = l($environment->consumerName, "sif/$environment->id");
      $row[] = $environment->solutionId;
      $row[] = $environment->defaultZoneId;
      //$row[] = $environment->sessionToken;

      // Infrastructure Services
      $items = array();
      foreach ($environment->infrastructureServices as $name => $url) {
        $items[] = l($name, 'sif/' . $environment->id . '/service/' . urlencode($name) . '/view');
      }
      $row[] = theme('item_list', array('items' => $items));

      // Provisioned Zones
      $items = array();
      foreach ($environment->provisionedZones[$environment->defaultZoneId]['services'] as $service_name => $service) {
        $children = array();
        foreach ($service['rights'] as $right => $status){
          $children[] = "$right: $status";
        }
        $items[] = array(
          'data' => l($service_name, 'sif/' . $environment->id . '/provisionedService/' . urlencode($service_name) . '/view'),
          'children' => $children,
        );
      }
      $row[] = theme('item_list', array('items' => $items));

      //$row[] = $environment->id;
      if ($environment->id == $default_env) {
        $row[] = t('DEFAULT');
      } else {
        $form = drupal_get_form('sif_ui_default_set', $environment->id);
        $row[] = drupal_render($form);
      }
      $row[] = l(t('View XML'), "sif/$environment->id/xml") . ' | ' . l(t('Delete'), "sif/$environment->id/delete");


      //$row[] = '<pre>' . htmlspecialchars(clean_xml($environment->xml)) . '</pre>';
      $rows[] = $row;
    }
  }
  $header = array('consumerName', 'solutionId', 'defaultZoneId', 'infrastructureServices', 'Rights', 'default', 'actions');

  $build['environments'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('You have not defined an environment for this site.'),
  );

  return $build;
}

/**
 * Create Environment form.
 */
function sif_ui_environment_create(){
  $form['consumer_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Consumer Name'),
    '#description' => t('Enter the Consumer Name you would like to use for this application.'),
    '#default_value' => variable_get('site_name', ''),
    '#required' => TRUE,
  );
  $form['solution_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Solution ID'),
    '#description' => t('Enter the solution ID to use for this environment.'),
    '#default_value' => variable_get('sif_server_solution_id', 'testSolution'),
    '#required' => TRUE,
  );
  $form['submit'] = array(
    '#value' => t('POST Environment'),
    '#type' => 'submit',
  );

  // Placeholder element for the validate function.
  $form['xml'] = array(
    '#type' => 'value',
  );
  $form['headers'] = array(
    '#type' => 'value',
  );
  return $form;
}

/**
 * Create Environment validation: POST happens here.
 */
function sif_ui_environment_create_validate($form, &$form_state) {
  $xml = sif_environment_xml($form_state['values']['consumer_name'], $form_state['values']['solution_id']);
  $results = sif_post('environment', $xml);

  if (!$results) {
    form_set_error('xml', t('Error processing request. See above errors.'));
  } else {
    form_set_value($form['xml'], $results->data, $form_state);
    form_set_value($form['headers'], $results->headers, $form_state);
  }
}

/**
 * Create Environment submit: entity_save() happens here.
 */
function sif_ui_environment_create_submit($form, &$form_state) {
  // Save Environment entity.
  $success = sif_store('environment', $form_state['values']['xml'], $form_state['values']['headers']);

  if ($success){
    //$sif_object = sif_data_object_load();
    //$default_env = variable_get('sif_default_environment_id');
    //if (!$default_env) {
    //  variable_set('sif_default_environment_id', )
    //}
    $form_state['redirect'] = 'admin/sif/environments';
  }
}

/**
 * Default environment form
 */
function sif_ui_default_set($form, &$form_state, $id) {
  $form['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Set Default'),
  );
  return $form;
}

/**
 * Default environment form submit
 */
function sif_ui_default_set_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/sif/environments';
  variable_set('sif_default_environment_id', $form['id']['#value']);
  drupal_set_message(t('Default Environment Updated!'));
}

/**
 * Generate a new environment XML object for POSTing
 */
function sif_environment_xml($consumerName, $solutionId){
  $applicationKey = sif_get_application_key();
  return <<<XML
<environment>
  <solutionId>$solutionId</solutionId>
  <authenticationMethod>Basic</authenticationMethod>
  <instanceId></instanceId>
  <userToken></userToken>
  <consumerName>$consumerName</consumerName>
  <applicationInfo>
    <applicationKey>$applicationKey</applicationKey>
    <supportedInfrastructureVersion>3.0</supportedInfrastructureVersion>
    <supportedDataModel>SIF-US</supportedDataModel>
    <supportedDataModelVersion>3.0</supportedDataModelVersion>
    <transport>REST</transport>
    <applicationProduct>
      <vendorName>Drupal</vendorName>
      <productName>Drupal</productName>
      <productVersion>7.x</productVersion>
    </applicationProduct>
  </applicationInfo>
</environment>
XML;
}
