<?php
/**
 * @file requests.inc
 * Defines a SIF Request class for making and handling requests
 */

/**
 * @property array arg_types
 * @property null environment
 * @property bool error
 * @property mixed nav_info
 * @property bool paged
 * @property mixed query_stamp
 * @property mixed request_type
 * @property bool response
 * @property mixed sif_type
 * @property mixed timeout
 * @property null token
 * @property bool two_oh_four_error
 * @property string url
 * @property bool paged_active
 * @file
 * Class sifRequest definition
 */

class sifRequest {
  // Set up URL argument types
  function __construct($request_type, $sif_type, $url_args, $environment = NULL) {
    $this->setArgTypes(array(
      'zoneId' => 'matrix',
      'deleteMessageId' => 'matrix',
      'page' => 'query',
      'pageSize' => 'query',
      'path' => 'last',
      'id' => 'id',
      'multi' => 'multi',
      'queryIntention' => 'query',
    ));
    if (!$environment) {
      $environment = variable_get('sif_default_environment_id');
    }
    $this->setRequestType($request_type);
    $this->setSifType($sif_type);
    $this->setUrlArgs($url_args);
    $this->setEnvironment($environment);
    $this->setError(FALSE);
    $this->setTwoOhFourError(FALSE);
    $this->setResponse(FALSE);

    $this->setToken(NULL);
    if ($environment != 'new') {
      $entities = entity_load('sif_data_object', array($environment));
      if (isset($entities[$environment])) {
        $this->setToken($entities[$environment]->sessionToken);
      }
    }

    // Set a unique identifier
    $this->setQueryStamp();

    $this->setTimeout();

    $this->setNavInfo();

    if (!isset($url_args['page'])) {
      $this->setPaged(FALSE);
    }
    else {
      $this->setPaged($url_args['page']);
    }

    if (!isset($this->url_args['multi'])) {
      $this->url_args['multi'] = FALSE;
    }

    if ($paged = $this->getPaged()) {
      $this->setPagedActive(TRUE);
      if ($nav_info = $this->getNavInfo()) {
        if (!preg_match("/^\d+$/", $paged)) {
          $this->setPage($nav_info['navigation_page'] + 1);
        }
        $page = $this->getPage();
        $navigation_last_page = $nav_info['navigation_last_page'];
        $timeout = $this->getTimeout();
        if (($nav_info['timestamp'] < strtotime("-$timeout minutes")) || ($page > $navigation_last_page && $navigation_last_page != -1)) {
          $this->setPagedActive(FALSE);
        }
      }
    }

    $this->setUrl();
  }

  /**
   * @param array $arg_types
   */
  protected function setArgTypes($arg_types) {
    $this->arg_types = $arg_types;
  }

  /**
   * @return array
   */
  protected function getArgTypes() {
    return $this->arg_types;
  }

  /**
   * @param null $environment
   */
  public function setEnvironment($environment) {
    $this->environment = $environment;
  }

  /**
   * @return null
   */
  public function getEnvironment() {
    return $this->environment;
  }

  /**
   * @param boolean $error
   */
  public function setError($error) {
    $this->error = $error;
  }

  /**
   * @return boolean
   */
  public function getError() {
    return $this->error;
  }

  /**
   * @internal param null $nav_info
   */
  public function setNavInfo() {
    $this->nav_info = variable_get("sif_nav_$this->query_stamp", FALSE);
  }

  /**
   * @return null
   */
  public function getNavInfo() {
    return $this->nav_info;
  }

  public function setPage($page) {
    $this->url_args['page'] = $page;
  }

  public function getPage() {
    return $this->url_args['page'];
  }

  /**
   * @param boolean $paged
   */
  public function setPaged($paged) {
    $this->paged = $paged;
  }

  /**
   * @return boolean
   */
  public function getPaged() {
    return $this->paged;
  }

  /**
   * @param boolean $paged_active
   */
  public function setPagedActive($paged_active) {
    $this->paged_active = $paged_active;
  }

  /**
   * @return boolean
   */
  public function getPagedActive() {
    return $this->paged_active;
  }

  /**
   * @internal param string $query_stamp
   */
  public function setQueryStamp() {
    $this->query_stamp = md5($this->getSifType() . serialize($this->getUrlArgs()));
  }

  /**
   * @return string
   */
  public function getQueryStamp() {
    return $this->query_stamp;
  }

  /**
   * @param mixed $request_type
   */
  public function setRequestType($request_type) {
    $this->request_type = $request_type;
  }

  /**
   * @return mixed
   */
  public function getRequestType() {
    return $this->request_type;
  }

  /**
   * @param boolean $response
   */
  public function setResponse($response) {
    $this->response = $response;
  }

  /**
   * @return boolean
   */
  public function getResponse() {
    return $this->response;
  }

  /**
   * @param mixed $sif_type
   */
  public function setSifType($sif_type) {
    $this->sif_type = $sif_type;
  }

  /**
   * @return mixed
   */
  public function getSifType() {
    return $this->sif_type;
  }

  /**
   * @internal param null $timeout
   */
  public function setTimeout() {
    $this->timeout = variable_get('sif_nav_timeout', 5);
  }

  /**
   * @return null
   */
  public function getTimeout() {
    return $this->timeout;
  }

  /**
   * @param null $token
   */
  public function setToken($token) {
    $this->token = $token;
  }

  /**
   * @return null
   */
  public function getToken() {
    return $this->token;
  }

  /**
   * @param boolean $two_oh_four_error
   */
  public function setTwoOhFourError($two_oh_four_error) {
    $this->two_oh_four_error = $two_oh_four_error;
  }

  /**
   * @return boolean
   */
  public function getTwoOhFourError() {
    return $this->two_oh_four_error;
  }

  /**
   * @internal param null|string $url
   */
  public function setUrl() {
    $last = $matrix = $id = '';
    $query = array();
    $url = variable_get('sif_server', 'http://rest3api.sifassociation.org/api');
    $url .= "/$this->sif_type" . 's';
    foreach ($this->url_args as $arg_type => $value) {
      switch ($this->arg_types[$arg_type]) {
//        case 'multi':
//          if (!$value) {
//            $url .= "/$this->sif_type";
//          }
//          break;
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

    $this->url = $url;
  }

  /**
   * @return null|string
   */
  public function getUrl() {
    return $this->url;
  }

  /**
   * @param mixed $url_args
   */
  public function setUrlArgs($url_args) {
    $this->url_args = $url_args;
  }

  /**
   * @return mixed
   */
  public function getUrlArgs() {
    return $this->url_args;
  }

  /**
   * Make the actual SIF request
   * @param null $xml
   *   The XML object to use in the request, if needed.
   */
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
          'Accept' => 'application/sif+xml',
        ),
      );
      if ($this->request_type == 'PUT' || $this->request_type == 'POST') {
        $options['headers']['Content-Type'] = 'application/sif+xml';
      }
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
        $error = check_plain($this->response->error);

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
