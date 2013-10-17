<?php

/**
 * @file
 * Administrative callbacks for the SIF UI module.
 */


/**
 * Builds the SIF Server settings form.
 */
function sif_settings_form($form, &$form_state) {
  $form['sif_server'] = array(
    '#type' => 'textfield',
    '#title' => t('SIF Server'),
    '#description' => t('Enter the SIF REST endpoint URL you would like to connect to.'),
    '#default_value' => variable_get('sif_server', 'http://rest3api.sifassociation.org/api'),
    '#required' => TRUE,
  );
  $form['sif_server_auth_token'] = array(
    '#type' => 'textfield',
    '#title' => t('Initial Token'),
    '#description' => t('Enter the token to access this server (Similar to a username).'),
    '#default_value' => variable_get('sif_server_auth_token', 'new'),
    '#required' => TRUE,
  );
  $form['sif_server_auth_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Consumer Secret'),
    '#description' => t('Enter the secret to access this server (Similar to a password).'),
    '#default_value' => variable_get('sif_server_auth_secret', 'guest'),
    '#required' => TRUE,
  );
  $form['sif_server_solution_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Solution ID'),
    '#description' => t('Enter the solution ID to use.'),
    '#default_value' => variable_get('sif_server_solution_id', 'testSolution'),
    '#required' => TRUE,
  );
  return system_settings_form($form);
}

/**
 * Form validate handler for the SIF settings form.
 */
function sif_settings_form_validate($form, &$form_state) {

  // Check if this is a valid URL
  if (!valid_url($form_state['values']['sif_server'])){
    form_set_error('sif_server', t('You have entered an invalid URL. Please try again.'));
  }

  // @TODO: Check that we can access the server.
}
