<?php
/**
 * @file
 * Defines rules for SIF module
 */

function sif_rules_event_info() {
  $items = array(
    'sif_before_request' => array(
      'label' => t('Triggered before each SIF request.'), 
      'group' => t('SIF'),
      'variables' => array(
        'type' => array(
          'label' => t('Type of Request'),
          'type' => 'text',
          'description' => t('GET, POST, PUT or DELETE'),
        ),
      ),
    ), 
    'sif_after_request' => array(
      'label' => t('Triggered after each SIF request.'), 
      'group' => t('SIF'), 
      'variables' => array(
        'type' => array(
          'label' => t('Type of Request'),
          'type' => 'text',
          'description' => t('GET, POST, PUT or DELETE'),
        ),
        'status' => array(
          'label' => t('Status Code'),
          'type' => 'text',
          'description' => t('The status code returned by the request.'),
        ),
      ),
    ), 
  );
  return $items;
}
