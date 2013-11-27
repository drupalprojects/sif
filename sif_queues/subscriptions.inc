<?php
/**
 * @file
 * Pages for the SIF subscriptions
 */

/**
 * Subscriptions Page
 */
function sif_subscriptions_page($queue) {

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'sif_data_object')
    ->entityCondition('bundle', 'subscription');
  $result = $query->execute();

  $rows = array();

  if (isset($result['sif_data_object'])) {
    foreach ($result['sif_data_object'] as $object) {
      $object_ids = array_keys($result['sif_data_object']);
      $subscriptions = entity_load('sif_data_object', $object_ids);
    }
    foreach ($subscriptions as $subscription) {
      if ($subscription->queueId == $queue) {
        $row = array();
        $row[] = l($subscription->id, "sif/$subscription->id");
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
  }
  $header = array(
    'name',
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
    '#empty' => t('You have no subscriptions in this queue.'),
  );

  return $build;
}

/**
 * Create Subscriptions Form
 */
function sif_subscriptions_create($form, &$form_state, $queue) {
  $types = sif_data_object_get_types();
  $types = array_keys($types);
  $types_select = array();
  foreach ($types as $type) {
    $types_select[$type . 's'] = $type;
  }

  $form['queue'] = array(
    '#type' => 'hidden',
    '#value' => $queue,
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
  $form['headers'] = array(
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
  }
  else {
    form_set_value($form['xml'], $results->data, $form_state);
    form_set_value($form['headers'], $results->headers, $form_state);
  }
}

/**
 * Create Environment submit: entity_save() happens here.
 */
function sif_subscriptions_create_submit($form, &$form_state) {
  $queue = $form_state['values']['queue'];
  // Save Environment entity.
  $success = sif_store('subscription', $form_state['values']['xml'], $form_state['values']['headers']);

  if ($success) {
    $form_state['redirect'] = "admin/sif/queues/$queue/subscriptions";
  }
}


/**
 * Generate a new environment XML object for POSTing
 */
function sif_subscriptions_create_xml($queue, $type) {
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
function sif_subscriptions_get($form, &$form_state, $queue) {
  $form['queue'] = array(
    '#type' => 'hidden',
    '#value' => $queue,
  );
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
function sif_subscriptions_get_submit($from, &$form_state) {
  $queue = $form_state['values']['queue'];
  $response = sif_get('subscription', array('multi' => TRUE));
  $xml = $response->data;
  $subscriptions =  new SimpleXMLElement($xml);
  foreach ($subscriptions->subscription as $i => $subscription) {
    if ($subscription->queueId == $queue) {
      sif_store('subscription',  $subscription->asXML(), $response->headers);
      $subscription_found = TRUE;
    }
  }
  if (!isset($subscription_found)) {
    drupal_set_message(t('No subscriptions were found.'));
  }
  drupal_goto("admin/sif/$queue/subscriptions");
}
