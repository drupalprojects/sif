<?php
/**
 * @file
 * API for hook_sif_data_obect_types()
 */

/**
 * Implements hook_sif_data_object_types()
 */
function hook_sif_data_object_types() {
  return array(
    'environment' => t('Environment'),
    'zone' => t('Zone'),
    'queue' => t('Queue'),
    'subscription' => t('Subscription'),
    'alert' => t('Alert'),
    'student' => t('Student'),
  );
}
