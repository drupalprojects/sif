<?php

/**
 * Queues Page
 */
function sif_queues_page(){

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'sif_data_object')
    ->entityCondition('bundle', 'queue');
  $result = $query->execute();

  $rows = array();

  if (isset($result['sif_data_object'])) {
    foreach ($result['sif_data_object'] as $object){
      $object_ids = array_keys($result['sif_data_object']);
      $queues = entity_load('sif_data_object', $object_ids);
    }
    foreach ($queues as $queue){
      $row = array();
      $row[] = l($queue->name, "sif/$queue->id");
      $row[] = $queue->id;
      $row[] = l(t('View XML'), "sif/$environment->id/xml") . ' | ' . l(t('Delete'), "sif/$environment->id/delete");
      $row[] = '<pre>' . htmlspecialchars(clean_xml($queue->xml)) . '</pre>';
      $rows[] = $row;
    }
  }
  $header = array('name', 'id', 'actions');

  $build['queues'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('You have no queues in this site.'),
  );

  return $build;
}
