<?php

/**
 * Subscriptions Page
 */
function sif_subscriptions_page(){

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'sif_data_object')
    ->entityCondition('bundle', 'subscription');
  $result = $query->execute();

  $rows = array();

  if (isset($result['sif_data_object'])) {
    foreach ($result['sif_data_object'] as $object){
      $object_ids = array_keys($result['sif_data_object']);
      $subscriptions = entity_load('sif_data_object', $object_ids);
    }
    foreach ($subscriptions as $subscription){
      $row = array();
      $row[] = l($subscription->id, "sif/$subscription->id");
      $row[] = $subscription->queueId;
      $row[] = $subscription->zoneId;
      $row[] = $subscription->contextId;
      $row[] = $subscription->serviceType;
      $row[] = $subscription->serviceName;

      $actions = array();
      $actions[] = l(t('View XML'), "sif/$subscription->id/xml");
      $actions[] = l(t('Delete'), "sif/$subscription->id/delete", array('query' => array('destination' => 'admin/sif/subscriptions')));
      $row[] = implode(' | ', $actions);

      $rows[] = $row;
    }
  }
  $header = array(
    'name',
    t('QueueID'),
    t('ZoneID'),
    t('ContextID'),
    t('ServiceType'),
    t('ServiceName'),
    t('Actions'),
  );

  $build['subscriptions'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('You have no subscriptions in this site.'),
  );

  return $build;
}

/**
 * Create Subscriptions Form
 */
function sif_subscriptions_create(){
  $types = sif_data_object_sif_data_object_types();
  $types = array_keys($types);
  $types_select = array();
  foreach ($types as $type) {
    $types_select[$type . 's'] = $type;
  }

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'sif_data_object')
    ->entityCondition('bundle', 'queue');
  $result = $query->execute();

  if (isset($result['sif_data_object'])) {
    foreach ($result['sif_data_object'] as $object){
      $object_ids = array_keys($result['sif_data_object']);
      $queues = entity_load('sif_data_object', $object_ids);
    }
  }

  $queues_select = array();
  foreach ($queues as $queue) {
    $queues_select[$queue->id] = $queue->name;
  }

  $form['queue'] = array(
    '#type' => 'select',
    '#title' => t('Queue'),
    '#options' => $queues_select,
    '#description' => t('Select the queue you are subscribing in.'),
    '#required' => TRUE,
  );
  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#options' => $types_select,
    '#description' => t('The type of events to subscribe to.'),
    '#required' => TRUE,
   );
  $form['submit'] = array(
    '#value' => t('POST Subscription'),
    '#type' => 'submit',
  );

  // Placeholder element for the validate function.
  $form['xml'] = array(
    '#type' => 'value',
  );
  return $form;
}

/**
 * Create Environment validation: POST happens here.
 */
function sif_subscriptions_create_validate($form, &$form_state) {
  $xml = sif_subscriptions_create_xml($form_state['values']['queue'], $form_state['values']['type']);
  $results = sif_post('subscription', $xml);

  if (!$results) {
    form_set_error('xml', t('Error processing request. See above errors.'));
  } else {
    form_set_value($form['xml'], $results, $form_state);
  }
}

/**
 * Create Environment submit: entity_save() happens here.
 */
function sif_subscriptions_create_submit($form, &$form_state) {
  // Save Environment entity.
  $success = sif_store('subscription', $form_state['values']['xml']);

  if ($success){
    $form_state['redirect'] = 'admin/sif/subscriptions';
  }
}


/**
 * Generate a new environment XML object for POSTing
 */
function sif_subscriptions_create_xml($queue, $type){
  return <<<XML
<subscription>
  <zoneId></zoneId>
  <contextId></contextId>
  <serviceType>OBJECT</serviceType>
  <serviceName>$type</serviceName>
  <queueId>$queue</queueId>
</subscription>
XML;
}


/**
 * Get Subscriptions Form
 */
function sif_subscriptions_get(){
  $form['note'] = array(
    '#type' => 'markup',
    '#markup' => '<p>' . t('Load all subscriptions from the server.  This will update any existing subscriptions as well.') . '</p>',
  );
  $form['submit'] = array(
    '#value' => t('GET Subscriptions'),
    '#type' => 'submit',
  );

  // Placeholder element for the validate function.
  $form['xml'] = array(
    '#type' => 'value',
  );
  return $form;
}


/**
 * Get Subscriptions Form
 */
function sif_subscriptions_get_submit(){
  $xml = sif_get('subscription');
  $subscriptions =  new SimpleXMLElement($xml);
  foreach ($subscriptions->subscription as $i => $subscription){
    sif_store('subscription',  $subscription->asXML());
    $subscription_found = TRUE;
  }
  if (!isset($subscription_found)){
    drupal_set_message(t('No subscriptions were found.'));
  }
  drupal_goto('admin/sif/subscriptions');
}
