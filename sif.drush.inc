<?php
/**
 * @file
 * Implements the drush commands for the sif module
 */

$options = array(
  'sif_connection_logging' => dt('    Toggle connection logging (on/[off])'),
  'sif_default_environment_id' => dt('Environment RefId (GUID)'),
  'sif_nav_timeout' => dt('           Paged navigation timeout (# of minutes [5])'),
  'sif_pause_processing' => dt('      Toggle whether requests will be processed ([on]/off)'),
  'sif_server' => dt('                The URL of the server on which to do requests ([http://rest3api.sifassociation.org/api])'),
  'sif_server_auth_header' => dt('    The HTTP header to use for authentication ([Authentication])'),
  'sif_server_auth_secret' => dt('    The secret for a token/secret auth type ([guest])'),
  'sif_server_auth_token' => dt('     The token for a token/secret auth type ([new])'),
  'sif_server_auth_type' => dt('      The auth type ([secret]/token)'),
  'sif_server_solution_id' => dt('    The Solution ID for this environment ([testSolution])'),
);

define('SIF_OPTIONS', serialize($options));

/**
 * Implements hook_drush_command()
 */
function sif_drush_command() {
  $items = array();

  $items['sif-set'] = array(
    'description' => 'Sets the various SIF settings and variables.',
    'arguments' => array(
      'variable' => 'The SIF variable or setting to set.',
      'setting' => 'The value of the variable you want to set. If omitted, set back to default.'
    ),
    'required arguments' => 1,
    'examples' => array(
      'drush sif-set sif_connection_logging on' => 'Turns on connection logging for debug purposes.',
      'drush sif-set sif_server' => 'Sets the SIF server back to the default (http://rest3api.sifassociation.org/api).',
    ),
  );

  $items['sif-list'] = array(
    'description' => 'Lists the various SIF settings and variables.',
    'arguments' => array(
      'variable' => 'The SIF variable or setting to show.',
    ),
    'examples' => array(
      'drush sif-list' => 'Lists all of the SIF settings.',
      'drush sif-list sif_server_auth_header' => 'Lists the HTTP header used for authentication.',
    ),
  );
}

function drush_sif_set_validate($variable, $setting) {
  $options = unserialize(SIF_OPTIONS);
  if (!array_key_exists($variable, $options)) {
    return drush_set_error('SIF_SET_ERROR', dt('The SIF variable "@variable" does not exist.', array('@variable' => $variable)));
  }
}

function drush_sif_list_validate($variable) {
  $options = unserialize(SIF_OPTIONS);
  if ($variable) {
    if (!array_key_exists($variable, $options)) {
      return drush_set_error('SIF_SET_ERROR', dt('The SIF variable "@variable" does not exist.', array('@variable' => $variable)));
    }
  }
}

function drush_sif_set($variable, $setting) {
  variable_set($variable, $setting);
  $msg = dt('"@variable" was successfully set to "@setting".', array('@variable' => $variable, '@setting' => $setting));
  drush_print("\n" . $msg . "\n");
  return TRUE;
}

function drush_sif_list($variable) {
  if ($variable) {
    $options = array($variable => '');
  }
  else {
    $options = unserialize(SIF_OPTIONS);
  }

  foreach ($options as $option => $description) {
    if ($setting = variable_get($option)) {
      $msg = dt('@variable - @setting', array('@variable' => $variable, '@setting' => $setting));
      drush_print($msg . "\n");
    }
    else {
      $msg = dt('@variable - UNSET', array('@variable' => $variable));
      drush_print($msg . "\n");
    }
  }
  return TRUE;
}

function sif_drush_help($section) {
  switch ($section) {
    case 'drush:sif-set':
      $options = unserialize(SIF_OPTIONS);
      $description = "Allows the setting of SIF variables and options. Default is in [].\n\n";
      foreach ($options as $option => $desc) {
        $description .= "$option $desc\n";
      }
      return $description;
    case 'drush:sif-list':
      $description = dt('Gets the specified SIF setting. If no setting is specified, it lists all of them.');
      return $description;
  }
}
