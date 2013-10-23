<?php


/**
 * A controller extending EntityAPIController.
 */
class SifEntityAPIController extends EntityAPIController {

  /**
   * Extends parent::load() in order to extract important type-specific
   * data.
   */
  public function load($ids = array(), $conditions = array()) {
    $entities = parent::load($ids, $conditions);
    $types = sif_data_object_get_types();
    foreach ($entities as &$entity){

      // Give all entities a SimpleXML object
      $entity->data = new SimpleXMLElement($entity->xml);

      // Load all desired properties
      foreach ($types[$entity->type]['properties'] as $property){
        $entity->{$property} = (string) $entity->data->{$property};
      }

      // Look for helper function
      $method = "load_" . $entity->type;
      if (method_exists('SifEntityAPIController', $method)){
        $this->{$method}($entity, $ids, $conditions);
      }
    }
    return $entities;
  }

  /**
   * A clone of EntityAPIController::create
   */
  public function create(array $values = array()) {
    // Add is_new property only if we don't have this entity yet.
    $created = db_query('SELECT created FROM {sif_data_object} WHERE id = :id', array(':id' => $values['id']))->fetchField();
    if ($created) {
      $values['created'] = $created;
      $values['changed'] = time();
      $values['is_new'] = FALSE;
    }
    else {
      $values['is_new'] = TRUE;
      $values['created'] = time();
      $values['changed'] = time();
    }
    return parent::create($values);
  }

  /**
   * Extends parent::save to allow setting of default environment.
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    $saved = parent::save($entity, $transaction);
    if ($saved) {
      // @TODO: move to post_save_TYPE() "hook"
      switch ($entity->type){
        case 'environment':
          if (!variable_get('sif_default_environment_id')) {
            $data = new SimpleXMLElement($entity->xml);
            variable_set('sif_default_environment_id', (string) $data->attributes()->id);
          }

          break;
      }
    }
    return $saved;
  }

  /**
   *
   * BUNDLE HOOKS
   *
   * load_TYPE()
   * @TODO save_TYPE()
   */

  /**
   * Load Environment
   */
  private function load_environment(&$entity, $ids = array(), $conditions = array()){

    // Infrastructure Services
    $entity->infrastructureServices = array();
    foreach ($entity->data->infrastructureServices->infrastructureService as $s){
      $entity->infrastructureServices[(string) $s->attributes()->name] = (string) $s;
    }

    // Provisioned Zones and Rights
    $entity->provisionedZones = array();
    foreach ($entity->data->provisionedZones->provisionedZone as $s){
      $id = (string) $s->attributes()->id;

      // Array of Services
      $entity->provisionedZones[$id] = array();
      $entity->provisionedZones[$id]['services'] = array();

      foreach ($s->services->service as $service) {
        // Array of Rights
        $service_name = (string) $service->attributes()->name;
        $entity->provisionedZones[$id]['services'][$service_name] = array();
        $entity->provisionedZones[$id]['services'][$service_name]['rights'] = array();
        foreach ($service->rights->right as $right){
          $right_type = (string) $right->attributes()->type;
          $entity->provisionedZones[$id]['services'][$service_name]['rights'][$right_type] = (string) $right;
        }
      }
    }

    // Authorization key
    $entity->authorizationToken = sif_get_application_key($entity->sessionToken);
  }

  /**
   * Load Queue
   */
  private function load_queue(&$entity, $ids = array(), $conditions = array()){
    $entity->pollingType = $entity->data->attributes()->type;
  }

  /**
   * Load Zone
   */
  private function load_zone(&$entity, $ids = array(), $conditions = array()) {
    $entity->zone_properties = array();
    if (property_exists($entity->data, 'properties')) {
      foreach ($entity->data->properties->property as $property) {
        $entity->zone_properties[(string) $property->attributes()->name] = (string) $property;
      }
    }
  }
}
