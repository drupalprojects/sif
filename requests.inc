<?php
/**
 * @file
 * Class sifRequest definition
 */

class sifRequest {
  // Set up URL argument types
  public $arg_types = array(
    'zoneId' => 'matrix',
    'deleteMessageId' => 'matrix',
    'page' => 'query',
    'pageSize' => 'query',
    'path' => 'last',
    'id' => 'id',
    'multi' => 'multi',
    'queryIntention' => 'query',
  );

  function __construct($request_type, $sif_type, $url_args, $environment = NULL) {
    if (!$environment) {
      $environment = variable_get('sif_default_environment_id');
    }
    $this->request_type = $request_type;
    $this->sif_type = $sif_type;
    $this->url_args = $url_args;
    $this->environment = $environment;
    $this->error = FALSE;
    $this->two_oh_four_error = FALSE;
    $this->response = FALSE;

    $this->token = NULL;
    if ($environment != 'new') {
      $entities = entity_load('sif_data_object', array($environment));
      if (isset($entities[$environment])) {
        $this->token = $entities[$environment]->sessionToken;
      }
    }

    // Set a unique identifier
    $this->query_stamp = md5($sif_type . serialize($url_args));

    $this->timeout = variable_get('sif_nav_timeout', 5);
    $this->nav_info = variable_get("sif_nav_$this->query_stamp", FALSE);

    if (!isset($this->url_args['page'])) {
      $this->paged = FALSE;
    }
    else {
      $this->paged = $url_args['page'];
    }

    if (!isset($this->url_args['multi'])) {
      $this->url_args['multi'] = FALSE;
    }

    if ($this->paged) {
      $this->paged_active = TRUE;
      if ($this->nav_info) {
        if (!preg_match("/^\d+$/", $this->paged)) {
          $this->url_args['page'] = $this->nav_info['navigation_page'] + 1;
        }
        $navigation_last_page = $this->nav_info['navigation_last_page'];
        if (($this->nav_info['timestamp'] < strtotime("-$this->timeout minutes")) || ($this->url_args['page'] > $navigation_last_page && $navigation_last_page != -1)) {
          $this->paged_active = FALSE;
        }
      }
    }

    $this->url = $this->sif_url_build();
  }

  /**
   * Helper to build the URL for the REST request
   */
  private function sif_url_build() {
    $last = $matrix = $id = '';
    $query = array();
    $url = variable_get('sif_server', 'http://rest3api.sifassociation.org/api');
    $url .= "/$this->sif_type" . 's';
    foreach ($this->url_args as $arg_type => $value) {
      switch ($this->arg_types[$arg_type]) {
        case 'multi':
          if (!$value) {
            $url .= "/$this->sif_type";
          }
          break;
        case 'matrix':
          if ($value) {
            $matrix .= ";$arg_type=$value";
          }
          break;
        case 'query':
          if ($value) {
            $query[] = "$arg_type=$value";
          }
          break;
        case 'id':
          if ($value) {
            $id = "/$value";
          }
          break;
        case 'last':
          if ($value) {
            $last = "/$value";
          }
          break;
      }
    }

    $query_str = '?' . implode('&', $query);

    $url .= $id . $last . $matrix . $query_str;
    return $url;
  }

  public function sif_request($xml = NULL) {
    module_invoke_all('sif_before_request', $this->request_type);
    if (module_exists('rules')) {
      rules_invoke_event('sif_before_request', $this->request_type);
    }

    $pause_requests = variable_get('sif_pause_processing', 0);
    if (!$pause_requests) {
      $authorization_header = variable_get('sif_server_auth_header', 'Authorization');
      $options = array(
        'method' => $this->request_type,
        'data' => $xml,
        'headers' => array(
          $authorization_header => sif_get_application_key($this->token),
          'Accept' => 'application/xml',
          'Content-Type' => 'application/xml',
        ),
      );
      if ($this->nav_info) {
        $options['headers']['navigationId'] = $this->nav_info['navigation_id'];
      }
      $this->response = drupal_http_request($this->url, $options);
      sif_log($options, $this->response);

      if (($this->request_type == 'GET' && $this->sif_type == 'queue') || $this->paged) {
        $this->two_oh_four_error = TRUE;
      }
      $this->sif_error_check();

      module_invoke_all('sif_after_request', $this->request_type, $this->response->code);
      if (module_exists('rules')) {
        rules_invoke_event('sif_after_request', $this->request_type, $this->response->code);
      }
    }
    else {
      $this->response = FALSE;
    }
  }

  /**
   * Checks the error code for a correct error message
   */
  private function sif_error_check() {
    $statuses = array(
      '200' => 'OK',
      '201' => 'OK',
      '204' => 'OK',
    );

    if ($this->two_oh_four_error && $this->response->code == '204') { // TODO: get rid of this cludginess
      $this->error = TRUE;
    }
    else {
      if (isset($statuses[$this->response->code])) {
        $this->error = FALSE;
      }
      else {
        $error = $this->response->error;

        if (property_exists($this->response, 'data')) {
          drupal_set_message(t('@type unsuccessful: @message', array('@type' => $this->sif_type, '@message' => strip_tags($this->response->data))));
        }
        else {
          drupal_set_message(t('@type unsuccessful: code: @code, error: @error', array('@type' => $this->sif_type, '@code' => strip_tags($this->response->code), '@error' => $error)));
        }

        $this->error = $error;
      }
    }
  }
}
