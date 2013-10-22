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
   * Load Environment
   */
  private function load_environment(&$entity, $ids = array(), $conditions = array()){

    // Infrastructure Services
    $entity->infrastructureServices = array();
    foreach ($entity->data->infrastructureServices->infrastructureService as $s){
      $entity->infrastructureServices[(string) $s->attributes()->name] = (string) $s;
    }

    // Authorization key
    $entity->authorizationToken = sif_get_application_key($entity->sessionToken);
  }

  /**
   * Extends parent::save to allow setting of default environment.
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    $saved = parent::save($entity, $transaction);
    if ($saved) {
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
}
