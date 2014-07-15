<?php
/**
 * @file
 * APIs for SIF module
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

/**
 * Implements hook_sif_before_request()
 */
function hook_sif_before_request($type) {
  $auth_type = variable_get('sif_server_auth_type');
  if ($auth_type == 'token') {
    $expire = variable_get('sif_server_auth_token_expire', 0);
    if ($expire <= time()) {
      variable_set('sif_pause_processing', 1);
      if (foo_token_request()) {
        variable_set('sif_pause_processing', 0);
      }
      else {
        watchdog('api', 'SIF requests have been paused due to an invalid token. Triggering request was of type: @type', array('@type' => $type));
      }
    }
  }
}

/**
 * Implements hook_sif_after_request()
 */
function hook_sif_after_request($type, $code) {
  $auth_type = variable_get('sif_server_auth_type');
  if ($auth_type == 'token') {
    $error_code = variable_get('foo_error_code', 401);
    if ($code == $error_code) {
      variable_set('sif_pause_processing', 1);
      if (foo_token_request()) {
        variable_set('sif_pause_processing', 0);
      }
      else {
        watchdog('api', 'SIF requests have been paused due to an invalid token. Triggering request was of type: @type', array('@type' => $type));
      }
    }
  }
}

/**
 * Implements hook_sif_after_store()
 */
function hoof_sif_after_store($type, $id) {
  watchdog('api', 'SIF object of type @type stored with object id @id.', array('@type' => $type, '@id' => $id));
}
