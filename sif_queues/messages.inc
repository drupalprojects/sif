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
  $last_message = variable_get("sif_queue_last_$id", '');

  $messages = sif_get('queue', array('id' => $id, 'path' => 'messages', 'multi' => TRUE, 'deleteMessageId' => $last_message));

  if (empty($messages->data)){
    variable_set("sif_queue_last_$id", '');
    drupal_set_message('There were no messages in the queue.', 'notice');
  } else {
    $object = new SimpleXMLElement($messages->data);
    foreach ($object->children() as $type => $xml) {
      $object_id = (string) $xml->attributes()->refId;
      variable_set("sif_queue_last_$id", $object_id);
      sif_store($type, $xml->asXML(), $messages->headers);
      $args = array(
        '%id' => $object_id,
        '%status' => $messages->headers['eventaction'],
      );
      drupal_set_message(t('Object %id was stored with a status of %status.', $args));
    }
  }
}
