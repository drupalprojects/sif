<?php

/**
 * Get Queues Form
 */
function sif_queues_get_messages(){
  $form['note'] = array(
    '#type' => 'markup',
    '#markup' => '<p>' . t('Load all messages from this queue.') . '</p>',
  );
  $form['submit'] = array(
    '#value' => t('GET Queue Messages'),
    '#type' => 'submit',
  );
  return $form;
}

/**
 * Submit handler for sif_queues_get_messages();
 */
function sif_queues_get_messages_submit(){
  $id = arg(1);
  $queues = entity_load('sif_data_object', array($id));
  $queue = $queues[$id];

  $messages = sif_get('queue', $id, "messages");

  if (empty($messages)){
    drupal_set_message('There were no messages in the queue.', 'notice');
  } else {
    drupal_set_message($messages);
  }
}
