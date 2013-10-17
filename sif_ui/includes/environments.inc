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

  if (isset($result['sif_data_object'])) {
    foreach ($result['sif_data_object'] as $object){
      $object_ids = array_keys($result['sif_data_object']);
      $environments = entity_load('sif_data_object', $object_ids);
    }
    $rows = array();
    foreach ($environments as $environment){
      $row = array();
      $row[] = l($environment->consumerName, "sif/$environment->id");
      $row[] = $environment->solutionId;
      $row[] = $environment->defaultZoneId;
      $row[] = $environment->sessionToken;

      $items = array();
      foreach ($environment->infrastructureServices as $name => $url) {
        $items[] = l($name, $url);
      }
      $row[] = theme('item_list', array('items' => $items));
      $row[] = $environment->id;

      //$row[] = '<pre>' . htmlspecialchars(clean_xml($environment->xml)) . '</pre>';
      $rows[] = $row;
    }
  }
  $header = array('consumerName', 'solutionId', 'defaultZoneId', 'session_token', 'infrastructureServices', 'id');

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
  $form['xml'] = array(
    '#type' => 'textarea',
    '#title' => 'Environment XML',
    '#default_value' => sif_environment_xml(),
    '#description' => t('Enter the environment XML to create.'),
  );
  $form['submit'] = array(
    '#value' => t('POST Environment'),
    '#type' => 'submit',
  );
  return $form;
}

/**
 * Create Environment validation: POST happens here.
 */
function sif_ui_environment_create_validate($form, &$form_state) {

  // POST this environment XML
  $url = variable_get('sif_server', 'http://rest3api.sifassociation.org/api');
  $url .= '/environments/environment';

  $options = array(
    'method' => 'POST',
    'data' => $form_state['values']['xml'],
    'headers' => array(
      'Authorization' => sif_get_application_key(),
      'Accept' => 'application/xml',
      'Content-Type' => 'application/xml',
    ),
  );
  $response = drupal_http_request($url, $options);

  if ($response->code != "201"){
    form_set_error('xml', t('POST unsuccessful: !message', array('!message' => strip_tags($response->data))));
  }
  else {
    form_set_value($form['xml'], $response->data, $form_state);
  }
}

/**
 * Create Environment submit: entity_save() happens here.
 */
function sif_ui_environment_create_submit($form, &$form_state) {
  // Save Environment entity.
  $environment = new SimpleXMLElement($form_state['values']['xml']);
  $id = (string) $environment->attributes()->id;
  $values = array(
    'id' => $id,
    'type' => 'environment',
    'created' => time(),
    'changed' => time(),
    'xml' => $form_state['values']['xml'],
  );
  $environment_entity = entity_create('sif_data_object', $values);
  if (entity_save('sif_data_object', $environment_entity)){
    drupal_set_message(t('Environment Created!'));
    $form_state['redirect'] = 'admin/sif/environments';
  } else {
    drupal_set_message(t('Environment Not Created! Something went wrong.'), 'error');
  }
}

/**
 * Generate a new environment XML object for POSTing
 */
function sif_environment_xml(){
  $solutionId = variable_get('sif_server_solution_id', 'testSolution');
  $applicationKey = sif_get_application_key();
  $consumerName = variable_get('site_name', '');
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

/**
 *
