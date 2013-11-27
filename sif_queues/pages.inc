<?php
/**
 * @file
 * Pages for the SIF queues
 */

/**
 * Queues Page
 */
function sif_queues_page() {

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'sif_data_object')
    ->entityCondition('bundle', 'queue');
  $result = $query->execute();

  $rows = array();

  if (isset($result['sif_data_object'])) {
    $object_ids = array_keys($result['sif_data_object']);
    $queues = entity_load('sif_data_object', $object_ids);
    foreach ($queues as $queue) {
      $row = array();
      $row[] = l($queue->name, "sif/$queue->id");
      $row[] = $queue->messageCount;
      $row[] = $queue->pollingType;
      $row[] = format_date($queue->created) . '<br /><em>' . format_interval(time() - $queue->created) . ' ' . t('ago') . '</em>';
      $row[] = format_date($queue->changed) . '<br /><em>' . format_interval(time() - $queue->changed) . ' ' . t('ago') . '</em>';
      $row[] = $queue->lastAccessed . '<br /><em>' . format_interval(time() - strtotime($queue->lastAccessed)) . ' ' . t('ago') . '</em>';

      $actions = array();
      $actions[] = l(t('View XML'), "sif/$queue->id/xml");
      $actions[] = l(t('Subscriptions'), "admin/sif/queues/$queue->id/subscriptions");
      $actions[] = l(t('Delete'), "sif/$queue->id/delete", array('query' => array('destination' => 'admin/sif/queues')));
      $actions[] = l(t('Get Messages'), "sif/$queue->id/messages");
      $actions[] = l(t('Schedule'), "admin/sif/queues/$queue->id/schedule");
      $row[] = implode(' | ', $actions);

      $rows[] = $row;
    }
  }
  $header = array(
    'name',
    t('Messages'),
    t('Polling Type'),
    t('Created'),
    t('Updated'),
    t('Last Accessed'),
    t('Actions'),
  );

  $build['queues'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('You have no queues in this site.'),
  );

  return $build;
}

/**
 * Create Queues Form
 */
function sif_queues_create() {
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Enter the name you would like to use for this queue.'),
    '#required' => TRUE,
  );
  $form['type'] = array(
    '#type' => 'radios',
    '#title' => t('Queue Type'),
    '#description' => t('Choose what type of queue this will act as.'),
    '#options' => array(
      'IMMEDIATE_POLLING' => t('Immediate Polling'),
      'LONG_POLLING' => t('Long Polling'),
    ),
    '#default_value' => 'IMMEDIATE_POLLING',
    '#required' => TRUE,
  );
  $form['submit'] = array(
    '#value' => t('POST Queue'),
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
function sif_queues_create_validate($form, &$form_state) {
  $xml = sif_queues_create_xml($form_state['values']['name'], $form_state['values']['type']);
  $results = sif_post('queue', $xml);

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
function sif_queues_create_submit($form, &$form_state) {
  // Save Environment entity.
  $success = sif_store('queue', $form_state['values']['xml'], $form_state['values']['headers']);

  if ($success) {
    $form_state['redirect'] = 'admin/sif/queues';
  }
}

/**
 * Generate a new environment XML object for POSTing
 */
function sif_queues_create_xml($name, $type) {
  return <<<XML
<queue type="$type">
  <name>$name</name>
</queue>
XML;
}

/**
 * Get Queues Form
 */
function sif_queues_get() {
  $form['note'] = array(
    '#type' => 'markup',
    '#markup' => '<p>' . t('Load all queues from the server.  This will update any existing queues as well.') . '</p>',
  );
  $form['submit'] = array(
    '#value' => t('GET Queues'),
    '#type' => 'submit',
  );

  // Placeholder element for the validate function.
  $form['xml'] = array(
    '#type' => 'value',
  );
  return $form;
}

/**
 * Get Queues Form
 */
function sif_queues_get_submit() {
  $response = sif_get('queue', array('multi' => TRUE));
  $xml = $response->data;
  $queues = new SimpleXMLElement($xml);
  foreach ($queues->queue as $i => $queue) {
    sif_store('queue',  $queue->asXML(), $response->headers);
    $queue_found = TRUE;
  }
  if (!isset($queue_found)) {
    drupal_set_message(t('No queues were found.'));
  }
  drupal_goto('admin/sif/queues');
}

/**
 * Schedule Queues Form
 */
function sif_queues_schedule($form, &$form_state, $queue) {
  $current_settings = variable_get('sif_queue_' . $queue, array('length' => '1', 'period' => 'hours', 'status' => 'enabled', 'drupalqueue' => 0));
  $form['queue'] = array(
    '#type' => 'hidden',
    '#value' => $queue,
  );
  $form['schedule'] = array(
    '#type' => 'fieldset',
    '#title' => t('Schedule'),
    '#attributes' => array('class' => array('container-inline')),
    '#required' => TRUE,
  );
  $form['schedule']['length'] = array(
    '#type' => 'textfield',
    '#title' => t('Length'),
    '#title_display' => 'invisible',
    '#prefix' => t('Every'),
    '#default_value' => $current_settings['length'],
    '#required' => TRUE,
  );
  $form['schedule']['period'] = array(
    '#type' => 'select',
    '#title' => 'Period',
    '#title_display' => 'invisible',
    '#options' => array(
      'hours' => t('Hours'),
      'days' => t('Days'),
    ),
    '#default_value' => $current_settings['period'],
    '#required' => TRUE,
  );
  $form['status'] = array(
    '#type' => 'radios',
    '#title' => t('Process This Queue?'),
    '#description' => t('Enable or diable the processing of this queue.'),
    '#options' => array(
      'enable' => t('Enable'),
      'disable' => t('Disable'),
    ),
    '#default_value' => $current_settings['status'],
    '#required' => TRUE,
  );
  $form['drupalqueue'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add items to Drupal Queue'),
    '#description' => t('Add any objects created or modified by this SIF queue to a Drupal queue named sif_queue_items.'),
    '#default_value' => $current_settings['drupalqueue'],
  );
  $form['submit'] = array(
    '#value' => t('Save'),
    '#type' => 'submit',
  );
  return $form;
}

/**
 * Schedule Queues Form Submit
 */
function sif_queues_schedule_submit($form, &$form_state) {
  $current = variable_get('sif_queue_' . $form_state['values']['queue'], FALSE);
  $last = '';
  if ($current) {
    $last = $current['last'];
  }
  $queue_schedule = array(
    'length' => $form_state['values']['length'],
    'period' => $form_state['values']['period'],
    'status' => $form_state['values']['status'],
    'last' => $last,
    'drupalqueue' => $form_state['values']['drupalqueue'],
  );
  variable_set('sif_queue_' . $form_state['values']['queue'], $queue_schedule);
  drupal_set_message(t('Queue schedule updated.'));
  drupal_goto('admin/sif/queues');
}
